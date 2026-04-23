#include <gst/gst.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>

#include <csignal>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

// Global quit flag for signal-safe access
static volatile sig_atomic_t g_quit_signal = 0;

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------
static const char* BASE_STATION_IP = "192.168.0.10";
static const char* ROVER_BIND_IP   = "0.0.0.0";
static constexpr int CMD_PORT   = 5005;
static constexpr int CTRL_PORT  = 5008;  // camera enable/disable control
static constexpr int TELEM_PORT = 5009;  // periodic Wi-Fi/network telemetry

// Wi-Fi interface used for link-layer metrics (/proc/net/wireless, /proc/net/dev).
// Override at runtime with env var KOMSIKI_WIFI_IFACE.
static const char* DEFAULT_WIFI_IFACE = "wlan0";

// Camera devices – adjust to your hardware
static const char* CAM1_DEV = "/dev/video4";
static const char* CAM2_DEV = "/dev/video7";

// ---------------------------------------------------------------------------
// GStreamer pipeline helpers
// ---------------------------------------------------------------------------
struct CameraStream {
    GstElement* pipeline = nullptr;
    std::atomic<bool> running{false};
    std::string name;
    std::string device;
    int dest_port = 0;
    int width = 1920;
    int height = 1080;
    int fps = 30;
    int bitrate = 3000;
};

static bool start_camera(CameraStream& cam) {
    if (cam.running.load()) return true;

    char pipeline_str[1024];
    std::snprintf(pipeline_str, sizeof(pipeline_str),
        "v4l2src device=%s ! videoconvert ! "
        "video/x-raw,width=%d,height=%d,framerate=%d/1 ! "
        "x264enc tune=zerolatency bitrate=%d speed-preset=ultrafast "
        "key-int-max=15 bframes=0 ! "
        "rtph264pay config-interval=1 pt=96 ! "
        "udpsink host=%s port=%d sync=false",
        cam.device.c_str(), cam.width, cam.height, cam.fps,
        cam.bitrate, BASE_STATION_IP, cam.dest_port);

    GError* err = nullptr;
    cam.pipeline = gst_parse_launch(pipeline_str, &err);
    if (err) {
        std::fprintf(stderr, "[%s] Pipeline error: %s\n", cam.name.c_str(), err->message);
        g_error_free(err);
        return false;
    }

    GstStateChangeReturn ret = gst_element_set_state(cam.pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        std::fprintf(stderr, "[%s] Failed to start pipeline\n", cam.name.c_str());
        gst_object_unref(cam.pipeline);
        cam.pipeline = nullptr;
        return false;
    }

    cam.running.store(true);
    std::printf("[%s] Stream started -> %s:%d\n", cam.name.c_str(), BASE_STATION_IP, cam.dest_port);
    return true;
}

static void stop_camera(CameraStream& cam) {
    if (!cam.running.load()) return;
    if (cam.pipeline) {
        gst_element_set_state(cam.pipeline, GST_STATE_NULL);
        gst_object_unref(cam.pipeline);
        cam.pipeline = nullptr;
    }
    cam.running.store(false);
    std::printf("[%s] Stream stopped\n", cam.name.c_str());
}

// ---------------------------------------------------------------------------
// UDP helpers
// ---------------------------------------------------------------------------
static int create_udp_socket(const char* bind_ip, int port) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) { perror("socket"); return -1; }

    int reuse = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port));
    inet_pton(AF_INET, bind_ip, &addr.sin_addr);

    if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        perror("bind");
        close(fd);
        return -1;
    }

    return fd;
}

// ---------------------------------------------------------------------------
// Wi-Fi / Network telemetry readers
// ---------------------------------------------------------------------------
struct WirelessStats {
    bool   valid = false;
    double rssi_dbm  = 0.0;
    double noise_dbm = 0.0;
    long   retries   = 0;
};

static WirelessStats read_wireless(const std::string& iface) {
    WirelessStats out;
    std::ifstream in("/proc/net/wireless");
    if (!in.is_open()) return out;

    std::string line;
    std::getline(in, line); // header 1
    std::getline(in, line); // header 2

    while (std::getline(in, line)) {
        size_t p = line.find_first_not_of(" \t");
        if (p == std::string::npos) continue;
        size_t colon = line.find(':', p);
        if (colon == std::string::npos) continue;
        std::string name = line.substr(p, colon - p);
        if (name != iface) continue;

        std::istringstream iss(line.substr(colon + 1));
        std::string status, link, level, noise, nwid, crypt, frag, retry;
        if (!(iss >> status >> link >> level >> noise >> nwid >> crypt >> frag >> retry)) break;

        auto strip_dot = [](std::string& s) {
            if (!s.empty() && s.back() == '.') s.pop_back();
        };
        strip_dot(level);
        strip_dot(noise);
        try {
            out.rssi_dbm  = std::stod(level);
            out.noise_dbm = std::stod(noise);
            out.retries   = std::stol(retry);
            out.valid = true;
        } catch (...) {
            out.valid = false;
        }
        break;
    }
    return out;
}

struct DevStats {
    bool      valid = false;
    long long rx_bytes = 0;
    long long rx_errs  = 0;
    long long tx_bytes = 0;
    long long tx_errs  = 0;
};

static DevStats read_dev(const std::string& iface) {
    DevStats out;
    std::ifstream in("/proc/net/dev");
    if (!in.is_open()) return out;

    std::string line;
    std::getline(in, line);
    std::getline(in, line);

    while (std::getline(in, line)) {
        size_t p = line.find_first_not_of(" \t");
        if (p == std::string::npos) continue;
        size_t colon = line.find(':', p);
        if (colon == std::string::npos) continue;
        std::string name = line.substr(p, colon - p);
        if (name != iface) continue;

        std::istringstream iss(line.substr(colon + 1));
        long long rxb, rxp, rxe, rxd, rxfi, rxfr, rxc, rxm;
        long long txb, txp, txe, txd, txfi, txco, txca, txc;
        if (iss >> rxb >> rxp >> rxe >> rxd >> rxfi >> rxfr >> rxc >> rxm
                >> txb >> txp >> txe >> txd >> txfi >> txco >> txca >> txc) {
            out.rx_bytes = rxb;
            out.rx_errs  = rxe;
            out.tx_bytes = txb;
            out.tx_errs  = txe;
            out.valid = true;
        }
        break;
    }
    return out;
}

// Telemetry sender thread – pushes sampled stats to station as plain-text UDP.
static void telemetry_thread(std::atomic<bool>& quit, const std::string& iface) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) { perror("telem socket"); return; }

    sockaddr_in dst{};
    dst.sin_family = AF_INET;
    dst.sin_port = htons(TELEM_PORT);
    inet_pton(AF_INET, BASE_STATION_IP, &dst.sin_addr);

    std::printf("Telemetry sender started (iface=%s -> %s:%d)\n",
                iface.c_str(), BASE_STATION_IP, TELEM_PORT);

    while (!quit.load() && !g_quit_signal) {
        WirelessStats w = read_wireless(iface);
        DevStats      d = read_dev(iface);

        auto now = std::chrono::system_clock::now();
        auto us  = std::chrono::duration_cast<std::chrono::microseconds>(
                       now.time_since_epoch()).count();

        double snr = w.valid ? (w.rssi_dbm - w.noise_dbm) : 0.0;

        char buf[512];
        int len = std::snprintf(buf, sizeof(buf),
            "TELEM:ts_us=%lld,iface=%s,rssi=%.1f,noise=%.1f,snr=%.1f,"
            "retries=%ld,rx_errs=%lld,tx_errs=%lld,rx_bytes=%lld,tx_bytes=%lld,"
            "wvalid=%d,dvalid=%d",
            static_cast<long long>(us), iface.c_str(),
            w.rssi_dbm, w.noise_dbm, snr,
            w.retries, d.rx_errs, d.tx_errs, d.rx_bytes, d.tx_bytes,
            w.valid ? 1 : 0, d.valid ? 1 : 0);

        if (len > 0) {
            sendto(fd, buf, static_cast<size_t>(len), 0,
                   reinterpret_cast<sockaddr*>(&dst), sizeof(dst));
        }

        for (int i = 0; i < 5 && !quit.load() && !g_quit_signal; ++i)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    close(fd);
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    gst_init(&argc, &argv);

    std::string wifi_iface = DEFAULT_WIFI_IFACE;
    if (const char* env = std::getenv("KOMSIKI_WIFI_IFACE")) {
        if (*env) wifi_iface = env;
    }

    CameraStream cam1;
    cam1.name = "CAM1_MAIN";
    cam1.device = CAM1_DEV;
    cam1.dest_port = 5006;
    cam1.width = 1920; cam1.height = 1080; cam1.fps = 30; cam1.bitrate = 3000;

    CameraStream cam2;
    cam2.name = "CAM2_AUX";
    cam2.device = CAM2_DEV;
    cam2.dest_port = 5007;
    cam2.width = 1280; cam2.height = 720; cam2.fps = 10; cam2.bitrate = 1500;

    std::printf("Starting camera streams...\n");
    start_camera(cam1);
    start_camera(cam2);

    std::atomic<bool> quit{false};

    // Telemetry sender
    std::thread telem_thread(telemetry_thread, std::ref(quit), std::cref(wifi_iface));

    // Keyboard/ping command receiver thread
    std::thread cmd_thread([&]() {
        int fd = create_udp_socket(ROVER_BIND_IP, CMD_PORT);
        if (fd < 0) return;

        std::printf("Command receiver listening on port %d\n", CMD_PORT);
        char buf[1024];
        while (!quit.load() && !g_quit_signal) {
            timeval tv{};
            tv.tv_sec = 1;
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(fd, &fds);
            int sel = select(fd + 1, &fds, nullptr, nullptr, &tv);
            if (sel <= 0) continue;

            sockaddr_in sender{};
            socklen_t slen = sizeof(sender);
            ssize_t n = recvfrom(fd, buf, sizeof(buf) - 1, 0,
                                 reinterpret_cast<sockaddr*>(&sender), &slen);
            if (n > 0) {
                buf[n] = '\0';
                if (std::strncmp(buf, "PING:", 5) == 0) {
                    sendto(fd, buf, static_cast<size_t>(n), 0,
                           reinterpret_cast<sockaddr*>(&sender), slen);
                } else {
                    std::printf("Command received: %s\n", buf);
                }
            }
        }
        close(fd);
    });

    // Camera control receiver thread
    std::thread ctrl_thread([&]() {
        int fd = create_udp_socket(ROVER_BIND_IP, CTRL_PORT);
        if (fd < 0) return;

        std::printf("Camera control listening on port %d\n", CTRL_PORT);
        char buf[256];
        while (!quit.load() && !g_quit_signal) {
            timeval tv{};
            tv.tv_sec = 1;
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(fd, &fds);
            int sel = select(fd + 1, &fds, nullptr, nullptr, &tv);
            if (sel <= 0) continue;

            sockaddr_in sender{};
            socklen_t slen = sizeof(sender);
            ssize_t n = recvfrom(fd, buf, sizeof(buf) - 1, 0,
                                 reinterpret_cast<sockaddr*>(&sender), &slen);
            if (n <= 0) continue;
            buf[n] = '\0';

            std::string msg(buf);
            if (msg == "CAM1:ON")       start_camera(cam1);
            else if (msg == "CAM1:OFF") stop_camera(cam1);
            else if (msg == "CAM2:ON")  start_camera(cam2);
            else if (msg == "CAM2:OFF") stop_camera(cam2);
            else std::fprintf(stderr, "Unknown control: %s\n", buf);
        }
        close(fd);
    });

    std::signal(SIGINT,  [](int) { g_quit_signal = 1; });
    std::signal(SIGTERM, [](int) { g_quit_signal = 1; });

    std::printf("Rover system running. Press Ctrl+C to stop.\n");
    while (!quit.load() && !g_quit_signal) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    quit.store(true);

    stop_camera(cam1);
    stop_camera(cam2);

    cmd_thread.join();
    ctrl_thread.join();
    telem_thread.join();

    gst_deinit();
    return 0;
}
