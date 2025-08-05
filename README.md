# server_rpi
라즈베리파이 카메라 서버, 한화 카메라 서버


## 라즈베리파이 카메라(피더) 실행하는 법

```bash
cd feeder_server
make
./feeder_server
```

## 라즈베리파이 카메라(컨베이어) 실행하는 법

```bash
cd conveyor_server
make
./conveyor_server
```

## pno/hanwha_rtsp 서버 돌리는 법
```bash
cd hanwha_rtsp
./hanwha_rtsp 'rtsp://admin:veda1357%21@192.168.0.23:554/profile2/media.smp'
```

# 현재 Feeder, Conveyor의 RTSP 서버는 영상처리 코드에 통합되어 있습니다.

# 영상 스트림 시스템 구성도

## 전체 시스템 아키텍처

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           Qt 클라이언트 (Windows)                           │
│  ┌─────────────────────────────────────────────────────────────────────┐   │
│  │                    client_qt/                                        │   │
│  │  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────────┐  │   │
│  │  │   Streamer      │  │   VideoPlayer   │  │   MainWindow        │  │   │
│  │  │   (OpenCV)      │  │   (QMediaPlayer)│  │   (UI)              │  │   │
│  │  └─────────────────┘  └─────────────────┘  └─────────────────────┘  │   │
│  └─────────────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    │ RTSP 연결
                                    │
┌─────────────────────────────────────────────────────────────────────────────┐
│                        라즈베리파이 서버들                                   │
│                                                                             │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────────────────┐ │
│  │   PNO 서버      │  │  Conveyor 서버   │  │      Feeder 서버            │ │
│  │  (Port: 8553)   │  │  (Port: 8555)   │  │     (Port: 8554)            │ │
│  │                 │  │                 │  │                             │ │
│  │ ┌─────────────┐ │  │ ┌─────────────┐ │  │ ┌─────────────────────────┐ │ │
│  │ │GStreamer    │ │  │ │GStreamer    │ │  │ │ GStreamer               │ │ │
│  │ │RTSP Server  │ │  │ │RTSP Server  │ │  │ │ RTSP Server             │ │ │
│  │ │(H.264 Relay)│ │  │ │(H.264 Enc)  │ │  │ │ (H.264 Enc)             │ │ │
│  │ └─────────────┘ │  │ └─────────────┘ │  │ └─────────────────────────┘ │ │
│  └─────────────────┘  └─────────────────┘  └─────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    │ 원본 카메라 연결
                                    │
┌─────────────────────────────────────────────────────────────────────────────┐
│                           카메라 장치들                                     │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────────────────┐ │
│  │   한화 카메라    │  │   컨베이어 카메라 │  │      피더 카메라            │ │
│  │  (RTSP)         │  │   (USB/CSI)     │  │     (USB/CSI)               │ │
│  └─────────────────┘  └─────────────────┘  └─────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────────────────┘
```

## RTSP 및 H.264 사용 방식

### 1. PNO 서버 (한화 카메라 릴레이)
```cpp
// pno/hanwha_rtsp.cpp
// RTSP 릴레이 방식 - H.264 패스스루
std::string pipeline = 
    "( rtspsrc location=" + hanwha_url + " latency=50 ! "
    "rtph264depay ! rtph264pay config-interval=1 name=pay0 pt=96 )";

// 서비스 포트: 8553
// 스트림 경로: /stream_pno
// 클라이언트 접속 URL: rtsp://192.168.0.78:8553/stream_pno
```

**특징:**
- 한화 카메라의 RTSP 스트림을 그대로 릴레이
- H.264 디코딩/인코딩 없이 패스스루 방식
- 지연시간 최소화 (latency=50)

### 2. Conveyor 서버 (컨베이어 영상 처리)
```cpp
// Image-Processing-main/conveyor_rtsp_tls/conveyor_cam_streamer.cpp
// 원본 영상 (1280x720)
"( appsrc name=rawsource ! "
"videoconvert ! video/x-raw,format=I420,width=1280,height=720,framerate=30/1 ! "
"x264enc tune=zerolatency bitrate=2000 speed-preset=ultrafast key-int-max=15 ! "
"rtph264pay name=pay0 pt=96 )"

// 처리된 영상 (640x480)  
"( appsrc name=processedsource ! "
"videoconvert ! video/x-raw,format=I420,width=640,height=480,framerate=30/1 ! "
"x264enc tune=zerolatency bitrate=1000 speed-preset=ultrafast key-int-max=15 ! "
"rtph264pay name=pay0 pt=96 )"

// 서비스 포트: 8555
// 스트림 경로: /process2 (처리된 영상)
// 클라이언트 접속 URL: rtsp://192.168.0.52:8555/process2
```

**특징:**
- OpenCV로 영상 처리 후 GStreamer로 H.264 인코딩
- 두 가지 해상도 제공 (원본/처리)
- 실시간 처리 (zerolatency, ultrafast)

### 3. Feeder 서버 (피더 영상 처리)
```cpp
// Image-Processing-main/feeder_rtsp_tls_ver2/src/main_headless.cpp
// 처리된 영상 (320x240)
"( appsrc name=process_source ! "
"videoconvert ! video/x-raw,format=I420,width=320,height=240,framerate=30/1 ! "
"x264enc tune=zerolatency bitrate=1000 speed-preset=ultrafast key-int-max=15 ! "
"rtph264pay name=pay0 pt=96 )"

// 원본 영상 (1280x720)
"( appsrc name=raw_source ! "
"videoconvert ! video/x-raw,format=I420,width=1280,height=720,framerate=30/1 ! "
"x264enc tune=zerolatency bitrate=2000 speed-preset=ultrafast key-int-max=15 ! "
"rtph264pay name=pay0 pt=96 )"

// 서비스 포트: 8554
// 스트림 경로: /process1 (처리된 영상), /stream1 (원본 영상)
// 클라이언트 접속 URL: rtsp://192.168.0.76:8554/process1
```

**특징:**
- OpenCV로 영상 처리 후 GStreamer로 H.264 인코딩
- 낮은 해상도 처리 영상 (320x240)으로 대역폭 절약
- 실시간 처리 최적화

## Qt 클라이언트 연결 방식

### 1. Streamer 클래스 (실시간 스트림)
```cpp
// client_qt/video/streamer.cpp
class Streamer : public QThread {
    // OpenCV VideoCapture로 RTSP 스트림 수신
    cv::VideoCapture cap;
    
    void run() {
        cap.set(cv::CAP_PROP_BUFFERSIZE, 1);  // 버퍼 최소화
        cap.open(streamUrl.toStdString());    // RTSP URL 연결
        
        while (running) {
            cv::Mat frame;
            if (cap.retrieve(frame)) {
                QImage image = cvMatToQImage(frame);  // OpenCV → QImage
                emit newFrame(image);                 // UI 업데이트
            }
        }
    }
};

// 사용 예시
rpiStreamer = new Streamer("rtsp://192.168.0.76:8554/process1", this);
connect(rpiStreamer, &Streamer::newFrame, this, &MainWindow::updateRPiImage);
rpiStreamer->start();
```

**특징:**
- QThread 기반 비동기 스트리밍
- OpenCV VideoCapture로 RTSP 수신
- 버퍼 최소화로 지연시간 감소
- QImage로 변환하여 UI 업데이트

### 2. VideoPlayer 클래스 (녹화 영상 재생)
```cpp
// client_qt/video/videoplayer.cpp
class VideoPlayer : public QWidget {
    QMediaPlayer* m_mediaPlayer;
    QVideoWidget* m_videoWidget;
    
    void loadAndPlayVideo() {
        QUrl videoUrl = QUrl::fromLocalFile(m_videoPath);
        m_mediaPlayer->setSource(videoUrl);
        m_mediaPlayer->setVideoOutput(m_videoWidget);
        m_mediaPlayer->play();
    }
};
```

**특징:**
- QMediaPlayer로 로컬 파일 재생
- HTTP URL도 지원
- 재생 컨트롤 (재생/일시정지, 슬라이더)

## 데이터 흐름

```
1. 카메라 → 라즈베리파이 서버
   - 한화 카메라: RTSP → GStreamer 릴레이
   - 컨베이어/피더: USB/CSI → OpenCV 처리 → GStreamer H.264 인코딩

2. 라즈베리파이 서버 → Qt 클라이언트
   - RTSP 스트림 전송 (포트: 8553, 8554, 8555)
   - H.264 압축으로 대역폭 절약

3. Qt 클라이언트 → UI 표시
   - Streamer: OpenCV → QImage → QLabel
   - VideoPlayer: QMediaPlayer → QVideoWidget
```

## 네트워크 구성

```
IP 주소 할당:
- Qt 클라이언트: 192.168.0.x (Windows)
- PNO 서버: 192.168.0.78 (라즈베리파이)
- Conveyor 서버: 192.168.0.52 (라즈베리파이)  
- Feeder 서버: 192.168.0.76 (라즈베리파이)

포트 할당:
- PNO: 8553
- Feeder: 8554  
- Conveyor: 8555
```

## 성능 최적화

1. **지연시간 최소화:**
   - OpenCV 버퍼 크기 1로 설정
   - GStreamer zerolatency 튜닝
   - ultrafast 인코딩 프리셋

2. **대역폭 절약:**
   - H.264 압축 사용
   - 처리된 영상은 낮은 해상도
   - 적절한 비트레이트 설정

3. **실시간 처리:**
   - QThread 기반 비동기 스트리밍
   - 프레임 드롭으로 성능 유지
   - CPU 부하 조절 (sleep 10ms) 
