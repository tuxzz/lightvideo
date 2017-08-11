import ctypes
import numpy as np
import numpy.ctypeslib as npct
from . import struct

IoContextRead = ctypes.CFUNCTYPE(ctypes.c_longlong, ctypes.POINTER(ctypes.c_char), ctypes.c_longlong)
IoContextSeek = ctypes.CFUNCTYPE(ctypes.c_bool, ctypes.c_longlong)
IoContextPos = ctypes.CFUNCTYPE(ctypes.c_longlong)
class _LVDecoder(ctypes.Structure):
    pass

class LVDecoderIOContext(ctypes.Structure):
    _fields_ = [
        ("read", IoContextRead),
        ("seek", IoContextSeek),
        ("pos", IoContextPos),
    ]

class LVVideoInformation(ctypes.Structure):
    _fields_ = [
        ("width", ctypes.c_uint),
        ("height", ctypes.c_uint),
        ("framerate", ctypes.c_uint),
        ("nFrame", ctypes.c_uint),
        ("colorFormat", ctypes.c_ubyte),
        ("formatVersion", ctypes.c_ubyte)
    ]

_pLVDecoder = ctypes.POINTER(_LVDecoder)
_pLVDecoderIOContext = ctypes.POINTER(LVDecoderIOContext)

dll = ctypes.CDLL("lightvideo-encoder-helper.dll")

lvCreateDecoder = dll.lvCreateDecoder
lvCreateDecoder.argtypes = [ctypes.POINTER(LVDecoderIOContext)]
lvCreateDecoder.restype = _pLVDecoder

lvGetDecoderInformation = dll.lvGetDecoderInformation
lvGetDecoderInformation.argtypes = [_pLVDecoder]
lvGetDecoderInformation.restype = LVVideoInformation

lvDecoderFramePos = dll.lvDecoderFramePos
lvDecoderFramePos.argtypes = [_pLVDecoder]
lvDecoderFramePos.restype = ctypes.c_uint

lvDecoderSeekFrame = dll.lvDecoderSeekFrame
lvDecoderSeekFrame.argtypes = [_pLVDecoder, ctypes.c_uint]
lvDecoderSeekFrame.restype = ctypes.c_int

lvGetFrame8 = dll.lvGetFrame8
lvGetFrame8.argtypes = [_pLVDecoder, ctypes.POINTER(ctypes.POINTER(ctypes.c_uint8)), ctypes.c_uint]
lvGetFrame8.restype = ctypes.c_int

lvDestroyDecoder = dll.lvDestroyDecoder
lvDestroyDecoder.argtypes = [_pLVDecoder]
lvDestroyDecoder.restype = None

RESULT_FAILED = 0
RESULT_EOF = 1
RESULT_SUCCESS = 2

class Decoder:
    def __init__(self, stream):
        self.stream = stream

        self.ioCtx = LVDecoderIOContext()
        self.ioCtx.read = IoContextRead(self._read)
        self.ioCtx.seek = IoContextSeek(self._seek)
        self.ioCtx.pos = IoContextPos(self._pos)
        self.pDecoder = lvCreateDecoder(ctypes.byref(self.ioCtx))
        if(not self.pDecoder):
            raise RuntimeError("Failed to create decoder instance.")

        info = lvGetDecoderInformation(self.pDecoder)
        self.width, self.height, self.framerate, self.nFrame, self.colorFormat, self.formatVersion = info.width, info.height, info.framerate, info.nFrame, info.colorFormat, info.formatVersion

    def __del__(self):
        if(self.pDecoder):
            lvDestroyDecoder(self.pDecoder)

    @property
    def framePos(self):
        return lvDecoderFramePos(self.pDecoder)

    def getFrame(self):
        channelList = []
        if(self.colorFormat == struct.COLOR_YUV420P or self.colorFormat == struct.COLOR_YUVA420P):
            halfHeight = max(1, self.height // 2)
            halfWidth = max(1, self.width // 2)
            channelList.append(np.zeros((self.height, self.width), dtype = np.uint8))
            channelList.append(np.zeros((halfHeight, halfWidth), dtype = np.uint8))
            channelList.append(np.zeros((halfHeight, halfWidth), dtype = np.uint8))
            if(self.colorFormat == struct.COLOR_YUVA420P):
                channelList.append(np.zeros((self.height, self.width), dtype = np.uint8))
        else:
            assert False

        pChannelList = []
        for channel in channelList:
            pChannelList.append(channel.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8)))
        pChannelList = (ctypes.POINTER(ctypes.c_uint8) * len(channelList))(*pChannelList)

        result = lvGetFrame8(self.pDecoder, pChannelList, len(channelList))
        if(result == RESULT_FAILED):
            raise RuntimeError("getFrame failed.")
        elif(result == RESULT_EOF):
            raise EOFError()
        return tuple(channelList)

    def _read(self, pDst, size):
        dst = (ctypes.c_char * size).from_address(ctypes.addressof(pDst.contents))
        try:
            data = self.stream.read(size)
        except IOError as e:
            print(str(e))
            return -1
        dst[:len(data)] = data
        return len(data)

    def _seek(self, pos):
        try:
            self.stream.seek(pos)
        except IOError as e:
            print(str(e))
            return False
        return True

    def _pos(self):
        return self.stream.tell()
