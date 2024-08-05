LIB_OBJS = \
	nanopb/pb_common.o\
	nanopb/pb_encode.o\
	nanopb/pb_decode.o\

MY_CFLAGS =-Wall

all: demo

%.o: %.c
	$(CC) -c -o $@ $< -Inanopb $(CFLAGS) $(MY_CFLAGS)

demo.o: demo.pb.c

demo.pb.c:
	python nanopb/generator/nanopb_generator.py demo.proto

demo: demo.o demo.pb.o $(LIB_OBJS)
	$(CC) -o demo demo.o demo.pb.o $(LIB_OBJS)

clean:
	rm -f demo *.o nanopb/*.o *.pb.?
