CC = gcc
CFLAGS = `pkg-config --cflags gstreamer-rtsp-server-1.0`
LIBS = `pkg-config --libs gstreamer-rtsp-server-1.0`

all: rtsp_server

rtsp_server: main.c
	$(CC) -o rtsp_server main.c $(CFLAGS) $(LIBS)

clean:
	rm -f rtsp_server