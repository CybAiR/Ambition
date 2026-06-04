import rclpy
from rclpy.node import Node

from std_msgs.msg import String
from sensor_msgs.msg import Image
from cv_bridge import CvBridge
import numpy as np

from ros_object_detection_msgs.msg import BoundingBox, BoundingBoxes


# debug:
#import matplotlib
#matplotlib.use('Agg')
#import matplotlib.pyplot as plt
# 

class MinimalSubscriber(Node):

    def __init__(self):
        super().__init__('minimal_subscriber')

        #Parametry
        self.image = None
        self.bridge = CvBridge()
        self.boxes = None
        self.image_height = 400
        self.image_width = 640

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
        self.pub = self.create_publisher(
            String,
            'rocks',
            10
        )
        self.subscription  # prevent unused variable warning
        self.sub_depth

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
            self.get_logger().info(f'point1: {point1} point2: {point2}')
            # Extacting points and applaing Median filter
            xmin_pix = int(box.xmin*self.image_width)
            xmax_pix = int(box.xmax*self.image_width)
            ymin_pix = int(box.ymin*self.image_height)
            ymax_pix = int(box.ymax*self.image_height)
            points = self.image[ymin_pix:ymax_pix, xmin_pix:xmax_pix]
            # filtering 0 values from depth image
            self.get_logger().info(f'W ROI jest {points.size} punktów, przed odżuceniem 0')
            points = points[points != 0]
            self.get_logger().info(f'W ROI jest {points.size} punktów, minimalna wartość: {points.min()}, maksymalna wartość: {points.max()}')
            
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
            self.get_logger().info(f'odległość Z: {z} w punkcie: {center}')
        msg = String()
        #msg.data = f'Wymiar obrazka: {self.image.shape}'
        #self.pub.publish(msg)

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