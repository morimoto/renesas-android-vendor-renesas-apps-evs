LOCAL_PATH:= $(call my-dir)

##################################
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    evs_app.cpp \
    EvsStateControl.cpp \
    StreamHandler.cpp \
    ConfigManager.cpp \

LOCAL_C_INCLUDES += \
    frameworks/base/include

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libutils \
    libui \
    libhwbinder \
    libhidlbase \
    libhidltransport \
    libhardware \
    android.hardware.automotive.evs@1.0 \
    android.hardware.automotive.vehicle@2.0 \

LOCAL_STATIC_LIBRARIES := \
    libjsoncpp \

LOCAL_STRIP_MODULE := keep_symbols

LOCAL_INIT_RC := evs_app.renesas.rc
LOCAL_MODULE:= evs_app.renesas
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES
LOCAL_CFLAGS += -Wall -Werror -Wunused -Wunreachable-code

include $(BUILD_EXECUTABLE)
