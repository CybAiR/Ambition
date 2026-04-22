import os, cv2
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import tensorflow as tf
from tensorflow.keras.layers import Dense
from tensorflow.keras import Model
from tensorflow.keras import optimizers
from tensorflow.keras.preprocessing.image import ImageDataGenerator
from tensorflow.keras.optimizers import Adam
from tensorflow.keras.applications.vgg16 import VGG16
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import LabelBinarizer
from tensorflow.keras.callbacks import ModelCheckpoint, EarlyStopping

path = "./TestImages/"
annot = "./TestAirplanes_Annotations/"

vggmodel = VGG16(weights='imagenet', include_top=True)
ss = cv2.ximgproc.segmentation.createSelectiveSearchSegmentation()

def get_iou(bb1, bb2):
    assert bb1['x1'] < bb1['x2']
    assert bb1['y1'] < bb1['y2']
    assert bb2['x1'] < bb2['x2']
    assert bb2['y1'] < bb2['y2']
    x_left = max(bb1['x1'], bb2['x1'])
    y_top = max(bb1['y1'], bb2['y1'])
    x_right = min(bb1['x2'], bb2['x2'])
    y_bottom = min(bb1['y2'], bb2['y2'])
    if x_right < x_left or y_bottom < y_top:
        return 0.0
    intersection_area = (x_right - x_left) * (y_bottom - y_top)
    bb1_area = (bb1['x2'] - bb1['x1']) * (bb1['y2'] - bb1['y1'])
    bb2_area = (bb2['x2'] - bb2['x1']) * (bb2['y2'] - bb2['y1'])
    iou = intersection_area / float(bb1_area + bb2_area - intersection_area)
    assert iou >= 0.0
    assert iou <= 1.0
    return iou

train_images=[]
train_labels=[]

for e, i in enumerate(os.listdir(annot)):
    try:
        if i.startswith("airplane") and i.endswith(".csv"):
            filename = i.split(".")[0] + ".jpg"
            image_path = os.path.join(path, filename)

            if not os.path.exists(image_path):
                continue

            image = cv2.imread(image_path)
            df = pd.read_csv(os.path.join(annot, i), header=None)
            gtvalues = []

            for index, row in df.iterrows():
                line = str(row.iloc[0])
                parts = line.split(" ")
                if len(parts) == 4:
                    x1, y1, x2, y2 = map(int, parts)
                    gtvalues.append({"x1": x1, "x2": x2, "y1": y1, "y2": y2})

            if not gtvalues:
                continue

            ss.setBaseImage(image)
            ss.switchToSelectiveSearchFast()
            ssresults = ss.process()

            counter = 0
            falsecounter = 0
            flag = 0

            print(f"Przetwarzam obraz {e}: {filename} (znaleziono {len(ssresults)} propozycji)")

            for j, result in enumerate(ssresults):
                if j < 2000 and flag == 0:
                    x, y, w, h = result
                    iou = get_iou(gtvalues[0], {"x1": x, "x2": x + w, "y1": y, "y2": y + h})

                    for gtval in gtvalues:
                        iou = max(iou, get_iou(gtval, {"x1": x, "x2": x + w, "y1": y, "y2": y + h}))

                    if iou > 0.70 and counter < 30:
                        timage = image[y:y + h, x:x + w]
                        resized = cv2.resize(timage, (224, 224), interpolation=cv2.INTER_AREA)
                        train_images.append(resized)
                        train_labels.append(1)
                        counter += 1
                    elif iou < 0.30 and falsecounter < 30:
                        timage = image[y:y + h, x:x + w]
                        resized = cv2.resize(timage, (224, 224), interpolation=cv2.INTER_AREA)
                        train_images.append(resized)
                        train_labels.append(0)
                        falsecounter += 1

                    if counter >= 30 and falsecounter >= 30:
                        flag = 1
    except Exception as ex:
        print(f"Blad w pliku {i}: {ex}")
        continue

X_new = np.array(train_images)
y_new = np.array(train_labels)

for layers in (vggmodel.layers)[:15]:
    layers.trainable = False

X = vggmodel.layers[-2].output
predictions = Dense(2, activation="softmax")(X)
model_final = Model(inputs=vggmodel.input, outputs=predictions)
opt = Adam(learning_rate=0.0001)
model_final.compile(loss=tf.keras.losses.categorical_crossentropy, optimizer=opt, metrics=["accuracy"])

class MyLabelBinarizer(LabelBinarizer):
    def transform(self, y):
        Y = super().transform(y)
        if self.y_type_ == 'binary':
            return np.hstack((Y, 1 - Y))
        else:
            return Y

    def inverse_transform(self, Y, threshold=None):
        if self.y_type_ == 'binary':
            return super().inverse_transform(Y[:, 0], threshold)
        else:
            return super().inverse_transform(Y, threshold)

lenc = MyLabelBinarizer()
Y = lenc.fit_transform(y_new)
X_train, X_test, y_train, y_test = train_test_split(X_new, Y, test_size=0.10)

trdata = ImageDataGenerator(horizontal_flip=True, vertical_flip=True, rotation_range=90)
traindata = trdata.flow(x=X_train, y=y_train, batch_size=4)
tsdata = ImageDataGenerator(horizontal_flip=True, vertical_flip=True, rotation_range=90)
testdata = tsdata.flow(x=X_test, y=y_test, batch_size=4)

checkpoint = ModelCheckpoint("ieeercnn_vgg16_1.h5", monitor='val_loss', verbose=1, save_best_only=True, save_weights_only=False, mode='auto', save_freq='epoch')
early = EarlyStopping(monitor='val_loss', min_delta=0, patience=100, verbose=1, mode='auto')

hist = model_final.fit(traindata, steps_per_epoch=10, epochs=1000, validation_data=testdata, validation_steps=2, callbacks=[checkpoint, early])

z = 0
for e, i in enumerate(os.listdir(path)):
    if i.startswith("4"):
        z += 1
        img = cv2.imread(os.path.join(path, i))
        ss.setBaseImage(img)
        ss.switchToSelectiveSearchFast()
        ssresults = ss.process()
        imout = img.copy()
        for e, result in enumerate(ssresults):
            if e < 2000:
                x, y, w, h = result
                timage = imout[y:y + h, x:x + w]
                resized = cv2.resize(timage, (224, 224), interpolation=cv2.INTER_AREA)
                img_array = np.expand_dims(resized, axis=0)
                out = model_final(img_array, training=False)
                if out[0][0] > 0.70:
                    cv2.rectangle(imout, (x, y), (x + w, y + h), (0, 255, 0), 1, cv2.LINE_AA)
        plt.figure()
        plt.imshow(imout)
        break