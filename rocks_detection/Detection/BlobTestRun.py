import cv2
import numpy as np
import depthai as dai

pipeline = dai.Pipeline()

cam = pipeline.create(dai.node.Camera).build()
cam_out = cam.requestOutput((416, 416), type=dai.ImgFrame.Type.BGR888p)

nn = pipeline.create(dai.node.NeuralNetwork)
nn.setBlobPath("depthai_model/best1_openvino_2022.1_6shave.blob")

cam_out.link(nn.input)

qRgb = cam_out.createOutputQueue(maxSize=4, blocking=False)
qDet = nn.out.createOutputQueue(maxSize=4, blocking=False)

pipeline.start()

while pipeline.isRunning():
    inRgb = qRgb.get()
    inDet = qDet.get()

    frame = inRgb.getCvFrame()

    if inDet.getAllLayerNames():
        out_tensor = inDet.getFirstTensor().reshape(10, 3549)
        predictions = out_tensor.T

        boxes = []
        confidences = []
        class_ids = []

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
                class_ids.append(class_id)

        indices = cv2.dnn.NMSBoxes(boxes, confidences, score_threshold=0.5, nms_threshold=0.5)

        for i in indices:
            idx = i[0] if isinstance(i, (list, np.ndarray)) else i
            x, y, w, h = boxes[idx][0], boxes[idx][1], boxes[idx][2], boxes[idx][3]

            cv2.rectangle(frame, (x, y), (x + w, y + h), (255, 0, 0), 2)
            cv2.putText(frame, f"Klasa {class_ids[idx]}: {confidences[idx]:.2f}",
                        (x, y - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 0, 0), 2)

    cv2.imshow("Kamera OAK-D Lite", frame)

    if cv2.waitKey(1) == ord('q'):
        break