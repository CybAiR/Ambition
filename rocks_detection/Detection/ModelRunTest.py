import cv2
import numpy as np
import tensorflow as tf
from tensorflow.keras.layers import Dense
from tensorflow.keras import Model
from tensorflow.keras.applications.vgg16 import VGG16
import threading
import time

# ==========================================
# 1. BUDOWA MODELU
# ==========================================
vggmodel = VGG16(weights='imagenet', include_top=True)
for layers in (vggmodel.layers)[:15]:
    layers.trainable = False

X = vggmodel.layers[-2].output
predictions = Dense(2, activation="softmax")(X)
model_final = Model(inputs=vggmodel.input, outputs=predictions)

model_final.load_weights("depthai_model/ieeercnn_vgg16_1.h5")

ss = cv2.ximgproc.segmentation.createSelectiveSearchSegmentation()


# ==========================================
# 2. WIELOWĄTKOWA KAMERA (Eliminuje lag!)
# ==========================================
class VideoCamera:
    def __init__(self, src=0):
        self.cap = cv2.VideoCapture(src)
        self.ret, self.frame = self.cap.read()
        self.running = True
        # Odpalamy pobieranie klatek w tle (osobny wątek)
        self.thread = threading.Thread(target=self.update, daemon=True)
        self.thread.start()

    def update(self):
        # Ten wątek robi TYLKO jedno: dba, aby self.frame było zawsze najświeższe
        while self.running:
            self.ret, self.frame = self.cap.read()

    def get_frame(self):
        return self.ret, self.frame

    def stop(self):
        self.running = False
        self.thread.join()
        self.cap.release()


# ==========================================
# 3. GŁÓWNA PĘTLA
# ==========================================
cam = VideoCamera(0)
time.sleep(2)  # Dajemy kamerze 2 sekundy na rozruch

last_boxes = []
frame_count = 0

print("Kamera działa w tle. Startujemy AI...")

while True:
    ret, frame = cam.get_frame()
    if not ret:
        break

    frame = cv2.resize(frame, (640, 480))
    imout = frame.copy()
    frame_count += 1

    # AI uruchamia się tylko co 15 klatek obrazu
    if frame_count % 15 == 0:
        ss.setBaseImage(frame)
        ss.switchToSelectiveSearchFast()
        ssresults = ss.process()

        batch_images = []
        batch_boxes = []

        # Analizujemy tylko 30 najsilniejszych krawędzi (maksymalne przyspieszenie)
        for i, result in enumerate(ssresults):
            if i < 30:
                x, y, w, h = result
                timage = imout[y:y + h, x:x + w]

                if timage.shape[0] == 0 or timage.shape[1] == 0:
                    continue

                resized = cv2.resize(timage, (224, 224), interpolation=cv2.INTER_AREA)
                batch_images.append(resized)
                batch_boxes.append((x, y, w, h))

        if len(batch_images) > 0:
            batch_array = np.array(batch_images)
            out = model_final.predict(batch_array, batch_size=4, verbose=0)

            last_boxes = []
            for idx, prediction in enumerate(out):
                if prediction[0] > 0.85:
                    last_boxes.append(batch_boxes[idx])

    # Rysowanie zapamiętanych ramek na najświeższej klatce z kamery
    for (x, y, w, h) in last_boxes:
        cv2.rectangle(imout, (x, y), (x + w, y + h), (0, 255, 0), 2, cv2.LINE_AA)

    # Wyświetlanie powinno być teraz płynne (wideo nie będzie "zostawać w tyle")
    cv2.imshow("Kamera R-CNN", imout)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cam.stop()
cv2.destroyAllWindows()