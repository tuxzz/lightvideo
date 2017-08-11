import ctypes
import numpy as np
import numpy.ctypeslib as npct

dll = ctypes.CDLL("lightvideo-encoder-helper.dll")

uint8_p_2d = npct.ndpointer(dtype = np.uint8, ndim = 2, flags = "C")
uint16_p_2d = npct.ndpointer(dtype = np.uint16, ndim = 2, flags = "C")

lvMotionResample8 = dll.lvMotionResample8
lvMotionResample8.argtypes = [uint8_p_2d, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int, uint8_p_2d]
lvMotionResample8.restype = None
lvMotionResample16 = dll.lvMotionResample16
lvMotionResample16.argtypes = [uint16_p_2d, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int, uint16_p_2d]
lvMotionResample16.restype = None

def motionResample(data, scale, moveX, moveY):
    height, width = data.shape
    if(height == 0 or width == 0):
        raise ValueError("Input size cannot be zero")
    out = data.astype(data.dtype, order = "C")
    if(data.dtype == np.uint8):
        lvMotionResample8(data, width, height, scale, moveX, moveY, out)
    elif(data.dtype == np.uint16):
        lvMotionResample16(data, width, height, scale, moveX, moveY, out)
    else:
        raise TypeError("Only uint8 or uint16 is supported")

    return out
