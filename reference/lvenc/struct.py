import ctypes

class MainStruct(ctypes.Structure):
    _fields_ = [
        ("aria", ctypes.c_char * 4),
        ("version", ctypes.c_uint8),
        ("nChannel", ctypes.c_uint8),
        ("flags", ctypes.c_uint8),
        ("framerate", ctypes.c_uint8),
        ("width", ctypes.c_uint32),
        ("height", ctypes.c_uint32),
        ("nFrame", ctypes.c_uint32),
        ("_reserved_0", ctypes.c_char * 4),
        ("userData", ctypes.c_uint64),
    ]

class VideoFrameStruct(ctypes.Structure):
    _fields_ = [
        ("vfrm", ctypes.c_char * 4),
        ("referenceType", ctypes.c_uint8),
        ("intraMethod", ctypes.c_uint8 * 2),
        ("_reserved_0", ctypes.c_char * 5),
        ("size", ctypes.c_uint32),
    ]

class MainStructASTCSeq(ctypes.Structure):
    _fields_ = [
        ("spica", ctypes.c_char * 5),
        ("version", ctypes.c_uint8),
        ("nChannel", ctypes.c_uint8),
        ("framerate", ctypes.c_uint8),
        ("blockSizeMode", ctypes.c_uint8),
        ("_reserved_0", ctypes.c_char * 7),
        ("width", ctypes.c_uint16),
        ("height", ctypes.c_uint16),
        ("nFrame", ctypes.c_uint32),
        ("userData", ctypes.c_uint64),
    ]

class VideoFrameStructASTCSeq(ctypes.Structure):
    _fields_ = [
        ("vfrm", ctypes.c_char * 4),
        ("size", ctypes.c_uint32),
        ("_reserved_0", ctypes.c_char * 8),
    ]

REFERENCE_NONE = 0x0
REFERENCE_PREVFULL = 0x1
REFERENCE_PREV = 0x2
referenceMethodStr = ("REFERENCE_NONE", "REFERENCE_PREVFULL", "REFERENCE_PREV")

FILTER_NONE = 0x00
FILTER_SUBTOP = 0x01
FILTER_SUBLEFT = 0x02
FILTER_SUBAVG = 0x03
FILTER_SUBLEFT_EX2 = 0x12
FILTER_SUBLEFT_EX4 = 0x22
FILTER_SUBLEFT_EX6 = 0x32
FILTER_SUBLEFT_EX8 = 0x42
FILTER_SUBAVG_EX2 = 0x13
FILTER_SUBAVG_EX4 = 0x23
FILTER_SUBAVG_EX6 = 0x33
FILTER_SUBAVG_EX8 = 0x43

intraFilterMethodStr = {
    FILTER_NONE: "FILTER_NONE", 
    FILTER_SUBTOP: "FILTER_SUBTOP", 
    FILTER_SUBLEFT: "FILTER_SUBLEFT",
    FILTER_SUBAVG: "FILTER_SUBAVG",
    FILTER_SUBLEFT_EX2: "FILTER_SUBLEFT_EX2",
    FILTER_SUBLEFT_EX4: "FILTER_SUBLEFT_EX4",
    FILTER_SUBLEFT_EX6: "FILTER_SUBLEFT_EX6",
    FILTER_SUBLEFT_EX8: "FILTER_SUBLEFT_EX8",
    FILTER_SUBAVG_EX2: "FILTER_SUBAVG_EX2",
    FILTER_SUBAVG_EX4: "FILTER_SUBAVG_EX4",
    FILTER_SUBAVG_EX6: "FILTER_SUBAVG_EX6",
    FILTER_SUBAVG_EX8: "FILTER_SUBAVG_EX8",
}

CHANNEL_FULL = False
CHANNEL_HALF = True