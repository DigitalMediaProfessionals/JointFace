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

class C12Net_56x42 : public CDMP_Network {
 private:
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 0 | FPGA-Layer | Convolution | (42, 56, 3) | (20, 27, 10) | - | - |
  | 0-0 | conv1 | Convolution | (42, 56, 3) | (40, 54, 10) | - | 1504 |
  | 0-0 | pool1 | Pooling | (40, 54, 10) | (20, 27, 10) | - | - |

  */
  void Layer_0();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 1 | FPGA-Layer | Convolution | (20, 27, 10) | (18, 25, 16) | - | - |
  | 1-0 | conv2 | Convolution | (20, 27, 10) | (18, 25, 16) | - | 4672 |

  */
  void Layer_1();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 2 | FPGA-Layer | Convolution | (18, 25, 16) | (16, 23, 32) | - | - |
  | 2-0 | conv3 | Convolution | (18, 25, 16) | (16, 23, 32) | - | 9344 |

  */
  void Layer_2();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 3 | FPGA-Layer | Convolution | (16, 23, 32) | (16, 23, 2) | - | - |
  | 3-0 | conv4-1 | Convolution | (16, 23, 32) | (16, 23, 2) | - | 304 |

  */
  void Layer_3();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 4 | FPGA-Layer | SoftMax | (16, 23, 2) | (16, 23, 2) | - | - |

  */
  void Layer_4();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 5 | FPGA-Layer | Convolution | (16, 23, 32) | (16, 23, 4) | - | - |
  | 5-0 | conv4-2 | Convolution | (16, 23, 32) | (16, 23, 4) | - | 592 |

  */
  void Layer_5();

 public:
  virtual bool Initialize();
  C12Net_56x42();
  virtual ~C12Net_56x42();
};
