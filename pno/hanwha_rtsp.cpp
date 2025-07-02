//g++ hanwha_rtsp.cpp -o relay   `pkg-config --cflags --libs gstreamer-1.0 gstreamer-rtsp-server-1.0`
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <iostream>

int main(int argc, char* argv[]) {
    gst_init(&argc, &argv);

    if (argc != 2) {
        std::cerr << "how to run: ./relay rtsp://<hanwha_camera_url>" << std::endl;
        return -1;
    }

    const char* hanwha_url = argv[1];

    // GStreamer RTSP 서버 설정
    GstRTSPServer* server = gst_rtsp_server_new();
    gst_rtsp_server_set_service(server, "8553");

    GstRTSPMountPoints* mounts = gst_rtsp_server_get_mount_points(server);
    GstRTSPMediaFactory* factory = gst_rtsp_media_factory_new();

    // RTSP 릴레이용 파이프라인: H.264 패스스루
    std::string pipeline = std::string(
        "( rtspsrc location=") + hanwha_url +
        " latency=50 ! rtph264depay ! rtph264pay config-interval=1 name=pay0 pt=96 )";

    gst_rtsp_media_factory_set_launch(factory, pipeline.c_str());
    gst_rtsp_media_factory_set_shared(factory, TRUE);
    gst_rtsp_mount_points_add_factory(mounts, "/stream_pno", factory);
    gst_object_unref(mounts);

    // 서버 시작
    if (gst_rtsp_server_attach(server, NULL) == 0) {
        std::cerr << "RTSP server attach fail" << std::endl;
        return -1;
    }

    std::cout << "rtsp on: rtsp://192.168.0.76:8554/stream_pno" << std::endl;

    GMainLoop* loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    return 0;
}
