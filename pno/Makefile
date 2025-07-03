#  hanwha_rtsp
CXX = g++
TARGET = hanwha_rtsp
SRC = hanwha_rtsp.cpp

CXXFLAGS = `pkg-config --cflags gstreamer-1.0 gstreamer-rtsp-server-1.0`
LDFLAGS = `pkg-config --libs gstreamer-1.0 gstreamer-rtsp-server-1.0`

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(SRC) -o $(TARGET) $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -f $(TARGET)
