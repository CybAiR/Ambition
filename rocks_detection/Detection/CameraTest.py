import cv2
import depthai as dai

pipeline = dai.Pipeline()

cam = pipeline.create(dai.node.Camera).build()
videoQueue = cam.requestOutput((1920, 1080)).createOutputQueue()

pipeline.start()

while pipeline.isRunning():
    videoIn = videoQueue.get()
    cv2.imshow("OAK-D Lite", videoIn.getCvFrame())

    if cv2.waitKey(1) == ord('q'):
        break