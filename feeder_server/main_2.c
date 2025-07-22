#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);

    GstRTSPServer *server = gst_rtsp_server_new();
    gst_rtsp_server_set_service(server, "8554");

    GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);

    const gchar *launch_desc =
        "( appsrc name=mysource is-live=true format=time ! "
        "videoconvert ! video/x-raw,format=I420 ! "
        "x264enc tune=zerolatency bitrate=1000 speed-preset=ultrafast key-int-max=30 ! "
        "rtph264pay name=pay0 pt=96 config-interval=1 )";
        
    GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();
    gst_rtsp_media_factory_set_launch(factory, launch_desc);
    gst_rtsp_media_factory_set_shared(factory, TRUE);
    gst_rtsp_media_factory_set_latency(factory, 500);  

    gst_rtsp_mount_points_add_factory(mounts, "/stream1", factory);
    g_object_unref(mounts);

    gst_rtsp_server_attach(server, NULL);
    g_print("Server start: rtsp://<raspi_IP>:8554/stream1\n");
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    return 0;
}

