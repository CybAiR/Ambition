import cv2
import depthai as dai
from ultralytics import YOLO

if __name__ == "__main__":
    model_path = r"D:\Studia\cybAIR\Ambition\rocks_detection\Detection\runs\detect\depthai_model\yolo_rocks-9\weights\best.pt"
    model = YOLO(model_path)

    pipeline = dai.Pipeline()

    cam = pipeline.create(dai.node.Camera).build()
    videoQueue = cam.requestOutput((1920, 1080)).createOutputQueue()

    pipeline.start()

    print("YOLO26 gotowe. Nacisnij 'q' aby wyjsc.")

    while pipeline.isRunning():
        videoIn = videoQueue.get()
        frame = videoIn.getCvFrame()

        results = model.predict(source=frame, conf=0.5, stream=False, verbose=False)

        annotated_frame = results[0].plot()

        cv2.imshow("YOLO26 - Detekcja Kamieni", annotated_frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cv2.destroyAllWindows()