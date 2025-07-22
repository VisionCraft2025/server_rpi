#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <iostream>
#include <gio/gio.h>
#include "config_struct.h"

// TLS 인증서 검증 콜백
gboolean accept_certificate(GstRTSPAuth *auth,
                           GTlsConnection *conn,
                           GTlsCertificate *peer_cert,
                           GTlsCertificateFlags errors,
                           gpointer user_data) {
    GError *error = NULL;
    GTlsCertificate *ca_tls_cert = (GTlsCertificate *) user_data;
    GTlsDatabase* database = g_tls_connection_get_database(G_TLS_CONNECTION(conn));
    
    // TLS 연결 정보 출력
    std::cout << "=== TLS CONNECTION ESTABLISHED ===" << std::endl;
    std::cout << "TLS Protocol Version: " << g_tls_connection_get_protocol_version(conn) << std::endl;
    std::cout << "TLS Ciphersuite: " << g_tls_connection_get_ciphersuite_name(conn) << std::endl;
    std::cout << "================================" << std::endl;
    
    if (database) {
        GSocketConnectable *peer_identity = NULL;
        errors = g_tls_database_verify_chain(database, peer_cert,
                                           G_TLS_DATABASE_PURPOSE_AUTHENTICATE_CLIENT, 
                                           peer_identity,
                                           g_tls_connection_get_interaction(conn), 
                                           G_TLS_DATABASE_VERIFY_NONE,
                                           NULL, &error);
        std::cout << "Certificate verification errors: " << errors << std::endl;

        if (error) {
            std::cerr << "Certificate verification failed: " << error->message << std::endl;
            g_clear_error(&error);
        } else {
            std::cout << "Certificate verification successful - Secure connection established" << std::endl;
        }
    }

    return (error == 0);
}

int main(int argc, char* argv[]) {
    gst_init(&argc, &argv);

    if (argc != 2) {
        std::cerr << "Usage: ./hanwha_rtsp_ssl rtsp://<hanwha_camera_url>" << std::endl;
        return -1;
    }

    const char* hanwha_url = argv[1];
    
    // 설정 파일 읽기
    struct config rtsp_config;
    rtsp_config = get_config((char*)"rtsp_parameters.conf");
    
    // RTSP 서버 생성
    GstRTSPServer* server = gst_rtsp_server_new();
    gst_rtsp_server_set_service(server, "8553");

    // TLS 설정
    GstRTSPAuth* auth = gst_rtsp_auth_new();
    
    // 인증서 로드
    GError* error = NULL;
    GTlsCertificate* cert = g_tls_certificate_new_from_files(
        rtsp_config.rtsp_cert_pem, 
        rtsp_config.rtsp_cert_key, 
        &error);
        
    if (cert == NULL) {
        std::cerr << "Failed to load certificate: " << error->message << std::endl;
        std::cerr << "Certificate path: " << rtsp_config.rtsp_cert_pem << std::endl;
        std::cerr << "Key path: " << rtsp_config.rtsp_cert_key << std::endl;
        return -1;
    }
    
    GTlsDatabase* database = g_tls_file_database_new(rtsp_config.rtsp_ca_cert, &error);
    gst_rtsp_auth_set_tls_database(auth, database);
    
    GTlsCertificate* ca_cert = g_tls_certificate_new_from_file(rtsp_config.rtsp_ca_cert, &error);
    if (ca_cert == NULL) {
        std::cerr << "Failed to load CA certificate: " << error->message << std::endl;
        return -1;
    }
    
    gst_rtsp_auth_set_tls_authentication_mode(auth, G_TLS_AUTHENTICATION_REQUIRED);
    gst_rtsp_auth_set_tls_certificate(auth, cert);
    g_signal_connect(auth, "accept-certificate", G_CALLBACK(accept_certificate), ca_cert);
    
    // 서버에 인증 설정 적용
    gst_rtsp_server_set_auth(server, auth);

    // 마운트 포인트 생성
    GstRTSPMountPoints* mounts = gst_rtsp_server_get_mount_points(server);
    GstRTSPMediaFactory* factory = gst_rtsp_media_factory_new();

    // RTSP 릴레이용 파이프라인: H.264 패스스루
    std::string pipeline = std::string(
        "( rtspsrc location=") + hanwha_url +
        " latency=50 ! rtph264depay ! rtph264pay config-interval=1 name=pay0 pt=96 )";

    gst_rtsp_media_factory_set_launch(factory, pipeline.c_str());
    gst_rtsp_media_factory_set_shared(factory, TRUE);
    
    // TLS 프로필 설정
    gst_rtsp_media_factory_set_profiles(factory, (GstRTSPProfile)(GST_RTSP_PROFILE_SAVP | GST_RTSP_PROFILE_SAVPF));
    
    gst_rtsp_mount_points_add_factory(mounts, "/stream_pno", factory);
    g_object_unref(mounts);

    // 서버 시작
    if (gst_rtsp_server_attach(server, NULL) == 0) {
        std::cerr << "RTSP server attach failed" << std::endl;
        return -1;
    }

    std::cout << "RTSP server running at rtsps://192.168.0.76:8553/stream_pno" << std::endl;

    GMainLoop* loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    return 0;
}