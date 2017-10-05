import numpy as np
import cv2

from . import cfilter, clz4
from .struct import *


class DecoderCore:
    def __init__(self, stream):
        self.stream = stream
        mainStruct = MainStruct()
        self.stream.readinto(mainStruct)
        if(mainStruct.aria != b'ARiA'):
            raise ValueError("Invalid input header(got %s)" % str(mainStruct.aria))
        if(mainStruct.version != 0):
            raise ValueError("Unsupported input version")
        
        self.dtype = np.uint16 if(bool(mainStruct.flags & 0x1)) else np.uint8
        self.itemsize = self.dtype(0).itemsize

        self.nFullSizeChannel = mainStruct.nChannel & 0x0f
        self.nHalfSizeChannel = (mainStruct.nChannel & 0xf0) >> 4
        if(self.nFullSizeChannel < 0 or self.nFullSizeChannel > 4):
            raise ValueError("Invalid full size channel count(got %d)" % self.nFullSizeChannel)
        if(self.nHalfSizeChannel < 0 or self.nHalfSizeChannel > 4):
            raise ValueError("Invalid half size channel count(got %d)" % self.nHalfSizeChannel)
        channelCount = self.nFullSizeChannel + self.nHalfSizeChannel
        if(channelCount <= 0 or channelCount > 8):
            raise ValueError("Invalid channel count(got %d)" % channelCount)

        self.framerate, self.nFrame, self.width, self.height, self.userData = mainStruct.framerate, mainStruct.nFrame, mainStruct.width, mainStruct.height, mainStruct.userData
        if(self.framerate <= 0):
            raise ValueError("Invalid framerate(got %d)" % self.framerate)
        if(self.nFrame <= 0):
            raise ValueError("No frame available")
        if(self.width <= 0 or self.width > 32767 or self.height <= 0 or self.height > 32767):
            raise ValueError("Invalid width or height(got w = %d, h = %d)" % (self.width, self.height))
        
        self.prevImg = None
        self.prevFullImg = None
    
    def readFrame(self):
        vf = VideoFrameStruct()
        self.stream.readinto(vf)

        # check video frame data header
        if(vf.vfrm != b'VFRM'):
            raise ValueError("Invalid frame header")
        if(vf.size <= 0):
            raise ValueError("Invalid video frame data size")
        if(not vf.referenceType in (REFERENCE_NONE, REFERENCE_PREVFULL, REFERENCE_PREV)):
            raise ValueError("Invalid referenceType(got 0x%s)" % hex(vf.referenceType))
        
        intraMethodList = (FILTER_NONE, FILTER_SUBTOP, FILTER_SUBLEFT, FILTER_SUBAVG, FILTER_SUBPAETH)
        if(not (vf.intraMethod[0] in intraMethodList and vf.intraMethod[1] in intraMethodList)):
           raise ValueError("Invalid intra filter method")
        del intraMethodList
        
        # decompress video frame data
        h, w = self.height, self.width
        hh, hw = max(1, h // 2), max(1, w // 2)
        decompressedSize = (self.nFullSizeChannel * h * w + self.nHalfSizeChannel * hh * hw) * self.itemsize
        decompressedData = clz4.decompressLZ4(self.stream.read(vf.size), decompressedSize)

        # read into array
        iRead = 0
        imgList = []
        if(self.nFullSizeChannel > 0):
            fullSizeChannel = np.fromstring(decompressedData[iRead:iRead + h * w * self.nFullSizeChannel * self.itemsize], dtype = self.dtype).reshape(h, w, self.nFullSizeChannel)
            imgList.append(fullSizeChannel)
            iRead += h * w * self.nFullSizeChannel * self.itemsize
        if(self.nHalfSizeChannel > 0):
            halfSizeChannel = np.fromstring(decompressedData[iRead:iRead + hh * hw * self.nHalfSizeChannel * self.itemsize], dtype = self.dtype).reshape(hh, hw, self.nHalfSizeChannel)
            imgList.append(halfSizeChannel)
            iRead += hh * hw * self.nHalfSizeChannel * self.itemsize
        del iRead

        # defilter intra
        defilterMethod = (None, cfilter.defilterSubTop, cfilter.defilterSubLeft, cfilter.defilterSubAvg, cfilter.defilterSubPaeth)
        if(self.nFullSizeChannel > 0):
            if(vf.intraMethod[0] != FILTER_NONE):
                fullSizeChannel[:,:,:] = defilterMethod[vf.intraMethod[0]](fullSizeChannel)
        if(self.nHalfSizeChannel > 0):
            if(vf.intraMethod[1] != FILTER_NONE):
                cv2.imwrite("uf.png", halfSizeChannel[:,:,0])
                halfSizeChannel[:,:,:] = defilterMethod[vf.intraMethod[1]](halfSizeChannel)
        
        # defilter delta
        if(vf.referenceType == REFERENCE_PREVFULL):
            if(self.prevFullImg is None):
                raise ValueError("No previous image avialable.")
            for i, channel in enumerate(imgList):
                channel += self.prevFullImg[i]
        elif(vf.referenceType == REFERENCE_PREV):
            if(self.prevImg is None):
                raise ValueError("No previous full image avialable.")
            for i, channel in enumerate(imgList):
                channel += self.prevImg[i]
        self.prevImg = imgList
        if(vf.referenceType == REFERENCE_NONE):
            self.prevFullImg = self.prevImg
        return (*imgList,)