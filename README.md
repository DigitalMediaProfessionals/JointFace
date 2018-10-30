# JointFace
Joint face detection demo application. The original network is from here: https://github.com/CongWeilin/mtcnn-caffe

## How to generate FPGA network configs and weight files

1. Under the model folder, git clone the original network:
```console
$ cd model
$ git clone https://github.com/CongWeilin/mtcnn-caffe.git
```

2. Apply the `model.patch` on `mtcnn-caffe` folder, since PReLU layer is not supported in FC module in FPGA, we change the two InnerProduct layers which use PReLU activations into Convolution layers:
```console
$ patch -p0 -i model.patch
```

3. Run the conversion tool to generate FPGA network configs and weight files.
```console
$ python ../../../tool/convertor.py 12net_20x15.ini
$ python ../../../tool/convertor.py 12net_28x21.ini
$ python ../../../tool/convertor.py 12net_40x30.ini
$ python ../../../tool/convertor.py 12net_56x42.ini
$ python ../../../tool/convertor.py 12net_80x60.ini
$ python ../../../tool/convertor.py 12net_112x84.ini
$ python ../../../tool/convertor.py 12net_160x120.ini
$ python ../../../tool/convertor.py 12net_224x168.ini
$ python ../../../tool/convertor.py 12net_320x240.ini
$ python ../../../tool/convertor.py 12net_448x336.ini
$ python ../../../tool/convertor.py 12net_640x480.ini
$ python ../../../tool/convertor.py 24net.ini
$ python ../../../tool/convertor.py 48net.ini
```
**NOTE**: The original only have 3 networks, but 12Net is run several times with different input size.
Because the limitation of the tool, each input size must have their own configs for the FPGA.

4. Copy the generated FPGA network configs to the root folder.

5. Copy the generated weight files to the FPGA SD card, to the `bin/`folder.
The 12Net generated 11 weight files, but they are all the same. Just copy one of them and rename it to `12Net_weights.bin`
