import numpy as np
import scipy.optimize as so
import cv2
from . import cfilter, cresampler, clz4, report
from .struct import *

_LZ4_COMPRESSION_LEVEL = 9

def applyBestIntraCompression(channel, dropThreshold, minRetSize):
    data = channel.tobytes()
    noneTask = clz4.LZ4CompressionTask(data, clz4.COMPRESS_MODE_HC, _LZ4_COMPRESSION_LEVEL)

    topFilteredChannel = cfilter.filterSubTop(channel, dropThreshold)
    data = topFilteredChannel.tobytes()
    topFilteredTask = clz4.LZ4CompressionTask(data, clz4.COMPRESS_MODE_HC, _LZ4_COMPRESSION_LEVEL)

    leftFilteredChannel = cfilter.filterSubLeft(channel, dropThreshold)
    data = leftFilteredChannel.tobytes()
    leftFilteredTask = clz4.LZ4CompressionTask(data, clz4.COMPRESS_MODE_HC, _LZ4_COMPRESSION_LEVEL)

    avgFilteredChannel = cfilter.filterSubAvg(channel, dropThreshold)
    data = avgFilteredChannel.tobytes()
    avgFilteredTask = clz4.LZ4CompressionTask(data, clz4.COMPRESS_MODE_HC, _LZ4_COMPRESSION_LEVEL)

    paethFilteredChannel = cfilter.filterSubTop(channel, dropThreshold)
    data = paethFilteredChannel.tobytes()
    paethFilteredTask = clz4.LZ4CompressionTask(data, clz4.COMPRESS_MODE_HC, _LZ4_COMPRESSION_LEVEL)

    if(dropThreshold > 0):
        topDroppedChannel = cfilter.defilterSubTop(topFilteredChannel)
        data = topDroppedChannel.tobytes()
        topDroppedTask = clz4.LZ4CompressionTask(data, clz4.COMPRESS_MODE_HC, _LZ4_COMPRESSION_LEVEL)

        leftDroppedChannel = cfilter.defilterSubLeft(leftFilteredChannel)
        data = leftDroppedChannel.tobytes()
        leftDroppedTask = clz4.LZ4CompressionTask(data, clz4.COMPRESS_MODE_HC, _LZ4_COMPRESSION_LEVEL)

        avgDroppedChannel = cfilter.defilterSubAvg(avgFilteredChannel)
        data = avgDroppedChannel.tobytes()
        avgDroppedTask = clz4.LZ4CompressionTask(data, clz4.COMPRESS_MODE_HC, _LZ4_COMPRESSION_LEVEL)

        paethDroppedChannel = cfilter.defilterSubPaeth(paethFilteredChannel)
        data = paethDroppedChannel.tobytes()
        paethDroppedTask = clz4.LZ4CompressionTask(data, clz4.COMPRESS_MODE_HC, _LZ4_COMPRESSION_LEVEL)

    del data
    if(dropThreshold > 0):
        bestTask = sorted((noneTask, topFilteredTask, leftFilteredTask, avgFilteredTask, paethFilteredTask, topDroppedTask, leftDroppedTask, avgDroppedTask, paethDroppedTask), key = lambda x:len(x.get()))[0]
    else:
        bestTask = sorted((noneTask, topFilteredTask, leftFilteredTask, avgFilteredTask, paethFilteredTask), key = lambda x:len(x.get()))[0]
    bestSize = len(bestTask.get())
    if(minRetSize == -1 or bestSize < minRetSize):
        if(bestTask is noneTask):
            return {
                "filteredChannel": channel.copy(),
                "decompressedChannel": channel.copy(),
                "intraFilterMode": FILTER_NONE,
                "hint": "lossless",
                "compressedSize": bestSize
            }
        elif(bestTask is topFilteredTask):
            return {
                "filteredChannel": topFilteredChannel,
                "decompressedChannel": cfilter.defilterSubTop(topFilteredChannel),
                "intraFilterMode": FILTER_SUBTOP,
                "hint": "filtered",
                "compressedSize": bestSize
            }
        elif(bestTask is leftFilteredTask):
            return {
                "filteredChannel": leftFilteredChannel,
                "decompressedChannel": cfilter.defilterSubLeft(leftFilteredChannel),
                "intraFilterMode": FILTER_SUBLEFT,
                "hint": "filtered",
                "compressedSize": bestSize
            }
        elif(bestTask is avgFilteredTask):
            return {
                "filteredChannel": avgFilteredChannel,
                "decompressedChannel": cfilter.defilterSubAvg(avgFilteredChannel),
                "intraFilterMode": FILTER_SUBAVG,
                "hint": "filtered",
                "compressedSize": bestSize
            }
        elif(bestTask is paethFilteredTask):
            return {
                "filteredChannel": paethFilteredChannel,
                "decompressedChannel": cfilter.defilterSubPaeth(paethFilteredChannel),
                "intraFilterMode": FILTER_SUBPAETH,
                "hint": "filtered",
                "compressedSize": bestSize
            }
        elif(dropThreshold > 0):
            if(bestTask is topDroppedTask):
                return {
                    "filteredChannel": topDroppedChannel,
                    "decompressedChannel": topDroppedChannel.copy(),
                    "intraFilterMode": FILTER_NONE,
                    "hint": "dropped",
                    "compressedSize": bestSize
                }
            elif(bestTask is leftDroppedTask):
                return {
                    "filteredChannel": leftDroppedChannel,
                    "decompressedChannel": leftDroppedChannel.copy(),
                    "intraFilterMode": FILTER_NONE,
                    "hint": "dropped",
                    "compressedSize": bestSize
                }
            elif(bestTask is avgDroppedTask):
                return {
                    "filteredChannel": avgDroppedChannel,
                    "decompressedChannel": avgDroppedChannel.copy(),
                    "intraFilterMode": FILTER_NONE,
                    "hint": "dropped",
                    "compressedSize": bestSize
                }
            elif(bestTask is paethDroppedTask):
                return {
                    "filteredChannel": paethDroppedChannel,
                    "decompressedChannel": paethDroppedChannel.copy(),
                    "intraFilterMode": FILTER_NONE,
                    "hint": "dropped",
                    "compressedSize": bestSize
                }
            else:
                assert False
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
        intraResult["decompressedChannel"] += refChannel
        return intraResult
    else:
        return None

def applyBestFilterOnChannels(currChannelList, prevFullChannelList, prevChannelList, dropThreshold):
    assert len(currChannelList) > 0
    if(prevChannelList):
        assert prevFullChannelList is not None
        assert len(currChannelList) == len(prevFullChannelList)
        assert len(currChannelList) == len(prevChannelList)

    # full
    intraResultList = []
    intraSize = 0
    for currChannel in currChannelList:
        result = applyBestIntraCompression(currChannel, dropThreshold, -1)
        intraSize += result["compressedSize"]
        intraResultList.append(result)
    del currChannel, result
    report.do("Intra size %d" % (intraSize))
    bestResultList = intraResultList
    bestSize = intraSize
    bestDeltaMode = REFERENCE_NONE

    if(prevChannelList is not None):
        # prevFull
        prevFullDeltaResultList = []
        prevFullDeltaSize = 0

        for iChannel, currChannel in enumerate(currChannelList):
            result = applyDeltaCompression(currChannel, prevFullChannelList[iChannel], dropThreshold, -1)
            prevFullDeltaSize += result["compressedSize"]
            prevFullDeltaResultList.append(result)
        del iChannel, currChannel, result
        report.do("PrevFull size %d" % (prevFullDeltaSize))

        if(prevFullDeltaSize < bestSize):
            bestResultList = prevFullDeltaResultList
            bestSize = prevFullDeltaSize
            bestDeltaMode = REFERENCE_PREVFULL

        if(prevChannelList is not prevFullChannelList):
            # prev
            prevDeltaResultList = []
            prevDeltaSize = 0

            for iChannel, currChannel in enumerate(currChannelList):
                result = applyDeltaCompression(currChannel, prevFullChannelList[iChannel], dropThreshold, -1)
                prevDeltaSize += result["compressedSize"]
                prevDeltaResultList.append(result)
            del iChannel, currChannel, result
            report.do("Prev size %d" % (prevDeltaSize))

            if(prevDeltaSize < bestSize):
                bestResultList = prevDeltaResultList
                bestSize = prevDeltaSize
                bestDeltaMode = REFERENCE_PREV
    report.do("Best delta mode is 0x%x" % bestDeltaMode)
    if(bestDeltaMode == REFERENCE_NONE):
        report.do("  Hint: %s" % (bestResultList[0]["hint"]))
    report.do("Best intra mode is %s" % str([result["intraFilterMode"] for result in bestResultList]))
    return {
        "resultList": bestResultList,
        "deltaMode": bestDeltaMode,
        "totalCompressedSize": bestSize
    }
