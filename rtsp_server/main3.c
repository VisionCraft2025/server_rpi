
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);

    // RTSP 서버 생성
    GstRTSPServer *server = gst_rtsp_server_new();
    gst_rtsp_server_set_service(server, "8554");

    // mount 포인트 생성
    GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);

    //  Raspberry Pi 카메라
    const gchar *launch_desc_stream1 =
        "( libcamerasrc ! video/x-raw,width=640,height=480,framerate=15/1 ! "
        "videoconvert ! x264enc tune=zerolatency bitrate=1000 speed-preset=ultrafast key-int-max=15 ! "
        "rtph264pay name=pay0 pt=96 )";

    GstRTSPMediaFactory *factory1 = gst_rtsp_media_factory_new();
    gst_rtsp_media_factory_set_launch(factory1, launch_desc_stream1);
    gst_rtsp_media_factory_set_shared(factory1, TRUE);
    gst_rtsp_media_factory_set_latency(factory1, 100);  // ms 단위

    gst_rtsp_mount_points_add_factory(mounts, "/stream1", factory1);


    // UDP 포트 5000
    const gchar *launch_desc_process1 =
        "( udpsrc port=5000 caps=\"application/x-rtp, media=video, encoding-name=H264, payload=96\" ! "
        "rtph264depay ! rtph264pay name=pay0 pt=96 )";

    GstRTSPMediaFactory *factory2 = gst_rtsp_media_factory_new();
    gst_rtsp_media_factory_set_launch(factory2, launch_desc_process1);
    gst_rtsp_media_factory_set_shared(factory2, TRUE);

    gst_rtsp_mount_points_add_factory(mounts, "/process1", factory2);


    // 리소스 해제 및 서버 시작
    g_object_unref(mounts);
    gst_rtsp_server_attach(server, NULL);

    g_print("Server start:\n");
    g_print("  - rtsp://<raspi_IP>:8554/stream1 (Raw camera stream)\n");
    g_print("  - rtsp://<raspi_IP>:8554/process1 (Processed result stream)\n");

    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);
}