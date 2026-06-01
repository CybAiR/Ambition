import cv2
import numpy as np
import depthai as dai

pipeline = dai.Pipeline()

cam = pipeline.create(dai.node.Camera).build(dai.CameraBoardSocket.CAM_A)
cam_out = cam.requestOutput((416, 416), type=dai.ImgFrame.Type.BGR888p)

monoLeft = pipeline.create(dai.node.Camera).build(dai.CameraBoardSocket.CAM_B)
monoRight = pipeline.create(dai.node.Camera).build(dai.CameraBoardSocket.CAM_C)

monoLeftOut = monoLeft.requestOutput((640, 400), type=dai.ImgFrame.Type.GRAY8)
monoRightOut = monoRight.requestOutput((640, 400), type=dai.ImgFrame.Type.GRAY8)

stereo = pipeline.create(dai.node.StereoDepth)
stereo.setDefaultProfilePreset(dai.node.StereoDepth.PresetMode.DEFAULT)
stereo.initialConfig.setMedianFilter(dai.MedianFilter.MEDIAN_OFF)
stereo.setDepthAlign(dai.CameraBoardSocket.CAM_A)
stereo.setOutputSize(416, 416)

monoLeftOut.link(stereo.left)
monoRightOut.link(stereo.right)

nn = pipeline.create(dai.node.NeuralNetwork)
nn.setBlobPath("depthai_model/best_openvino_2022.1_6shave.blob")
cam_out.link(nn.input)

qRgb = cam_out.createOutputQueue(maxSize=4, blocking=False)
qDet = nn.out.createOutputQueue(maxSize=4, blocking=False)
qDepth = stereo.depth.createOutputQueue(maxSize=4, blocking=False)

pipeline.start()

device = pipeline.getDefaultDevice()

calibData = device.readCalibration()
intrinsics = calibData.getCameraIntrinsics(dai.CameraBoardSocket.CAM_A, 416, 416)
fx = intrinsics[0][0]
fy = intrinsics[1][1]

while pipeline.isRunning():
    inRgb = qRgb.get()
    inDet = qDet.get()
    inDepth = qDepth.get()

    frame = inRgb.getCvFrame()
    depthFrame = inDepth.getFrame()

    if inDet.getAllLayerNames():
        out_tensor = inDet.getFirstTensor().reshape(5, 3549)
        predictions = out_tensor.T

        boxes = []
        confidences = []

        for row in predictions:
            scores = row[4:]
            class_id = np.argmax(scores)
            confidence = scores[class_id]

            if confidence > 0.5:
                x_center, y_center, w, h = row[0:4]

                x_min = int(x_center - (w / 2))
                y_min = int(y_center - (h / 2))

                boxes.append([x_min, y_min, int(w), int(h)])
                confidences.append(float(confidence))

        indices = cv2.dnn.NMSBoxes(boxes, confidences, score_threshold=0.5, nms_threshold=0.5)

        for i in indices:
            idx = i[0] if isinstance(i, (list, np.ndarray)) else i
            x, y, w, h = boxes[idx]

            x_c = int(x + w / 2)
            y_c = int(y + h / 2)
            roi_size = 10

            x1 = max(0, x_c - roi_size)
            y1 = max(0, y_c - roi_size)
            x2 = min(depthFrame.shape[1], x_c + roi_size)
            y2 = min(depthFrame.shape[0], y_c + roi_size)

            depth_roi = depthFrame[y1:y2, x1:x2]
            depth_roi = depth_roi[depth_roi > 0]

            if len(depth_roi) > 0:
                z_mm = np.median(depth_roi)

                real_width_mm = (w * z_mm) / fx
                real_height_mm = (h * z_mm) / fy

                cv2.putText(frame, f"X: {real_width_mm:.0f} mm", (x, y - 25), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)
                cv2.putText(frame, f"Y: {real_height_mm:.0f} mm", (x, y - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)
                cv2.putText(frame, f"Z: {int(z_mm)} mm", (x, y + h + 40), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)

            cv2.rectangle(frame, (x, y), (x + w, y + h), (255, 0, 0), 2)
            cv2.putText(frame, f"Rock: {confidences[idx]:.2f}", (x, y + h + 20), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 0, 0), 2)

    cv2.imshow("Color frame", frame)

    if cv2.waitKey(1) == ord('q'):
        break