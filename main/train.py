# -*- coding: utf-8 -*-
"""Copy of Markdown Guide

Automatically generated by Colaboratory.

Original file is located at
    https://colab.research.google.com/drive/1Mttjtaf6MnhIi4DTnmAYnhINKSbn2-Oo
"""

import numpy as np 

inp1 = np.array([0,1,2,3,4,5])
inp2 = np.array([0,1,2,3,4,5])
tar = np.array([0,1,2,3,4,5])

print(inp1.shape, tar.shape) 

print(inp1[:5], tar[:5])

from tensorflow.keras import Input  
from tensorflow.keras import Model 
from tensorflow.keras.layers import *

x1 = Input(shape =(1,))
x2 = Input(shape =(1,))

input_layer = concatenate([x1,x2])
hidden_layer = Dense(units=4, activation='relu')(input_layer)

prediction = Dense(1, activation='linear')(hidden_layer)
model = Model(inputs=[x1, x2], outputs=prediction)

model.compile(loss="mean_squared_error", 
              optimizer='adam', 
              metrics=['mae'])
model.fit([inp1, inp2], tar, epochs=300, 
          batch_size=32, verbose=2)

model([np.expand_dims(np.array(4), 0), 
       np.expand_dims(np.array(4), 0)]).numpy()

model([np.expand_dims(np.array(10), 0), 
       np.expand_dims(np.array(10), 0)]).numpy()

model([np.expand_dims(np.array(50), 0), 
       np.expand_dims(np.array(40), 0)]).numpy()

model.summary()

import tensorflow as tf

converter = tf.lite.TFLiteConverter.from_keras_model(model)
print(converter.inference_input_type)
print(converter.inference_output_type)
tflite_model = converter.convert()
tflite_model_size = open('./model.tflite', "wb").write(tflite_model)