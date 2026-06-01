import cv2
import depthai as dai
from ultralytics import YOLO
import numpy as np

## Creating Pipeline and and connecting to Dethai camera interface ##
#pipeline = dai.Pipeline()
#cam = pipeline.create(dai.node.Camera).build()
#videoQueue = cam.requestOutput((1920, 1080)).createOutputQueue()
#pipeline.start()

## Starting Stereo ##
stereo_pipeline = dai.Pipeline()
monoLeft = stereo_pipeline.create(dai.node.Camera).build(dai.CameraBoardSocket.CAM_B)
monoRight = stereo_pipeline.create(dai.node.Camera).build(dai.CameraBoardSocket.CAM_C)
stereo = stereo_pipeline.create(dai.node.StereoDepth)
# Linking
monoLeftOut = monoLeft.requestFullResolutionOutput()
monoRightOut = monoRight.requestFullResolutionOutput()
monoLeftOut.link(stereo.left)
monoRightOut.link(stereo.right)
#Some improvements for stereo
stereo.setRectification(True)
stereo.setExtendedDisparity(True)
stereo.setLeftRightCheck(True)

disparityQueue = stereo.disparity.createOutputQueue()
#videoQueue = cam.requestOutput((1920, 1080)).createOutputQueue()

cam = stereo_pipeline.create(dai.node.Camera).build()
videoQueue = cam.requestOutput((1920, 1080)).createOutputQueue()
stereo_pipeline.start()

colorMap = cv2.applyColorMap(np.arange(256, dtype=np.uint8), cv2.COLORMAP_JET)
colorMap[0] = [0, 0, 0]  # to make zero-disparity pixels black
maxDisparity = 1

## Loading YOLO model ##
model = YOLO(r"models\best2.pt")

while stereo_pipeline.isRunning():
    videoIn = videoQueue.get()
    # extracting frame from camera pipeline
    frame = videoIn.getCvFrame()
    # applay YOLO model
    results = model(frame, verbose=False)
    # draw annotations
    annotated_frame = results[0].plot()
    # Show results

    # disparity
    disparity = disparityQueue.get()
    npDisparity = disparity.getFrame()
    maxDisparity = max(maxDisparity, np.max(npDisparity))
    colorizedDisparity = cv2.applyColorMap(((npDisparity / maxDisparity) * 255).astype(np.uint8), colorMap)

    #print(results[0].boxes.xyxy)
    #print(disparity.getTransformation())
    for box in results[0].boxes:
        #print("confidence: ", box.conf)
        if box.conf > 0.6:
            x1, y1, x2, y2 = map(int, box.xyxy[0])
            point1 = dai.Point2f(x1,y1)
            point2 = dai.Point2f(x2,y2)
            center = dai.Point2f((x1+x2)/2,(y1+y2)/2)
            #print(center)
            remap_center = videoIn.getTransformation().remapPointTo(disparity.getTransformation(),center)
            remap_point1 = videoIn.getTransformation().remapPointTo(disparity.getTransformation(),point1)
            remap_point2 = videoIn.getTransformation().remapPointTo(disparity.getTransformation(),point2)
            #print(remap_point.x,remap_point.y)
            cv2.circle(colorizedDisparity, (int(remap_center.x), int(remap_center.y)), 5, (0, 100, 255), -1)
            cv2.circle(colorizedDisparity, (int(remap_point1.x), int(remap_point1.y)), 5, (255, 0, 0), -1)
            cv2.circle(colorizedDisparity, (int(remap_point2.x), int(remap_point2.y)), 5, (0, 255, 0), -1)


    cv2.imshow("OAK-D Lite", annotated_frame)
    cv2.imshow("disparity", colorizedDisparity)

# działa to do góry, teraz by trzeba zmapować punkt na stereo i może narysować żeby zobaczyć czy działa


    if cv2.waitKey(1) == ord('q'):
        break