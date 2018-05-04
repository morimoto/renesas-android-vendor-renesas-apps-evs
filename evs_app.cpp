/*
 * Copyright (C) 2016 The Android Open Source Project
 * Copyright (C) 2018 GlobalLogic
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "EVSAPP.renesas"

#include <stdio.h>

#include <hidl/HidlTransportSupport.h>
#include <utils/Errors.h>
#include <utils/StrongPointer.h>
#include <utils/Log.h>
#include <cutils/properties.h>
#include "android-base/macros.h"    // arraysize

#include <android/hardware/automotive/evs/1.0/IEvsEnumerator.h>
#include <android/hardware/automotive/evs/1.0/IEvsDisplay.h>

#include <hwbinder/ProcessState.h>

#include "EvsStateControl.h"
#include "EvsVehicleListener.h"
#include "ConfigManager.h"


// libhidl:
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;

#define APP_NAME "renesas"
#define APP_PROPERTY "ro.boot.evs.app"

// Main entry point
int main(int /* argc */, char** /* argv */)
{
    printf("EVS app starting\n");
    char value[PROPERTY_VALUE_MAX];
    memset(value, 0, PROPERTY_VALUE_MAX);
    property_get(APP_PROPERTY, value, "");
    ALOGI("Property value %s = [%s]", APP_PROPERTY, value);
    if (strncmp(value, APP_NAME, sizeof(APP_NAME)) != 0) {
        ALOGI("Property %s not match renesas. Exiting.", APP_PROPERTY);
        return 0;
    }

    // Load our configuration information
    ConfigManager config;
    config.initialize("/system/etc/automotive/evs/config.json");

    // Set thread pool size to one to avoid concurrent events from the HAL.
    // This pool will handle the EvsCameraStream callbacks.
    // Note:  This _will_ run in parallel with the EvsListener run() loop below which
    // runs the application logic that reacts to the async events.
    configureRpcThreadpool(1, false /* callerWillJoin */);

    // Construct our async helper object
    sp<EvsVehicleListener> pEvsListener = new EvsVehicleListener();

    // Get the EVS manager service
    ALOGI("Acquiring EVS Enumerator");
    android::sp<IEvsEnumerator> pEvs = IEvsEnumerator::getService();
    if (pEvs.get() == nullptr) {
        ALOGE("getService returned NULL.  Exiting.");
        return 1;
    }

    // Request exclusive access to the EVS display
    ALOGI("Acquiring EVS Display");
    android::sp <IEvsDisplay> pDisplay;
    pDisplay = pEvs->openDisplay();
    if (pDisplay.get() == nullptr) {
        ALOGE("EVS Display unavailable.  Exiting.");
        return 1;
    }

    // Connect to the Vehicle HAL so we can monitor state
    ALOGI("Connecting to Vehicle HAL");
    android::sp <IVehicle> pVnet = IVehicle::getService();
    if (pVnet.get() == nullptr) {
#if 0
        ALOGE("Vehicle HAL getService returned NULL.  Exiting.");
        return 1;
#else
        // While testing, at least, we want to be able to run without a vehicle
        ALOGE("getService returned NULL, but we're in test, so we'll pretend to be in reverse");
#endif
    } else {
        // Register for vehicle state change callbacks we care about
        // Changes in these values are what will trigger a reconfiguration of the EVS pipeline
        SubscribeOptions optionsData[1] = {
                {
                    .propId = static_cast<int32_t>(VehicleProperty::GEAR_SELECTION),
                    .flags = SubscribeFlags::EVENTS_FROM_CAR
                }
        };
        hidl_vec<SubscribeOptions> options;
        options.setToExternal(optionsData, arraysize(optionsData));
        StatusCode status = pVnet->subscribe(pEvsListener, options);
        if (status != StatusCode::OK) {
            ALOGE("Subscription to vehicle notifications failed with code %d.  Exiting.", status);
            return 1;
        }
    }

    // Configure ourselves for the current vehicle state at startup
    ALOGI("Constructing state controller");
    EvsStateControl *pStateController = new EvsStateControl(pVnet, pEvs, pDisplay, config);
    if (!pStateController->configureForVehicleState()) {
        ALOGE("Initial configuration failed.  Exiting.");
        return 1;
    } else {
        // Run forever, reacting to events as necessary
        ALOGI("Entering running state");
        pEvsListener->run(pStateController);
    }

    // In normal operation, we expect to run forever, but in some error conditions we'll quit.
    // One known example is if another process preempts our registration for our service name.
    printf("EVS Listener stopped.  Exiting.\n");
    return 0;
}
