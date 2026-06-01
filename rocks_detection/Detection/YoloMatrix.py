import cv2
import os
from ultralytics import YOLO

if __name__ == '__main__':
    model_path = r"D:\Studia\cybAIR\Ambition\rocks_detection\Detection\runs\detect\depthai_model\yolo_rocks-5\weights\best.pt"
    data_yaml_path = r"D:\Studia\cybAIR\Ambition\rocks_detection\Detection\data.yaml"

    model = YOLO(model_path)

    metrics = model.val(data=data_yaml_path)

    print(f"Wyniki zapisano w katalogu: {metrics.save_dir}")

    matrix_path = os.path.join(metrics.save_dir, "confusion_matrix.png")

    if os.path.exists(matrix_path):
        img = cv2.imread(matrix_path)
        cv2.imshow("Macierz Bledow", img)
        cv2.waitKey(0)
        cv2.destroyAllWindows()
    else:
        print("Nie znaleziono pliku z macierza bledow.")