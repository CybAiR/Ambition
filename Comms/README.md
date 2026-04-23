# Komsiki – Rover Control System (C++)

System komunikacji łazik ↔ stacja bazowa oparty na C++, GStreamer i Dear ImGui.

## Architektura

```
┌─────────────────────┐  UDP video (H.264/RTP)  ┌──────────────────────────┐
│                     │ ──────────────────────► │                          │
│   Łazik (rover)     │  port 5006, 5007         │   Stacja bazowa          │
│                     │                          │   (station)              │
│  • GStreamer encode  │ ◄────────────────────── │                          │
│  • UDP cmd receiver  │  UDP komendy (port 5005) │  • ImGui dashboard       │
│  • Camera on/off     │ ◄────────────────────── │  • OpenCV + GStreamer     │
│    control           │  UDP ctrl (port 5008)    │  • Pomiar latencji       │
│  • Wi-Fi telemetria  │ ──────────────────────► │  • Metryki L1/L2/L4       │
│    (/proc/net/*)     │  UDP telem (port 5009)   │  • CSV logging (10 Hz)   │
└─────────────────────┘                          │  • Sterowanie klawiaturą │
                                                 └──────────────────────────┘
```

## Wymagania

- **Kompilator**: GCC/Clang z obsługą C++17
- **CMake** ≥ 3.20
- **OpenCV** (z backendem GStreamer)
- **GStreamer 1.0** + plugins (good, bad, ugly, libav)
- **GLFW3**
- **OpenGL** ≥ 3.3
- **Dear ImGui** (pobierane automatycznie przez CMake FetchContent)

### Instalacja zależności (Ubuntu/Debian)

```bash
sudo apt-get install -y \
  build-essential cmake \
  libopencv-dev \
  libglfw3-dev \
  libgl1-mesa-dev \
  libgstreamer1.0-dev \
  libgstreamer-plugins-base1.0-dev \
  gstreamer1.0-plugins-good \
  gstreamer1.0-plugins-bad \
  gstreamer1.0-plugins-ugly \
  gstreamer1.0-libav \
  libx264-dev
```

## Kompilacja

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

Binaria:
- `build/rover/komsiki_rover` – program łazika
- `build/station/komsiki_station` – stacja bazowa z GUI

## Konfiguracja

Edytuj adresy IP i porty bezpośrednio w plikach źródłowych:

| Parametr | Plik | Domyślna wartość |
|---|---|---|
| `BASE_STATION_IP` | `rover/main.cpp` | `192.168.0.10` |
| `ROVER_IP` | `station/main.cpp` | `192.168.0.11` |
| `CAM1_DEV` | `rover/main.cpp` | `/dev/video4` |
| `CAM2_DEV` | `rover/main.cpp` | `/dev/video6` |
| `CMD_PORT` | oba pliki | `5005` |
| `CTRL_PORT` | oba pliki | `5008` |
| `TELEM_PORT` | oba pliki | `5009` |
| `DEFAULT_WIFI_IFACE` | `rover/main.cpp` | `wlan0` |

Interfejs Wi-Fi po stronie łazika można nadpisać bez rekompilacji przez zmienną środowiskową:

```bash
KOMSIKI_WIFI_IFACE=wlp3s0 ./build/rover/komsiki_rover
```

## Uruchomienie

### Na łaziku:
```bash
./build/rover/komsiki_rover
```

### Na stacji bazowej:
```bash
./build/station/komsiki_station
```

## Funkcje stacji bazowej (UI)

- **Dual camera feed** – podgląd z dwóch kamer w czasie rzeczywistym
- **Warstwa transportowa** – RTT, latencja (RTT/2), jitter (|ΔRTT|), packet loss w oknie 50 pingów
- **Warstwa fizyczna (Wi-Fi)** – RSSI [dBm], Noise floor [dBm], SNR [dB], MAC retries, Tx/Rx errors, Throughput Tx/Rx [Mbps]
- **Status klawiatury** – pokazuje aktualnie wysyłany klawisz
- **Checkboxy kamer** – włączanie/wyłączanie poszczególnych strumieni wideo
- **Logowanie CSV** – checkbox „Loguj do CSV"; po zaznaczeniu tworzy plik `komsiki_log_YYYYMMDD_HHMMSS.csv` i zapisuje 10 próbek/s ze znacznikiem czasu z dokładnością do milisekund
- **Informacje** – FPS, IP łazika

### Architektura metryk

| Warstwa | Metryka | Źródło |
|---|---|---|
| Fizyczna | RSSI | `/proc/net/wireless` (pole `level`) – łazik |
| Fizyczna | Noise floor | `/proc/net/wireless` (pole `noise`) – łazik |
| Fizyczna | SNR | `RSSI − Noise` – łazik |
| MAC | Retries / Tx/Rx errors | `/proc/net/wireless` (`retry`) + `/proc/net/dev` – łazik |
| Transportowa | RTT / latencja | PING:`<seq>`:`<ts_us>` → echo z łazika |
| Transportowa | Jitter | `|RTT_n − RTT_{n-1}|` – stacja |
| Transportowa | Packet loss | Brakujące `seq` w oknie 50 PING-ów – stacja |
| Transportowa | Throughput Tx/Rx | Δbajtów `/proc/net/dev` / Δczasu – łazik → stacja |

### Format pliku CSV

```
timestamp,rtt_ms,latency_ms,jitter_ms,packet_loss_pct,pings_sent,pings_received,
rssi_dbm,noise_dbm,snr_db,mac_retries,rx_errs,tx_errs,
rx_bytes,tx_bytes,rx_mbps,tx_mbps
```

`timestamp` ma format `YYYY-MM-DD HH:MM:SS.mmm` (czas lokalny, ms).

## Optymalizacja latencji

- `x264enc tune=zerolatency speed-preset=ultrafast` – minimalne opóźnienie kodowania
- `bframes=0 key-int-max=15` – brak B-ramek, częste keyframe'y
- `appsink drop=true max-buffers=1 sync=false` – odrzucanie starych klatek
- `glfwSwapInterval(0)` – wyłączony vsync
- UDP transport (brak narzutu TCP)
