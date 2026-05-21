import os
from ultralytics import YOLO
import blobconverter

model = YOLO(r"D:\Studia\cybAIR\Ambition\rocks_detection\Detection\runs\detect\depthai_model\yolo_rocks-9\weights\best.pt")

onnx_model_path = model.export(format="onnx", imgsz=416, opset=11, nms=False)
print(f"Model wyeksportowany do ONNX: {onnx_model_path}")

blob_path = blobconverter.from_onnx(
    model=onnx_model_path,
    data_type="FP16",
    shaves=6,
    version="2022.1",
    output_dir="depthai_model"
)

print(f"Sukces! Plik skompilowany do OAK-D Lite: {blob_path}")