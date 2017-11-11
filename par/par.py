from __future__ import division
from PIL import Image
im = Image.open('IMG_20170614_153357464.jpg')
pix = im.load()

r = 0
g = 0
b = 0
res = 0
width, height = im.size
print im.size
for i in range(width):
	for j in range(height):
		_r = pix[i,j][0]
		_g = pix[i,j][1]
		_b = pix[i,j][2]
		
		r += _r
		g += _g
		b += _b

		# avoid division with zero
		res += (_b/(_r+1))

print res/(width*height)
print b/r