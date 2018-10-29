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

class C48Net : public CDMP_Network {
 private:
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 0 | FPGA-Layer | Convolution | (48, 48, 3) | (23, 23, 32) | - | - |
  | 0-0 | conv1 | Convolution | (48, 48, 3) | (46, 46, 32) | - | 4736 |
  | 0-0 | pool1 | Pooling | (46, 46, 32) | (23, 23, 32) | - | - |

  */
  void Layer_0();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 1 | FPGA-Layer | Convolution | (23, 23, 32) | (10, 10, 64) | - | - |
  | 1-0 | conv2 | Convolution | (23, 23, 32) | (21, 21, 64) | - | 37120 |
  | 1-0 | pool2 | Pooling | (21, 21, 64) | (10, 10, 64) | - | - |

  */
  void Layer_1();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 2 | FPGA-Layer | Convolution | (10, 10, 64) | (4, 4, 64) | - | - |
  | 2-0 | conv3 | Convolution | (10, 10, 64) | (8, 8, 64) | - | 73984 |
  | 2-0 | pool3 | Pooling | (8, 8, 64) | (4, 4, 64) | - | - |

  */
  void Layer_2();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 3 | FPGA-Layer | Convolution | (4, 4, 64) | (3, 3, 128) | - | - |
  | 3-0 | conv4 | Convolution | (4, 4, 64) | (3, 3, 128) | - | 147968 |

  */
  void Layer_3();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 4 | FPGA-Layer | Convolution | (3, 3, 128) | (1, 1, 256) | - | - |
  | 4-0 | conv5 | Convolution | (3, 3, 128) | (1, 1, 256) | - | 590848 |

  */
  void Layer_4();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 5 | FPGA-Layer | InnerProduct | (1, 1, 256) | (2,) | - | - |

  */
  void Layer_5();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 6 | FPGA-Layer | SoftMax | (2,) | (2,) | - | - |

  */
  void Layer_6();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 7 | FPGA-Layer | InnerProduct | (1, 1, 256) | (10,) | - | - |

  */
  void Layer_7();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 8 | FPGA-Layer | InnerProduct | (1, 1, 256) | (4,) | - | - |

  */
  void Layer_8();

 public:
  virtual bool Initialize();
  C48Net();
  virtual ~C48Net();
};
