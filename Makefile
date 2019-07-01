include ../../env.mk

INC = -I../common/include -I/usr/include/agg2 `freetype-config --cflags`
LIB = -ldmpdv -L../common/lib -ldv700_util -lagg -lfreetype

CFLAGS = -pthread -std=c++11 $(OPT) -Wall -Werror -c $(INC)
LFLAGS = -pthread -std=c++11 $(OPT)

DEPS = 12Net_20x15_gen.h 12Net_28x21_gen.h 12Net_40x30_gen.h 12Net_56x42_gen.h 12Net_80x60_gen.h 12Net_112x84_gen.h 12Net_160x120_gen.h 12Net_224x168_gen.h 12Net_320x240_gen.h 12Net_448x336_gen.h 12Net_640x480_gen.h 24Net_gen.h 48Net_gen.h face_process.h
OBJS = 12Net_20x15_gen.o 12Net_28x21_gen.o 12Net_40x30_gen.o 12Net_56x42_gen.o 12Net_80x60_gen.o 12Net_112x84_gen.o 12Net_160x120_gen.o 12Net_224x168_gen.o 12Net_320x240_gen.o 12Net_448x336_gen.o 12Net_640x480_gen.o 24Net_gen.o 48Net_gen.o face_process.o JointFace.o
TGT  = bin/JointFace

%.o: %.cpp $(DEPS)
	$(GPP) $(CFLAGS) -o $@ $<

all : $(OBJS)
	mkdir -p bin
	$(GPP) $(LFLAGS) $(OBJS) -o $(TGT) $(LIB)

clean:
	rm -f *.o $(TGT)
