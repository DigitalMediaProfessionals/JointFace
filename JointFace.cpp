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
#include "demo_common.h"
#include "face_process.h"

using namespace std;
using namespace dmp;
using namespace util;

#define SCREEN_W (get_screen_width())
#define SCREEN_H (get_screen_height())

#define IMAGE_W 640
#define IMAGE_H 480

#define FILENAME_WEIGHTS12 "12Net_weights.bin"
#define FILENAME_WEIGHTS24 "24Net_weights.bin"
#define FILENAME_WEIGHTS48 "48Net_weights.bin"

// Define CNN network model objects
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

// C12Net scaled network series
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

// Buffer for decoded image data
uint32_t imgView[IMAGE_W * IMAGE_H];
// Buffer for pre-processed image data
__fp16 imgProc[IMAGE_W * IMAGE_H * 3];

int main(int argc, char **argv) {
  // Initialize FB
  if (!init_fb()) {
    cout << "init_fb() failed." << endl;
    return 1;
  }

  // Initialize WebCam
  if (dmp::util::open_cam(IMAGE_W, IMAGE_H, 20)) {
    return -1;
  }

  // Initialize network objects
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

  // Get HW module frequency
  string conv_freq;
  conv_freq = to_string(net_12_640x480.get_dv_info().conv_freq);

  // Create background and image overlay
  COverlayRGB bg_overlay(SCREEN_W, SCREEN_H);
  bg_overlay.alloc_mem_overlay(SCREEN_W, SCREEN_H);
  bg_overlay.load_ppm_img("fpgatitle");
  COverlayRGB cam_overlay(SCREEN_W, SCREEN_H);
  cam_overlay.alloc_mem_overlay(IMAGE_W, IMAGE_H);

  // Draw background two times for front and back buffer
  const char *titles[] = {
    "Joint Face",
    "Face and Feature Points Detection",
  };
  for (int i = 0; i < 2; ++i) {
    bg_overlay.print_to_display(0, 0);
    print_demo_title(bg_overlay, titles);
    swap_buffer();
  }

  int exit_code = -1;
  bool pause = false;
  vector<float> prob, roi, pts;
  vector<rectangle> rect;
  vector<rect_point> result;
  int cpu_time_tot, conv_time_tot;
  TimeInterval ti;
  // Enter main loop
  while (exit_code == -1) {
    // If not pause, get a new image from WebCam and do Pre-processing
    if (!pause) {
      if (capture_cam(imgView, IMAGE_W, IMAGE_H, 0, 0, IMAGE_W, IMAGE_H)) {
        break;
      }
      cam_overlay.convert_to_overlay_pixel_format(imgView, IMAGE_W * IMAGE_H);
      preproc_image(imgView, imgProc, IMAGE_W, IMAGE_H, -127.5f, -127.5f, -127.5f,
                    1.0f / 128.0f, false);
    }

    // Run 12Net
    rect.clear();
    cpu_time_tot = 0;
    conv_time_tot = 0;

    // Copy image to FPGA memory
    ti.reset();
    prepare_net_12_inputs(net_12_size);
    cpu_time_tot += ti.get_us();

    for (int i = 0; i < net_12_size; ++i) {
      // start a new HW run
      net_12[i].net->RunNetwork();
      conv_time_tot += net_12[i].net->get_conv_usec();

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
      ti.reset();
      crop_resample(imgProc, net_input_24, it->x1, it->y1, w, h,
        IMAGE_W, IMAGE_H, 24, 24);
      cpu_time_tot += ti.get_us();
      // run 24Net
      net_24.RunNetwork();
      conv_time_tot += net_24.get_conv_usec();
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
      ti.reset();
      crop_resample(imgProc, net_input_48, it->x1, it->y1, w, h,
        IMAGE_W, IMAGE_H, 48, 48);
      cpu_time_tot += ti.get_us();
      // run 48Net
      net_48.RunNetwork();
      conv_time_tot += net_48.get_conv_usec();
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

    // Draw result
    draw_bboxes(result, cam_overlay);
    cam_overlay.print_to_display(((SCREEN_W - IMAGE_W) / 2), 115);

    // Output CPU processing times
    string text = COverlayRGB::convert_time_to_text("CPU Re-size time                 : ", cpu_time_tot);
    unsigned text_size = 14;
    unsigned w = 0;
    unsigned h = 0;
    COverlayRGB::calculate_boundary_text(text, text_size, w, h);
    int x = ((SCREEN_W - w) / 2);
    int y = (135 + IMAGE_H);
    COverlayRGB overlay_time_cpu(SCREEN_W, SCREEN_H);
    overlay_time_cpu.alloc_mem_overlay(w, h);
    overlay_time_cpu.copy_overlay(bg_overlay, x, y);
    overlay_time_cpu.set_text(0, 0, text, text_size, 0x00f4419d);
    overlay_time_cpu.print_to_display(x, y);
    // Output HW processing times
    print_conv_time(bg_overlay, (165 + IMAGE_H), conv_time_tot, conv_freq);

    swap_buffer();

    handle_keyboard_input(exit_code, pause);
 }

  dmp::util::shutdown();
  dmp::util::close_cam();

  return exit_code;
}
