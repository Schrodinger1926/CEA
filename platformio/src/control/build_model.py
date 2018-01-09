from __future__ import division
import sys
import os
import random
import cPickle as pickle
import numpy as np

from keras.models import Sequential
from keras.layers import Dense
from keras.utils import np_utils



def run_grid_search(max_hidden_units):
	models = []
	for num_hidden_units in range(3, max_hidden_units):
		model = Sequential()

		# Add hidden layer
		model.add(Dense(num_hidden_units, kernel_initializer = 'uniform', activation = 'relu', input_dim = input_feature))

		# Adding the output layer
		model.add(Dense(num_labels, kernel_initializer = 'uniform', activation = 'sigmoid'))

		# Compiling Neural Network
		model.compile(optimizer = 'adam', loss = 'binary_crossentropy', metrics = ['accuracy'])

		model.fit(dataset, label, verbose = 1, validation_split = 0.2)

		score = model.evaluate(valid_dataset, valid_label)

		res.append((model,score[1]))

		# sys.stdout.write('\rProgress: {:05.2f}%'.format((num_hidden_units+1)*100/max_hidden_units))
		# sys.stdout.flush()

	print
	# return model with max score
	return max(models, key = lambda x: x[1])


def main():
	#1. Load data (Randomized pending)
	data_serialized = 'trainDatabase.pickle'
	data = None
	with open(data_serialized, 'rb') as f:
		data = pickle.load(f)

	num_labels = 4

	# extract data
	"""
	Dataset:
	T(t-2), H(t-2), T(t-1), H(t-1), T(t), H(t), T(rf), H(rf), C(t-2), F(t-2), C(t-1), F(t-1)
	
	Labels:
	C(t+1), F(t+1)
	"""
	dataset = data['dataset']
	label = data['label']

	# one-hot encode labels 
	label = np_utils.to_categorical(label, num_labels)
	
	input_feature = dataset.shape[1]
	max_hidden_units = 30	

	#2. build model
	model = run_grid_search(max_hidden_units = max_hidden_units,
							input_feature = input_feature,
							num_labels = num_labels,
							valid_dataset = valid_dataset,
							valid_label = valid_label)


	#3. save model
	model.save('control/control_model.h5')


if __name__ == '__main__':
	main()