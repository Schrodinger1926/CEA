import time
import serial
import keras
from keras.models import load_model
import numpy as np

def update_cache(airT, humidity, compressor, airthrower):
	global cache
	# shift elements to left
	i = 0
	while i < cache.shape[1] -1:
		cache[i, :] = cache[i+1, :]
		i += 1
	cache[i, :] = np.array([airT, humidity, compressor, airthrower], dtype = np.float32)



def get_actuator_values():
	# Restructure cache
	x = np.empty((1, 12), dtype = np.float32)

	x[:, :6] = cache[:, :2].reshape((6,))
	x[:, 6:8] = np.array([airT_ref, humidity_ref], dtype = np.float32)
	x[:, 8:] = cache[:2, 2:].reshape((4,))

	# Run through model parameter
	res = model.predict(x) > 0.5

	# Restructure response
	# <code>

	return res[0], res[1]


#1. Open serial communication
ser = serial.Serial('/dev/ttyACM0', 9600)
airT_ref = 25
humidity_ref = 60
cache = np.empty((3,4), dtype = np.float32)

#2. load model
model = load_model('control/control_model.h5')

#2. Process events
while True:
	f = open("data.log", 'a')
	time.sleep(1)
	data = ser.readline()

	# parse data, and send actuator commands
	airT, humidity, compressor, airthrower = parseData(data)

	# update new value
	update_cache(airT, humidity, compressor, airthrower)

	# Run through the model
	compressor_new, airthrower_new = get_actuator_values()

	# Return new compressor and airthrower states
	ser.write('stuff')

	f.write(str(data) + "\r\n")
	f.close()

