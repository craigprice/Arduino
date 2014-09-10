#! python

import win32api
import winsound
import time

for i in range(0,10):
    winsound.Beep(2500,500)
    time.sleep(2)
    