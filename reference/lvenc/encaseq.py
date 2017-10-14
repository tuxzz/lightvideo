import numpy as np
import ctypes, io, struct
from .struct import *
from . import clz4, castc, report
import signal, threading, multiprocessing, time
import cv2

_LZ4_COMPRESSION_LEVEL = 11

def calcPSNR(x, y):
    assert x.dtype == y.dtype
    assert x.shape == y.shape
    if(x.dtype == np.uint8):
        maxI = 255.0
    elif(x.dtype == np.uint16):
        maxI = 65535.0
    elif(x.dtype == np.float16):
        maxI = 1.0
    else:
        raise TypeError("only uint8 and uint16 and float16 are supported")
    mse = np.mean((x - y) ** 2)
    psnr = 20.0 * np.log10(maxI / np.sqrt(mse))
    return psnr

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

class EncoderASTCSeq:
    def __init__(self, stream, width, height, framerate, dtype, nChannel, **kwargs):
        self.stream = stream
        self.stream.seek(ctypes.sizeof(MainStructASTCSeq))

        self.width, self.height = int(width), int(height)
        self.framerate = int(framerate)
        self.dtype = dtype
        self.nChannel = int(nChannel)
        self.baseBlockSize = kwargs.get("baseBlockSize", (12, 10))
        self.quality = kwargs.get("quality", castc.COMPRESSION_GOOD)
        self.userData = kwargs.get("userData", 0)

        if(self.quality < castc.COMPRESSION_VERYFAST or self.quality > castc.COMPRESSION_EXTERME):
            raise ValueError("invalid quality")
        if(not self.baseBlockSize in castc._blockSizeDict):
            raise ValueError("invalid baseBlockSize")

        if(not self.dtype in (np.uint8, np.uint16, np.float16)):
            raise TypeError("Only uint8 and uint16 and float16 is supported")
        if(self.width <= 0 or self.height <= 0 or self.width > 32767 or self.height > 32768):
            raise ValueError("width and height must be in range [1, 32767]")
        if(self.framerate <= 0 or self.framerate > 255):
            raise ValueError("framerate must be in range [1, 255]")
        if(self.nChannel < 1 or self.nChannel > 4):
            raise ValueError("nChannel must be in range [1, 4]")
        
        self.nFrame = 0
    
    def __enter__(self):
        return self
    def __exit__(self, type, value, trace):
        with DelayedKeyboardInterrupt():
            self.stream.seek(0)
            mainStruct = MainStructASTCSeq()
            mainStruct.spica = b'SPiCa'
            mainStruct.version = 0
            mainStruct.nChannel = self.nChannel
            mainStruct.framerate = self.framerate

            for i, k in enumerate(castc._blockSizeDict):
                if(k == self.baseBlockSize):
                    break
            mainStruct.blockSizeMode = i
            
            mainStruct.width = self.width
            mainStruct.height = self.height
            mainStruct.nFrame = self.nFrame
            mainStruct.userData = np.uint64(self.userData)
            
            self.stream.write(mainStruct)
            self.stream.seek(0, io.SEEK_END)
            self.stream.flush()

    def __del__(self):
        pass

    def feedFrame(self, img):
        assert img.ndim == 3
        assert img.dtype == self.dtype
        assert img.shape[:2] == (self.height, self.width)
        assert img.shape[2] == self.nChannel
        h, w, nChannel = img.shape

        astc = castc.encodeASTC(img, self.baseBlockSize, self.quality)
        data = clz4.LZ4CompressionTask(astc, clz4.COMPRESS_MODE_HC, _LZ4_COMPRESSION_LEVEL).get()
        vf = VideoFrameStructASTCSeq()
        vf.vfrm = b'VFrM'
        vf.size = len(data)

        with DelayedKeyboardInterrupt():
            self.stream.write(vf)
            self.stream.write(data)
            print("===> Frame %d OK, %d -> %d, PSNR = %lf" % (self.nFrame, len(astc), len(data), calcPSNR(castc.decodeASTC(astc, nChannel, img.dtype), img)))
            self.nFrame += 1


        

