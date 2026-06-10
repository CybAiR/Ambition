import rclpy
from rclpy.node import Node

from std_msgs.msg import String
from sensor_msgs.msg import Image
from sensor_msgs.msg import CameraInfo
from cv_bridge import CvBridge
import numpy as np

from ros_object_detection_msgs.msg import BoundingBox, BoundingBoxes


# debug:
#import matplotlib
#matplotlib.use('Agg')
#import matplotlib.pyplot as plt
# 
from dataclasses import dataclass


@dataclass
class Point:
    x: float
    y: float
    z: float

class MinimalSubscriber(Node):

    def __init__(self):
        super().__init__('minimal_subscriber')

        #Parametry
        self.image = None
        self.bridge = CvBridge()
        self.boxes = None
        self.image_height = 400
        self.image_width = 640
        self.focal_lengthX = None
        self.focal_lengthY = None
        self.cx = None
        self.cy = None

        self.subscription = self.create_subscription(
            BoundingBoxes,
            'bounding_box',
            self.box_callback,
            10)
        self.sub_depth = self.create_subscription(
            Image,
            '/oak/stereo/image_raw',
            self.depth_callback,
            10
        )
        self.info_sub = self.create_subscription(
            CameraInfo,
            'oak/stereo/camera_info',
            self.camera_info_callback,
            10
        )
        self.pub = self.create_publisher(
            String,
            'rocks',
            10
        )
        self.subscription  # prevent unused variable warning
        self.sub_depth
    def camera_info_callback(self,msg):
        if self.focal_lengthX is None:
            self.get_logger().warning(f'Camera info: {msg.k}')
            self.focal_lengthX = msg.k[0]
            self.focal_lengthY = msg.k[4]
            self.cx = msg.k[2]
            self.cy = msg.k[5]
        else:
            pass
    def listener_callback(self, msg):
        self.get_logger().info('I heard: "%s"' % msg.data)
    def depth_callback(self, msg):
        self.image = self.bridge.imgmsg_to_cv2(msg,desired_encoding='passthrough')
    def box_callback(self, msg):
        self.get_logger().info('dostałem Bounding Boxy')
        if self.image is None:
            self.get_logger().warning('Czekam na pierwszy obraz z kamery...')
            return
        self.boxes = msg.data
        for box in self.boxes:
            point1 = np.array([box.xmin, box.ymin])
            point2 = np.array([box.xmax, box.ymax])
            center = (point1 + point2)/2
            #self.get_logger().info(f'point1: {point1} point2: {point2}')
            # Extacting points and applaing Median filter
            xmin_pix = int(box.xmin*self.image_width)
            xmax_pix = int(box.xmax*self.image_width)
            ymin_pix = int(box.ymin*self.image_height)
            ymax_pix = int(box.ymax*self.image_height)
            points = self.image[ymin_pix:ymax_pix, xmin_pix:xmax_pix]
            # filtering 0 values from depth image
            #self.get_logger().info(f'W ROI jest {points.size} punktów, przed odżuceniem 0')
            points = points[points != 0]
            if points.size == 0:
                self.get_logger().warning('Brak poprawnych danych głębi (same zera) dla tego kamienia. Pomijam.')
                continue 
            #self.get_logger().info(f'W ROI jest {points.size} punktów, minimalna wartość: {points.min()}, maksymalna wartość: {points.max()}')
            
            # Wieving histogram of points in ROI:
            #points_to_plot = points.flatten()
            #plt.hist(points_to_plot,bins='auto')
            #plt.title("Points in ROI")
            #plt.savefig('histogram_odleglosci.png')
            #plt.clf()
            #self.get_logger().info('Zapisano wykres do pliku: histogram_odleglosci.png')
            ###

            # Calculating Median from points in ROI
            z = np.median(points)
            #self.get_logger().info(f'odległość Z: {z} w punkcie: {center}')

            # Calculating X,Y,Z coordinates
            u = center[0]*self.image_width
            v = center[1]*self.image_height
            
            cords = Point
            cords.x = ((u-self.cy)*z)/self.focal_lengthX
            cords.y = ((v-self.cx)*z)/self.focal_lengthY
            cords.z = float(z)
            #self.get_logger().info(f'parametr cy: {self.cy} parametr cx: {self.cx}')

            # Calculating rock size
            du = xmax_pix - xmin_pix
            dv = ymax_pix - ymin_pix
            dx = (du * z)/self.focal_lengthX 
            dy = (dv * z)/self.focal_lengthY


            # Publishing results:
            msg = String()
            size = [dx,dy]
            msg.data = f'Kamień o wielkości: [{(size[0]*0.1):.2f}, {(size[1]*0.1):.2f}]cm znajduje się na X: {(cords.x*0.001):.2f}m Y: {(cords.y*0.001):.2f}m Z: {(cords.z*0.001):.2f}m '
            self.pub.publish(msg)

def main(args=None):
    rclpy.init(args=args)

    minimal_subscriber = MinimalSubscriber()

    rclpy.spin(minimal_subscriber)

    # Destroy the node explicitly
    # (optional - otherwise it will be done automatically
    # when the garbage collector destroys the node object)
    minimal_subscriber.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()