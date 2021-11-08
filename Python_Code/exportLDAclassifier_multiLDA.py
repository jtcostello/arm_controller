"""
pip install micro-learn
pip install pyperclip
pip install matplotlib
pip install sklearn
"""
import numpy as np
import pandas as pd
import pickle
from matplotlib import pyplot as plt
from sklearn.discriminant_analysis import LinearDiscriminantAnalysis as LDA
from sklearn.model_selection import train_test_split
from sklearn.model_selection import cross_val_score
from sklearn.metrics import confusion_matrix
from sklearn.metrics import accuracy_score
from sklearn.metrics import plot_confusion_matrix
from sklearn import decomposition
from mpl_toolkits.mplot3d import Axes3D
import pyperclip



# ------------ INPUT OPTIONS --------------
data_fname = 'emgdata08.pkl'
binwidth_samps = 100        # 20 = 100ms,   40 = 200ms,  100 = 500ms
classes = [0,1,2,3,4]
class_labels = ['neutral', 'open', 'thumb pinch', 'fist', 'finger gun']
# -----------------------------------------


def export_classifier(model, weightname):
    # account for change in sampliing rate:
    rate_multiplier = 200/50
    weights = model.coef_.T * rate_multiplier
    intercepts = model.intercept_
    # format weights
    weightstr = f'const float weights_{weightname}[{weights.shape[0]}][{weights.shape[1]}] = {{\n'
    for row in weights:
        weightstr += '\t{' + ', '.join([str(x) for x in row]) + '},\n'
    weightstr = weightstr[:-2] + ' };\n\n'
    # format intercepts
    interceptstr = f'const float intercepts_{weightname}[{intercepts.shape[0]}] = {{'\
        + ', '.join([str(x) for x in intercepts]) \
        + '};\n'
    return weightstr+interceptstr



# Load data and format
with open(data_fname, 'rb') as handle:
    alldata = pickle.load(handle)

classdata = []
for c in classes:
    thisclass = []
    for dataset in alldata:
        if dataset[0]==c:
            thisclass += dataset[1] # append
    classdata.append(thisclass)

datalist = []
for thisclass in classes:
    for sample in classdata[thisclass]:
        datalist.append([thisclass, sample[0]]+sample[1])
datamat = np.array(datalist)


# Bin up data
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
    allbindataX = np.vstack((allbindataX, X))
    allbindataY = np.vstack((allbindataY, Y))


# Visualize with PCA
pca = decomposition.PCA(n_components=3)
pca.fit(allbindataX)
x_pca = pca.transform(allbindataX)
fig = plt.figure()
ax = Axes3D(fig)
scatter = ax.scatter(x_pca[:, 0], x_pca[:, 1], x_pca[:,2], s=40, c=allbindataY)
ax.legend(handles=scatter.legend_elements()[0], labels=class_labels)
ax.w_xaxis.set_ticklabels([])
ax.w_yaxis.set_ticklabels([])
ax.w_zaxis.set_ticklabels([])
plt.show()


# --------------- LDA - Neutral ---------------
# get 'neutral' labels
thisY = allbindataY==0

# fit & predict
X_train, X_test, y_train, y_test = train_test_split(allbindataX, np.ravel(thisY), test_size=0.8) #, random_state=0)
model_neutral = LDA()
model_neutral.fit(X_train, y_train)
y_test_hat = model_neutral.predict(X_test)
scores = cross_val_score(model_neutral, allbindataX, np.ravel(thisY), cv=10)
print(f'cross validation average = {np.mean(scores)}')

# fit final model using all available data
model_neutral.fit(allbindataX, np.ravel(thisY))


# --------------- LDA - Pinch ---------------
# get 'pinch' labels
# closed: (2,3)  open: (1,4)  neutral: (0)
tempX = allbindataX[np.ravel(allbindataY)!=0,:]
tempY = np.ravel( allbindataY[np.ravel(allbindataY)!=0,:] )
thisY = np.logical_or(tempY==2, tempY==3)

# fit & predict
X_train, X_test, y_train, y_test = train_test_split(tempX, thisY, test_size=0.8) #, random_state=0)
model_pinch = LDA()
model_pinch.fit(X_train, y_train)
y_test_hat = model_pinch.predict(X_test)
scores = cross_val_score(model_pinch, tempX, thisY, cv=10)
print(f'cross validation average = {np.mean(scores)}')

# fit final model using all available data
model_pinch.fit(tempX, thisY)


# --------------- LDA - MRP ---------------
# get 'mrp' labels
# closed: (3,4)  open: (1,2)  neutral: (0)
tempX = allbindataX[np.ravel(allbindataY)!=0,:]
tempY = np.ravel( allbindataY[np.ravel(allbindataY)!=0,:] )
thisY = np.logical_or(tempY==3, tempY==4)

# fit & predict
X_train, X_test, y_train, y_test = train_test_split(tempX, thisY, test_size=0.8) #, random_state=0)
model_mrp = LDA()
model_mrp.fit(X_train, y_train)
y_test_hat = model_mrp.predict(X_test)
scores = cross_val_score(model_mrp, tempX, thisY, cv=10)
print(f'cross validation average = {np.mean(scores)}')

# fit final model using all available data
model_mrp.fit(tempX, thisY)





# Export Weights (copy to clipboard)
print('\n\n')
w1 = export_classifier(model_neutral, 'neutral')
w2 = export_classifier(model_pinch, 'pinch')
w3 = export_classifier(model_mrp, 'mrp')
allweights = w1 + w2 + w3
pyperclip.copy(allweights)
print(allweights)
print('\n--- weights copied to clipboard ---')