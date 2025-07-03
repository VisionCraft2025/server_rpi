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