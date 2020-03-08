#!/bin/python

import sys, getopt, warnings
import numpy as np
import matplotlib.pyplot as plt
from glob import glob
from os.path import basename
from sklearn.svm import SVC
#sklearn_porter includes deprecated code and generates FutureWarnings
with warnings.catch_warnings():
    warnings.filterwarnings("ignore")
    from sklearn_porter import Porter
from sklearn.decomposition import PCA
from mlxtend.plotting import plot_decision_regions

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
    plot_decision_regions(X, y.astype(np.uint8), clf=classifier.fit(X, y), legend=1)
    plt.show()

features, classmap = load_features('dataset/')
X, y = features[:, :-1], features[:, -1]
classifier = SVC(kernel='poly', gamma=(1/(len(X[0])*X.var()))).fit(X, y)
#sklearn_porter includes deprecated code and generates FutureWarnings
with warnings.catch_warnings():
    warnings.filterwarnings("ignore")
    porter = Porter(classifier, language='c')

code = porter.export()[:-259] #strips main() from generated C code
code = ''.join([code,\
                '\nconst char* classIdxToName(uint8_t classIdx)\n',\
                '{\n',\
                '     switch (classIdx)\n',
                '    {\n'])
for x in range(len(classmap)):
    code +=     '        case ' + str(x) + ':\n' +\
                '            return \"' + classmap[x] + '\";\n'
code = ''.join([code,\
                '        default:\n',\
                '            return "UNKNOWN";\n',\
                '    }\n}'])

f = open("model.h", "w")
f.write(code)
f.close()

if len(sys.argv) > 1 and sys.argv[1] == '-p':
    plot_boundaries(classifier, X, y, classmap)
