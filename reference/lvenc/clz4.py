import ctypes
import numpy as np
import numpy.ctypeslib as npct

class _LZ4CompressionTask(ctypes.Structure):
    pass
_pLZ4CompressionTask = ctypes.POINTER(_LZ4CompressionTask)

dll = ctypes.CDLL("lightvideo-encoder-helper.dll")

COMPRESS_MODE_FAST = 0
COMPRESS_MODE_HC = 1

COMPRESS_LEVEL_HC_MAX = 12

TASK_NOTRUNNED = 0
TASK_RUNNING = 1
TASK_FINISHED = 2

lvCreateLZ4CompressionTask = dll.lvCreateLZ4CompressionTask
lvCreateLZ4CompressionTask.argtypes = [ctypes.c_char_p, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_bool]
lvCreateLZ4CompressionTask.restype = _pLZ4CompressionTask

lvWaitLZ4CompressionTask = dll.lvWaitLZ4CompressionTask
lvWaitLZ4CompressionTask.argtypes = [_pLZ4CompressionTask, ctypes.c_int]
lvWaitLZ4CompressionTask.restype = ctypes.c_int

lvGetLZ4CompressionTaskResultSize = dll.lvGetLZ4CompressionTaskResultSize
lvGetLZ4CompressionTaskResultSize.argtypes = [_pLZ4CompressionTask]
lvGetLZ4CompressionTaskResultSize.restype = ctypes.c_int

lvGetLZ4CompressionTaskResultData = dll.lvGetLZ4CompressionTaskResultData
lvGetLZ4CompressionTaskResultData.argtypes = [_pLZ4CompressionTask, ctypes.c_char_p, ctypes.c_int]
lvGetLZ4CompressionTaskResultData.restype = None

lvGetLZ4CompressionTaskResultAdler32 = dll.lvGetLZ4CompressionTaskResultAdler32
lvGetLZ4CompressionTaskResultAdler32.argtypes = [_pLZ4CompressionTask]
lvGetLZ4CompressionTaskResultAdler32.restype = ctypes.c_uint

lvDestroyLZ4CompressionTask = dll.lvDestroyLZ4CompressionTask
lvDestroyLZ4CompressionTask.argtypes = [_pLZ4CompressionTask]
lvDestroyLZ4CompressionTask.restype = None

lvDecompressLZ4 = dll.lvDecompressLZ4
lvDecompressLZ4.argtypes = [ctypes.c_char_p, ctypes.c_int, ctypes.c_char_p, ctypes.c_int]
lvDecompressLZ4.restype = ctypes.c_int

def decompressLZ4(data, decompressedSize):
    assert decompressedSize > 0
    assert isinstance(data, bytes)
    assert len(data) > 0
    out = ctypes.create_string_buffer(decompressedSize)
    realDecompressedSize = lvDecompressLZ4(data, len(data), out, decompressedSize)
    if(realDecompressedSize != decompressedSize):
        raise ValueError("Invalid decompressedSize(expected %d, got %d)" % (decompressedSize, realDecompressedSize))
    return out.raw

class LZ4CompressionTask:
    def __init__(self, data, mode, level, calcAdler32 = False):
        self.task = None
        if(len(data) <= 0):
            raise ValueError("Data size must be greater than 0.")
        self.task = lvCreateLZ4CompressionTask(data, len(data), mode, level, calcAdler32)
        self.calcAdler32 = calcAdler32
        self._cache = None

    def __del__(self):
        if(self.task is not None):
            lvDestroyLZ4CompressionTask(self.task)
            self.task = None

    def wait(self, time = -1):
        if(self.task is not None):
            result = lvWaitLZ4CompressionTask(self.task, time)
            if(result == TASK_FINISHED):
                self.get()
            return result
        else:
            return TASK_FINISHED

    def get(self):
        if(self._cache is None):
            size = lvGetLZ4CompressionTaskResultSize(self.task)
            if(size == 0):
                raise ValueError("LZ4 compression failed.")
            out = ctypes.create_string_buffer(size)
            lvGetLZ4CompressionTaskResultData(self.task, out, size)
            if(self.calcAdler32):
                self._cache = (out.raw, lvGetLZ4CompressionTaskResultAdler32(self.task))
            else:
                self._cache = out.raw
            if(self.task is not None):
                lvDestroyLZ4CompressionTask(self.task)
                self.task = None
            return self._cache
        else:
            return self._cache
