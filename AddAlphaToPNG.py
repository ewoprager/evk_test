import numpy as np
import sys
import cv2
import matplotlib.pyplot as plt

fileName = sys.argv[1]
img = cv2.imread(fileName)


#print(img.shape)

rgba = cv2.cvtColor(img, cv2.COLOR_RGB2RGBA)

#print(rgba.shape)

# Then assign the mask to the last channel of the image
rgba[:, :, 3] = 255

#print(rgba.shape)
#print(rgba)

#plt.imshow(rgba)
#plt.show()
cv2.imwrite(fileName, rgba)
