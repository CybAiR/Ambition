import cv2
import socket
import threading
import numpy as np
from pynput import keyboard

# Konfiguracjay
ROVER_IP = "192.168.0.11"
CMD_PORT = 5005

# 1. WĄTEK KLAWIATURY
def on_press(key):
    try: msg = key.char
    except AttributeError: msg = str(key)
    sock_cmd = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock_cmd.sendto(msg.encode('utf-8'), (ROVER_IP, CMD_PORT))

def start_keyboard():
    print(f"🎮 Klawiatura aktywna. Wysyłam na {ROVER_IP}:{CMD_PORT}")
    with keyboard.Listener(on_press=on_press) as listener:
        listener.join()

kbd_thread = threading.Thread(target=start_keyboard, daemon=True)
kbd_thread.start()

# 2. KONFIGURACJA ODBIORNIKÓW WIDEO
print("🎥 Łączenie ze strumieniami GStreamer...")

# Dekoder dla Kamery 1 (Port 5006)
pipeline_1 = (
    'udpsrc port=5006 caps="application/x-rtp, media=(string)video, clock-rate=(int)90000, '
    'encoding-name=(string)H264, payload=(int)96" ! rtph264depay ! decodebin ! '
    'videoconvert ! appsink drop=true max-buffers=1 sync=false'
)

# Dekoder dla Kamery 2 (Port 5007)
pipeline_2 = (
    'udpsrc port=5007 caps="application/x-rtp, media=(string)video, clock-rate=(int)90000, '
    'encoding-name=(string)H264, payload=(int)96" ! rtph264depay ! decodebin ! '
    'videoconvert ! appsink drop=true max-buffers=1 sync=false'
)

cap1 = cv2.VideoCapture(pipeline_1, cv2.CAP_GSTREAMER)
cap2 = cv2.VideoCapture(pipeline_2, cv2.CAP_GSTREAMER)

if not cap1.isOpened() or not cap2.isOpened():
    print("❌ Błąd krytyczny: Nie udało się otworzyć jednego z portów wideo.")
    exit()

print("✅ Zestawiono oba strumienie! Otwieram Dashboard...")

while True:
    # Pobieramy klatki z obu kamer
    ret1, frame1 = cap1.read()
    ret2, frame2 = cap2.read()

    # Jeśli obie kamery dostarczyły klatkę
    if ret1 and ret2:
        # Zmniejszamy każdą klatkę do 960x540, żeby zmieściły się na monitorze
        f1_small = cv2.resize(frame1, (960, 540))
        f2_small = cv2.resize(frame2, (960, 540))

        # Dodajemy podpisy
        cv2.putText(f1_small, "KAMERA GLOWNA", (20, 40), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
        cv2.putText(f2_small, "KAMERA ZAPASOWA", (20, 40), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 255), 2)

        # Sklejamy oba obrazy w poziomie (h-stack)
        dashboard = np.hstack((f1_small, f2_small))

        cv2.imshow("LUNAR ROVER - DUAL FPV DASHBOARD", dashboard)

    # Wyjście pod klawiszem 'q'
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap1.release()
cap2.release()
cv2.destroyAllWindows()