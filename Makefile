all: kmsdraw fbdraw fbdraw_vsync

kmsdraw: kmsdraw.c
	${CC} -o kmsdraw -I/usr/include/libdrm -ldrm kmsdraw.c

fbdraw: fbdraw.c
	${CC} -o fbdraw fbdraw.c

fbdraw_vsync: fbdraw_vsync.c
	${CC} -o fbdraw_vsync fbdraw_vsync.c

clean:
	rm -f kmsdraw
	rm -f fbdraw
	rm -f fbdraw_vsync