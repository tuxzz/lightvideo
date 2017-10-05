import numpy as np
import ctypes, io, struct
from .struct import *
from . import enccore, clz4, report
import signal, threading, multiprocessing, time
import cv2

_LZ4_COMPRESSION_LEVEL = 11

class DelayedKeyboardInterrupt(object):
    def __enter__(self):
        self.signal_received = False
        self.old_handler = signal.getsignal(signal.SIGINT)
        signal.signal(signal.SIGINT, self.handler)

    def handler(self, sig, frame):
        self.signal_received = (sig, frame)

    def __exit__(self, type, value, traceback):
        if(self.old_handler is not None):
            signal.signal(signal.SIGINT, self.old_handler)
            if(self.signal_received):
                self.old_handler(*self.signal_received)

class Encoder:
    def __init__(self, stream, width, height, framerate, dtype, nFullSizeChannel, nHalfSizeChannel, **kwargs):
        self.stream = stream
        self.stream.seek(ctypes.sizeof(MainStruct))

        self.width = int(width)
        self.height = int(height)
        self.framerate = int(framerate)
        self.dtype = dtype
        self.userData = np.uint64(kwargs.get("userData", 0))

        self.dropThreshold = int(kwargs.get("dropThreshold", 1 if self.dtype == np.uint8 else 128))

        if(not self.dtype in (np.uint8, np.uint16)):
            raise TypeError("Only uint8 and uint16 is supported")
        
        self.nFullSizeChannel = int(nFullSizeChannel)
        self.nHalfSizeChannel = int(nHalfSizeChannel)
        if(self.nHalfSizeChannel < 0 or self.nHalfSizeChannel > 4):
            raise ValueError("Channel layout for each type must be in range [0, 4]")
        if(self.nFullSizeChannel < 0 or self.nFullSizeChannel > 4):
            raise ValueError("Channel layout for each type must be in range [0, 4]")
        if(self.nHalfSizeChannel + self.nFullSizeChannel <= 0):
            raise ValueError("Total channel count must be greater than 0")

        wInvalid = self.width <= 0 or self.width > 32767
        hInvalid = self.height <= 0 or self.height > 32767
        if(wInvalid or hInvalid):
            raise ValueError("weight and height must be in range [1, 32767]")

        if(self.framerate <= 0 or self.framerate > 255):
            raise ValueError("framerate must be in range [1, 255]")

        elif(self.dropThreshold > 65535):
            raise ValueError("dropThreshold for 16bit color format must be less than 65536")
        
        self.prevImgList = None
        self.prevFullImgList = None

        self.nFrame = 0

    def __enter__(self):
        return self
    def __exit__(self, type, value, trace):
        with DelayedKeyboardInterrupt():
            self.stream.seek(0)
            mainStruct = MainStruct()
            mainStruct.aria = b'ARiA'
            mainStruct.version = 0
            mainStruct.nChannel = self.nFullSizeChannel | (self.nHalfSizeChannel << 4)
            mainStruct.flags = int(self.dtype == np.uint16)
            mainStruct.framerate = self.framerate
            mainStruct.width = self.width
            mainStruct.height = self.height
            mainStruct.nFrame = self.nFrame
            mainStruct.userData = np.uint64(self.userData)
            
            self.stream.write(mainStruct)
            self.stream.seek(0, io.SEEK_END)
            self.stream.flush()

    def __del__(self):
        pass

    def feedFrame(self, fullImg, halfImg):
        assert fullImg is None or (fullImg.ndim == 3 and fullImg.dtype == self.dtype and fullImg.shape[-1] == self.nFullSizeChannel)
        assert halfImg is None or (halfImg.ndim == 3 and halfImg.dtype == self.dtype and halfImg.shape[-1] == self.nHalfSizeChannel)
        assert fullImg is not None or halfImg is not None
        
        report.enter("frame %d" % self.nFrame)

        # do filter
        imgList = []
        if(fullImg is not None):
            imgList.append(fullImg)
        if(halfImg is not None):
            imgList.append(halfImg)
        result = enccore.applyBestFilter(imgList, self.prevFullImgList, self.prevImgList, self.dropThreshold)
        cv2.imwrite("ho.png", result["bestResult"][1]["filtered"][:,:,0])

        # serialize and compress
        vf = VideoFrameStruct()
        vf.vfrm = b'VFRM'
        vf.referenceType = result["deltaMethod"]
        usedIndex = 0
        if(fullImg is not None):
            vf.intraMethod[0] = result["bestResult"][0]["intraMethod"]
            usedIndex += 1
        if(halfImg is not None):
            vf.intraMethod[1] = result["bestResult"][usedIndex]["intraMethod"]
        del usedIndex
        data = io.BytesIO()
        
        for channelResult in result["bestResult"]:
            data.write(channelResult["filtered"])
        
        # write
        data = clz4.LZ4CompressionTask(data.getvalue(), clz4.COMPRESS_MODE_HC, _LZ4_COMPRESSION_LEVEL).get()
        vf.size = len(data)
        with DelayedKeyboardInterrupt():
            self.stream.write(vf)
            self.stream.write(data)

        self.prevImgList = []
        for channelResult in result["bestResult"]:
            self.prevImgList.append(channelResult["decompressed"])
        if(result["deltaMethod"] == REFERENCE_NONE):
            self.prevFullImgList = self.prevImgList

        # clean up
        self.nFrame += 1
        report.leave()