import pandas as pd
import tensorflow as tf
from sklearn.model_selection import train_test_split

df = pd.read_csv("sensor_data.csv")
X = df[['temp', 'hum', 'distance']].values
y = df['label'].values

model = tf.keras.Sequential([
    tf.keras.layers.Input(shape=(3,)),
    tf.keras.layers.Dense(8, activation='relu'),
    tf.keras.layers.Dense(4, activation='relu'),
    tf.keras.layers.Dense(2, activation='softmax')
])

model.compile(optimizer='adam', loss='sparse_categorical_crossentropy', metrics=['accuracy'])
model.fit(X, y, epochs=30)

# TFLite 변환
converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()

# Arduino용 헤더 파일로 저장
with open("model.h", "w") as f:
    f.write("const unsigned char model[] = {")
    f.write(','.join(str(b) for b in tflite_model))
    f.write("};")
