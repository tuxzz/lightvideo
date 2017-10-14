import numpy as np
import cv2
import os
import lvenc, ctypes, profile, gc
import pickle

def main():
    inputFileList = []
    #for root, dirList, fileList in os.walk("./work/3p"):
    for root, dirList, fileList in os.walk("./work/ARiASeq"):
        for fileName in fileList:
            path = os.path.join(root, fileName)
            if(os.path.splitext(fileName)[-1].lower() == ".png"):
                inputFileList.append(path)
            else:
                print("Ignored", path)
    #inputFileList.sort(key = lambda x:int(os.path.splitext(os.path.basename(x))[0]))
    inputFileList.sort()
    h, w = cv2.imread(inputFileList[0]).shape[:2]
    with open("out.acv", "wb") as f:
        with lvenc.EncoderASTCSeq(f, w, h, 24, np.uint8, 3, quality = 4) as enc:
            halfWidth = max(1, enc.width // 2)
            halfHeight = max(1, enc.height // 2)
            for path in inputFileList[2048:2048+16]:
                print(path)
                rgb = cv2.imread(path)[:, :, (2, 1, 0)]
                enc.feedFrame(rgb)
#profile.run("main()")
main()
gc.collect()
dll = ctypes.CDLL("lightvideo-encoder-helper.dll")
lvCheckAllocated = dll.lvCheckAllocated
lvCheckAllocated.argtypes = []
lvCheckAllocated.restype = None
lvCheckAllocated()
