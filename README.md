# JointFace
Joint face detection demo application. The original network is from here: https://github.com/CongWeilin/mtcnn-caffe

## How to generate FPGA network configs and weight files

1. Under the model folder, git clone the original network:
```console
$ cd model
$ git clone https://github.com/CongWeilin/mtcnn-caffe.git
```

2. Run the conversion tool to generate FPGA network configs and weight files.
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

3. Copy the generated FPGA network configs to the root folder.

4. Copy the generated weight files to the FPGA SD card, to the `bin/`folder.
