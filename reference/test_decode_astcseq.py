import numpy as np
import cv2
import os
import lvenc, ctypes
import gc, profile, time

dec = lvenc.DecoderCoreASTCSeq(open("out.acv", "rb"))
frame = dec.readFrame()
cv2.imshow("", frame.astype(np.float64)[:,:,(2, 1, 0)])
cv2.waitKey()

gc.collect()
dll = ctypes.CDLL("lightvideo-encoder-helper.dll")
lvCheckAllocated = dll.lvCheckAllocated
lvCheckAllocated.argtypes = []
lvCheckAllocated.restype = None
lvCheckAllocated()
