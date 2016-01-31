import numpy as np
from skimage import morphology
import time


# import cv2


# cap = cv2.VideoCapture("vid_in.mpg")

# ret, frame1 = cap.read()
# prvs = cv2.cvtColor(frame1,cv2.COLOR_BGR2GRAY)
# hsv = np.zeros_like(frame1)
# hsv[...,1] = 255

# while(1):
#     ret, frame2 = cap.read()
#     next = cv2.cvtColor(frame2,cv2.COLOR_BGR2GRAY)

#     flow = cv2.calcOpticalFlowFarneback(prvs,next, None, 0.5, 3.0, 15.0, 3.0, 5.0, 1.2, 0.0)

#     mag, ang = cv2.cartToPolar(flow[...,0], flow[...,1])
#     hsv[...,0] = ang*180/np.pi/2
#     hsv[...,2] = cv2.normalize(mag,None,0,255,cv2.NORM_MINMAX)
#     bgr = cv2.cvtColor(hsv,cv2.COLOR_HSV2BGR)

#     cv2.imshow('frame2',bgr)
#     k = cv2.waitKey(30) & 0xff
#     if k == 27:
#         break
#     elif k == ord('s'):
#         cv2.imwrite('opticalfb.png',frame2)
#         cv2.imwrite('opticalhsv.png',bgr)
#     prvs = next

# cap.release()
# cv2.destroyAllWindows()


import cv2

def diffImg(t0, t1, t2):
  d1 = cv2.absdiff(t2, t1)
  d2 = cv2.absdiff(t1, t0)
  # return cv2.bitwise_and(d1, d2)
  return d1

# cam = cv2.VideoCapture("vid_in_init-still.mpg" )
cam = cv2.VideoCapture("vid_in_rev.mpg" )


winName = "Movement Indicator"
cv2.namedWindow(winName, cv2.CV_WINDOW_AUTOSIZE)

# Read three images first:
t_minus_color = cam.read()[1]
t_minus =  cv2.cvtColor(t_minus_color, cv2.COLOR_RGB2GRAY)
initF =  t_minus
initF_color = t_minus_color
t =  cv2.cvtColor(cam.read()[1], cv2.COLOR_RGB2GRAY)
t_plus =cv2.cvtColor(cam.read()[1], cv2.COLOR_RGB2GRAY)



while True:
  # time.sleep( 0.1 )
  d =  diffImg(t_minus, initF , t_plus)

  ret , dt = cv2.threshold(d,167,255,cv2.THRESH_BINARY)
  dt2 = cv2.adaptiveThreshold(dt,255,cv2.ADAPTIVE_THRESH_MEAN_C,cv2.THRESH_BINARY,11,2)
  dm = cv2.medianBlur(d,5)
  # cv2.imshow( winName,  dt2 )

  # cv2.imshow( winName+"aaa", 255 -  dt )
  # cv2.imshow( winName+"aasa",  d )
  # cv2.imshow( winName, diffImg(t_minus, t , t_plus) )

  # detector = cv2.SimpleBlobDetector()
  # keypoints = detector.detect(  dt  )
  # im_with_keypoints = cv2.drawKeypoints(d , keypoints, np.array([]), (0,0,255), cv2.DRAW_MATCHES_FLAGS_DRAW_RICH_KEYPOINTS)
  # cv2.imshow("Keypoints", im_with_keypoints)
 

  # Read next image
  t_minus = t
  t = t_plus
  # t_plus =  cv2.cvtColor(cam.read()[1], cv2.COLOR_RGB2GRAY)
  t_plus_color = cam.read()[1]
  t_plus =  cv2.cvtColor(t_plus_color, cv2.COLOR_RGB2GRAY)
  edges = cv2.Canny(dt2 ,50,150,apertureSize = 3)
  edges2 = cv2.Canny(t_plus ,50,150,apertureSize = 3)

  dcolor =  cv2.absdiff(t_plus_color, initF_color )
  dcolor = cv2.cvtColor(dcolor, cv2.COLOR_RGB2GRAY)

  cv2.imshow("edges", edges )
  cv2.imshow("edges2", edges2 )
  cv2.imshow("d", d )
  cv2.imshow("t_plus_color", t_plus_color )
  cv2.imshow("dcolor", dcolor )
  
  yolo =cv2.cvtColor(edges ,cv2.COLOR_GRAY2RGB)
  yolo[:,:,1] = 0
  comb = cv2.add(   cv2.cvtColor(t ,cv2.COLOR_GRAY2RGB)  , yolo)
  

  # cv2.imshow("hhh" , 255-dt2 )


  minLineLength = 100
  maxLineGap = 10
  # skel = morphology.skeletonize((255-dt2) )
  # cv2.imshow( "winName" , skel )

  lines = cv2.HoughLinesP(255-dt2,1,np.pi/180,275, minLineLength = 600, maxLineGap = 100)


  print (lines)

  try:
    for rho,theta in lines[0]:
      a = np.cos(theta)
      b = np.sin(theta)
      x0 = a*rho
      y0 = b*rho
      x1 = int(x0 + 1000*(-b))
      y1 = int(y0 + 1000*(a))
      x2 = int(x0 - 1000*(-b))
      y2 = int(y0 - 1000*(a))

      cv2.line(comb,(x1,y1),(x2,y2),(0,0,255),2)

  except Exception, e:
      print "e"
      print e

  



  cv2.imshow( winName , comb )
  # cv2.imshow( winName + "hohoh" , dt2)

  key = cv2.waitKey(10)
  if key == 27:
    cv2.destroyWindow(winName)
    break

print "Goodbye"

