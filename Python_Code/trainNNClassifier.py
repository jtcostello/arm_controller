
'''
pip install keras
pip install tensorflow
pip install matplotlib
pip install sklearn
'''

import numpy as np
import pandas as pd
import pickle
from matplotlib import pyplot as plt
from sklearn.model_selection import train_test_split
from sklearn.metrics import confusion_matrix
from sklearn.metrics import accuracy_score
from sklearn import decomposition
from mpl_toolkits.mplot3d import Axes3D
from keras.models import Sequential
from keras.layers import Dense
from sklearn.preprocessing import OneHotEncoder
import tensorflow as tf


'''
This script loads in recorded EMG data (from 'recordtrainingdata_fromESP32.py') and trains a neural network classifier.
The network structure is defined in 'make_model()'.
'''


# ---------------------------- Parameters ---------------------------- 
data_fname = 'emgdata11.pkl'                                                # what dataset to load (exported from 'recordtrainingdata.py')
binwidth_samps = 25        # 5 = 100ms,   10 = 200ms,  25 = 500ms           # how many samples per classification bin
classes = [0, 1, 2, 3, 4]
class_labels = ['neutral', 'open', 'thumb pinch', 'fist', 'finger gun']     # (this is optional)

model_save_fname = 'nnmodel01'                                              # where to save the trained model
training_epochs = 500
verbose_training = False

NUM_ELCTRODES = 8
SAMPLING_RATE = 50
# --------------------------------------------------------------------- 





# ---------------------------- Load Data, format, and bin ---------------------------- 

# load training data
with open(data_fname, 'rb') as handle:
    alldata = pickle.load(handle)
    print('Data loaded')

# group data for each class
classdata = []
for c in classes:
    thisclass = []
    for dataset in alldata:
        if dataset[0]==c:
            thisclass += dataset[1] # append
    classdata.append(thisclass)

# form into big array of form  [class timestamp ch1 ... ch8]
datalist = []
for thisclass in classes:
    for sample in classdata[thisclass]:
        datalist.append([thisclass, sample[0]]+sample[1])
datamat = np.array(datalist)

# bin up data
bindata = []
allbindataX = np.zeros((0,8))
allbindataY = np.zeros((0,1))
for classnum in classes:
    thisclassdata = datamat[datamat[:,0]==classnum, :]
    bintimes = np.arange(0, thisclassdata.shape[0]+1, binwidth_samps)
    X = np.zeros((0,8))
    for binidx in range(len(bintimes)-1):
        X = np.vstack((X, np.sum(np.abs(thisclassdata[bintimes[binidx]:bintimes[binidx+1], 2:]), axis=0)))

    Y = classnum * np.ones((X.shape[0],1))
    bindata.append((X,Y))
    allbindataX = np.vstack((allbindataX, X)).astype('float32')
    allbindataY = np.vstack((allbindataY, Y)).astype('float32')

# Run PCA for visualization
pca = decomposition.PCA(n_components=3)
pca.fit(allbindataX)
x_pca = pca.transform(allbindataX)

fig = plt.figure()
ax = Axes3D(fig)
scatter = ax.scatter(x_pca[:, 0], x_pca[:, 1], x_pca[:,2], s=40, c=allbindataY)
ax.legend(handles=scatter.legend_elements()[0], labels=classes)
ax.w_xaxis.set_ticklabels([])
ax.w_yaxis.set_ticklabels([])
ax.w_zaxis.set_ticklabels([])
plt.title('PCA of EMG data')
plt.show(block=True)




# ---------------------------- Train and Test Neural Network ---------------------------- 

# Example references:
# https://github.com/sanchit2843/dlworkshop/blob/master/Lesson%204%20-%20Neuralnetworkkeras.ipynb
# https://machinelearningmastery.com/binary-classification-tutorial-with-the-keras-deep-learning-library/


def plot_confusion_matrix(y_test, y_pred):

    cm = confusion_matrix(y_test, y_pred)
    fig = plt.figure()
    ax = fig.add_subplot(111)
    cax = ax.matshow(cm)
    fig.colorbar(cax)
    ax.set_xticklabels([''] + class_labels)
    ax.set_yticklabels([''] + class_labels)
    ax.xaxis.set_ticks_position('bottom')
    plt.xlabel('Predicted Class')
    plt.ylabel('True Class')
    plt.title('Confusion Matrix')
    plt.show(block=True)


# params
num_classes = len(classes)
input_dim = NUM_ELCTRODES


# define model
def make_model():
    model = Sequential()
    model.add(Dense(50, input_dim=input_dim, activation='relu'))
    model.add(Dense(20, activation='relu'))
    # (add additional layers here...)
    model.add(Dense(num_classes, activation='softmax'))
    model.compile(loss='categorical_crossentropy', optimizer='adam', metrics=['accuracy'])
#     model.summary()   # prints a summary of the layers (optional)
    return model

# train
ohe = OneHotEncoder()
y_onehot = ohe.fit_transform(allbindataY.reshape(-1, 1)).toarray()
X_train, X_test, y_train, y_test = train_test_split(allbindataX, y_onehot, test_size=0.5) #, random_state=0)
model = make_model()
history = model.fit(X_train, y_train, epochs=training_epochs, batch_size=64, verbose=verbose_training)

# evaluate on test set
y_pred = model.predict(X_test)
y_pred_integer = np.argmax(y_pred, axis=1)
y_test_integer = np.argmax(y_test, axis=1)
plot_confusion_matrix(y_test_integer, y_pred_integer)
print('\n----------------- Overall accuracy: ' + str(accuracy_score(y_test_integer, y_pred_integer)) + ' -----------------')

# save model
model.save(model_save_fname)
print(f'----------------- model saved to {model_save_fname} ----------------- ')

input('Press return to exit')