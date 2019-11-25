/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef SHADER_SIMPLE_TEX_H
#define SHADER_SIMPLE_TEX_H

const char vtxShader_undistortedTexture[] = ""
        "#version 300 es                    \n"
        "layout(location = 0) in vec4 pos;  \n"
        "layout(location = 1) in vec2 tex;  \n"
        "uniform mat4 cameraMat;            \n"
        "out vec2 uv;                       \n"
        "void main()                        \n"
        "{                                  \n"
        "   gl_Position = cameraMat * pos;  \n"
        "   uv = tex;                       \n"
        "}                                  \n";

        const char pixShader_undistortedTexture[] =
        "#version 300 es                                              \n"
        "precision highp float;                                       \n"
        "uniform sampler2D tex;                                       \n"
        "in vec2 uv;                                                  \n"
        "out vec4 color;                                              \n"
        "#define K1 -1.5                                              \n"
        "#define K2 1.5                                               \n"
        "#define SCALE_X 1.0                                          \n"
        "#define SCALE_Y 1.0                                          \n"
        "                                                             \n"
        "vec2 undistort(vec2 uv)                                      \n"
        "{                                                            \n"
        "    vec2 center = vec2(0.5, 0.5);                            \n"
        "    float rr, r2, theta, distortion_x, distortion_y;         \n"
        "                                                             \n"
        "    vec2 dist = uv - center;                                 \n"
        "    rr = sqrt(dot(dist, dist));                              \n"
        "                                                             \n"
        "    float rrs = rr * rr;                                     \n"
        "    r2 = rr * (1.0 + K1 * rrs + K2 * rrs * rrs);             \n"
        "    theta = atan(uv.x - center.x, uv.y - center.y);          \n"
        "                                                             \n"
        "    distortion_x = sin(theta) * r2 * SCALE_X;                \n"
        "    distortion_y = cos(theta) * r2 * SCALE_Y;                \n"
        "                                                             \n"
        "    return vec2(distortion_x + 0.5,distortion_y + 0.5);      \n"
        "}                                                            \n"
        "                                                             \n"
        "void main()                                                  \n"
        "{                                                            \n"
        "    vec4 texel = texture(tex, undistort(uv));                \n"
        "    color = texel;                                           \n"
        "}                                                            \n";

#endif // SHADER_SIMPLE_TEX_H