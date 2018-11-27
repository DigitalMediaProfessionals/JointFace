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

#include <stdlib.h>
#include <string.h>
#include "face_process.h"

using namespace std;
using namespace dmp;
using namespace util;

#define USE_LINEAR_INTERPOLATION 0

#define IMAGE_W 640
#define IMAGE_H 480

extern __fp16 imgProc[IMAGE_W * IMAGE_H * 3];
extern C12Net net_12[];

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

void detect_face_12net(vector<rectangle> &rect, vector<float> &prob,
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

void filter_face_24net(vector<rectangle> &rect, vector<float> &roi) {
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

void filter_face_48net(vector<rectangle> &rect, vector<float> &roi,
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
#if USE_LINEAR_INTERPOLATION
    float ry = ty - iy;
#endif
    for (int x = 0; x < dst_w; x++) {
      float tx = (x + 0.5f) * sx;
      int ix = int(tx);
#if USE_LINEAR_INTERPOLATION
      float rx = tx - ix;
#endif
      float r = 0.f, g = 0.f, b = 0.f;
      __fp16 *p;
#if USE_LINEAR_INTERPOLATION
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
#else
      p = &src_img[((iy) * src_w + (ix)) * 3];
      r = p[0];
      g = p[1];
      b = p[2];
#endif
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
#if USE_LINEAR_INTERPOLATION
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
#else
      p = &src_img[((y * 2) * dst_w * 2 + (x * 2)) * 3];
      r = p[0];
      g = p[1];
      b = p[2];
      p = &dst_img[(y * dst_w + x) * 3];
      p[0] = static_cast<__fp16>(r);
      p[1] = static_cast<__fp16>(g);
      p[2] = static_cast<__fp16>(b);
#endif
    }
  }
}

void crop_resample(__fp16 *src_img, __fp16 *dst_img, int c_x, int c_y,
    int c_w, int c_h, int src_w, int src_h, int dst_w, int dst_h) {
  float sx = float(c_w) / float(dst_w);
  float sy = float(c_h) / float(dst_h);

  for (int y = 0; y < dst_h; y++) {
    float ty = (y + 0.5f) * sy;
    int iy = int(ty);
#if USE_LINEAR_INTERPOLATION
    float ry = ty - iy;
#endif
    for (int x = 0; x < dst_w; x++) {
      float tx = (x + 0.5f) * sx;
      int ix = int(tx);
#if USE_LINEAR_INTERPOLATION
      float rx = tx - ix;
#endif
      float r = 0.f, g = 0.f, b = 0.f;
      __fp16 *p;
#if USE_LINEAR_INTERPOLATION
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
#else
      p = &src_img[((c_y + iy) * src_w + (c_x + ix)) * 3];
      r = p[0];
      g = p[1];
      b = p[2];
#endif
      p = &dst_img[(y * dst_w + x) * 3];
      p[0] = static_cast<__fp16>(r);
      p[1] = static_cast<__fp16>(g);
      p[2] = static_cast<__fp16>(b);
    }
  }
}

void prepare_net_12_inputs(int net_12_size) {
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

void draw_bboxes(const vector<rect_point> &boxes, COverlayRGB &overlay) {
  for (auto it = boxes.begin(); it != boxes.end(); ++it) {
    uint32_t color1 = 0xFFu;
    uint32_t color2 = 0xFF00u;

    overlay.set_box(it->x1, it->y1, it->x2, it->y2, color1);
    for (int i = 0; i < 5; i++) {
      overlay.set_box(it->px[i], it->py[i], it->px[i] + 1, it->py[i] + 1, color2);
    }
  }
}
