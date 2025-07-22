# RTSP SSL 서버 (한화 카메라용)

이 프로그램은 한화 카메라의 RTSP 스트림을 SSL/TLS로 보호된 RTSP 스트림으로 변환합니다.

## 필요 패키지

```bash
sudo apt-get install libgstreamer1.0-dev libgstrtspserver-1.0-dev gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly
```

## 빌드 방법

```bash
make
```

## 실행 방법

```bash
./hanwha_rtsp_ssl rtsp://admin:password@192.168.0.23:554/profile2/media.smp
```

## 접속 방법

클라이언트에서 다음 URL로 접속:

```
rtsps://서버IP:8553/stream_pno
```

## 인증서 정보

- 인증서 파일들은 `digital_certificates` 폴더에 있습니다.
- 설정은 `rtsp_parameters.conf` 파일에서 변경할 수 있습니다.