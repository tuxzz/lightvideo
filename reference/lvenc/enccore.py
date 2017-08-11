import numpy as np
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

    del data
    bestTask = sorted((noneTask, topFilteredTask, leftFilteredTask, avgFilteredTask, paethFilteredTask), key = lambda x:len(x.get()))[0]
    bestSize = len(bestTask.get())
    if(minRetSize == -1 or bestSize < minRetSize):
        if(bestTask is noneTask):
            return {
                "filteredChannel": channel.copy(),
                "decompressedChannel": channel.copy(),
                "intraFilterMode": FILTER_NONE,
                "compressedSize": bestSize
            }
        elif(bestTask is topFilteredTask):
            return {
                "filteredChannel": topFilteredChannel,
                "decompressedChannel": cfilter.defilterSubTop(topFilteredChannel),
                "intraFilterMode": FILTER_SUBTOP,
                "compressedSize": bestSize
            }
        elif(bestTask is leftFilteredTask):
            return {
                "filteredChannel": leftFilteredChannel,
                "decompressedChannel": cfilter.defilterSubLeft(leftFilteredChannel),
                "intraFilterMode": FILTER_SUBLEFT,
                "compressedSize": bestSize
            }
        elif(bestTask is avgFilteredTask):
            return {
                "filteredChannel": avgFilteredChannel,
                "decompressedChannel": cfilter.defilterSubAvg(avgFilteredChannel),
                "intraFilterMode": FILTER_SUBAVG,
                "compressedSize": bestSize
            }
        elif(bestTask is paethFilteredTask):
            return {
                "filteredChannel": paethFilteredChannel,
                "decompressedChannel": cfilter.defilterSubPaeth(paethFilteredChannel),
                "intraFilterMode": FILTER_SUBPAETH,
                "compressedSize": bestSize
            }
        else:
            assert False
    else:
        return None

def applyDeltaCompression(channel, refChannel, scale, moveX, moveY, dropThreshold, minRetSize):
    if(scale != 0 or moveX != 0 or moveY != 0):
        refChannel = cresampler.motionResample(refChannel, scale, moveX, moveY)
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

def applyBestDeltaCompressionFullSearch(channel, refChannel, maxScaleRadius, maxMoveRadius, dropThreshold, minRetSize):
    best = None
    for scale in range(-maxScaleRadius, maxScaleRadius + 1):
        for moveY in range(-maxMoveRadius, maxMoveRadius + 1):
            for moveX in range(-maxMoveRadius, maxMoveRadius + 1):
                report.do("FS scale: %d, moveX: %d, moxeY: %d" % (scale, moveX, moveY))
                result = applyDeltaCompression(channel, refChannel, scale, moveX, moveY, dropThreshold, minRetSize)
                if(result is not None):
                    result["scale"] = scale
                    result["moveX"] = moveX
                    result["moveY"] = moveY
                    best = result
                    minRetSize = result["compressedSize"]
                    report.do("  -> Size %d" % (minRetSize))
                del result
    return best

def applyBestFilterOnChannels(currChannelList, prevFullChannelList, prevChannelList, maxScaleRadius, maxMoveRadius, dropThreshold):
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
            if(iChannel == 0):
                result = applyBestDeltaCompressionFullSearch(currChannel, prevFullChannelList[iChannel], maxScaleRadius, maxMoveRadius, dropThreshold, -1)
                scale, moveX, moveY = result["scale"], result["moveX"], result["moveY"]
            else:
                result = applyDeltaCompression(currChannel, prevFullChannelList[iChannel], scale, moveX, moveY, dropThreshold, -1)
            prevFullDeltaSize += result["compressedSize"]
            prevFullDeltaResultList.append(result)
        del iChannel, currChannel, result, scale, moveX, moveY
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
                if(iChannel == 0):
                    result = applyBestDeltaCompressionFullSearch(currChannel, prevChannelList[iChannel], maxScaleRadius, maxMoveRadius, dropThreshold, -1)
                    scale, moveX, moveY = result["scale"], result["moveX"], result["moveY"]
                else:
                    result = applyDeltaCompression(currChannel, prevChannelList[iChannel], scale, moveX, moveY, dropThreshold, -1)
                prevDeltaSize += result["compressedSize"]
                prevDeltaResultList.append(result)
            del iChannel, currChannel, result, scale, moveX, moveY
            report.do("Prev size %d" % (prevDeltaSize))

            if(prevDeltaSize < bestSize):
                bestResultList = prevDeltaResultList
                bestSize = prevDeltaSize
                bestDeltaMode = REFERENCE_PREV
    report.do("Best delta mode is 0x%x" % bestDeltaMode)
    if(bestDeltaMode != REFERENCE_NONE):
        report.do("  Parameter: scale %d, moveX %d, moveY %d" % (bestResultList[0]["scale"], bestResultList[0]["moveX"], bestResultList[0]["moveY"]))
    report.do("Best intra mode is %s" % str([result["intraFilterMode"] for result in bestResultList]))
    return {
        "resultList": bestResultList,
        "deltaMode": bestDeltaMode,
        "totalCompressedSize": bestSize
    }
