import rclpy
from rclpy.node import Node
from std_msgs.msg import Float64MultiArray
import sys, tty, termios

class KeyboardControl(Node):
    def __init__(self):
        super().__init__('keyboard_control')
        self.publisher = self.create_publisher(
            Float64MultiArray,
            '/pan_position_controller/commands',
            10
        )
        self.pan = 1.57   # 90°
        self.tilt = 1.57
        self.step = 0.05  # ~3°
        self.get_logger().info("W/S: tilt, A/D: pan, Q: quit")

    def getch(self):
        fd = sys.stdin.fileno()
        old = termios.tcgetattr(fd)
        try:
            tty.setraw(fd)
            return sys.stdin.read(1)
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old)

    def run(self):
        while rclpy.ok():
            key = self.getch()
            if key == 'w':
                self.tilt = min(3.14, self.tilt + self.step)
            elif key == 's':
                self.tilt = max(0.0, self.tilt - self.step)
            elif key == 'a':
                self.pan = max(0.0, self.pan - self.step)
            elif key == 'd':
                self.pan = min(3.14, self.pan + self.step)
            elif key == 'q':
                break

            msg = Float64MultiArray()
            msg.data = [self.pan, self.tilt]
            self.publisher.publish(msg)
            print(f"\r Pan:{self.pan*180.0/3.14:.1f}° Tilt:{self.tilt*180.0/3.14:.1f}°", end='')
        print("\nExiting")

def main(args=None):
    rclpy.init(args=args)
    node = KeyboardControl()
    try:
        node.run()
    except KeyboardInterrupt:
        pass
    node.destroy_node()
    rclpy.shutdown()