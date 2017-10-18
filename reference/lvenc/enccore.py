import numpy as np
import scipy.optimize as so
import cv2
from . import cfilter, cresampler, clz4, report
from .struct import *

_LZ4_COMPRESSION_LEVEL = 9

def applyBestIntraCompression(img, dropThreshold, minRetSize, fastDecodeMode = 2):
    h, w, nChannel = img.shape
    def _addEx(filterModeList, baseMethod, baseFilter, baseDefilter, mode):
        assert not baseMethod & 0xf0
        EX2, EX4, EX6, EX8 = 0x10, 0x20, 0x30, 0x40
        if(nChannel == 1):
            if(mode < 2):
                if(w % 4 == 0):
                    filterModeList.append((baseMethod | EX4, "filtered_ex", lambda x, d:cfilter.filterEx(baseFilter, x, 4, dropThreshold), lambda x:cfilter.defilterEx(baseDefilter, x, 4)))
                if(w % 6 == 0):
                    filterModeList.append((baseMethod | EX6, "filtered_ex", lambda x, d:cfilter.filterEx(baseFilter, x, 6, dropThreshold), lambda x:cfilter.defilterEx(baseDefilter, x, 6)))
            if(w % 8 == 0):
                filterModeList.append((baseMethod | EX8, "filtered_ex", lambda x, d:cfilter.filterEx(baseFilter, x, 8, dropThreshold), lambda x:cfilter.defilterEx(baseDefilter, x, 8)))
        elif(nChannel == 2):
            if(mode < 2):
                if(w % 2 == 0):
                    filterModeList.append((baseMethod | EX2, "filtered_ex", lambda x, d:cfilter.filterEx(baseFilter, x, 2, dropThreshold), lambda x:cfilter.defilterEx(baseDefilter, x, 2)))
            if(w % 4 == 0):
                filterModeList.append((baseMethod | EX4, "filtered_ex", lambda x, d:cfilter.filterEx(baseFilter, x, 4, dropThreshold), lambda x:cfilter.defilterEx(baseDefilter, x, 4)))
        elif(nChannel == 4 or nChannel == 3):
            if(w % 2 == 0):
                filterModeList.append((baseMethod | EX2, "filtered_ex", lambda x, d:cfilter.filterEx(baseFilter, x, 2, dropThreshold), lambda x:cfilter.defilterEx(baseDefilter, x, 2)))
    
    filterModeList = [
        # intraMethod, hint, filterFunc, defilterFunc
        (FILTER_NONE, "lossless", lambda x, d:x.copy(), lambda x:x.copy()),
        (FILTER_SUBTOP, "filtered", cfilter.filterSubTop, cfilter.defilterSubTop),
        (FILTER_SUBLEFT, "filtered", cfilter.filterSubLeft, cfilter.defilterSubLeft),
    ]
    if(fastDecodeMode < 1):
        filterModeList.append((FILTER_SUBAVG, "filtered", cfilter.filterSubAvg, cfilter.defilterSubAvg))
    _addEx(filterModeList, FILTER_SUBLEFT, cfilter.filterSubLeft, cfilter.defilterSubLeft, 0)
    _addEx(filterModeList, FILTER_SUBAVG, cfilter.filterSubAvg, cfilter.defilterSubAvg, fastDecodeMode)
    resultList = []
    for intraMethod, hint, filterFunc, defilterFunc in filterModeList:
        filtered = filterFunc(img, dropThreshold)
        data = filtered.tobytes()
        task = clz4.LZ4CompressionTask(data, clz4.COMPRESS_MODE_HC, _LZ4_COMPRESSION_LEVEL)
        resultList.append((filtered, data, task, intraMethod, hint, filterFunc, defilterFunc))
        del filtered, data, task
    
    filtered, data, task, intraMethod, hint, filterFunc, defilterFunc = sorted(tuple(x for x in resultList), key = lambda x:len(x[2].get()))[0]
    bestSize = len(task.get())
    if(minRetSize == -1 or bestSize < minRetSize):
        return {
            "filtered": filtered,
            "decompressed": defilterFunc(filtered),
            "intraMethod": intraMethod,
            "hint": hint,
            "compressedSize": bestSize,
        }
    else:
        return None

def applyDeltaCompression(channel, refChannel, dropThreshold, minRetSize):
    if(dropThreshold > 0):
        deltaedChannel = channel.astype(int) - refChannel.astype(int)
        needDrop = np.logical_and(~np.logical_and(channel < dropThreshold, refChannel > dropThreshold), np.abs(deltaedChannel) <= dropThreshold)
        deltaedChannel[needDrop] = 0
        del needDrop
        deltaedChannel = deltaedChannel.astype(channel.dtype)
    else:
        deltaedChannel = channel - refChannel

    intraResult = applyBestIntraCompression(deltaedChannel, 0, minRetSize)
    if(intraResult is not None):
        intraResult["decompressed"] += refChannel
        return intraResult
    else:
        return None

def applyBestFilter(currImgList, prevFullImgList, prevImgList, dropThreshold):
    assert len(currImgList) == 2
    assert prevFullImgList is None or len(prevFullImgList) == 2
    assert prevImgList is None or len(prevImgList) == 2
    assert dropThreshold >= 0
    
    bestResult = []
    bestSize = -1
    bestMethod = REFERENCE_NONE
    # full
    for img in currImgList:
        bestResult.append(applyBestIntraCompression(img, dropThreshold, -1))
        bestSize += bestResult[-1]["compressedSize"]
    report.do("Full: intra %s, size %d" % (str([intraFilterMethodStr[x["intraMethod"]] for x in bestResult]), bestSize))
    
    # prevFull
    if(prevFullImgList is not None):
        resultList = []
        size = 0
        for i, img in enumerate(currImgList):
            resultList.append(applyDeltaCompression(img, prevFullImgList[i], dropThreshold, -1))
            size += resultList[-1]["compressedSize"]
        if(size < bestSize):
            bestResult = resultList
            bestSize = size
            bestMethod = REFERENCE_PREVFULL
        report.do("PrevFull: intra %s, size %d" % (str([intraFilterMethodStr[x["intraMethod"]] for x in resultList]), size))
        del resultList, size
    
    # prev
    if(prevImgList is not None and prevImgList is not prevFullImgList):
        resultList = []
        size = 0
        for i, img in enumerate(currImgList):
            resultList.append(applyDeltaCompression(img, prevImgList[i], dropThreshold, -1))
            size += resultList[-1]["compressedSize"]
        if(size < bestSize):
            bestResult = resultList
            bestSize = size
            bestMethod = REFERENCE_PREV
        report.do("Prev: intra %s, size %d" % (str([intraFilterMethodStr[x["intraMethod"]] for x in resultList]), size))
        del resultList, size
    
    report.do("Best delta method is %s" % (referenceMethodStr[bestMethod]))
    
    return {
        "bestResult": bestResult,
        "bestSize": bestSize,
        "deltaMethod": bestMethod,
    }
    

