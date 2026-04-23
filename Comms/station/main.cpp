// Komsiki Station – ImGui dashboard with dual camera feeds, keyboard control,
// latency measurement, and camera enable/disable checkboxes.
//
// Libraries: Dear ImGui (GLFW+OpenGL3), OpenCV (GStreamer backend), POSIX sockets.

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <opencv2/opencv.hpp>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <deque>
#include <fstream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------
static const char* ROVER_IP  = "192.168.0.11";
static constexpr int CMD_PORT   = 5005;   // keyboard commands / PING
static constexpr int CTRL_PORT  = 5008;   // camera on/off control
static constexpr int TELEM_PORT = 5009;   // Wi-Fi telemetry from rover

// Sliding window size for packet-loss calculation (number of PINGs)
static constexpr int PING_WINDOW = 50;

// ---------------------------------------------------------------------------
// Shared state (protected by mutexes where needed)
// ---------------------------------------------------------------------------
struct SharedState {
    // Camera frames
    std::mutex frame_mtx[2];
    cv::Mat    frame[2];
    bool       frame_ready[2] = {false, false};

    // Keyboard
    std::mutex key_mtx;
    std::string last_key_sent;
    std::atomic<double> rtt_ms{0.0};
    std::atomic<double> jitter_ms{0.0};
    std::atomic<double> packet_loss_pct{0.0};
    std::atomic<long>   pings_sent{0};
    std::atomic<long>   pings_received{0};

    // Physical / link-layer telemetry (from rover)
    std::mutex telem_mtx;
    bool       telem_valid = false;
    double     rssi_dbm  = 0.0;
    double     noise_dbm = 0.0;
    double     snr_db    = 0.0;
    long       mac_retries = 0;
    long long  rx_errs = 0;
    long long  tx_errs = 0;
    long long  rx_bytes = 0;
    long long  tx_bytes = 0;
    double     rx_mbps = 0.0;
    double     tx_mbps = 0.0;
    std::chrono::steady_clock::time_point telem_last_time{};
    long long  telem_last_rx_bytes = -1;
    long long  telem_last_tx_bytes = -1;

    // Logging
    std::atomic<bool> logging_enabled{false};
    std::string last_key_received;  // echoed back via PING

    // Latency
    std::atomic<double> latency_ms{0.0};

    // Camera enable flags (true = streaming)
    std::atomic<bool> cam_enabled[4] = {true, true, true, true};

    // Quit flag
    std::atomic<bool> quit{false};
};

static SharedState g_state;

// ---------------------------------------------------------------------------
// UDP helpers
// ---------------------------------------------------------------------------
// Persistent UDP socket for outgoing messages (avoids repeated socket
// creation on every key press / camera control command).
static int g_udp_send_fd = -1;

static void init_udp_sender() {
    if (g_udp_send_fd >= 0) return;
    g_udp_send_fd = socket(AF_INET, SOCK_DGRAM, 0);
}

static void close_udp_sender() {
    if (g_udp_send_fd >= 0) { close(g_udp_send_fd); g_udp_send_fd = -1; }
}

static void udp_send(const char* ip, int port, const char* data, size_t len) {
    if (g_udp_send_fd < 0) return;
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port));
    inet_pton(AF_INET, ip, &addr.sin_addr);
    sendto(g_udp_send_fd, data, len, 0, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
}

static void send_camera_ctrl(int cam_idx, bool enable) {
    char msg[32];
    std::snprintf(msg, sizeof(msg), "CAM%d:%s", cam_idx + 1, enable ? "ON" : "OFF");
    udp_send(ROVER_IP, CTRL_PORT, msg, std::strlen(msg));
}

// ---------------------------------------------------------------------------
// GStreamer receive pipelines (OpenCV VideoCapture with GStreamer backend)
// ---------------------------------------------------------------------------
static const char* gst_pipeline(int port) {
    // Use a thread-local buffer to return the string
    thread_local char buf[512];
    std::snprintf(buf, sizeof(buf),
        "udpsrc port=%d caps=\"application/x-rtp, media=(string)video, "
        "clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96\" ! "
        "rtph264depay ! decodebin ! videoconvert ! "
        "appsink drop=true max-buffers=1 sync=false",
        port);
    return buf;
}

// Camera capture thread
static void camera_thread(int idx, int port) {
    cv::VideoCapture cap;
    bool was_enabled = false;

    while (!g_state.quit.load()) {
        bool enabled = g_state.cam_enabled[idx].load();

        // Open capture when transitioning to enabled
        if (enabled && !was_enabled) {
            cap.open(gst_pipeline(port), cv::CAP_GSTREAMER);
            if (!cap.isOpened()) {
                std::fprintf(stderr, "[CAM%d] Failed to open GStreamer pipeline on port %d\n",
                             idx + 1, port);
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }
            std::printf("[CAM%d] Pipeline opened on port %d\n", idx + 1, port);
        }

        // Release capture when transitioning to disabled
        if (!enabled && was_enabled) {
            cap.release();
            // Clear the frame
            std::lock_guard<std::mutex> lk(g_state.frame_mtx[idx]);
            g_state.frame[idx] = cv::Mat();
            g_state.frame_ready[idx] = false;
            std::printf("[CAM%d] Pipeline released\n", idx + 1);
        }
        was_enabled = enabled;

        if (!enabled || !cap.isOpened()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        cv::Mat f;
        if (cap.read(f) && !f.empty()) {
            cv::cvtColor(f, f, cv::COLOR_BGR2RGBA);  // ImGui expects RGBA
            std::lock_guard<std::mutex> lk(g_state.frame_mtx[idx]);
            g_state.frame[idx] = std::move(f);
            g_state.frame_ready[idx] = true;
        }
    }

    if (cap.isOpened()) cap.release();
}

// ---------------------------------------------------------------------------
// Latency probe thread – sends PING:<seq>:<timestamp_us> to rover, rover
// echoes the exact payload. Computes RTT, one-way latency (RTT/2), jitter
// (|delta RTT|) and packet loss (missing seqs within a sliding window).
// ---------------------------------------------------------------------------
static void latency_thread() {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return;

    sockaddr_in local{};
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = 0;
    bind(fd, reinterpret_cast<sockaddr*>(&local), sizeof(local));

    sockaddr_in rover{};
    rover.sin_family = AF_INET;
    rover.sin_port = htons(CMD_PORT);
    inet_pton(AF_INET, ROVER_IP, &rover.sin_addr);

    long seq = 0;
    double last_rtt_ms = -1.0;

    // Sliding window of outcomes: 1 = received, 0 = lost
    std::deque<int> window;

    while (!g_state.quit.load()) {
        ++seq;
        auto now = std::chrono::steady_clock::now();
        auto us  = std::chrono::duration_cast<std::chrono::microseconds>(
                       now.time_since_epoch()).count();
        char buf[64];
        int len = std::snprintf(buf, sizeof(buf), "PING:%ld:%ld",
                                seq, static_cast<long>(us));

        sendto(fd, buf, static_cast<size_t>(len), 0,
               reinterpret_cast<sockaddr*>(&rover), sizeof(rover));
        g_state.pings_sent.fetch_add(1);

        bool received_match = false;

        // Wait up to 500 ms for the matching echo (drain stale responses too)
        auto deadline = std::chrono::steady_clock::now() +
                        std::chrono::milliseconds(500);
        while (std::chrono::steady_clock::now() < deadline) {
            auto remaining = std::chrono::duration_cast<std::chrono::microseconds>(
                                 deadline - std::chrono::steady_clock::now()).count();
            if (remaining <= 0) break;

            timeval tv{};
            tv.tv_sec  = static_cast<time_t>(remaining / 1000000);
            tv.tv_usec = static_cast<suseconds_t>(remaining % 1000000);
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(fd, &fds);
            int sel = select(fd + 1, &fds, nullptr, nullptr, &tv);
            if (sel <= 0) break;

            char rbuf[64];
            ssize_t n = recv(fd, rbuf, sizeof(rbuf) - 1, 0);
            if (n <= 0) break;
            rbuf[n] = '\0';
            if (std::strncmp(rbuf, "PING:", 5) != 0) continue;

            // Parse "PING:<seq>:<ts_us>"
            long rseq = 0, rts = 0;
            if (std::sscanf(rbuf + 5, "%ld:%ld", &rseq, &rts) != 2) continue;

            auto recv_time = std::chrono::steady_clock::now();
            auto recv_us = std::chrono::duration_cast<std::chrono::microseconds>(
                               recv_time.time_since_epoch()).count();
            double rtt_ms = static_cast<double>(recv_us - rts) / 1000.0;

            if (rseq == seq) {
                g_state.rtt_ms.store(rtt_ms);
                g_state.latency_ms.store(rtt_ms / 2.0);
                if (last_rtt_ms >= 0.0) {
                    g_state.jitter_ms.store(std::fabs(rtt_ms - last_rtt_ms));
                }
                last_rtt_ms = rtt_ms;
                g_state.pings_received.fetch_add(1);
                received_match = true;
                break;
            }
            // Stale echo from a prior seq – ignore and keep waiting
        }

        window.push_back(received_match ? 1 : 0);
        if (static_cast<int>(window.size()) > PING_WINDOW) window.pop_front();
        int lost = 0;
        for (int v : window) if (!v) ++lost;
        g_state.packet_loss_pct.store(
            100.0 * static_cast<double>(lost) / static_cast<double>(window.size()));

        // Pace to ~2 Hz overall (roughly 500 ms period including wait)
        if (received_match) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
    close(fd);
}

// ---------------------------------------------------------------------------
// Telemetry receiver thread – listens for TELEM:... packets from rover and
// updates the shared state. Also derives Tx/Rx throughput (Mbps) from the
// delta of tx_bytes/rx_bytes between two consecutive samples.
// ---------------------------------------------------------------------------
static void telemetry_recv_thread() {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return;

    int reuse = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(TELEM_PORT);
    if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::fprintf(stderr, "telemetry bind failed\n");
        close(fd);
        return;
    }

    char buf[1024];
    while (!g_state.quit.load()) {
        timeval tv{};
        tv.tv_sec = 1;
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        int sel = select(fd + 1, &fds, nullptr, nullptr, &tv);
        if (sel <= 0) continue;

        ssize_t n = recv(fd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) continue;
        buf[n] = '\0';
        if (std::strncmp(buf, "TELEM:", 6) != 0) continue;

        // Parse comma-separated key=value pairs
        double rssi = 0, noise = 0, snr = 0;
        long   retries = 0;
        long long rx_errs = 0, tx_errs = 0, rx_bytes = 0, tx_bytes = 0;
        int wvalid = 0, dvalid = 0;

        std::string body(buf + 6);
        std::stringstream ss(body);
        std::string tok;
        while (std::getline(ss, tok, ',')) {
            auto eq = tok.find('=');
            if (eq == std::string::npos) continue;
            std::string k = tok.substr(0, eq);
            std::string v = tok.substr(eq + 1);
            try {
                if      (k == "rssi")     rssi = std::stod(v);
                else if (k == "noise")    noise = std::stod(v);
                else if (k == "snr")      snr = std::stod(v);
                else if (k == "retries")  retries = std::stol(v);
                else if (k == "rx_errs")  rx_errs = std::stoll(v);
                else if (k == "tx_errs")  tx_errs = std::stoll(v);
                else if (k == "rx_bytes") rx_bytes = std::stoll(v);
                else if (k == "tx_bytes") tx_bytes = std::stoll(v);
                else if (k == "wvalid")   wvalid = std::stoi(v);
                else if (k == "dvalid")   dvalid = std::stoi(v);
            } catch (...) { /* ignore malformed field */ }
        }

        auto now = std::chrono::steady_clock::now();
        std::lock_guard<std::mutex> lk(g_state.telem_mtx);
        g_state.rssi_dbm   = rssi;
        g_state.noise_dbm  = noise;
        g_state.snr_db     = snr;
        g_state.mac_retries = retries;
        g_state.rx_errs    = rx_errs;
        g_state.tx_errs    = tx_errs;

        if (dvalid && g_state.telem_last_rx_bytes >= 0) {
            double dt = std::chrono::duration<double>(
                            now - g_state.telem_last_time).count();
            if (dt > 1e-3) {
                long long drx = rx_bytes - g_state.telem_last_rx_bytes;
                long long dtx = tx_bytes - g_state.telem_last_tx_bytes;
                if (drx < 0) drx = 0;
                if (dtx < 0) dtx = 0;
                g_state.rx_mbps = (static_cast<double>(drx) * 8.0) / (dt * 1.0e6);
                g_state.tx_mbps = (static_cast<double>(dtx) * 8.0) / (dt * 1.0e6);
            }
        }
        g_state.rx_bytes = rx_bytes;
        g_state.tx_bytes = tx_bytes;
        g_state.telem_last_rx_bytes = rx_bytes;
        g_state.telem_last_tx_bytes = tx_bytes;
        g_state.telem_last_time = now;
        g_state.telem_valid = (wvalid != 0) || (dvalid != 0);
    }
    close(fd);
}

// ---------------------------------------------------------------------------
// CSV logger thread – when g_state.logging_enabled is set, samples current
// metrics at 10 Hz and appends a row with millisecond-precision wall-clock
// timestamp. Creates a new file on each logging-enable transition.
// ---------------------------------------------------------------------------
static std::string make_log_filename() {
    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm_local;
#ifdef _WIN32
    localtime_s(&tm_local, &tt);
#else
    localtime_r(&tt, &tm_local);
#endif
    char buf[64];
    std::strftime(buf, sizeof(buf), "komsiki_log_%Y%m%d_%H%M%S.csv", &tm_local);
    return std::string(buf);
}

static std::string iso_timestamp_ms() {
    auto now = std::chrono::system_clock::now();
    auto ms_part = std::chrono::duration_cast<std::chrono::milliseconds>(
                       now.time_since_epoch()).count() % 1000;
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm_local;
#ifdef _WIN32
    localtime_s(&tm_local, &tt);
#else
    localtime_r(&tt, &tm_local);
#endif
    char buf[40];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_local);
    char out[64];
    std::snprintf(out, sizeof(out), "%s.%03lld",
                  buf, static_cast<long long>(ms_part));
    return std::string(out);
}

static void logger_thread() {
    std::ofstream ofs;
    std::string   current_path;
    bool          was_enabled = false;

    const char* header =
        "timestamp,rtt_ms,latency_ms,jitter_ms,packet_loss_pct,"
        "pings_sent,pings_received,"
        "rssi_dbm,noise_dbm,snr_db,mac_retries,rx_errs,tx_errs,"
        "rx_bytes,tx_bytes,rx_mbps,tx_mbps\n";

    while (!g_state.quit.load()) {
        bool enabled = g_state.logging_enabled.load();

        if (enabled && !was_enabled) {
            current_path = make_log_filename();
            ofs.open(current_path, std::ios::out | std::ios::trunc);
            if (ofs.is_open()) {
                ofs << header;
                ofs.flush();
                std::printf("[LOG] Started -> %s\n", current_path.c_str());
            } else {
                std::fprintf(stderr, "[LOG] Failed to open %s\n", current_path.c_str());
            }
        }
        if (!enabled && was_enabled) {
            if (ofs.is_open()) ofs.close();
            std::printf("[LOG] Stopped -> %s\n", current_path.c_str());
        }
        was_enabled = enabled;

        if (enabled && ofs.is_open()) {
            double rssi, noise, snr, rx_mbps, tx_mbps;
            long retries;
            long long rx_errs, tx_errs, rx_bytes, tx_bytes;
            {
                std::lock_guard<std::mutex> lk(g_state.telem_mtx);
                rssi = g_state.rssi_dbm;
                noise = g_state.noise_dbm;
                snr = g_state.snr_db;
                retries = g_state.mac_retries;
                rx_errs = g_state.rx_errs;
                tx_errs = g_state.tx_errs;
                rx_bytes = g_state.rx_bytes;
                tx_bytes = g_state.tx_bytes;
                rx_mbps = g_state.rx_mbps;
                tx_mbps = g_state.tx_mbps;
            }

            char row[512];
            std::snprintf(row, sizeof(row),
                "%s,%.3f,%.3f,%.3f,%.2f,%ld,%ld,"
                "%.1f,%.1f,%.1f,%ld,%lld,%lld,%lld,%lld,%.3f,%.3f\n",
                iso_timestamp_ms().c_str(),
                g_state.rtt_ms.load(),
                g_state.latency_ms.load(),
                g_state.jitter_ms.load(),
                g_state.packet_loss_pct.load(),
                g_state.pings_sent.load(),
                g_state.pings_received.load(),
                rssi, noise, snr, retries,
                static_cast<long long>(rx_errs),
                static_cast<long long>(tx_errs),
                rx_bytes, tx_bytes, rx_mbps, tx_mbps);
            ofs << row;
            ofs.flush();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    if (ofs.is_open()) ofs.close();
}

// ---------------------------------------------------------------------------
// OpenGL texture helpers
// ---------------------------------------------------------------------------
static GLuint create_placeholder_texture(int width, int height,
                                         unsigned char r, unsigned char g,
                                         unsigned char b) {
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    const int channels = 4;
    unsigned char* pixels = new unsigned char[width * height * channels];
    for (int i = 0; i < width * height; ++i) {
        pixels[i * channels + 0] = r;
        pixels[i * channels + 1] = g;
        pixels[i * channels + 2] = b;
        pixels[i * channels + 3] = 255;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    delete[] pixels;
    return tex;
}

static GLuint create_texture() {
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return tex;
}

static void update_texture(GLuint tex, const cv::Mat& img) {
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.cols, img.rows, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, img.data);
}

// ---------------------------------------------------------------------------
// GLFW key callback – sends key name to rover via UDP
// ---------------------------------------------------------------------------
static void key_callback(GLFWwindow* /*window*/, int key, int /*scancode*/,
                          int action, int /*mods*/) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    // Map GLFW key to readable name
    const char* name = glfwGetKeyName(key, 0);
    std::string key_str;
    if (name) {
        key_str = name;
    } else {
        // Special keys
        switch (key) {
            case GLFW_KEY_SPACE:      key_str = "SPACE"; break;
            case GLFW_KEY_ENTER:      key_str = "ENTER"; break;
            case GLFW_KEY_ESCAPE:     key_str = "ESC"; break;
            case GLFW_KEY_UP:         key_str = "UP"; break;
            case GLFW_KEY_DOWN:       key_str = "DOWN"; break;
            case GLFW_KEY_LEFT:       key_str = "LEFT"; break;
            case GLFW_KEY_RIGHT:      key_str = "RIGHT"; break;
            case GLFW_KEY_LEFT_SHIFT: key_str = "LSHIFT"; break;
            case GLFW_KEY_RIGHT_SHIFT:key_str = "RSHIFT"; break;
            case GLFW_KEY_LEFT_CONTROL: key_str = "LCTRL"; break;
            case GLFW_KEY_RIGHT_CONTROL:key_str = "RCTRL"; break;
            case GLFW_KEY_TAB:        key_str = "TAB"; break;
            case GLFW_KEY_BACKSPACE:  key_str = "BACKSPACE"; break;
            default:                  key_str = "KEY_" + std::to_string(key); break;
        }
    }

    // Store for UI display
    {
        std::lock_guard<std::mutex> lk(g_state.key_mtx);
        g_state.last_key_sent = key_str;
    }

    // Send via UDP (non-blocking, fire and forget)
    udp_send(ROVER_IP, CMD_PORT, key_str.c_str(), key_str.size());
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
int main(int /*argc*/, char* /*argv*/[]) {
    // ---- GLFW + OpenGL init ----
    if (!glfwInit()) {
        std::fprintf(stderr, "Failed to initialize GLFW\n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1920, 1080,
        "KOMSIKI - Rover Control Station", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);  // disable vsync for lowest latency
    glfwSetKeyCallback(window, key_callback);

    // ---- ImGui init ----
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    ImGui::StyleColorsDark();

    // When viewports are enabled, tweak WindowRounding/WindowBg so platform
    // windows look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Scale UI a bit
    ImGui::GetStyle().ScaleAllSizes(1.2f);
    io.FontGlobalScale = 1.2f;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // ---- OpenGL textures for camera feeds ----
    // CAM1 & CAM2: real GStreamer feeds, CAM3 & CAM4: placeholders
    GLuint cam_tex[4] = {
        create_texture(),
        create_texture(),
        create_placeholder_texture(320, 240, 30, 100, 20),
        create_placeholder_texture(320, 240, 30, 130, 0),
    };
    const float placeholder_aspect = 16.0f / 9.0f;

    // ---- Persistent UDP socket for sends ----
    init_udp_sender();

    // ---- Start background threads ----
    std::thread t_cam1(camera_thread, 0, 5006);
    std::thread t_cam2(camera_thread, 1, 5007);
    std::thread t_lat(latency_thread);
    std::thread t_telem(telemetry_recv_thread);
    std::thread t_log(logger_thread);

    // Camera enable checkboxes (mirrored from atomic state)
    bool cam_checkbox[4] = {true, true, true, true};

    const char* cam_names[4] = {"CAM1", "CAM2", "CAM3", "CAM4"};
    bool log_checkbox = false;

    const char* mode_list[3] = {"SMART", "POPOUT", "PREMADE"};
    int wybrany = 0;

    // ---- Main render loop ----
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Get window size for layout
        int win_w, win_h;
        glfwGetFramebufferSize(window, &win_w, &win_h);

        // Main viewport position (screen-space origin for window positioning)
        ImGuiViewport* main_vp = ImGui::GetMainViewport();
        ImVec2 vp_pos = main_vp->Pos;

        // ================================================================
        // Control Panel (left side) — always pinned to main viewport
        // ================================================================
        ImGui::SetNextWindowViewport(main_vp->ID);
        ImGui::SetNextWindowPos(ImVec2(vp_pos.x + 10, vp_pos.y + 10), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(360, static_cast<float>(win_h) - 20), ImGuiCond_Always);
        ImGui::Begin("Panel sterowania", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking);

        // ---- Warstwa transportowa ----
        ImGui::SeparatorText("Warstwa transportowa");
        double lat = g_state.latency_ms.load();
        double rtt = g_state.rtt_ms.load();
        double jit = g_state.jitter_ms.load();
        double loss = g_state.packet_loss_pct.load();
        ImVec4 lat_color = (lat < 20.0) ? ImVec4(0, 1, 0, 1) :
                           (lat < 50.0) ? ImVec4(1, 1, 0, 1) :
                                          ImVec4(1, 0, 0, 1);
        ImGui::TextColored(lat_color, "RTT:          %.1f ms", rtt);
        ImGui::TextColored(lat_color, "Latencja:     %.1f ms (RTT/2)", lat);
        ImGui::Text            ("Jitter:       %.1f ms", jit);
        ImVec4 loss_color = (loss < 1.0)  ? ImVec4(0, 1, 0, 1) :
                            (loss < 5.0)  ? ImVec4(1, 1, 0, 1) :
                                            ImVec4(1, 0, 0, 1);
        ImGui::TextColored(loss_color, "Packet loss:  %.1f %% (okno %d)", loss, PING_WINDOW);
        ImGui::Text            ("Pingi: %ld / %ld",
                                g_state.pings_received.load(),
                                g_state.pings_sent.load());

        // ---- Warstwa fizyczna (Wi-Fi) ----
        ImGui::SeparatorText("Warstwa fizyczna (Wi-Fi)");
        {
            std::lock_guard<std::mutex> lk(g_state.telem_mtx);
            if (g_state.telem_valid) {
                ImVec4 rssi_c = (g_state.rssi_dbm > -60.0) ? ImVec4(0, 1, 0, 1) :
                                (g_state.rssi_dbm > -75.0) ? ImVec4(1, 1, 0, 1) :
                                                              ImVec4(1, 0, 0, 1);
                ImGui::TextColored(rssi_c, "RSSI:         %.1f dBm", g_state.rssi_dbm);
                ImGui::Text        ("Noise floor:  %.1f dBm", g_state.noise_dbm);
                ImVec4 snr_c  = (g_state.snr_db > 25.0) ? ImVec4(0, 1, 0, 1) :
                                (g_state.snr_db > 15.0) ? ImVec4(1, 1, 0, 1) :
                                                           ImVec4(1, 0, 0, 1);
                ImGui::TextColored(snr_c,  "SNR:          %.1f dB",  g_state.snr_db);
                ImGui::Text        ("MAC retries:  %ld",   g_state.mac_retries);
                ImGui::Text        ("Tx/Rx errs:   %lld / %lld",
        // ---- Logowanie CSV ----
        ImGui::SeparatorText("Logowanie");
        bool prev_log = log_checkbox;
        ImGui::Checkbox("Loguj do CSV", &log_checkbox);
        if (log_checkbox != prev_log) {
            g_state.logg8ng_enabled.store(log_checkbox);
        }
        if (log_checkbox) {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "  [REC]  10 Hz");
        }

                                    static_cast<long long>(g_state.tx_errs),
                                    static_cast<long long>(g_state.rx_errs));
                ImGui::Text        ("Throughput Rx: %.2f Mbps", g_state.rx_mbps);
                ImGui::Text        ("Throughput Tx: %.2f Mbps", g_state.tx_mbps);
            } else {
                ImGui::TextColored(ImVec4(1, 1, 0, 1),
                                   "Oczekiwanie na telemetrie z rovera...");
            }
        }

        // Keyboard info
        ImGui::SeparatorText("Klawiatura");
        {
            std::lock_guard<std::mutex> lk(g_state.key_mtx);
            ImGui::Text("Wyslany:  %s", g_state.last_key_sent.empty()
                        ? "-" : g_state.last_key_sent.c_str());
        }

        // Camera controls
        ImGui::SeparatorText("Kamery");
        for (int i = 0; i < 4; ++i) {
            bool prev = cam_checkbox[i];
            ImGui::Checkbox(cam_names[i], &cam_checkbox[i]);
            if (cam_checkbox[i] != prev) {
                g_state.cam_enabled[i].store(cam_checkbox[i]);
                if (i < 2) send_camera_ctrl(i, cam_checkbox[i]);
            }
            if (cam_checkbox[i]) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0, 1, 0, 1), i < 2 ? " [LIVE]" : " [PLACEHOLDER]");
            }
        }

        ImGui::SeparatorText("Info");
        ImGui::Text("FPS: %.0f", io.Framerate);
        ImGui::Text("Rover IP: %s", ROVER_IP);

        ImGui::SeparatorText("Tryb wyswietlania");
        ImGui::Combo("Opcje", &wybrany, mode_list, IM_ARRAYSIZE(mode_list));
        ImGui::Text("Wybrany tryb: %s", mode_list[wybrany]);

        ImGui::End();

        // ================================================================
        // Camera Feeds
        // ================================================================
        float feed_x = vp_pos.x + 320.0f;
        float feed_total_w = static_cast<float>(win_w) - 320.0f - 10.0f;
        float feed_total_h = static_cast<float>(win_h) - 20.0f;
        float feed_y0 = vp_pos.y + 10.0f;
        const float gap = 10.0f;

        // Collect enabled camera indices
        int active_cams[4];
        int active_count = 0;
        for (int i = 0; i < 4; ++i) {
            if (g_state.cam_enabled[i].load())
                active_cams[active_count++] = i;
        }

        // Shared: render camera image + overlay inside current window
        auto render_cam_content = [&](int cam_idx) {
            if (!g_state.cam_enabled[cam_idx].load()) {
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1), "Kamera wylaczona");
                return;
            }

            // CAM1 & CAM2: real GStreamer feed
            if (cam_idx < 2) {
                std::lock_guard<std::mutex> lk(g_state.frame_mtx[cam_idx]);
                if (g_state.frame_ready[cam_idx] && !g_state.frame[cam_idx].empty()) {
                    update_texture(cam_tex[cam_idx], g_state.frame[cam_idx]);
                    ImVec2 avail = ImGui::GetContentRegionAvail();
                    float img_aspect = static_cast<float>(g_state.frame[cam_idx].cols) /
                                       static_cast<float>(g_state.frame[cam_idx].rows);
                    float disp_w = avail.x;
                    float disp_h = disp_w / img_aspect;
                    if (disp_h > avail.y) {
                        disp_h = avail.y;
                        disp_w = disp_h * img_aspect;
                    }
                    ImGui::SetCursorPosX((avail.x - disp_w) * 0.5f + ImGui::GetCursorPosX());
                    ImGui::Image(static_cast<ImTextureID>(static_cast<uintptr_t>(cam_tex[cam_idx])),
                                 ImVec2(disp_w, disp_h));
                } else {
                    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Oczekiwanie na strumien...");
                }
            } else {
                // CAM3 & CAM4: placeholder texture
                ImVec2 avail = ImGui::GetContentRegionAvail();
                float disp_w = avail.x;
                float disp_h = disp_w / placeholder_aspect;
                if (disp_h > avail.y) {
                    disp_h = avail.y;
                    disp_w = disp_h * placeholder_aspect;
                }
                ImGui::SetCursorPosX((avail.x - disp_w) * 0.5f + ImGui::GetCursorPosX());
                ImGui::Image(static_cast<ImTextureID>(static_cast<uintptr_t>(cam_tex[cam_idx])),
                             ImVec2(disp_w, disp_h));
            }
        };

        if (wybrany == 0) {
            // ---- SMART mode: fixed layout in the feed area ----
            auto render_cam_window = [&](int cam_idx, float x, float y, float w, float h) {
                ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
                ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);
                ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);

                char title[64];
                std::snprintf(title, sizeof(title), "%s###cam%d", cam_names[cam_idx], cam_idx);
                ImGui::Begin(title, nullptr,
                             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoDocking);
                render_cam_content(cam_idx);
                ImGui::End();
            };

            if (active_count == 1) {
                render_cam_window(active_cams[0], feed_x, feed_y0, feed_total_w, feed_total_h);
            } else if (active_count == 2) {
                float h = (feed_total_h - gap) / 2.0f;
                render_cam_window(active_cams[0], feed_x, feed_y0, feed_total_w, h);
                render_cam_window(active_cams[1], feed_x, feed_y0 + h + gap, feed_total_w, h);
            } else if (active_count == 3) {
                float row_h = (feed_total_h - gap) / 2.0f;
                float half_w = (feed_total_w - gap) / 2.0f;
                render_cam_window(active_cams[0], feed_x, feed_y0, half_w, row_h);
                render_cam_window(active_cams[1], feed_x + half_w + gap, feed_y0, half_w, row_h);
                render_cam_window(active_cams[2], feed_x, feed_y0 + row_h + gap, feed_total_w, row_h);
            } else if (active_count >= 4) {
                float row_h = (feed_total_h - gap) / 2.0f;
                float half_w = (feed_total_w - gap) / 2.0f;
                render_cam_window(active_cams[0], feed_x, feed_y0, half_w, row_h);
                render_cam_window(active_cams[1], feed_x + half_w + gap, feed_y0, half_w, row_h);
                render_cam_window(active_cams[2], feed_x, feed_y0 + row_h + gap, half_w, row_h);
                render_cam_window(active_cams[3], feed_x + half_w + gap, feed_y0 + row_h + gap, half_w, row_h);
            }
        } else if (wybrany == 1) {
            // ---- POPOUT mode: independent OS windows (via ImGui viewports) ----
            for (int j = 0; j < active_count; ++j) {
                int cam_idx = active_cams[j];

                float init_w = 640.0f;
                float init_h = init_w / (16.0f / 9.0f) + 40.0f;
                ImVec2 main_pos = ImGui::GetMainViewport()->Pos;
                ImGui::SetNextWindowPos(
                    ImVec2(main_pos.x + static_cast<float>(win_w) + 20.0f + 40.0f * j,
                           main_pos.y + 40.0f * j),
                    ImGuiCond_Once);
                ImGui::SetNextWindowSize(ImVec2(init_w, init_h), ImGuiCond_Once);

                char title[64];
                std::snprintf(title, sizeof(title), "%s###cam%d", cam_names[cam_idx], cam_idx);
                ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking);
                render_cam_content(cam_idx);
                ImGui::End();
            }
        } else if (wybrany == 2) {
            // ---- PREMADE mode: free-floating windows clamped inside the main viewport ----
            ImGuiViewport* vp = ImGui::GetMainViewport();
            ImVec2 vp_min = vp->WorkPos;
            ImVec2 vp_max = ImVec2(vp_min.x + vp->WorkSize.x, vp_min.y + vp->WorkSize.y);

            for (int j = 0; j < active_count; ++j) {
                int cam_idx = active_cams[j];

                float init_w = 480.0f;
                float init_h = init_w / (16.0f / 9.0f) + 40.0f;
                ImGui::SetNextWindowPos(
                    ImVec2(feed_x + 30.0f * j, feed_y0 + 30.0f * j),
                    ImGuiCond_Once);
                ImGui::SetNextWindowSize(ImVec2(init_w, init_h), ImGuiCond_Once);
                ImGui::SetNextWindowViewport(vp->ID);

                char title[64];
                std::snprintf(title, sizeof(title), "%s###cam%d", cam_names[cam_idx], cam_idx);
                ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking);

                // Clamp window position to stay inside the main viewport
                ImVec2 wpos = ImGui::GetWindowPos();
                ImVec2 wsz  = ImGui::GetWindowSize();
                float vp_w = vp_max.x - vp_min.x;
                float vp_h = vp_max.y - vp_min.y;
                if (wsz.x > vp_w) wsz.x = vp_w;
                if (wsz.y > vp_h) wsz.y = vp_h;
                bool clamped = false;
                if (wpos.x < vp_min.x) { wpos.x = vp_min.x; clamped = true; }
                if (wpos.y < vp_min.y) { wpos.y = vp_min.y; clamped = true; }
                if (wpos.x + wsz.x > vp_max.x) { wpos.x = vp_max.x - wsz.x; clamped = true; }
                if (wpos.y + wsz.y > vp_max.y) { wpos.y = vp_max.y - wsz.y; clamped = true; }
                if (clamped) ImGui::SetWindowPos(wpos);

                render_cam_content(cam_idx);
                ImGui::End();
            }
        }

        // ---- Render ----
        ImGui::Render();
        glViewport(0, 0, win_w, win_h);
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and render additional platform windows (multi-viewport)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }

    // ---- Cleanup ----
    g_state.quit.store(true);

    t_cam1.join();
    t_cam2.join();
    t_lat.join();
    t_telem.join();
    t_log.join();

    close_udp_sender();
    glDeleteTextures(4, cam_tex);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
