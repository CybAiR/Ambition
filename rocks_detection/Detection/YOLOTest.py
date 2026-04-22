import cv2
import threading
import time
from ultralytics import YOLO

class VideoCamera:
    def __init__(self, src=0):
        self.cap = cv2.VideoCapture(src)
        self.ret, self.frame = self.cap.read()
        self.running = True
        self.thread = threading.Thread(target=self.update, daemon=True)
        self.thread.start()

    def update(self):
        while self.running:
            self.ret, self.frame = self.cap.read()

    def get_frame(self):
        return self.ret, self.frame

    def stop(self):
        self.running = False
        self.thread.join()
        self.cap.release()

if __name__ == "__main__":
    model_path = r"D:\Studia\cybAIR\Ambition\rocks_detection\Detection\runs\detect\depthai_model\yolo_rocks-9\weights\best.pt"
    model = YOLO(model_path)

    cam = VideoCamera(0)
    time.sleep(2)

    print("YOLO26 gotowe. Nacisnij 'q' aby wyjsc.")

    while True:
        ret, frame = cam.get_frame()
        if not ret:
            break

        results = model.predict(source=frame, conf=0.5, stream=False, verbose=False)

        annotated_frame = results[0].plot()

        cv2.imshow("YOLO26 - Detekcja Kamieni", annotated_frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cam.stop()
    cv2.destroyAllWindows()