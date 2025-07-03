# server_rpi
라즈베리파이 카메라 서버, 한화 카메라 서버

## 빌드
각 폴더 내에서 
```bash
make
```

## 라즈베리파이 카메라 실행하는 법

```bash
cd rtsp_server
./rtsp_server
```

## pno/hanwha_rtsp 서버 돌리는 법
```bash
cd hanwha_rtsp
./hanwha_rtsp 'rtsp://admin:veda1357%21@192.168.0.23:554/profile2/media.smp'
```