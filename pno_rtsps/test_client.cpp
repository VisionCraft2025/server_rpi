#include <gst/gst.h>
#include <iostream>
#include <gio/gio.h>

// TLS 인증서 검증 콜백
static gboolean
on_accept_certificate(GTlsConnection *conn, GTlsCertificate *peer_cert,
                     GTlsCertificateFlags errors, gpointer user_data)
{
    std::cout << "=== TLS 인증서 검증 ===\n";
    std::cout << "TLS Protocol: " << g_tls_connection_get_protocol_version(conn) << "\n";
    std::cout << "TLS Cipher: " << g_tls_connection_get_ciphersuite_name(conn) << "\n";
    std::cout << "======================\n";
    
    // 테스트를 위해 모든 인증서 허용
    return TRUE;
}

static gboolean
bus_call(GstBus *bus, GstMessage *msg, gpointer data)
{
    GMainLoop *loop = (GMainLoop *)data;

    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_ERROR: {
        GError *err;
        gchar *debug;
        gst_message_parse_error(msg, &err, &debug);
        std::cerr << "Error: " << err->message << std::endl;
        g_error_free(err);
        g_free(debug);
        g_main_loop_quit(loop);
        break;
    }
    case GST_MESSAGE_EOS:
        std::cout << "End of stream" << std::endl;
        g_main_loop_quit(loop);
        break;
    default:
        break;
    }
    return TRUE;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <rtsps-url>" << std::endl;
        return -1;
    }

    gst_init(&argc, &argv);
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);

    // 파이프라인 생성
    GstElement *pipeline = gst_pipeline_new("rtsps-client");
    GstElement *source = gst_element_factory_make("rtspsrc", "source");
    GstElement *depay = gst_element_factory_make("rtph264depay", "depay");
    GstElement *parse = gst_element_factory_make("h264parse", "parse");
    GstElement *decoder = gst_element_factory_make("avdec_h264", "decoder");
    GstElement *convert = gst_element_factory_make("videoconvert", "convert");
    GstElement *sink = gst_element_factory_make("autovideosink", "sink");

    if (!pipeline || !source || !depay || !parse || !decoder || !convert || !sink) {
        std::cerr << "Elements could not be created." << std::endl;
        return -1;
    }

    // TLS 설정
    g_object_set(G_OBJECT(source), "location", argv[1], NULL);
    g_object_set(G_OBJECT(source), "do-rtcp", TRUE, NULL);
    g_object_set(G_OBJECT(source), "latency", 100, NULL);
    g_object_set(G_OBJECT(source), "tls-validation-flags", 0, NULL);
    
    // TLS 인증서 검증 콜백 설정
    g_signal_connect(source, "accept-certificate", G_CALLBACK(on_accept_certificate), NULL);

    // 파이프라인에 요소 추가
    gst_bin_add_many(GST_BIN(pipeline), source, depay, parse, decoder, convert, sink, NULL);

    // 요소 연결 (rtspsrc는 동적 패드를 사용하므로 나중에 연결)
    if (!gst_element_link_many(depay, parse, decoder, convert, sink, NULL)) {
        std::cerr << "Elements could not be linked." << std::endl;
        gst_object_unref(pipeline);
        return -1;
    }

    // rtspsrc의 패드가 생성될 때 연결하는 콜백
    g_signal_connect(source, "pad-added", G_CALLBACK([](GstElement *src, GstPad *pad, gpointer data) {
        GstElement *depay = (GstElement *)data;
        GstPad *sinkpad = gst_element_get_static_pad(depay, "sink");
        
        if (gst_pad_link(pad, sinkpad) != GST_PAD_LINK_OK) {
            std::cerr << "Failed to link pads!" << std::endl;
        } else {
            std::cout << "Pads linked successfully!" << std::endl;
        }
        
        gst_object_unref(sinkpad);
    }), depay);

    // 버스 설정
    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_watch(bus, bus_call, loop);
    gst_object_unref(bus);

    // 파이프라인 시작
    std::cout << "Starting pipeline..." << std::endl;
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // 메인 루프 실행
    g_main_loop_run(loop);

    // 정리
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(GST_OBJECT(pipeline));
    g_main_loop_unref(loop);

    return 0;
}