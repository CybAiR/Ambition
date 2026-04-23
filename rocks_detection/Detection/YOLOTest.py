import cv2
import depthai as dai
import numpy as np
from ultralytics import YOLO

if __name__ == "__main__":
    model_path = r"D:\Studia\cybAIR\Ambition\rocks_detection\Detection\runs\detect\depthai_model\yolo_rocks-9\weights\best.pt"
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

    disparityQueue = stereo.disparity.createOutputQueue()

    colorMap = cv2.applyColorMap(np.arange(256, dtype=np.uint8), cv2.COLORMAP_JET)
    colorMap[0] = [0, 0, 0]

    with pipeline:
        pipeline.start()

        print("YOLO26 i stereowizja gotowe. Nacisnij 'q' aby wyjsc.")
        maxDisparity = 1

        while pipeline.isRunning():
            videoIn = videoQueue.tryGet()
            if videoIn is not None:
                frame = videoIn.getCvFrame()
                results = model.predict(source=frame, conf=0.5, stream=False, verbose=False)
                annotated_frame = results[0].plot()
                cv2.imshow("YOLO26 - Detekcja Kamieni", annotated_frame)

            disparity = disparityQueue.tryGet()
            if disparity is not None:
                npDisparity = disparity.getFrame()
                if np.max(npDisparity) > 0:
                    maxDisparity = max(maxDisparity, np.max(npDisparity))

                colorizedDisparity = cv2.applyColorMap(((npDisparity / maxDisparity) * 255).astype(np.uint8), colorMap)
                cv2.imshow("Stereowizja - Disparity", colorizedDisparity)

            if cv2.waitKey(1) & 0xFF == ord('q'):
                pipeline.stop()
                break

        cv2.destroyAllWindows()