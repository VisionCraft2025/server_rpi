#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);

    // RTSP 서버 생성
    GstRTSPServer *server = gst_rtsp_server_new();
    gst_rtsp_server_set_service(server, "8554");

    // mount 포인트 생성
    GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);

    
    // appsrc에서 처리된 영상 수신받기 → H.264 인코딩 → RTP 패킷화
    const gchar *launch_desc =
        "( appsrc name=mysource is-live=true format=time do-timestamp=true ! "
        "videoconvert ! video/x-raw,format=I420 ! "
        "x264enc tune=zerolatency bitrate=1000 speed-preset=ultrafast key-int-max=15 ! "
        "rtph264pay name=pay0 pt=96 )";
        
    // 미디어 팩토리 생성 및 파이프라인 연결
    GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();
    gst_rtsp_media_factory_set_launch(factory, launch_desc);
    gst_rtsp_media_factory_set_shared(factory, TRUE);

    // /test URI에 연결
    gst_rtsp_media_factory_set_latency(factory, 100);  // ms 단위
    gst_rtsp_mount_points_add_factory(mounts, "/stream1", factory);
    g_object_unref(mounts);

    // 서버 시작
    gst_rtsp_server_attach(server, NULL);
    g_print("Server start: rtsp://<raspi_IP>:8554/stream1\n");
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    return 0;
}