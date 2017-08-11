import ctypes
import numpy as np
import numpy.ctypeslib as npct

dll = ctypes.CDLL("lightvideo-encoder-helper.dll")
uint8_p = ctypes.POINTER(ctypes.c_ubyte)
uint16_p = ctypes.POINTER(ctypes.c_ushort)
uint8_p_2d = npct.ndpointer(dtype = np.uint8, ndim = 2, flags = "C")
uint16_p_2d = npct.ndpointer(dtype = np.uint16, ndim = 2, flags = "C")
uint8_p_3d = npct.ndpointer(dtype = np.uint8, ndim = 3, flags = "C")
uint16_p_3d = npct.ndpointer(dtype = np.uint16, ndim = 3, flags = "C")

lvYUVP2RGBI8 = dll.lvYUVP2RGBI8
lvYUVP2RGBI8.argtypes = [uint8_p_2d, uint8_p_2d, uint8_p_2d, uint8_p_3d, ctypes.c_int, ctypes.c_int, ctypes.c_int]
lvYUVP2RGBI8.restype = None
lvRGBI2YUVP8 = dll.lvRGBI2YUVP8
lvRGBI2YUVP8.argtypes = [uint8_p_3d, ctypes.c_int, uint8_p_2d, uint8_p_2d, uint8_p_2d, ctypes.c_int, ctypes.c_int]
lvRGBI2YUVP8.restype = None
lvYUVAP2RGBAI8 = dll.lvYUVAP2RGBAI8
lvYUVAP2RGBAI8.argtypes = [uint8_p_2d, uint8_p_2d, uint8_p_2d, uint8_p_2d, uint8_p_3d, ctypes.c_int, ctypes.c_int, ctypes.c_int]
lvYUVAP2RGBAI8.restype = None
lvRGBAI2YUVAP8 = dll.lvRGBAI2YUVAP8
lvRGBAI2YUVAP8.argtypes = [uint8_p_3d, ctypes.c_int, uint8_p_2d, uint8_p_2d, uint8_p_2d, uint8_p_2d, ctypes.c_int, ctypes.c_int]
lvRGBAI2YUVAP8.restype = None

lvYUVP2RGBI16 = dll.lvYUVP2RGBI16
lvYUVP2RGBI16.argtypes = [uint16_p_2d, uint16_p_2d, uint16_p_2d, uint16_p_3d, ctypes.c_int, ctypes.c_int, ctypes.c_int]
lvYUVP2RGBI16.restype = None
lvRGBI2YUVP16 = dll.lvRGBI2YUVP16
lvRGBI2YUVP16.argtypes = [uint16_p_3d, ctypes.c_int, uint16_p_2d, uint16_p_2d, uint16_p_2d, ctypes.c_int, ctypes.c_int]
lvRGBI2YUVP16.restype = None
lvYUVAP2RGBAI16 = dll.lvYUVAP2RGBAI16
lvYUVAP2RGBAI16.argtypes = [uint16_p_2d, uint16_p_2d, uint16_p_2d, uint16_p_2d, uint16_p_3d, ctypes.c_int, ctypes.c_int, ctypes.c_int]
lvYUVAP2RGBAI16.restype = None
lvRGBAI2YUVAP16 = dll.lvRGBAI2YUVAP16
lvRGBAI2YUVAP16.argtypes = [uint16_p_3d, ctypes.c_int, uint16_p_2d, uint16_p_2d, uint16_p_2d, uint16_p_2d, ctypes.c_int, ctypes.c_int]
lvRGBAI2YUVAP16.restype = None

def YUVPToRGBI(y, u, v, a = None):
    height, width = y.shape
    if(y.ndim != 2 or (not (y.shape == u.shape == v.shape)) or (a is not None and a.shape != y.shape)):
        raise ValueError("Invalid input shape")
    if((not (y.dtype == u.dtype == v.dtype)) or (a is not None and a.dtype != y.dtype)):
        raise TypeError("Invalid input data type")
    if(height == 0 or width == 0):
        raise ValueError("Input size cannot be zero")
    y = np.require(y, y.dtype, requirements = "C")
    u = np.require(u, u.dtype, requirements = "C")
    v = np.require(v, v.dtype, requirements = "C")

    if(a is not None):
        a = np.require(a, a.dtype, requirements = "C")
        out = np.zeros((*y.shape, 4), dtype = y.dtype)
    else:
        out = np.zeros((*y.shape, 3), dtype = y.dtype)

    if(y.dtype == np.uint8):
        if(a is None):
            lvYUVP2RGBI8(y, u, v, out, 3, width, height)
        else:
            lvYUVAP2RGBAI8(y, u, v, a, out, 4, width, height)
    elif(y.dtype == np.uint16):
        if(a is None):
            lvYUVP2RGBI16(y, u, v, out, 3, width, height)
        else:
            lvYUVAP2RGBAI16(y, u, v, a, out, 4, width, height)
    else:
        raise TypeError("Only uint8 or uint16 is supported")
    return out

def RGBIToYUVP(rgb):
    height, width, channel = rgb.shape
    if(channel == 3):
        alpha = False
    elif(channel == 4):
        alpha = True
    else:
        raise TypeError("Invalid input shape")
    rgb = np.require(rgb, rgb.dtype, requirements = "C")

    y, u, v = np.zeros((height, width), dtype = rgb.dtype), np.zeros((height, width), dtype = rgb.dtype), np.zeros((height, width), dtype = rgb.dtype)
    if(alpha):
        a = np.zeros((height, width), dtype = rgb.dtype)
    if(rgb.dtype == np.uint8):
        if(alpha):
            lvRGBAI2YUVAP8(rgb, 4, y, u, v, a, width, height)
        else:
            lvRGBI2YUVP8(rgb, 3, y, u, v, width, height)
    elif(rgb.dtype == np.uint16):
        if(alpha):
            lvRGBAI2YUVAP16(rgb, 4, y, u, v, a, width, height)
        else:
            lvRGBI2YUVP16(rgb, 3, y, u, v, width, height)
    else:
        raise TypeError("Only uint8 or uint16 is supported")

    if(alpha):
        return y, u, v, a
    else:
        return y, u, v
