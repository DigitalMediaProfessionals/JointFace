/*
 *  Copyright 2018 Digital Media Professionals Inc.

 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at

 *      http://www.apache.org/licenses/LICENSE-2.0

 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.

 *  This source code was generated using DMP-DV700 tools.
 */
#pragma once

#include "dmp_network.h"

class C12Net_112x84 : public CDMP_Network {
 private:
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 0 | FPGA-Layer | Convolution | (84, 112, 3) | (41, 55, 10) | - | - |
  | 0-0 | conv1 | Convolution | (84, 112, 3) | (82, 110, 10) | - | 1504 |
  | 0-0 | pool1 | Pooling | (82, 110, 10) | (41, 55, 10) | - | - |

  */
  void Layer_0();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 1 | FPGA-Layer | Convolution | (41, 55, 10) | (39, 53, 16) | - | - |
  | 1-0 | conv2 | Convolution | (41, 55, 10) | (39, 53, 16) | - | 4672 |

  */
  void Layer_1();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 2 | FPGA-Layer | Convolution | (39, 53, 16) | (37, 51, 32) | - | - |
  | 2-0 | conv3 | Convolution | (39, 53, 16) | (37, 51, 32) | - | 9344 |

  */
  void Layer_2();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 3 | FPGA-Layer | Convolution | (37, 51, 32) | (37, 51, 2) | - | - |
  | 3-0 | conv4-1 | Convolution | (37, 51, 32) | (37, 51, 2) | - | 304 |

  */
  void Layer_3();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 4 | FPGA-Layer | SoftMax | (37, 51, 2) | (37, 51, 2) | - | - |

  */
  void Layer_4();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 5 | FPGA-Layer | Convolution | (37, 51, 32) | (37, 51, 4) | - | - |
  | 5-0 | conv4-2 | Convolution | (37, 51, 32) | (37, 51, 4) | - | 592 |

  */
  void Layer_5();

 public:
  virtual bool Initialize();
  C12Net_112x84();
  virtual ~C12Net_112x84();
};
