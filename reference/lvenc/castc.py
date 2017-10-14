import ctypes, collections
import numpy as np
import numpy.ctypeslib as npct

dll = ctypes.CDLL("lightvideo-encoder-helper.dll")
uint8_p_3d = npct.ndpointer(dtype = np.uint8, ndim = 3, flags = "C")
float16_p_3d = npct.ndpointer(dtype = np.float16, ndim = 3, flags = "C")

class _LvASTCInformation(ctypes.Structure):
    _fields_ = [
        ("width", ctypes.c_int),
        ("height", ctypes.c_int),
        ("blockWidth", ctypes.c_int),
        ("blockHeight", ctypes.c_int),
    ]

COMPRESSION_VERYFAST = 0x0
COMPRESSION_FAST = 0x1
COMPRESSION_MEDIUM = 0x2
COMPRESSION_GOOD = 0x3
COMPRESSION_EXTERME = 0x4

lvGetASTCCompressedSize = dll.lvGetASTCCompressedSize
lvGetASTCCompressedSize.argtypes = [ctypes.POINTER(_LvASTCInformation)]
lvGetASTCCompressedSize.restype = ctypes.c_int

lvGetASTCInformation = dll.lvGetASTCInformation
lvGetASTCInformation.argtypes = [ctypes.c_char_p]
lvGetASTCInformation.restype = _LvASTCInformation

lvEncodeASTC8 = dll.lvEncodeASTC8
lvEncodeASTC8.argtypes = [uint8_p_3d, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_char_p]
lvEncodeASTC8.restype = ctypes.c_int
lvDecodeASTC8 = dll.lvDecodeASTC8
lvDecodeASTC8.argtypes = [ctypes.c_char_p, uint8_p_3d]
lvDecodeASTC8.restype = ctypes.c_int

lvEncodeASTC16 = dll.lvEncodeASTC16
lvEncodeASTC16.argtypes = [float16_p_3d, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_char_p]
lvEncodeASTC16.restype = ctypes.c_int
lvDecodeASTC16 = dll.lvDecodeASTC16
lvDecodeASTC16.argtypes = [ctypes.c_char_p, float16_p_3d]
lvDecodeASTC16.restype = ctypes.c_int

_blockSizeDict = collections.OrderedDict((
    ((4, 4), 8.0),
    ((5, 4), 6.4),
    ((5, 5), 5.12),
    ((6, 5), 4.27),
    ((6, 6), 3.56),
    ((8, 5), 3.2),
    ((8, 6), 2.67),
    ((10, 5), 2.56),
    ((10, 6), 2.13),
    ((8, 8), 2.0),
    ((10, 8), 1.6),
    ((10, 10), 1.28),
    ((12, 10), 1.07),
    ((12, 12), 0.89),
))

def calcCompressedSize(w, h, blockSize):
    info = _LvASTCInformation()
    info.width = w
    info.height = h
    info.blockWidth, info.blockHeight = blockSize
    outSize = lvGetASTCCompressedSize(ctypes.byref(info))
    assert outSize > 0
    return outSize

def encodeASTC(img, blockSize, level):
    assert blockSize in _blockSizeDict
    h, w, nChannel = img.shape
    assert nChannel < 4 and nChannel > 0
    assert level >= COMPRESSION_VERYFAST and level <= COMPRESSION_EXTERME

    if(img.dtype == np.uint16):
        img = (img.astype(np.float64) / 65535.0).astype(np.float16)
    if(img.dtype != np.uint8 and img.dtype != np.float16):
        raise TypeError("only uint8 and uint16(autocast to float16) and float16 are supported")
    
    if(img.dtype == np.uint8):
        vmax = 255
    else:
        vmax = 1.0

    if(nChannel == 4):
        img = np.require(img, img.dtype, requirements = "C")
    else:
        newImg = np.zeros((h, w, 4), dtype = img.dtype)
        newImg[:,:,3] = vmax
        if(nChannel == 3):
            newImg[:,:,(0, 1, 2)] = img
        elif(nChannel == 2):
            newImg[:,:,(0, 1)] = img
        elif(nChannel == 1):
            newImg[:,:,0] = img[:,:,0]
            newImg[:,:,1] = img[:,:,0]
            newImg[:,:,2] = img[:,:,0]
        img = newImg

    outSize = calcCompressedSize(w, h, blockSize)
    out = ctypes.create_string_buffer(outSize)
    if(img.dtype == np.uint8):
        result = lvEncodeASTC8(img, w, h, blockSize[0], blockSize[1], level, out)
    elif(img.dtype == np.float16):
        result = lvEncodeASTC16(img, w, h, blockSize[0], blockSize[1], level, out)
    else:
        if(img.dtype != np.uint8 and img.dtype != np.float16):
            raise TypeError("only uint8 and uint16 and float16 are supported")
    
    if(result != 0):
        raise RuntimeError("Failed to encode astc(code %d)" % result)

    return out.raw

def decodeASTC(data, nChannel, dtype):
    assert len(data) > 24
    info = lvGetASTCInformation(data)
    h, w = info.height, info.width

    out = np.zeros((h, w, 4), dtype = np.float16)
    result = lvDecodeASTC16(data, out)
    if(result != 0):
        raise RuntimeError("Failed to decode astc(code %d)" % result)
    
    if(dtype == np.uint8):
        out = np.round(out.astype(np.float64) * 255.0).astype(np.uint8)
    elif(dtype == np.uint16):
        out = np.round(out.astype(np.float64) * 65535.0).astype(np.uint16)
    elif(dtype != np.float16):
        raise TypeError("onlu ")

    if(nChannel == 1):
        return out[:,:,0]
    elif(nChannel == 2):
        return out[:,:,(0, 1)]
    elif(nChannel == 3):
        return out[:,:,(0, 1, 2)]
    elif(nChannel == 4):
        return out
    else:
        assert False

def chooseBestBlockSize(bpp):
    bestDst = np.inf
    bestBpp = np.inf
    bestBlockSize = (0, 0)
    for k, v in _blockSizeDict:
        dst = np.abs(v - bpp)
        if(dst < bestDst):
            bestBlockSize = k
            bestBpp = v
            bestDst = dst
    return bestBlockSize, bestBpp

def prevBlockSize(blockSize):
    prevItem = None
    for k in _blockSizeDict:
        if(k == blockSize):
            if(prevItem is None):
                raise StopIteration
            return _blockSizeDict[prevItem]
        prevItem = k
    raise ValueError("invalid blockSize")

def nextBlockSize(blockSize):
    prevItem = None
    for k in reversed(_blockSizeDict):
        if(k == blockSize):
            if(prevItem is None):
                raise StopIteration
            return _blockSizeDict[prevItem]
        prevItem = k
    raise ValueError("invalid blockSize")

