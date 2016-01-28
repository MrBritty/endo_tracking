import numpy as np
from skimage import morphology
import time
import cv2
import operator
import matplotlib.pyplot as plt
import sys
import math

import joincontours

# import plotter as plotter1
# del sys.modules['plotter']
# import plotter as plotter2
# del sys.modules['plotter']
# import plotter as plotter3
# del sys.modules['plotter']

cam = cv2.VideoCapture("vid_in_init-still.mpg" )

# cam = cv2.VideoCapture("vid_in_rev.mpg" )

prev_frame_color = cam.read()[1]
prev_frame =  cv2.cvtColor(prev_frame_color, cv2.COLOR_RGB2GRAY)

initF_color = prev_frame_color
initF_hvc = cv2.cvtColor(prev_frame_color, cv2.COLOR_BGR2HSV)


# fourcc = cv2.cv.CV_FOURCC('m', 'p', '4', 'v') # note the lower case
# vout = cv2.VideoWriter()
# vout.open('onlyred.mov',fourcc,15,(854,480),True) 


pointHistory = []
pointsOfRings = []

framRateDelay = 10

while True:
	# time.sleep( 0.5 )

	cur_frame_color = cam.read()[1]
	cur_frame = cv2.cvtColor(cur_frame_color, cv2.COLOR_RGB2GRAY)
	cur_frame_hvc = cv2.cvtColor(cur_frame_color, cv2.COLOR_BGR2HSV) 


	
	d_color =  cv2.absdiff(cur_frame_color, initF_color )
	d_hvc =  cv2.absdiff(cur_frame_hvc, initF_hvc )
	


	onlyred = cv2.inRange( cur_frame_hvc  , np.array([0, 30, 30] , dtype=np.uint8), np.array([45,225,255] , dtype=np.uint8 )  )
	onlyred = cv2.medianBlur(onlyred , 15)
	d_without_red = cv2.bitwise_and(d_color ,  cv2.cvtColor(255-onlyred,cv2.COLOR_GRAY2RGB))
	img_masked_onlyred =   cv2.bitwise_and(cur_frame_color ,  cv2.cvtColor(onlyred,cv2.COLOR_GRAY2RGB)) # main img masked with onlyred

	d_onlyred = cv2.inRange( d_hvc  , np.array([50, 30, 30] , dtype=np.uint8), np.array([100,225,225] , dtype=np.uint8 )  )
	# d_onlyred = cv2.medianBlur(d_onlyred , 15)
	# d_without_red = cv2.bitwise_and(d_without_red ,  cv2.cvtColor(255-d_onlyred,cv2.COLOR_GRAY2RGB))

	d =  cv2.cvtColor(d_without_red, cv2.COLOR_RGB2GRAY) 
	# d= cv2.medianBlur(d , 5)

	ret , dt = cv2.threshold(d,167,255,cv2.THRESH_BINARY)
	dt2 = cv2.adaptiveThreshold(dt,255,cv2.ADAPTIVE_THRESH_MEAN_C,cv2.THRESH_BINARY,11,2)
	edges_stick = cv2.Canny(dt2 ,50,150,apertureSize = 3)
	edges_rings = cv2.Canny(onlyred ,50,150,apertureSize = 3)

	yolo =cv2.cvtColor(edges_rings ,cv2.COLOR_GRAY2RGB)
	yolo[:,:,1] = 0
	yolo2 =cv2.cvtColor(edges_stick ,cv2.COLOR_GRAY2RGB)
	yolo2[:,:,2] = 0
	comb = cv2.add(  cur_frame_color   , yolo)
	comb = cv2.add(  yolo   , yolo2)

	ring_contours,hierarchy = cv2.findContours(edges_rings, 1, 2)


	# now get to ring contorus
	prevPointsOnRing = pointsOfRings

	try:
		nRings = 12
		sortedRings = [ (cv2.contourArea(cnt) , cnt)  for cnt in ring_contours ] 
		sortedRings.sort(key=operator.itemgetter(0))
		sortedRings = sortedRings[::-1]
		sortedRings = sortedRings[:nRings]
		sortedRings = [c[1] for c in sortedRings]
		ring_contours = sortedRings
		print len(ring_contours)

		pointsOfRings = [ ( int(M['m10']/M['m00'] ) , int(M['m01']/M['m00']))  for M in [ cv2.moments(cnt) for cnt in  ring_contours  ]  ]
		print pointsOfRings

		for p in pointsOfRings:
			cv2.circle(cur_frame_color ,( int(p[0]) , int(p[1])  ),10 ,(0,255,255),5)
	except Exception, e:
		print e


	# hammingDist = []



	


	hull = [cv2.convexHull( np.vstack(cnt) ) for cnt in ring_contours]
	cv2.drawContours(cur_frame_color, hull ,-1,(0,0,255),3)


	stick_contours,hierarchy = cv2.findContours(dt, 1, 2)
	# cv2.drawContours(cur_frame_color,stick_contours,-1,(255,0,0),3)

	# cv2.drawContours(cur_frame_color,ring_contours,-1,(0,255,0),3)
	try:
		joined_contours = joincontours.joinContours(stick_contours)
		joined_contours = [cnt for cnt in joined_contours if cv2.contourArea(cnt) > 70 ]
		mxArea , maxAreaCnt = max([ (cv2.contourArea(cnt) , cnt)  for cnt in joined_contours ] , key=operator.itemgetter(0) ) #import operator
		
		#draw rect around maxAreaCnt
		# rectsOnSticks = [  np.int0(cv2.cv.BoxPoints(cv2.minAreaRect(cnt)))  for cnt in [maxAreaCnt] ]
		# cv2.drawContours(cur_frame_color, rectsOnSticks  ,-1,(0,0,255),3)






		

		mnY , mnPoint = max ([   (maxAreaCnt[i][0][1] , maxAreaCnt[i][0])  for i in range(len(maxAreaCnt))   ]  , key=operator.itemgetter(0) )
		lowerMostPoint = mnPoint

		# draw a circle at the f-ing tip
		cv2.circle(cur_frame_color ,( int(lowerMostPoint[0]) , int(lowerMostPoint[1])  ),10 ,(0,255,0),5)

		#pass a line
		[vx,vy,x,y] = cv2.fitLine(maxAreaCnt, cv2.cv.CV_DIST_L2 ,0,0.01,0.01)
		x , y = int(lowerMostPoint[0]) , int(lowerMostPoint[1])
		rows,cols = cur_frame_color.shape[:2]
		lefty = int((-x*vy/vx) + y)
		righty = int(((cols-x)*vy/vx)+y)
		cv2.line(cur_frame_color,(cols-1,righty),(0,lefty),(255,0,0),2)

		lineTheeta = math.degrees(math.atan(   ((vy+0.0)/(vx+0.0))   )) + 0

		if lineTheeta < 0:
			lineTheeta = -1*(90 + lineTheeta)
		else:
			lineTheeta = 90 - lineTheeta

		lineTheeta += 30
		lineTheeta *= 3

		print lineTheeta

		# plotter1.newData( lowerMostPoint[0] )
		# plotter2.newData( lowerMostPoint[1] )
		# plotter3.newData( lineTheeta )

		pointHistory.append((lowerMostPoint[0] , lowerMostPoint[1] ))

		

		

	except Exception, e:
		print e

	
	# drawing the path of the tracked points
	for i in range(len(pointHistory)-1):
		# cv2.line(cur_frame_color, pointHistory[i], pointHistory[i+1] , (0,0,255)  , 3)
		pass


	for cnt in ring_contours:
		try:
			ellipse = cv2.fitEllipse(cnt)
			# cv2.ellipse(cur_frame_color,ellipse,(255,0,0),2)
		except Exception, e:
			print e




	# if len(stick_contours) > 0:
	# 	mxArea , maxAreaCnt = max([ (cv2.contourArea(cnt) , cnt)  for cnt in stick_contours ] , key=operator.itemgetter(0) ) #import operator
		
	# 	hull = cv2.convexHull( np.vstack(stick_contours) )
	# 	# cv2.drawContours(cur_frame_color, hull ,-1,(0,0,255),3)

	# 	rectsOnSticks = [  np.int0(cv2.cv.BoxPoints(cv2.minAreaRect(cnt)))  for cnt in stick_contours ]
	# 	# cv2.drawContours(cur_frame_color, rectsOnSticks  ,-1,(0,0,255),3)

	# 	for cnt in stick_contours:
	# 		try:
	# 			[vx,vy,x,y] = cv2.fitLine(cnt, cv2.cv.CV_DIST_L2 ,0,0.01,0.01)
	# 			rows,cols = cur_frame_color.shape[:2]
	# 			lefty = int((-x*vy/vx) + y)
	# 			righty = int(((cols-x)*vy/vx)+y)
	# 			# cv2.line(cur_frame_color,(cols-1,righty),(0,lefty),(0,255,0),2)
	# 		except  Exception, e:
	# 			print e

		





	cv2.imshow("cur_frame_color", cur_frame_color )
	

	# vout.write(cv2.cvtColor( onlyred,cv2.COLOR_GRAY2RGB))
	cv2.imshow("img_masked_onlyred", img_masked_onlyred )
	# cv2.imshow("d", d )
	# cv2.imshow("edges_stick", edges_stick )
	# cv2.imshow("cur_frame_hvc", cur_frame_hvc )
	# cv2.imshow("onlyred", cv2.cvtColor(255- onlyred,cv2.COLOR_GRAY2RGB) )
	# cv2.imshow("d_without_red", d_without_red )
	# cv2.imshow("comb", comb )
	# vout.write(cur_frame_color) 


	
	k = cv2.waitKey(framRateDelay)
	if k == ord('c'):
		pointHistory = []
	if k == ord('a'):
		framRateDelay = 1500
	if k == ord('b'):
		framRateDelay = 10


		# print 333

	# if key:
	# 	exit()


# vout.release()


# todo drop ring evemt

# kalman filter


	# tld tracking lerning detection jiri