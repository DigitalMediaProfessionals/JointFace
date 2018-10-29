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

class C24Net : public CDMP_Network {
 private:
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 0 | FPGA-Layer | Convolution | (24, 24, 3) | (11, 11, 28) | - | - |
  | 0-0 | conv1 | Convolution | (24, 24, 3) | (22, 22, 28) | - | 4160 |
  | 0-0 | pool1 | Pooling | (22, 22, 28) | (11, 11, 28) | - | - |

  */
  void Layer_0();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 1 | FPGA-Layer | Convolution | (11, 11, 28) | (4, 4, 48) | - | - |
  | 1-0 | conv2 | Convolution | (11, 11, 28) | (9, 9, 48) | - | 27840 |
  | 1-0 | pool2 | Pooling | (9, 9, 48) | (4, 4, 48) | - | - |

  */
  void Layer_1();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 2 | FPGA-Layer | Convolution | (4, 4, 48) | (3, 3, 64) | - | - |
  | 2-0 | conv3 | Convolution | (4, 4, 48) | (3, 3, 64) | - | 55552 |

  */
  void Layer_2();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 3 | FPGA-Layer | Convolution | (3, 3, 64) | (1, 1, 128) | - | - |
  | 3-0 | conv4 | Convolution | (3, 3, 64) | (1, 1, 128) | - | 147968 |

  */
  void Layer_3();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 4 | FPGA-Layer | InnerProduct | (1, 1, 128) | (2,) | - | - |

  */
  void Layer_4();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 5 | FPGA-Layer | SoftMax | (2,) | (2,) | - | - |

  */
  void Layer_5();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 6 | FPGA-Layer | InnerProduct | (1, 1, 128) | (4,) | - | - |

  */
  void Layer_6();

 public:
  virtual bool Initialize();
  C24Net();
  virtual ~C24Net();
};
