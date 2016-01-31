# // https://github.com/jayrambhia/MFTracker

# from mftracker import *
import cv2
# _, img = cap.read()
# _, img = cap.read()

bb = [420,470,36,130]
# cv2.imshow("image", img)
# cv2.waitKey(0)
# mftrack()
# # mftrack("vid_in.mpg" , bb)


import mftracker
mftracker.mftrack("onlyred.mov"  )