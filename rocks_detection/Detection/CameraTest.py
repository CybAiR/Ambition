import cv2
import depthai as dai
import os

pipeline = dai.Pipeline()

cam = pipeline.create(dai.node.Camera).build()
videoQueue = cam.requestOutput((3840, 2160)).createOutputQueue()

save_dir = r"D:\Studia\cybAIR\Ambition\rocks_detection\Detection\img\6"
os.makedirs(save_dir, exist_ok=True)

img_counter = 0

pipeline.start()

while pipeline.isRunning():
    videoIn = videoQueue.get()
    frame = videoIn.getCvFrame()

    frame = cv2.rotate(frame, cv2.ROTATE_180)

    frame_fhd = cv2.resize(frame, (1920, 1080))
    cv2.imshow("OAK-D Lite", frame_fhd)

    key = cv2.waitKey(1)

    if key == ord('q'):
        break
    elif key == ord('z'):
        img_name = os.path.join(save_dir, f"zdjecie_{img_counter}.png")
        cv2.imwrite(img_name, frame)
        img_counter += 1