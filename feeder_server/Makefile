CC = gcc
CFLAGS = `pkg-config --cflags gstreamer-rtsp-server-1.0`
LIBS = `pkg-config --libs gstreamer-rtsp-server-1.0`

TARGET = feeder_server

all: $(TARGET)

$(TARGET): main.c
	$(CC) -o $(TARGET) main.c $(CFLAGS) $(LIBS)

clean:
	rm -f $(TARGET)
