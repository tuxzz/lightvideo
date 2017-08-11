import numpy as np
import cv2, pygame
import os
import lvenc, ctypes
import gc, profile, time
from pygame.locals import *

def main():
    dec = lvenc.Decoder(open("out.rcv", "rb"))
    pygame.init()
    screen = pygame.display.set_mode((dec.width, dec.height), HWSURFACE | DOUBLEBUF, 24)
    pygame.display.set_caption("Display")
    surface = pygame.Surface((dec.width, dec.height), flags = HWSURFACE | DOUBLEBUF, depth = 24)
    clock = pygame.time.Clock()
    while(True):
        for event in pygame.event.get():
            if(event.type == QUIT):
                return
            del event
        channelList = dec.getFrame()
        y = channelList[0]
        u = cv2.resize(channelList[1], (dec.width, dec.height), cv2.INTER_LINEAR)
        v = cv2.resize(channelList[2], (dec.width, dec.height), cv2.INTER_LINEAR)
        pygame.surfarray.blit_array(surface, lvenc.cyuv.YUVPToRGBI(y, u, v).transpose(1, 0, 2))
        screen.blit(surface, (0, 0))
        pygame.display.update()
        clock.tick(dec.framerate)
        print(clock.get_fps())

def speedtest():
    dec = lvenc.Decoder(open("out.rcv", "rb"))
    totalTime = dec.nFrame / dec.framerate
    t = time.time()
    try:
        while(True):
            dec.getFrame()
    except EOFError:
        pass
    except Exception as e:
        print(e)
        pass
    t = time.time() - t
    print("Decoded %lfs in %lfs, raito %lfx" % (totalTime, t, totalTime / t))

profile.run("speedtest()")

gc.collect()
dll = ctypes.CDLL("lightvideo-encoder-helper.dll")
lvCheckAllocated = dll.lvCheckAllocated
lvCheckAllocated.argtypes = []
lvCheckAllocated.restype = None
lvCheckAllocated()
