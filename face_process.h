/*
 *  Copyright 2018 Digital Media Professionals Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef FACE_PROCESS_H
#define FACE_PROCESS_H

#include <vector>
#include "dmp_network.h"
#include "util_draw.h"

struct rectangle {
  int x1;
  int y1;
  int x2;
  int y2;
  float p;
};

struct rect_point {
  int x1;
  int y1;
  int x2;
  int y2;
  float p;
  int px[5];
  int py[5];
};

// C12Net dimension info
struct C12Net {
  CDMP_Network *net;
  int in_w;
  int in_h;
  int out_w;
  int out_h;
  float scale;
  void *in_buf;
};

void detect_face_12net(std::vector<rectangle> &rect, std::vector<float> &prob,
                       std::vector<float> &roi, float scale, int o_width,
                       int o_height, float threshold);
void filter_face_24net(std::vector<rectangle> &rect, std::vector<float> &roi);
void filter_face_48net(std::vector<rectangle> &rect, std::vector<float> &roi,
                       std::vector<float> &pts, std::vector<rect_point> &result);
void crop_resample(__fp16 *src_img, __fp16 *dst_img, int c_x, int c_y,
                   int c_w, int c_h, int src_w, int src_h, int dst_w, int dst_h);
void prepare_net_12_inputs(int net_12_size);
void draw_bboxes(const std::vector<rect_point> &boxes, dmp::util::COverlayRGB &overlay);

#endif
