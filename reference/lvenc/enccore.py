import numpy as np
import scipy.optimize as so
import cv2
from . import cfilter, cresampler, clz4, report
from .struct import *

_LZ4_COMPRESSION_LEVEL = 9

def applyBestIntraCompression(img, dropThreshold, minRetSize):
    data = img.tobytes()
    noneTask = clz4.LZ4CompressionTask(data, clz4.COMPRESS_MODE_HC, _LZ4_COMPRESSION_LEVEL)

    topFiltered = cfilter.filterSubTop(img, dropThreshold)
    data = topFiltered.tobytes()
    topFilteredTask = clz4.LZ4CompressionTask(data, clz4.COMPRESS_MODE_HC, _LZ4_COMPRESSION_LEVEL)

    leftFiltered = cfilter.filterSubLeft(img, dropThreshold)
    data = leftFiltered.tobytes()
    leftFilteredTask = clz4.LZ4CompressionTask(data, clz4.COMPRESS_MODE_HC, _LZ4_COMPRESSION_LEVEL)

    avgFiltered = cfilter.filterSubAvg(img, dropThreshold)
    data = avgFiltered.tobytes()
    avgFilteredTask = clz4.LZ4CompressionTask(data, clz4.COMPRESS_MODE_HC, _LZ4_COMPRESSION_LEVEL)

    paethFiltered = cfilter.filterSubTop(img, dropThreshold)
    data = paethFiltered.tobytes()
    paethFilteredTask = clz4.LZ4CompressionTask(data, clz4.COMPRESS_MODE_HC, _LZ4_COMPRESSION_LEVEL)

    del data
    bestTask = sorted((noneTask, topFilteredTask, leftFilteredTask, avgFilteredTask, paethFilteredTask), key = lambda x:len(x.get()))[0]
    bestSize = len(bestTask.get())
    if(minRetSize == -1 or bestSize < minRetSize):
        if(bestTask is noneTask):
            return {
                "filtered": img.copy(),
                "decompressed": img.copy(),
                "intraMethod": FILTER_NONE,
                "hint": "lossless",
                "compressedSize": bestSize
            }
        elif(bestTask is topFilteredTask):
            return {
                "filtered": topFiltered,
                "decompressed": cfilter.defilterSubTop(topFiltered),
                "intraMethod": FILTER_SUBTOP,
                "hint": "filtered",
                "compressedSize": bestSize
            }
        elif(bestTask is leftFilteredTask):
            return {
                "filtered": leftFiltered,
                "decompressed": cfilter.defilterSubLeft(leftFiltered),
                "intraMethod": FILTER_SUBLEFT,
                "hint": "filtered",
                "compressedSize": bestSize
            }
        elif(bestTask is avgFilteredTask):
            return {
                "filtered": avgFiltered,
                "decompressed": cfilter.defilterSubAvg(avgFiltered),
                "intraMethod": FILTER_SUBAVG,
                "hint": "filtered",
                "compressedSize": bestSize
            }
        elif(bestTask is paethFilteredTask):
            return {
                "filtered": paethFiltered,
                "decompressed": cfilter.defilterSubPaeth(paethFiltered),
                "intraMethod": FILTER_SUBPAETH,
                "hint": "filtered",
                "compressedSize": bestSize
            }
        else:
            assert False
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
    

