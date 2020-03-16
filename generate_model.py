#!/bin/python

import sys, getopt, re
import numpy as np
import matplotlib.pyplot as plt
from glob import glob
from os.path import basename
from sklearn.svm import SVC
from sklearn.model_selection import train_test_split
from sklearn import metrics
from sklearn.decomposition import PCA
from mlxtend.plotting import plot_decision_regions
from micromlgen import port
from sklearn.impute import SimpleImputer

def load_features(folder):
    dataset = None
    classmap = {}
    for class_idx, filename in enumerate(glob('%s/*.csv' % folder)):
        class_name = basename(filename)[:-4]
        classmap[class_idx] = class_name
        samples = np.loadtxt(filename, dtype=float, delimiter=',')
        imp = SimpleImputer(missing_values=-100.00, strategy='mean')
        imp.fit(samples)
        samples = imp.transform(samples)
        lables = np.ones((len(samples), 1)) * class_idx
        samples = np.hstack((samples, lables))
        dataset = samples if dataset is None else np.vstack((dataset, samples))
    return dataset, classmap

def plot_boundaries(classifier, X, y, classmap=None):
    labels = None
    if classmap is not None:
        labels = classmap.values()
    X = PCA(n_components=2).fit_transform(X)
    ax = plot_decision_regions(X, y.astype(np.uint8), clf=classifier.fit(X, y), legend=0)
    handles, oldlabels = ax.get_legend_handles_labels()
    ax.legend(handles, labels, framealpha=0.3, scatterpoints=1)
    plt.show()

features, classmap = load_features('dataset/')
if (len(sys.argv) > 1 and sys.argv[1] == '-t') or (len(sys.argv) > 2 and sys.argv[2] == '-t'):
    X, X_test, y, y_test = train_test_split(features[:, :-1], features[:, -1], test_size = 0.3)
else:
    X, y = features[:, :-1], features[:, -1]


#classifier = SVC(kernel='linear', gamma=1.0).fit(X, y)
#classifier = SVC(kernel='rbf', gamma=(1/(len(X[0])*X.var()))).fit(X, y)
#classifier = SVC(kernel='rbf', gamma=(0.001)).fit(X, y)
classifier = SVC(kernel='rbf', gamma=(0.001), C=10).fit(X, y)
#classifier = SVC(kernel='poly', gamma=(1/(len(X[0])*X.var()))).fit(X, y)

code = port(classifier, classmap=classmap)
code_list = code.splitlines(True)
code_list.insert(1,'#include <stdarg.h>\n#include <stdint.h>\n')
kernel_count = re.split('\[|\]',code_list[19].split()[1])[1]
decision_count = re.split('\[|\]',code_list[20].split()[1])[1]
code_list[19] = '    double* kernels = (double*)calloc(' + kernel_count + ', sizeof(double));\n'
code_list[20] = '    double* decisions = (double*)calloc(' + decision_count + ', sizeof(double));\n'
code = ''.join(code_list)

f = open("model.h", "w")
f.write(code)
f.close()

if (len(sys.argv) > 1 and sys.argv[1] == '-t') or (len(sys.argv) > 2 and sys.argv[2] == '-t'):
    y_pred = classifier.predict(X_test)
    print("Accuracy:", metrics.accuracy_score(y_test, y_pred))

if (len(sys.argv) > 1 and sys.argv[1] == '-p') or (len(sys.argv) > 2 and sys.argv[2] == '-p'):
    plot_boundaries(classifier, X, y, classmap)
