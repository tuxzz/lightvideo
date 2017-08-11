import ctypes
import numpy as np
import numpy.ctypeslib as npct

dll = ctypes.CDLL("lightvideo-encoder-helper.dll")
uint8_p = ctypes.POINTER(ctypes.c_ubyte)
uint16_p = ctypes.POINTER(ctypes.c_ushort)
uint8_p_2d = npct.ndpointer(dtype = np.uint8, ndim = 2, flags = "C")
uint16_p_2d = npct.ndpointer(dtype = np.uint16, ndim = 2, flags = "C")

# 8bit
lvFilterSubTop8 = dll.lvFilterSubTop8
lvFilterSubTop8.argtypes = [uint8_p_2d, ctypes.c_int, ctypes.c_int, ctypes.c_ubyte]
lvFilterSubTop8.restype = None
lvDefilterSubTop8 = dll.lvDefilterSubTop8
lvDefilterSubTop8.argtypes = [uint8_p_2d, ctypes.c_int, ctypes.c_int]
lvDefilterSubTop8.restype = None

lvFilterSubLeft8 = dll.lvFilterSubLeft8
lvFilterSubLeft8.argtypes = [uint8_p_2d, ctypes.c_int, ctypes.c_int, ctypes.c_ubyte]
lvFilterSubLeft8.restype = None
lvDefilterSubLeft8 = dll.lvDefilterSubLeft8
lvDefilterSubLeft8.argtypes = [uint8_p_2d, ctypes.c_int, ctypes.c_int]
lvDefilterSubLeft8.restype = None

lvFilterSubAvg8 = dll.lvFilterSubAvg8
lvFilterSubAvg8.argtypes = [uint8_p_2d, ctypes.c_int, ctypes.c_int, ctypes.c_ubyte, uint8_p]
lvFilterSubAvg8.restype = None
lvDefilterSubAvg8 = dll.lvDefilterSubAvg8
lvDefilterSubAvg8.argtypes = [uint8_p_2d, ctypes.c_int, ctypes.c_int]
lvDefilterSubAvg8.restype = None

lvFilterSubPaeth8 = dll.lvFilterSubPaeth8
lvFilterSubPaeth8.argtypes = [uint8_p_2d, ctypes.c_int, ctypes.c_int, ctypes.c_ubyte, uint8_p]
lvFilterSubPaeth8.restype = None
lvDefilterSubPaeth8 = dll.lvDefilterSubPaeth8
lvDefilterSubPaeth8.argtypes = [uint8_p_2d, ctypes.c_int, ctypes.c_int]
lvDefilterSubPaeth8.restype = None

# 16bit
lvFilterSubTop16 = dll.lvFilterSubTop16
lvFilterSubTop16.argtypes = [uint16_p_2d, ctypes.c_int, ctypes.c_int, ctypes.c_ubyte]
lvFilterSubTop16.restype = None
lvDefilterSubTop16 = dll.lvDefilterSubTop16
lvDefilterSubTop16.argtypes = [uint16_p_2d, ctypes.c_int, ctypes.c_int]
lvDefilterSubTop16.restype = None

lvFilterSubLeft16 = dll.lvFilterSubLeft16
lvFilterSubLeft16.argtypes = [uint16_p_2d, ctypes.c_int, ctypes.c_int, ctypes.c_ubyte]
lvFilterSubLeft16.restype = None
lvDefilterSubLeft16 = dll.lvDefilterSubLeft16
lvDefilterSubLeft16.argtypes = [uint16_p_2d, ctypes.c_int, ctypes.c_int]
lvDefilterSubLeft16.restype = None

lvFilterSubAvg16 = dll.lvFilterSubAvg16
lvFilterSubAvg16.argtypes = [uint16_p_2d, ctypes.c_int, ctypes.c_int, ctypes.c_ubyte, uint16_p]
lvFilterSubAvg16.restype = None
lvDefilterSubAvg16 = dll.lvDefilterSubAvg16
lvDefilterSubAvg16.argtypes = [uint16_p_2d, ctypes.c_int, ctypes.c_int]
lvDefilterSubAvg16.restype = None

lvFilterSubPaeth16 = dll.lvFilterSubPaeth16
lvFilterSubPaeth16.argtypes = [uint16_p_2d, ctypes.c_int, ctypes.c_int, ctypes.c_ubyte, uint16_p]
lvFilterSubPaeth16.restype = None
lvDefilterSubPaeth16 = dll.lvDefilterSubPaeth16
lvDefilterSubPaeth16.argtypes = [uint16_p_2d, ctypes.c_int, ctypes.c_int]
lvDefilterSubPaeth16.restype = None

def _callFilter(func8, func16, data, threshold):
    assert np.asarray((threshold,), dtype = data.dtype)[0] == threshold, "Threshold must be integer and in dtype range"
    threshold = np.asarray((threshold,), dtype = data.dtype)[0]
    height, width = data.shape
    if(height == 0 or width == 0):
        raise ValueError("Input size cannot be zero")
    out = data.astype(data.dtype, order = "C")
    if(data.dtype == np.uint8):
        func = func8
    elif(data.dtype == np.uint16):
        func = func16
    else:
        raise TypeError("Only uint8 or uint16 is supported")
    if(len(func.argtypes) == 5):
        func(out, width, height, threshold, None)
    elif(len(func.argtypes) == 4):
        func(out, width, height, threshold)
    else:
        assert False, "Bad ctypes function"
    return out

def _callDefilter(func8, func16, data):
    height, width = data.shape
    if(height == 0 or width == 0):
        raise ValueError("Input size cannot be zero")
    out = data.astype(data.dtype, order = "C")
    if(data.dtype == np.uint8):
        func = func8
    elif(data.dtype == np.uint16):
        func = func16
    else:
        raise TypeError("Only uint8 or uint16 is supported")
    assert len(func.argtypes) == 3, "Bad ctypes function"
    func(out, width, height)
    return out

def filterSubTop(data, threshold = 0):
    return _callFilter(lvFilterSubTop8, lvFilterSubTop16, data, threshold)

def filterSubLeft(data, threshold = 0):
    return _callFilter(lvFilterSubLeft8, lvFilterSubLeft16, data, threshold)

def filterSubAvg(data, threshold = 0):
    return _callFilter(lvFilterSubAvg8, lvFilterSubAvg16, data, threshold)

def filterSubPaeth(data, threshold = 0):
    return _callFilter(lvFilterSubPaeth8, lvFilterSubPaeth16, data, threshold)

def defilterSubTop(data):
    return _callDefilter(lvDefilterSubTop8, lvDefilterSubTop16, data)

def defilterSubLeft(data):
    return _callDefilter(lvDefilterSubLeft8, lvDefilterSubLeft16, data)

def defilterSubAvg(data):
    return _callDefilter(lvDefilterSubAvg8, lvDefilterSubAvg16, data)

def defilterSubPaeth(data):
    return _callDefilter(lvDefilterSubPaeth8, lvDefilterSubPaeth16, data)
