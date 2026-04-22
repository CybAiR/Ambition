import tensorflow as tf
import numpy as np

print(f"Wersja TensorFlow: {tf.__version__}")
print(f"Wersja NumPy: {np.__version__}")
print("---")

gpus = tf.config.list_physical_devices('GPU')
if gpus:
    print(f"SUKCES! Znaleziono GPU: {gpus}")
else:
    print("BŁĄD: Nadal nie widzę GPU. Sprawdź czy foldery bin z CUDA i cuDNN są w zmiennych środowiskowych PATH.")