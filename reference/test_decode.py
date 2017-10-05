import numpy as np
import cv2
import os
import lvenc, ctypes
import gc, profile, time

dec = lvenc.DecoderCore(open("out.rcv", "rb"))

frame = dec.readFrame()
y = frame[0].reshape(*frame[0].shape[:2])
u = cv2.resize(frame[1][:,:,0], (*y.shape[::-1],), cv2.INTER_LINEAR)
v = cv2.resize(frame[1][:,:,1], (*y.shape[::-1],), cv2.INTER_LINEAR)
rgb = lvenc.YUVPToRGBI(y, u, v)
cv2.imwrite("out1.png", rgb[:,:,(2, 1, 0)])

frame = dec.readFrame()
y = frame[0].reshape(*frame[0].shape[:2])
u = cv2.resize(frame[1][:,:,0], (*y.shape[::-1],), cv2.INTER_LINEAR)
v = cv2.resize(frame[1][:,:,1], (*y.shape[::-1],), cv2.INTER_LINEAR)
rgb = lvenc.YUVPToRGBI(y, u, v)
cv2.imwrite("out2.png", rgb[:,:,(2, 1, 0)])

gc.collect()
dll = ctypes.CDLL("lightvideo-encoder-helper.dll")
lvCheckAllocated = dll.lvCheckAllocated
lvCheckAllocated.argtypes = []
lvCheckAllocated.restype = None
lvCheckAllocated()
