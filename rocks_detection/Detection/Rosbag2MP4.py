import cv2
import numpy as np
from pathlib import Path
from rosbags.highlevel import AnyReader

if __name__ == '__main__':
    base_dir = Path(__file__).parent.resolve()
    mcap_path = base_dir / 'video' / 'camera_data_2026_04_24_10_32_52_0.mcap'
    output_video = str(base_dir / 'video' / 'extracted_video.mp4')

    max_frames = 1500
    frame_count = 0
    out = None

    target_topic = None
    target_size = None

    print(f"Otwieram plik: {mcap_path}")

    with AnyReader([mcap_path]) as reader:
        # Najpierw wypiszemy wszystkie strumienie wideo z pliku, żebyś wiedział co tam jest
        print("\nZnalezione kamery w pliku ROSBAG:")
        for topic, topic_info in reader.topics.items():
            if topic_info.msgtype in ['sensor_msgs/msg/Image', 'sensor_msgs/msg/CompressedImage']:
                print(f" - {topic}")
        print("-" * 40)

        for connection, timestamp, rawdata in reader.messages():
            if frame_count >= max_frames:
                break

            if connection.msgtype not in ['sensor_msgs/msg/Image', 'sensor_msgs/msg/CompressedImage']:
                continue

            # IGNORUJEMY kamery głębi, podczerwieni i inne techniczne strumienie
            topic_lower = connection.topic.lower()
            if 'depth' in topic_lower or 'infra' in topic_lower or 'stereo' in topic_lower:
                continue

            # Blokujemy się na pierwszej znalezionej, normalnej kamerze kolorowej
            if target_topic is None:
                target_topic = connection.topic
                print(f"\n[!] Wybrano kamerę: {target_topic}")

            if connection.topic != target_topic:
                continue

            img = None

            if connection.msgtype == 'sensor_msgs/msg/Image':
                msg = reader.deserialize(rawdata, connection.msgtype)
                encoding = getattr(msg, 'encoding', '').lower()

                # Zabezpieczenie: jeśli to format odległości, zignoruj
                if '16uc1' in encoding or '32fc1' in encoding:
                    continue

                try:
                    # Ręczne wymuszanie odpowiedniego formatu obrazu na podstawie Encodingu ROS
                    if encoding in ['rgb8', 'bgr8']:
                        img = np.frombuffer(msg.data, dtype=np.uint8).reshape((msg.height, msg.width, 3))
                        if encoding == 'rgb8':
                            img = cv2.cvtColor(img, cv2.COLOR_RGB2BGR)
                    elif encoding in ['yuv422', 'yuyv', 'yuyv422']:
                        img = np.frombuffer(msg.data, dtype=np.uint8).reshape((msg.height, msg.width, 2))
                        img = cv2.cvtColor(img, cv2.COLOR_YUV2BGR_YUYV)
                    elif encoding in ['mono8', '8uc1']:
                        img = np.frombuffer(msg.data, dtype=np.uint8).reshape((msg.height, msg.width, 1))
                        img = cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)
                    elif encoding in ['bgra8', 'rgba8']:
                        img = np.frombuffer(msg.data, dtype=np.uint8).reshape((msg.height, msg.width, 4))
                        if encoding == 'rgba8':
                            img = cv2.cvtColor(img, cv2.COLOR_RGBA2BGR)
                        else:
                            img = cv2.cvtColor(img, cv2.COLOR_BGRA2BGR)
                    else:
                        # Gdy format jest nietypowy, ale widać, że ma 3 kanały
                        img_raw = np.frombuffer(msg.data, dtype=np.uint8).reshape((msg.height, msg.width, -1))
                        if img_raw.shape[2] == 3:
                            img = img_raw
                        else:
                            continue
                except Exception as e:
                    print(f"Błąd dekodowania klatki: {e}")
                    continue

            elif connection.msgtype == 'sensor_msgs/msg/CompressedImage':
                msg = reader.deserialize(rawdata, connection.msgtype)
                np_arr = np.frombuffer(msg.data, np.uint8)
                img = cv2.imdecode(np_arr, cv2.IMREAD_COLOR)

            if img is not None:
                if out is None:
                    target_size = (img.shape[1], img.shape[0])
                    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
                    out = cv2.VideoWriter(output_video, fourcc, 30, target_size)
                    print(f"Rozpoczęto nagrywanie pliku MP4... (Rozdzielczość: {target_size[0]}x{target_size[1]})")

                if (img.shape[1], img.shape[0]) != target_size:
                    img = cv2.resize(img, target_size)

                out.write(img)
                frame_count += 1

    if out is not None:
        out.release()
        print(f"\nGotowe! Zapisano {frame_count} klatek do pliku {output_video}")
    else:
        print("\nNie udało się zgrać wideo. Prawdopodobnie w pliku nie ma zwykłej, kolorowej kamery.")