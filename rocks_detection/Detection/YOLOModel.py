import os
import cv2
from ultralytics import YOLO

if __name__ == '__main__':
    model = YOLO('yolov8n.pt')

    model.train(
        data=r'D:\Studia\cybAIR\Ambition\rocks_detection\Detection\data.yaml',
        epochs=50,
        imgsz=640,
        batch=4,
        workers=2,
        project='depthai_model',
        name='yolo_rocks'
    )

    best_model = YOLO('depthai_model/yolo_rocks/weights/best.pt')

    path = "./TestImages/"
    for filename in os.listdir(path):
        if filename.startswith("4"):
            img_path = os.path.join(path, filename)
            results = best_model.predict(source=img_path, conf=0.70)
            img = cv2.imread(img_path)
            for box in results[0].boxes:
                x1, y1, x2, y2 = map(int, box.xyxy[0])
                cv2.rectangle(img, (x1, y1), (x2, y2), (0, 255, 0), 2, cv2.LINE_AA)
            cv2.imshow("YOLO Detekcja", img)
            cv2.waitKey(0)
            break

    cv2.destroyAllWindows()