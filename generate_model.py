#!/bin/python

import numpy as np
from glob import glob
from os.path import basename
from sklearn.svm import SVC
from sklearn_porter import Porter
import matplotlib.pyplot as plt
from sklearn.decomposition import PCA
from mlxtend.plotting import plot_decision_regions
import sys, getopt

def load_features(folder):
    dataset = None
    classmap = {}
    for class_idx, filename in enumerate(glob('%s/*.csv' % folder)):
        class_name = basename(filename)[:-4]
        classmap[class_idx] = class_name
        samples = np.loadtxt(filename, dtype=float, delimiter=',')
        lables = np.ones((len(samples), 1)) * class_idx
        samples = np.hstack((samples, lables))
        dataset = samples if dataset is None else np.vstack((dataset, samples))
    return dataset, classmap

def plot_boundaries(classifier, X, y, classmap=None):
    labels = None
    if classmap is not None:
        labels = classmap.values()
    X = PCA(n_components=2).fit_transform(X)
    plot_decision_regions(X, y.astype(np.uint8),
                          clf=classifier.fit(X, y),
                          legend=1)
    plt.show()

features, classmap = load_features('dataset/')
print(classmap)
X, y = features[:, :-1], features[:, -1]
classifier = SVC(kernel='poly', gamma=(1/(4*X.var()))).fit(X, y)
#classifier = SVC(kernel='poly', gamma=1).fit(X, y)
porter = Porter(classifier, language='c')
f = open("model.h", "w")
f.write(porter.export()[:-259])
f.close()

if len(sys.argv) > 1 and sys.argv[1] == '-p':
    plot_boundaries(classifier, X, y, classmap)
