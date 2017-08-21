import numpy as np
import cv2
import os
import lvenc, ctypes, profile, gc

def main():
    inputFileList = []
    for root, dirList, fileList in os.walk("./work/src"):
    #for root, dirList, fileList in os.walk("./work/ARiASeq"):
        for fileName in fileList:
            path = os.path.join(root, fileName)
            if(os.path.splitext(fileName)[-1].lower() == ".png"):
                inputFileList.append(path)
            else:
                print("Ignored", path)
    inputFileList.sort(key = lambda x:int(os.path.splitext(os.path.basename(x))[0]))
    #inputFileList.sort()
    h, w = cv2.imread(inputFileList[0]).shape[:2]
    with open("out.rcv", "wb") as f:
        with lvenc.Encoder(f, w, h, 24, lvenc.struct.COLOR_YUV420P, dropThreshold = 1, maxPacketSize = 3, maxScaleRadius = 0, maxMovingRadius = 0) as enc:
            halfWidth = max(1, enc.width // 2)
            halfHeight = max(1, enc.height // 2)
            for path in inputFileList:
                print(path)
                y, u, v = lvenc.RGBIToYUVP(cv2.imread(path)[:, :, (2, 1, 0)])
                u = cv2.resize(u, (halfWidth, halfHeight), cv2.INTER_AREA)
                v = cv2.resize(v, (halfWidth, halfHeight), cv2.INTER_AREA)
                enc.feedFrame([y, u, v])
profile.run("main()")

gc.collect()
dll = ctypes.CDLL("lightvideo-encoder-helper.dll")
lvCheckAllocated = dll.lvCheckAllocated
lvCheckAllocated.argtypes = []
lvCheckAllocated.restype = None
lvCheckAllocated()
