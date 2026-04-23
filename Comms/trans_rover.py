import socket
import subprocess
import time

# Konfiguracja sieci
BASE_STATION_IP = "192.168.0.10"
ROVER_IP = "0.0.0.0"
CMD_PORT = 5005

# NUMERY TWOICH KAMER (PODMIEŃ JEŚLI SĄ INNE!)
CAM1_DEV = "/dev/video4" # Główna
CAM2_DEV = "/dev/video6" # Zapasowa / Ramię

print("🎥 Uruchamiam kompresory wideo...")

# Kamera 1 -> Port 5006
gst_cmd_1 = (
    f"gst-launch-1.0 v4l2src device={CAM1_DEV} ! videoconvert ! "
    f"video/x-raw,width=1920,height=1080,framerate=30/1 ! "
    f"x264enc tune=zerolatency bitrate=3000 speed-preset=superfast ! "
    f"rtph264pay config-interval=1 ! udpsink host={BASE_STATION_IP} port=5006"
)

# Kamera 2 -> Port 5007 (Zauważ nowy port!)
gst_cmd_2 = (
    f"gst-launch-1.0 v4l2src device={CAM2_DEV} ! videoconvert ! "
    f"video/x-raw,width=1280,height=720,framerate=10/1 ! "
    f"x264enc tune=zerolatency bitrate=3000 speed-preset=superfast ! "
    f"rtph264pay config-interval=1 ! udpsink host={BASE_STATION_IP} port=5007"
)

# Uruchamiamy oba strumienie w tle
process1 = subprocess.Popen(gst_cmd_1, shell=True)
process2 = subprocess.Popen(gst_cmd_2, shell=True)

# 2. GŁÓWNA PĘTLA: ODBIORNIK KLAWIATURY
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((ROVER_IP, CMD_PORT))
print(f"📡 Odbiornik komend uzbrojony na porcie {CMD_PORT}")

try:
    while True:
        data, addr = sock.recvfrom(1024)
        komenda = data.decode('utf-8')
        print(f"🕹️ Odebrano komendę: {komenda}")

except KeyboardInterrupt:
    print("\nZamykanie systemu łazika...")
    process1.terminate()
    process2.terminate()
    sock.close()