import cv2
import os
from ultralytics import YOLO

if __name__ == "__main__":
    base_dir = os.path.dirname(os.path.abspath(__file__))

    model_path = os.path.join(base_dir, "runs", "detect", "depthai_model", "yolo_rocks_btr_opensource", "weights", "best.pt")
    input_video_path = os.path.join(base_dir, "video", "extracted_video.mp4")
    output_video_path = os.path.join(base_dir, "video", "extracted_video_detected.mp4")

    model = YOLO(model_path)

    cap = cv2.VideoCapture(input_video_path)

    if not cap.isOpened():
        print("BLAD: OpenCV nie moze znalezc lub otworzyc pliku wideo!")
        print("Szukana sciezka to:", input_video_path)
        exit()

    width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    fps = cap.get(cv2.CAP_PROP_FPS)

    if width == 0 or height == 0:
        print("BLAD: Wideo zostalo otwarte, ale ma rozdzielczosc 0x0.")
        exit()

    print(f"Wideo zaladowane poprawnie: {width}x{height} @ {fps} FPS")

    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    out = cv2.VideoWriter(output_video_path, fourcc, fps, (width, height))

    print("Rozpoczeto przetwarzanie wideo. Nacisnij 'q' aby przerwac.")

    while cap.isOpened():
        success, frame = cap.read()
        if not success:
            print("Koniec pliku wideo lub problem z pobraniem klatki.")
            break

        results = model.predict(source=frame, conf=0.5, stream=False, verbose=False)

        annotated_frame = results[0].plot()

        out.write(annotated_frame)

        downscaled_frame = cv2.resize(annotated_frame, (1280, 720))
        cv2.imshow("YOLOv8 - Detekcja Kamieni", downscaled_frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    out.release()
    cv2.destroyAllWindows()
    print("Przetwarzanie zakonczone. Zapisano plik wyjsciowy.")