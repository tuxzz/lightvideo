import ctypes

class MainStruct(ctypes.Structure):
    _fields_ = [
        ("aria", ctypes.c_char * 4),
        ("version", ctypes.c_ubyte),
        ("colorFormat", ctypes.c_ubyte),
        ("framerate", ctypes.c_ubyte),
        ("maxPacketSize", ctypes.c_ubyte),
        ("_reserved_1", ctypes.c_char * 8),
        ("width", ctypes.c_uint32),
        ("height", ctypes.c_uint32),
        ("nFrame", ctypes.c_uint32),
        ("_reserved_2", ctypes.c_char * 4),
    ]

class VideoFramePacket(ctypes.Structure):
    _fields_ = [
        ("vfpk", ctypes.c_char * 4),
        ("nFrame", ctypes.c_ubyte),
        ("nFullFrame", ctypes.c_ubyte),
        ("compressionMethod", ctypes.c_ubyte),
        ("_reserved_0", ctypes.c_ubyte),
        ("size", ctypes.c_uint32),
        ("checksum", ctypes.c_uint32),
    ]

class VideoFrameStruct(ctypes.Structure):
    _fields_ = [
        ("vfrm", ctypes.c_char * 4),
        ("referenceType", ctypes.c_ubyte),
        ("_reserved_0", ctypes.c_char * 9),
        ("intraPredictModeList", ctypes.c_ubyte * 8),
        ("_reserved_1", ctypes.c_char * 10),
    ]

COLOR_YUV420P = 0x0
COLOR_YUVA420P = 0x1

COLOR_LDR = (COLOR_YUV420P, COLOR_YUVA420P)
COLOR_HDR = tuple()
COLOR_CHANNELCOUNT = (3, 4)

COMPRESS_NONE = 0x0
COMPRESS_LZ4 = 0x1

REFERENCE_NONE = 0x0
REFERENCE_PREVFULL = 0x1
REFERENCE_PREV = 0x2

FILTER_NONE = 0x0
FILTER_SUBTOP = 0x1
FILTER_SUBLEFT = 0x2
FILTER_SUBAVG = 0x3
FILTER_SUBPAETH = 0x4

COMPRESSION_NONE = 0x0
COMPRESSION_LZ4 = 0x1
