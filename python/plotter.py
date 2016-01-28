#!/usr/bin/env python
# Plot a graph of Data which is comming in on the fly
# uses pylab
# Author: Norbert Feurle
# Date: 12.1.2012
# License: if you get any profit from this then please share it with me
import pylab
from pylab import *
import numpy as np
import matplotlib.pyplot as plt

xAchse=np.arange(0,100,1)
yAchse=np.array([0]*100)

fig = plt.figure(1)
ax = fig.add_subplot(111)
ax.grid(True)
ax.set_title("Realtime Waveform Plot")
ax.set_xlabel("Time")
ax.set_ylabel("Amplitude")
ax.axis([0,100,-1.5,1.5])
line1=ax.plot(xAchse,yAchse,'-')

manager = plt.get_current_fig_manager()

values=[]
values = [0 for x in range(100)]

Ta=0.01
fa=1.0/Ta
fcos=3.5

Konstant=0.6
T0=1.0
T1=Konstant

i = 0

plt.ion()

def RealtimePloter(arg):
  global values
  CurrentXAxis=np.arange(len(values)-100,len(values),1)
  line1[0].set_data(CurrentXAxis,np.array(values[-100:]))
  ax.axis([CurrentXAxis.min(),CurrentXAxis.max(),00,800])
  manager.canvas.draw()
  #manager.show()


def newData( val ):
  global values,T1,Konstant,T0
  #ohmegaCos=arccos(T1)/Ta
  #print "fcos=", ohmegaCos/(2*pi), "Hz"
  global i
  i += 1

  values.append(val )

  RealtimePloter(1)



# timer = fig.canvas.new_timer(interval=20)
# timer.add_callback(RealtimePloter, ())
# timer2 = fig.canvas.new_timer(interval=20)
# timer2.add_callback(SinwaveformGenerator, ())
# timer.start()
# timer2.start()
# print "habba"
plt.show()
# print "babba"