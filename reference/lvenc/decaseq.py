import numpy as np
import cv2

from . import cfilter, clz4, castc
from .struct import *

class DecoderCoreASTCSeq:
    def __init__(self, stream):
        self.stream = stream
        mainStruct = MainStructASTCSeq()
        self.stream.readinto(mainStruct)
        if(mainStruct.spica != b'SPiCa'):
            raise ValueError("Invalid input header(got %s)" % str(mainStruct.spica))
        if(mainStruct.version != 0):
            raise ValueError("Unsupported input version")
        self.nChannel, self.framerate, self.blockSizeMode, self.width, self.height, self.nFrame, self.userData = mainStruct.nChannel, mainStruct.framerate, mainStruct.blockSizeMode, mainStruct.width, mainStruct.height, mainStruct.nFrame, mainStruct.userData
        if(self.nChannel <= 0 or self.nChannel > 4):
            raise ValueError("Invalid channel count(got %d)" % self.nChannel)
        if(self.framerate <= 0):
            raise ValueError("Invalid framerate(got %d)" % self.framerate)
        if(self.blockSizeMode <= 0 or self.blockSizeMode > len(castc._blockSizeDict)):
            raise ValueError("invalid blockSizeMode")
        if(self.nFrame <= 0):
            raise ValueError("No frame available")
        if(self.width <= 0 or self.width > 32767 or self.height <= 0 or self.height > 32767):
            raise ValueError("Invalid width or height(got w = %d, h = %d)" % (self.width, self.height))
        
        for i, k in enumerate(castc._blockSizeDict):
            if(i == self.blockSizeMode):
                self.blockSize = k
        
    def readFrame(self):
        vf = VideoFrameStructASTCSeq()
        self.stream.readinto(vf)
        # check video frame data header
        if(vf.vfrm != b'VFrM'):
            raise ValueError("Invalid frame header")
        if(vf.size <= 0):
            raise ValueError("Invalid video frame data size")
        
        data = self.stream.read(vf.size)
        if(len(data) != vf.size):
            raise ValueError("no enough data")
        
        decompressedSize = castc.calcCompressedSize(self.width, self.height, self.blockSize)
        data = clz4.decompressLZ4(data, decompressedSize)
        img = castc.decodeASTC(data, self.nChannel, np.float16)
        del data
        if(img.shape[:2] != (self.height, self.width)):
            raise ValueError("Invalid data shape")
        return img