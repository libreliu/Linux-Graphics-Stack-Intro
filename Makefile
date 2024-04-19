all: kmsdraw.elf fbdraw.elf fbdraw_vsync.elf mouse_listener.elf
all_sources = kmsdraw.c fbdraw.c fbdraw_vsync.c mouse_listener.c Makefile

kmsdraw.elf: kmsdraw.c
	$(CC) -o kmsdraw.elf -I/usr/include/libdrm -ldrm kmsdraw.c $(CFLAGS)

fbdraw.elf: fbdraw.c
	$(CC) -o fbdraw.elf fbdraw.c $(CFLAGS)

fbdraw_vsync.elf: fbdraw_vsync.c
	$(CC) -o fbdraw_vsync.elf fbdraw_vsync.c $(CFLAGS)

mouse_listener.elf: mouse_listener.c
	$(CC) -o mouse_listener.elf mouse_listener.c $(CFLAGS)

pack:
	tar -cf all_sources.tar $(all_sources)

clean:
	rm -f kmsdraw.elf
	rm -f fbdraw.elf
	rm -f fbdraw_vsync.elf
	rm -f mouse_listener.elf