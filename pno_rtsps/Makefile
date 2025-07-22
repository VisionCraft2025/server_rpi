CXX = g++
TARGET = hanwha_rtsp_ssl
SRC = hanwha_rtsp_ssl.cpp

CXXFLAGS = `pkg-config --cflags gstreamer-1.0 gstreamer-rtsp-server-1.0 gio-2.0`
LDFLAGS = `pkg-config --libs gstreamer-1.0 gstreamer-rtsp-server-1.0 gio-2.0`

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(SRC) -o $(TARGET) $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -f $(TARGET)