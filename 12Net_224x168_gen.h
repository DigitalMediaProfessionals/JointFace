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

class C12Net_224x168 : public CDMP_Network {
 private:
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 0 | FPGA-Layer | Convolution | (168, 224, 3) | (166, 222, 10) | - | - |
  | 0-0 | conv1 | Convolution | (168, 224, 3) | (166, 222, 10) | - | 1504 |

  */
  void Layer_0();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 1 | FPGA-Layer | Convolution | (166, 222, 10) | (83, 111, 10) | - | - |
  | 1-0 | pool1 | Pooling | (166, 222, 10) | (83, 111, 10) | - | - |

  */
  void Layer_1();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 2 | FPGA-Layer | Convolution | (83, 111, 10) | (81, 109, 16) | - | - |
  | 2-0 | conv2 | Convolution | (83, 111, 10) | (81, 109, 16) | - | 4672 |

  */
  void Layer_2();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 3 | FPGA-Layer | Convolution | (81, 109, 16) | (79, 107, 32) | - | - |
  | 3-0 | conv3 | Convolution | (81, 109, 16) | (79, 107, 32) | - | 9344 |

  */
  void Layer_3();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 4 | FPGA-Layer | Convolution | (79, 107, 32) | (79, 107, 2) | - | - |
  | 4-0 | conv4-1 | Convolution | (79, 107, 32) | (79, 107, 2) | - | 304 |

  */
  void Layer_4();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 5 | FPGA-Layer | SoftMax | (79, 107, 2) | (79, 107, 2) | - | - |

  */
  void Layer_5();
  /*!

  Layer description

  | ID | Layers | Type | Dim In | Dim Out | Param | Mem |
  | :- | :- | :-: | :-: | :-: | :-: | :-: |
  | 6 | FPGA-Layer | Convolution | (79, 107, 32) | (33812,) | - | - |
  | 6-0 | conv4-2 | Convolution | (79, 107, 32) | (79, 107, 4) | - | 592 |

  */
  void Layer_6();

 public:
  virtual bool Initialize();
  C12Net_224x168();
  virtual ~C12Net_224x168();
};
