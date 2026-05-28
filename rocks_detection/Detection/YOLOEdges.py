import cv2
import depthai as dai
import numpy as np
from ultralytics import YOLO

if __name__ == "__main__":
    model_path = r"D:\Studia\cybAIR\Ambition\rocks_detection\Detection\runs\detect\depthai_model\yolo_rocks-5\weights\best.pt"
    model = YOLO(model_path)

    pipeline = dai.Pipeline()

    cam = pipeline.create(dai.node.Camera).build(dai.CameraBoardSocket.CAM_A)
    videoQueue = cam.requestOutput((1920, 1080)).createOutputQueue()

    monoLeft = pipeline.create(dai.node.Camera).build(dai.CameraBoardSocket.CAM_B)
    monoRight = pipeline.create(dai.node.Camera).build(dai.CameraBoardSocket.CAM_C)
    stereo = pipeline.create(dai.node.StereoDepth)

    monoLeftOut = monoLeft.requestFullResolutionOutput()
    monoRightOut = monoRight.requestFullResolutionOutput()
    monoLeftOut.link(stereo.left)
    monoRightOut.link(stereo.right)

    stereo.setRectification(True)
    stereo.setExtendedDisparity(True)
    stereo.setLeftRightCheck(True)
    stereo.setDepthAlign(dai.CameraBoardSocket.CAM_A)
    stereo.setOutputSize(1920, 1080)

    depthQueue = stereo.depth.createOutputQueue()

    fx = 1300.0
    fy = 1300.0

    with pipeline:
        pipeline.start()

        print("YOLO26 i stereowizja gotowe. Nacisnij 'q' aby wyjsc.")

        while pipeline.isRunning():
            videoIn = videoQueue.get()
            depthIn = depthQueue.get()

            frame = videoIn.getCvFrame()
            latest_depth = depthIn.getFrame()

            results = model.predict(source=frame, conf=0.5, stream=False, verbose=False)
            annotated_frame = results[0].plot()

            for box in results[0].boxes:
                x1, y1, x2, y2 = map(int, box.xyxy[0])

                x1_safe = max(0, x1)
                y1_safe = max(0, y1)
                x2_safe = min(frame.shape[1], x2)
                y2_safe = min(frame.shape[0], y2)

                roi_depth = latest_depth[y1_safe:y2_safe, x1_safe:x2_safe]
                valid_depths = roi_depth[roi_depth > 0]

                if len(valid_depths) > 0:
                    z_mm = np.median(valid_depths)

                    w_px = x2 - x1
                    h_px = y2 - y1

                    width_cm = ((w_px * z_mm) / fx) / 10.0
                    height_cm = ((h_px * z_mm) / fy) / 10.0
                    z_cm = z_mm / 10.0

                    text = f"W:{width_cm:.1f}cm H:{height_cm:.1f}cm Z:{z_cm:.1f}cm"
                    cv2.putText(annotated_frame, text, (x1, y2 + 25), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 255), 2)

            cv2.imshow("YOLO26 - Detekcja Kamieni", annotated_frame)

            if cv2.waitKey(1) & 0xFF == ord('q'):
                pipeline.stop()
                break

        cv2.destroyAllWindows()