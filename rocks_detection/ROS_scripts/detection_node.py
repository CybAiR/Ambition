#!/usr/bin/env python3

from __future__ import annotations

import threading
from typing import List, Optional

import cv2
from cv_bridge import CvBridge
import rclpy
from rclpy.node import Node
from rclpy.qos import qos_profile_sensor_data
from sensor_msgs.msg import Image

from ros_object_detection_msgs.msg import BoundingBox, BoundingBoxes

try:
    from ultralytics import YOLO
except ImportError as exc:
    raise RuntimeError(
        "Biblioteka 'ultralytics' jest wymagana dla modelu YOLOv8. "
        "Zainstaluj ją używając: pip install ultralytics"
    ) from exc


class YoloV8DetectionNode(Node):
    def __init__(self) -> None:
        super().__init__('yolov8_object_detection')

        # Parametry ROS 2
        self.declare_parameter('model', 'best.pt')  # Ścieżka do Twojego modelu .pt lub .onnx
        self.declare_parameter('device', '')           # '' to auto (CUDA jeśli dostępne, inaczej CPU), 'cpu' lub '0'
        self.declare_parameter('publish_rate_hz', 100.0)
        self.declare_parameter('score_thresh', 0.6)

        self.bridge = CvBridge()
        self.latest_image: Optional[Image] = None
        self.image_lock = threading.Lock()
        
        # Inicjalizacja parametrów
        self.min_score = float(self.get_parameter('score_thresh').value)
        model_path = self.get_parameter('model').value
        device_param = self.get_parameter('device').value

        # Ładowanie modelu YOLOv8
        self.get_logger().info(f"Ładowanie modelu YOLO z: {model_path}")
        self.model = YOLO(model_path)
        self.device = device_param if device_param else None

        # Publikatory i subskrybenty
        self.image_pub = self.create_publisher(Image, 'image_annotated', 1)
        self.box_pub = self.create_publisher(BoundingBoxes, 'bounding_box', 1)
        
        self.sub = self.create_subscription(
            Image,
            '/oak/rgb/image_raw',
            self.get_image,
            qos_profile=qos_profile_sensor_data,
        )

        # Timer kontrolujący częstotliwość inferencji (zapobiega kolejkowaniu)
        publish_rate_hz = float(self.get_parameter('publish_rate_hz').value)
        publish_period = 1.0 / max(publish_rate_hz, 1.0)
        self.timer = self.create_timer(publish_period, self.detect)

        self.get_logger().info("Węzeł YOLOv8 gotowy do pracy.")

    def get_image(self, data: Image) -> None:
        """Odbiera nową klatkę obrazu i zapisuje ją w buforze."""
        with self.image_lock:
            self.latest_image = data

    def detect(self) -> None:
        """Pobiera klatkę z bufora, uruchamia inferencję i publikuje wyniki."""
        with self.image_lock:
            image_msg = self.latest_image
            self.latest_image = None

        if image_msg is None:
            return

        # Konwersja obrazu ROS na OpenCV
        cv_image = self.bridge.imgmsg_to_cv2(image_msg, desired_encoding='bgr8')
        height, width = cv_image.shape[:2]

        # Inferencja YOLOv8
        # verbose=False wyłącza spamowanie logami w terminalu dla każdej klatki
        results = self.model.predict(
            source=cv_image, 
            conf=self.min_score, 
            device=self.device, 
            verbose=False
        )
        
        result = results[0]  # Pobieramy wynik dla pierwszego (jedynego) obrazka

        # Generowanie obrazu z naniesionymi ramkami (wbudowana funkcja YOLO)
        annotated_frame = result.plot()

        # Konwersja z powrotem do formatu ROS i publikacja
        annotated_msg = self.bridge.cv2_to_imgmsg(annotated_frame, encoding='bgr8')
        annotated_msg.header = image_msg.header
        self.image_pub.publish(annotated_msg)

        # Publikacja BoundingBoxes na dedykowany topic
        boxes_msg = BoundingBoxes()
        boxes_msg.data = self._extract_bounding_boxes(result)
        self.box_pub.publish(boxes_msg)

    def _extract_bounding_boxes(self, result) -> List[BoundingBox]:
        """Konwertuje obiekty detekcji YOLO na niestandardowe wiadomości ROS."""
        ros_boxes = []
        
        # boxes.xyxyn zwraca znormalizowane współrzędne (0.0 do 1.0)
        # boxes.conf zwraca pewność
        # boxes.cls zwraca index klasy
        for box, conf, cls in zip(result.boxes.xyxyn, result.boxes.conf, result.boxes.cls):
            xmin, ymin, xmax, ymax = box.tolist()
            confidence = float(conf)
            class_id = int(cls)
            label = result.names[class_id]  # Pobranie nazwy z wbudowanej mapy YOLO

            box_msg = BoundingBox()
            box_msg.data = label
            box_msg.confidence = confidence
            
            # Bezpieczne przycięcie wartości między 0.0 a 1.0 (zgodnie z poprzednim kodem)
            box_msg.xmin = max(0.0, min(1.0, float(xmin)))
            box_msg.ymin = max(0.0, min(1.0, float(ymin)))
            box_msg.xmax = max(0.0, min(1.0, float(xmax)))
            box_msg.ymax = max(0.0, min(1.0, float(ymax)))
            
            ros_boxes.append(box_msg)

        return ros_boxes


def main(args: Optional[List[str]] = None) -> None:
    rclpy.init(args=args)
    node = YoloV8DetectionNode()

    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()

if __name__ == '__main__':
    main()