import cv2
import numpy as np
import os

os.chdir("C:\Codes\ESP32\Fuel_Monitor\py")
img = cv2.imread("schlumberger.jpg")

text = "{"
for row in range(240):
    text += "\n"
    for col in range(320):
        b,g,r = img[row,col]
        
        r,g,b = (r >> 3), (g >> 2), (b >> 3) 
        color = (r << 11) + (g << 5) + b
        
        b2,b1 = (color & 0x00ff) , ((color & 0xff00) >> 8)
        
        text += str(b1) + "," + str(b2) + ","
text = text[:-1] + '};'
with open("img.txt" , 'w') as f:
    f.write(text)
color = (0x8c01)
# color = ((color & 0x00ff) << 8) + ((color & 0xff00) >> 8)
r,g,b = (color >> 11) & 0x1F, (color >> 5) & 0x3F , color & 0x1F
print(r << 3,g << 2,b << 3)

