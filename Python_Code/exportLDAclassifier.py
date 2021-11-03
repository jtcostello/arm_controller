
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
data_fname = 'emgdata06.pkl'
binwidth_samps = 160        # 20 = 100ms,   40 = 200ms,  100 = 500ms
# -----------------------------------------



# Load data and format
with open(data_fname, 'rb') as handle:
    alldata = pickle.load(handle)

classdata = []
classes = [0,1,2,3]
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
ax.legend(handles=scatter.legend_elements()[0], labels=[0,1,2,3])
ax.w_xaxis.set_ticklabels([])
ax.w_yaxis.set_ticklabels([])
ax.w_zaxis.set_ticklabels([])
plt.show()


# Train LDA
X_train, X_test, y_train, y_test = train_test_split(allbindataX, np.ravel(allbindataY), test_size=0.8) #, random_state=0)
model = LDA()
model.fit(X_train, y_train)
scores = cross_val_score(model, allbindataX, np.ravel(allbindataY), cv=10)
print(f'cross validation average = {np.mean(scores)}')


# Export Weights (copied to clipboard)
rate_multiplier = 200/50                    # account for change in sampling rate:
weights = model.coef_.T * rate_multiplier
intercepts = model.intercept_

weightstr = f'const float weights[{weights.shape[0]}][{weights.shape[1]}] = {{\n'
for row in weights:
    weightstr += '\t{' + ', '.join([str(x) for x in row]) + '},\n'
weightstr = weightstr[:-2] + ' };\n\n'

interceptstr = f'const float intercepts[{intercepts.shape[0]}] = {{'\
    + ', '.join([str(x) for x in intercepts]) \
    + '};'

# print(weightstr+interceptstr)
pyperclip.copy(weightstr+interceptstr)
print('--- weights copied to clipboard ---')