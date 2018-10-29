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

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include "12Net_640x480_gen.h"
#include "12Net_448x336_gen.h"
#include "12Net_320x240_gen.h"
#include "12Net_224x168_gen.h"
#include "12Net_160x120_gen.h"
#include "12Net_112x84_gen.h"
#include "12Net_80x60_gen.h"
#include "12Net_56x42_gen.h"
#include "12Net_40x30_gen.h"
#include "12Net_28x21_gen.h"
#include "12Net_20x15_gen.h"
#include "24Net_gen.h"
#include "48Net_gen.h"
#include "util_draw.h"
#include "util_input.h"

C12Net_640x480 net_12_640x480;
C12Net_448x336 net_12_448x336;
C12Net_320x240 net_12_320x240;
C12Net_224x168 net_12_224x168;
C12Net_160x120 net_12_160x120;
C12Net_112x84 net_12_112x84;
C12Net_80x60 net_12_80x60;
C12Net_56x42 net_12_56x42;
C12Net_40x30 net_12_40x30;
C12Net_28x21 net_12_28x21;
C12Net_20x15 net_12_20x15;
C24Net net_24;
C48Net net_48;

struct C12Net {
  CDMP_Network *net;
  int in_w;
  int in_h;
  int out_w;
  int out_h;
  float scale;
  void *in_buf;
};

C12Net net_12[] = {
  { &net_12_640x480, 640, 480, 315, 235, 1.0f,       nullptr },
  { &net_12_448x336, 448, 336, 219, 163, 1.4285714f, nullptr },
  { &net_12_320x240, 320, 240, 155, 115, 2.0f,       nullptr },
  { &net_12_224x168, 224, 168, 107,  79, 2.8571429f, nullptr },
  { &net_12_160x120, 160, 120,  75,  55, 4.0f,       nullptr },
  { &net_12_112x84,  112,  84,  51,  37, 5.7142857f, nullptr },
  { &net_12_80x60,    80,  60,  35,  25, 8.0f,       nullptr },
  { &net_12_56x42,    56,  42,  23,  16, 11.428571f, nullptr },
  { &net_12_40x30,    40,  30,  15,  10, 16.0f,      nullptr },
  { &net_12_28x21,    28,  21,   9,   6, 22.857143f, nullptr },
  { &net_12_20x15,    20,  15,   5,   3, 32.0f,      nullptr },
};

const int net_12_size = sizeof(net_12) / sizeof(net_12[0]);

using namespace dmp;
using namespace util;

#define FILENAME_WEIGHTS12 "12Net_weights.bin"
#define FILENAME_WEIGHTS24 "24Net_weights.bin"
#define FILENAME_WEIGHTS48 "48Net_weights.bin"

using namespace std;

#define SCREEN_W (dmp::util::get_screen_width())
#define SCREEN_H (dmp::util::get_screen_height())

#define IMAGE_W 640
#define IMAGE_H 480

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

uint32_t imgView[IMAGE_W * IMAGE_H];
__fp16 imgProc[IMAGE_W * IMAGE_H * 3];

uint32_t fc = 0;

volatile int conv_time_tot = 0;
volatile int fc_time_tot = 0;

static void rect2square(float &x1, float &y1, float &x2, float &y2) {
  float w = x2 - x1;
  float h = y2 - y1;
  float l = max(w, h);
  x1 = x1 + w * 0.5f - l * 0.5f;
  y1 = y1 + h * 0.5f - l * 0.5f;
  x2 = x1 + l;
  y2 = y1 + l;
}

template <typename T>
static inline T overlap(T x1, T w1, T x2, T w2) {
  T left = max(x1, x2);
  T right = min(x1 + w1, x2 + w2);
  return max(right - left, 0);
}

template <typename T>
static float box_iou(const T *a, const T *b) {
  T ow = overlap(a[0], a[2], b[0], b[2]);
  T oh = overlap(a[1], a[3], b[1], b[3]);
  float box_int = ow * oh;
  float box_uni = a[2] * a[3] + b[2] * b[3] - box_int;
  return box_int / box_uni;
}

static void do_nms(vector<rectangle> &rect, float threshold) {
  sort(rect.begin(), rect.end(),
       [](const rectangle &a, const rectangle &b) -> bool {
         return a.p > b.p;
       });
  size_t i, j;
  for (i = 0; i < rect.size() - 1; ++i) {
    if (rect[i].p == 0.0f)
      continue;
    for (j = i + 1; j < rect.size(); ++j) {
      if (rect[j].p == 0.0f)
        continue;
      if (box_iou(&rect[i].x1, &rect[j].x1) > threshold)
        rect[j].p = 0.0f;
    }
  }
}

static void detect_face_12net(vector<rectangle> &rect, vector<float> &prob,
    vector<float> &roi, float scale, int o_width, int o_height,
    float threshold) {
  int x, y;
  float x1, y1, x2, y2, p;
  rectangle r;
  vector<rectangle> pick;

  for (x = 0; x < o_width; ++x) {
    for (y = 0; y < o_height; ++y) {
      p = prob[(x * o_height + y) * 2 + 1];
      if (p < threshold)
        continue;
      x1 = floor(x * 2.f * scale);
      y1 = floor(y * 2.f * scale);
      x2 = floor((x * 2.f + 12.f) * scale);
      y2 = floor((y * 2.f + 12.f) * scale);
      x1 += roi[(y * o_width + x) * 4 + 0] * 12.f * scale;
      y1 += roi[(y * o_width + x) * 4 + 1] * 12.f * scale;
      x2 += roi[(y * o_width + x) * 4 + 2] * 12.f * scale;
      y2 += roi[(y * o_width + x) * 4 + 3] * 12.f * scale;
      rect2square(x1, y1, x2, y2);
      r.x1 = max(0, int(x1));
      r.y1 = max(0, int(y1));
      r.x2 = min(IMAGE_W, int(x2));
      r.y2 = min(IMAGE_H, int(y2));
      r.p = p;
      if (r.x2 > r.x1 && r.y2 > r.y1)
        pick.push_back(r);
    }
  }
  if (pick.size() > 0)
    do_nms(pick, 0.5);
  for (auto it = pick.begin(); it != pick.end(); ++it) {
    if (it->p != 0.0f)
      rect.push_back(*it);
  }
}

static void filter_face_24net(vector<rectangle> &rect, vector<float> &roi) {
  vector<rectangle> pick;
  for (size_t i = 0; i < rect.size(); ++i) {
    if (rect[i].p == 0.0f)
      continue;
    float w = rect[i].x2 - rect[i].x1;
    float h = rect[i].y2 - rect[i].y1;
    float x1 = rect[i].x1 + roi[i * 4 + 0] * w;
    float y1 = rect[i].y1 + roi[i * 4 + 1] * h;
    float x2 = rect[i].x2 + roi[i * 4 + 2] * w;
    float y2 = rect[i].y2 + roi[i * 4 + 3] * h;
    rect2square(x1, y1, x2, y2);
    rect[i].x1 = max(0, int(x1));
    rect[i].y1 = max(0, int(y1));
    rect[i].x2 = min(IMAGE_W, int(x2));
    rect[i].y2 = min(IMAGE_H, int(y2));
    if (rect[i].x2 > rect[i].x1 && rect[i].y2 > rect[i].y1)
      pick.push_back(rect[i]);
  }
  if (pick.size() > 0)
    do_nms(pick, 0.7);
  rect.clear();
  for (auto it = pick.begin(); it != pick.end(); ++it) {
    if (it->p != 0.0f)
      rect.push_back(*it);
  }
}

static void filter_face_48net(vector<rectangle> &rect, vector<float> &roi,
    vector<float> &pts, vector<rect_point> &result) {
  rect_point r;
  vector<rect_point> pick;
  for (size_t i = 0; i < rect.size(); ++i) {
    if (rect[i].p == 0.0f)
      continue;
    float w = rect[i].x2 - rect[i].x1;
    float h = rect[i].y2 - rect[i].y1;
    float x1 = rect[i].x1 + roi[i * 4 + 0] * w;
    float y1 = rect[i].y1 + roi[i * 4 + 1] * h;
    float x2 = rect[i].x2 + roi[i * 4 + 2] * w;
    float y2 = rect[i].y2 + roi[i * 4 + 3] * h;
    for (int j = 0; j < 5; ++j) {
      r.px[j] = rect[i].x1 + pts[i * 10 + j] * w;
      r.py[j] = rect[i].y1 + pts[i * 10 + j + 5] * h;
    }
    r.x1 = max(0, int(x1));
    r.y1 = max(0, int(y1));
    r.x2 = min(IMAGE_W, int(x2));
    r.y2 = min(IMAGE_H, int(y2));
    r.p = rect[i].p;
    if (r.x2 > r.x1 && r.y2 > r.y1)
      pick.push_back(r);
  }
  //if (pick.size() > 0)
  //  do_nms(pick, 0.7);
  result.clear();
  for (auto it = pick.begin(); it != pick.end(); ++it) {
    if (it->p != 0.0f)
      result.push_back(*it);
  }
}

static void resample(__fp16 *src_img, __fp16 *dst_img, int src_w, int src_h,
                     int dst_w, int dst_h) {
  float sx = float(src_w) / float(dst_w);
  float sy = float(src_h) / float(dst_h);

  for (int y = 0; y < dst_h; y++) {
    float ty = (y + 0.5f) * sy;
    int iy = int(ty);
    float ry = ty - iy;
    for (int x = 0; x < dst_w; x++) {
      float tx = (x + 0.5f) * sx;
      int ix = int(tx);
      float rx = tx - ix;
      float r = 0.f, g = 0.f, b = 0.f;
      __fp16 *p;
      p = &src_img[((iy + 0) * src_w + (ix + 0)) * 3];
      r += p[0] * rx * ry;
      g += p[1] * rx * ry;
      b += p[2] * rx * ry;
      p = &src_img[((iy + 0) * src_w + (ix + 1)) * 3];
      r += p[0] * (1.f - rx) * ry;
      g += p[1] * (1.f - rx) * ry;
      b += p[2] * (1.f - rx) * ry;
      p = &src_img[((iy + 1) * src_w + (ix + 0)) * 3];
      r += p[0] * rx * (1.f - ry);
      g += p[1] * rx * (1.f - ry);
      b += p[2] * rx * (1.f - ry);
      p = &src_img[((iy + 1) * src_w + (ix + 1)) * 3];
      r += p[0] * (1.f - rx) * (1.f - ry);
      g += p[1] * (1.f - rx) * (1.f - ry);
      b += p[2] * (1.f - rx) * (1.f - ry);
      p = &dst_img[(y * dst_w + x) * 3];
      p[0] = static_cast<__fp16>(r);
      p[1] = static_cast<__fp16>(g);
      p[2] = static_cast<__fp16>(b);
    }
  }
}

static void downsample(__fp16 *src_img, __fp16 *dst_img, int dst_w, int dst_h) {
  for (int y = 0; y < dst_h; y++) {
    for (int x = 0; x < dst_w; x++) {
      float r = 0.f, g = 0.f, b = 0.f;
      __fp16 *p;
      p = &src_img[((y * 2 + 0) * dst_w * 2 + (x * 2 + 0)) * 3];
      r += p[0];
      g += p[1];
      b += p[2];
      p = &src_img[((y * 2 + 0) * dst_w * 2 + (x * 2 + 1)) * 3];
      r += p[0];
      g += p[1];
      b += p[2];
      p = &src_img[((y * 2 + 1) * dst_w * 2 + (x * 2 + 0)) * 3];
      r += p[0];
      g += p[1];
      b += p[2];
      p = &src_img[((y * 2 + 1) * dst_w * 2 + (x * 2 + 1)) * 3];
      r += p[0];
      g += p[1];
      b += p[2];
      p = &dst_img[(y * dst_w + x) * 3];
      p[0] = static_cast<__fp16>(r * 0.25f);
      p[1] = static_cast<__fp16>(g * 0.25f);
      p[2] = static_cast<__fp16>(b * 0.25f);
    }
  }
}

static void crop_resample(__fp16 *src_img, __fp16 *dst_img, int c_x, int c_y,
    int c_w, int c_h, int src_w, int src_h, int dst_w, int dst_h) {
  float sx = float(c_w) / float(dst_w);
  float sy = float(c_h) / float(dst_h);

  for (int y = 0; y < dst_h; y++) {
    float ty = (y + 0.5f) * sy;
    int iy = int(ty);
    float ry = ty - iy;
    for (int x = 0; x < dst_w; x++) {
      float tx = (x + 0.5f) * sx;
      int ix = int(tx);
      float rx = tx - ix;
      float r = 0.f, g = 0.f, b = 0.f;
      __fp16 *p;
      p = &src_img[((c_y + iy + 0) * src_w + (c_x + ix + 0)) * 3];
      r += p[0] * rx * ry;
      g += p[1] * rx * ry;
      b += p[2] * rx * ry;
      p = &src_img[((c_y + iy + 0) * src_w + (c_x + ix + 1)) * 3];
      r += p[0] * (1.f - rx) * ry;
      g += p[1] * (1.f - rx) * ry;
      b += p[2] * (1.f - rx) * ry;
      p = &src_img[((c_y + iy + 1) * src_w + (c_x + ix + 0)) * 3];
      r += p[0] * rx * (1.f - ry);
      g += p[1] * rx * (1.f - ry);
      b += p[2] * rx * (1.f - ry);
      p = &src_img[((c_y + iy + 1) * src_w + (c_x + ix + 1)) * 3];
      r += p[0] * (1.f - rx) * (1.f - ry);
      g += p[1] * (1.f - rx) * (1.f - ry);
      b += p[2] * (1.f - rx) * (1.f - ry);
      p = &dst_img[(y * dst_w + x) * 3];
      p[0] = static_cast<__fp16>(r);
      p[1] = static_cast<__fp16>(g);
      p[2] = static_cast<__fp16>(b);
    }
  }
}

static void prepare_net_12_inputs(void) {
  __fp16 *src_img, *dst_img;
  memcpy(net_12[0].in_buf, (void *)imgProc, IMAGE_W * IMAGE_H * 3 * 2);
  
  // resample for level 1
  src_img = reinterpret_cast<__fp16 *>(imgProc);
  dst_img = reinterpret_cast<__fp16 *>(net_12[1].in_buf);
  resample(src_img, dst_img, net_12[0].in_w, net_12[0].in_h,
    net_12[1].in_w, net_12[1].in_h);
    
  // downsample for level 2~
  for (int i = 2; i < net_12_size; ++i) {
    src_img = reinterpret_cast<__fp16 *>(net_12[i - 2].in_buf);
    dst_img = reinterpret_cast<__fp16 *>(net_12[i].in_buf);
    downsample(src_img, dst_img, net_12[i].in_w, net_12[i].in_h);
  }
}

static void print_demo_title(COverlayRGB &bg_overlay) {
  unsigned text_size = 30;
  string font_file = "font/NotoSerif-Black.ttf";
  string text;
  unsigned w = 0;
  unsigned h = 0;
  int x;
  int y;
  COverlayRGB bg_text(SCREEN_W, SCREEN_H);

  text = "Joint Face Detection";
  COverlayRGB::calculate_boundary_text_with_font(font_file, text, text_size, w, h);
  x = ((SCREEN_W - w) / 2);
  y = 25;
  bg_text.alloc_mem_overlay(w, h);
  bg_text.copy_overlay(bg_overlay, x, y);
  bg_text.set_text_with_font(font_file, text, 0, 3*h/4, text_size, 0x00ffffff);
  bg_text.print_to_display(x, y);

  text = "FPGA Demonstration";
  COverlayRGB::calculate_boundary_text_with_font(font_file, text, text_size, w, h);
  x = ((SCREEN_W - w) / 2);
  y = 65;
  bg_text.delete_overlay();
  bg_text.alloc_mem_overlay(w, h);
  bg_text.copy_overlay(bg_overlay, x, y);
  
  bg_text.set_text_with_font(font_file, text, 0, 3*h/4, text_size, 0x00ffffff);
  bg_text.print_to_display(x, y);

  text_size = 11;
  text = "Copyright 2018. Digital Media Professionals Inc.";
  COverlayRGB::calculate_boundary_text_with_font(font_file, text, text_size, w, h);
  x = 5;
  y = SCREEN_H - 20;
  bg_text.delete_overlay();
  bg_text.alloc_mem_overlay(w, h);
  bg_text.copy_overlay(bg_overlay, x, y);
  bg_text.set_text_with_font(font_file, text, 0, 3*h/4, text_size, 0x00ffffff);
  bg_text.print_to_display(x, y);
}

static void draw_bboxes(const vector<rect_point> &boxes, COverlayRGB &overlay) {
  for (auto it = boxes.begin(); it != boxes.end(); ++it) {
    uint32_t color1 = 0xFFu;
    uint32_t color2 = 0xFF00u;

    overlay.set_box(it->x1, it->y1, it->x2, it->y2, color1);
    for (int i = 0; i < 5; i++) {
      overlay.set_box(it->px[i], it->py[i], it->px[i] + 1, it->py[i] + 1, color2);
    }
  }
}

int main(int argc, char **argv) {
  if (!dmp::util::init_fb()) {
    fprintf(stderr, "dmp::util::init_fb() failed\n");
    return 1;
  }

  vector<float> prob, roi, pts;
  vector<rectangle> rect;
  vector<rect_point> result;

  if (dmp::util::open_cam(IMAGE_W, IMAGE_H, 20)) {
    return -1;
  }
  COverlayRGB bg_overlay(SCREEN_W, SCREEN_H);
  bg_overlay.alloc_mem_overlay(SCREEN_W, SCREEN_H);
  bg_overlay.load_ppm_img("fpgatitle");
  COverlayRGB cam_overlay(SCREEN_W, SCREEN_H);
  cam_overlay.alloc_mem_overlay(IMAGE_W, IMAGE_H);

  for (int i = 0; i < net_12_size; ++i) {
    net_12[i].net->Verbose(0);
    if (!net_12[i].net->Initialize())
      return -1;
    if (!net_12[i].net->LoadWeights(FILENAME_WEIGHTS12))
      return -1;
    if (!net_12[i].net->Commit())
      return -1;
    net_12[i].in_buf = net_12[i].net->get_network_input_addr_cpu();
  }
  
  net_24.Verbose(0);
  if (!net_24.Initialize())
    return -1;
  if (!net_24.LoadWeights(FILENAME_WEIGHTS24))
    return -1;
  if (!net_24.Commit())
    return -1;
  __fp16 *net_input_24 = 
    reinterpret_cast<__fp16 *>(net_24.get_network_input_addr_cpu());
  
  net_48.Verbose(0);
  if (!net_48.Initialize())
    return -1;
  if (!net_48.LoadWeights(FILENAME_WEIGHTS48))
    return -1;
  if (!net_48.Commit())
    return -1;
  __fp16 *net_input_48 = 
    reinterpret_cast<__fp16 *>(net_48.get_network_input_addr_cpu());
  
  string conv_freq, fc_freq;
  conv_freq = to_string(net_12_640x480.get_dv_info().conv_freq);
  fc_freq = to_string(net_12_640x480.get_dv_info().fc_freq);

  int exit_code = -1;
  bool pause = false;

  while (exit_code == -1) {
    // Static Images
    if (fc < 2) {
      bg_overlay.print_to_display(0, 0);
      print_demo_title(bg_overlay);
      dmp::util::swap_buffer();
      fc++;  // Frame Counter
      continue;
    }

    int key = getchar();
    switch (key) {
      case 27:  // ESC
      {
        int next_key = getchar();
        switch (next_key) {
          case 91:  // there are more value to read: UP/DOWN/LEFT/RIGHT pressed
            break;
          case 79:  // F3 pressed
            break;
          default:  // nothing special was pressed, will exit
            exit_code = 0;
            break;
        }
        break;
      }
      case 32:  // SPACE
        pause = !pause;
        break;
    }

    if (!pause) {
      if (dmp::util::capture_cam(imgView, IMAGE_W, IMAGE_H, 0, 0,
                                 IMAGE_W, IMAGE_H)) {
        break;
      }
      cam_overlay.convert_to_overlay_pixel_format(imgView, IMAGE_W*IMAGE_H);
      dmp::util::preproc_image(imgView, imgProc, IMAGE_W, IMAGE_H,
        -127.5f, -127.5f, -127.5f, 1.0f / 128.0f, false);
    }

    // Run 12Net
    rect.clear();
    conv_time_tot = 0;
    fc_time_tot = 0;
    // Copy image to FPGA memory
    prepare_net_12_inputs();

    for (int i = 0; i < net_12_size; ++i) {
      // start a new HW run
      net_12[i].net->RunNetwork();
      conv_time_tot += net_12[i].net->get_conv_usec();
      fc_time_tot += net_12[i].net->get_fc_usec();

      // handle output
      net_12[i].net->get_final_output(prob, 0);
      net_12[i].net->get_final_output(roi, 1);
      detect_face_12net(rect, prob, roi, net_12[i].scale,
        net_12[i].out_w, net_12[i].out_h, 0.6f);
    }
    
    // Run 24Net
    vector<float> temp;
    roi.clear();
    for (auto it = rect.begin(); it != rect.end(); ++it) {
      int w = it->x2 - it->x1;
      int h = it->y2 - it->y1;
      crop_resample(imgProc, net_input_24, it->x1, it->y1, w, h,
        IMAGE_W, IMAGE_H, 24, 24);
      // run 24Net
      net_24.RunNetwork();
      conv_time_tot += net_24.get_conv_usec();
      fc_time_tot += net_24.get_fc_usec();
      // handle output
      net_24.get_final_output(prob, 0);
      net_24.get_final_output(temp, 1);
      it->p = prob[1];
      if (it->p < 0.6f)
        it->p = 0.0f;
      copy(temp.begin(), temp.end(), back_inserter(roi));
    }
    filter_face_24net(rect, roi);
    
    // Run 48Net
    vector<float> temp2;
    roi.clear();
    pts.clear();
    for (auto it = rect.begin(); it != rect.end(); ++it) {
      int w = it->x2 - it->x1;
      int h = it->y2 - it->y1;
      crop_resample(imgProc, net_input_48, it->x1, it->y1, w, h,
        IMAGE_W, IMAGE_H, 48, 48);
      // run 48Net
      net_48.RunNetwork();
      conv_time_tot += net_48.get_conv_usec();
      fc_time_tot += net_48.get_fc_usec();
      // handle output
      net_48.get_final_output(prob, 0);
      net_48.get_final_output(temp, 2); // roi
      net_48.get_final_output(temp2, 1); // pts
      it->p = prob[1];
      if (it->p < 0.7f)
        it->p = 0.0f;
      copy(temp.begin(), temp.end(), back_inserter(roi));
      copy(temp2.begin(), temp2.end(), back_inserter(pts));
    }
    filter_face_48net(rect, roi, pts, result);
    
    draw_bboxes(result, cam_overlay);
    
    int x = ((SCREEN_W - IMAGE_W) / 2);
    int y = 115;
    cam_overlay.print_to_display(x, y);

    dmp::util::swap_buffer();
    fc++;

    // HW processing times
    if (conv_time_tot != 0) {
      string text = COverlayRGB::convert_time_to_text("Convolution (" + conv_freq + " MHz HW ACC)     : ", conv_time_tot);
      unsigned text_size = 14;

      unsigned w = 0;
      unsigned h = 0;
      COverlayRGB::calculate_boundary_text(text, text_size, w, h);

      int x = ((SCREEN_W - w) / 2);
      int y = (145 + IMAGE_H);

      COverlayRGB overlay_time(SCREEN_W, SCREEN_H);
      overlay_time.alloc_mem_overlay(w, h);
      overlay_time.copy_overlay(bg_overlay, x, y);
      overlay_time.set_text(0, 0, text, text_size, 0x00f4419d);
      overlay_time.print_to_display(x, y);
    }

  }

  dmp::util::shutdown();
  dmp::util::close_cam();

  return exit_code;
}
