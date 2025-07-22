#!/bin/bash

# RTSP 암호화 검증 스크립트
echo "RTSP 암호화 검증 도구"
echo "======================"

# 인자 확인
if [ "$#" -ne 1 ]; then
    echo "사용법: $0 <rtsps-url>"
    echo "예시: $0 rtsps://192.168.0.76:8553/stream_pno"
    exit 1
fi

URL=$1

# URL 스키마 확인
if [[ $URL != rtsps://* ]]; then
    echo "경고: URL이 'rtsps://'로 시작하지 않습니다. 암호화되지 않은 연결일 수 있습니다."
    echo "암호화된 연결은 'rtsps://'로 시작해야 합니다."
else
    echo "URL 스키마 확인: OK (rtsps:// 프로토콜 사용)"
fi

# tcpdump로 패킷 캡처 (root 권한 필요)
echo -e "\n패킷 캡처 시작 (5초간)..."
echo "암호화된 연결이면 패킷 내용이 암호화되어 있어야 합니다."
echo "Ctrl+C로 중단할 수 있습니다."

# URL에서 포트 추출
PORT=$(echo $URL | sed -n 's/.*:\([0-9]\+\)\/.*/\1/p')
if [ -z "$PORT" ]; then
    PORT=8553  # 기본 포트
fi

# tcpdump 실행 (root 권한 필요)
sudo tcpdump -A -s 0 "tcp port $PORT" -c 20 2>/dev/null

echo -e "\n패킷 캡처 완료"
echo "암호화된 연결이면 위 출력에서 일반 텍스트가 아닌 암호화된 데이터가 보여야 합니다."
echo "RTSP 명령어(DESCRIBE, SETUP 등)가 일반 텍스트로 보인다면 암호화되지 않은 것입니다."

# GStreamer 테스트 클라이언트 실행
echo -e "\nGStreamer 테스트 클라이언트로 연결 테스트..."
echo "테스트 클라이언트를 빌드하려면 다음 명령을 실행하세요:"
echo "make -f Makefile.client"
echo "그런 다음 다음 명령으로 테스트할 수 있습니다:"
echo "./test_client $URL"