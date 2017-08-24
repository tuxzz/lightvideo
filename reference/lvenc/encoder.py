import numpy as np
import ctypes, io, struct
from .struct import *
from . import enccore, clz4, report
import signal, threading, multiprocessing, time

_LZ4_COMPRESSION_LEVEL = 11

class DelayedKeyboardInterrupt(object):
    def __enter__(self):
        self.signal_received = False
        self.old_handler = signal.getsignal(signal.SIGINT)
        signal.signal(signal.SIGINT, self.handler)

    def handler(self, sig, frame):
        self.signal_received = (sig, frame)

    def __exit__(self, type, value, traceback):
        if(self.old_handler is not None):
            signal.signal(signal.SIGINT, self.old_handler)
            if(self.signal_received):
                self.old_handler(*self.signal_received)

class Encoder:
    def __init__(self, stream, width, height, framerate, colorFormat, **kwargs):
        self.stream = stream
        self.stream.seek(ctypes.sizeof(MainStruct))

        self.width = int(width)
        self.height = int(height)
        self.framerate = int(framerate)
        self.colorFormat = colorFormat

        minDim = min(width, height)
        self.maxPacketSize = int(kwargs.get("maxPacketSize", max(1, int(np.ceil(framerate / 10)))))
        self.dropThreshold = int(kwargs.get("dropThreshold", 1 if colorFormat in COLOR_LDR else 128))

        wInvalid = self.width <= 0 or self.width > 32767
        hInvalid = self.height <= 0 or self.height > 32767
        if(wInvalid or hInvalid):
            raise ValueError("weight and height must be in range [1, 32767]")

        if(self.framerate <= 0 or self.framerate > 255):
            raise ValueError("framerate must be in range [1, 255]")
        if(not (self.colorFormat in COLOR_LDR or self.colorFormat in COLOR_HDR)):
            raise ValueError("Invalid colorFormat %s" % (str(self.colorFormat)))

        if(self.maxPacketSize <= 0 or self.maxPacketSize > 255):
            raise ValueError("maxPacketSize must be in range [1, 255].")

        if(colorFormat in COLOR_LDR and self.dropThreshold > 255):
            raise ValueError("dropThreshold for 8bit color format must be less than 256.")
        elif(self.dropThreshold > 65535):
            raise ValueError("dropThreshold for 16bit color format must be less than 65536.")

        if(self.colorFormat == COLOR_YUV420P):
            self.channelShape = ((height, width), (height // 2, width // 2), (height // 2, width // 2))
        elif(self.colorFormat == COLOR_YUVA420P):
            self.channelShape = ((height, width), (height // 2, width // 2), (height // 2, width // 2), (height, width))
        self.prevChannelList = None
        self.prevFullChannelList = None

        self.packetBuffer = io.BytesIO()
        self.onBufferFrameCount = 0
        self.onBufferFullFrameCount = 0

        self.frameCount = 0
        self.flushQueue = []
        self.maxFlushQueueSize = multiprocessing.cpu_count() * 2

    def __enter__(self):
        return self
    def __exit__(self, type, value, trace):
        with DelayedKeyboardInterrupt():
            self.flush(True)
            self.stream.seek(0)

            mainStruct = MainStruct()
            mainStruct.aria = b'ARiA'
            mainStruct.version = 0
            mainStruct.colorFormat = self.colorFormat
            mainStruct.framerate = self.framerate
            mainStruct.maxPacketSize = self.maxPacketSize
            mainStruct.width = self.width
            mainStruct.height = self.height
            mainStruct.nFrame = self.frameCount
            self.stream.write(mainStruct)
            self.stream.seek(0, io.SEEK_END)
            self.stream.flush()

    def __del__(self):
        pass

    def flush(self, force = False):
        with DelayedKeyboardInterrupt():
            if(self.onBufferFrameCount > 0):
                report.enter("async flush")
                uncompressedData = self.packetBuffer.getvalue()
                uncompressedDataSize = len(uncompressedData)
                task = clz4.LZ4CompressionTask(uncompressedData, clz4.COMPRESS_MODE_HC, _LZ4_COMPRESSION_LEVEL, calcAdler32 = True)
                del uncompressedData

                videoPacket = VideoFramePacket()
                videoPacket.vfpk = b'VFPK'
                videoPacket.nFrame = self.onBufferFrameCount
                videoPacket.nFullrame = self.onBufferFullFrameCount
                videoPacket.compressionMethod = COMPRESSION_LZ4

                report.do("Enqueued a packet with %d full frame in %d frame" % (self.onBufferFrameCount, self.onBufferFullFrameCount))
                self.packetBuffer = io.BytesIO()
                self.onBufferFrameCount = 0
                self.onBufferFullFrameCount = 0

                self.flushQueue.append((videoPacket, task, uncompressedDataSize))
                report.leave()

            report.enter("flush write back")
            while(len(self.flushQueue)):
                videoPacket, task, uncompressedDataSize = self.flushQueue[0]
                if(len(self.flushQueue) < self.maxFlushQueueSize and (not force)):
                    result = task.wait(0)
                    if(result != clz4.TASK_FINISHED):
                        break
                compressedData, adler32 = task.get()
                videoPacket.size = len(compressedData)
                videoPacket.checksum = adler32
                self.stream.write(videoPacket)
                self.stream.write(compressedData)
                report.do("%d -> %d" % (uncompressedDataSize, len(compressedData)))
                del self.flushQueue[0], videoPacket, task, uncompressedDataSize
            report.leave()

    def feedFrame(self, channelList):
        report.enter("frame %d" % self.frameCount)
        channelList = list(channelList)
        if(len(channelList) != len(self.channelShape)):
            raise ValueError("Invalid channel count, excepted %d but got %d" % (self.channelCount, len(channelList)))
        for i, currChannel in enumerate(channelList):
            if(currChannel.shape != self.channelShape[i]):
                raise ValueError("Invalid channel shape for channel %d, excepted %s but got %s" % (i, self.channelShape[i], currChannel.shape))
        del i, currChannel

        # do filter
        result = enccore.applyBestFilterOnChannels(channelList, self.prevFullChannelList, self.prevChannelList, self.dropThreshold)
        channelResultList = result["resultList"]

        # send to packet buffer
        videoFrame = VideoFrameStruct()
        videoFrame.vfrm = b'VFRM'
        videoFrame.referenceType = result["deltaMode"]
        with DelayedKeyboardInterrupt():
            if(result["deltaMode"] == REFERENCE_NONE):
                self.onBufferFullFrameCount += 1
            self.onBufferFrameCount += 1
            self.frameCount += 1
            intraPredictModeList = []
            for i, channelResult in enumerate(channelResultList):
                videoFrame.intraPredictModeList[i] = channelResult["intraFilterMode"]
            self.packetBuffer.write(videoFrame)

        self.prevChannelList = []
        for i, channelResult in enumerate(channelResultList):
            self.packetBuffer.write(channelResult["filteredChannel"])
            self.prevChannelList.append(channelResult["decompressedChannel"])
        if(result["deltaMode"] == REFERENCE_NONE):
            self.prevFullChannelList = self.prevChannelList

        del result
        report.leave()

        # check and write
        if(self.onBufferFrameCount >= self.maxPacketSize):
            self.flush()
