#!/usr/bin/env python3

import matplotlib.pyplot as plt
import numpy as np
import fileinput
import os



colorSet = [
    '#a9a9a930',
    '#ff0000',
    '#00ff00',
    '#0000ff',
    '#ffff00',
    '#00ffff',
    'tab:olive',
    'orange',
    'chartreuse',
    'aquamarine',
    'maroon',
    'azure',
    'violet',
    'magenta',
    'tab:blue',
    'tab:brown'
]



if __name__ == "__main__":
    print("Reading in data")
    times = []
    x = []
    y = []
    z = []
    speed = []
    course = []
    runNum = []
    colors = []
    #dwell      # can be size of circle based on timeDelta


    for line in fileinput.input():
        line = line.strip()
        dat = line.split(',')

        if dat[0] == "stamp":
            continue

        #print(dat)
        times.append(dat[1])
        x.append(dat[2])
        y.append(dat[3])
        z.append(dat[4])
        speed.append(dat[5])
        course.append(dat[6])
        runNum.append(dat[7])
        colors.append(colorSet[int(dat[7])])

        # make types from strings in all lists
        times = [float(val) for val in times]
        x = [float(val) for val in x]
        y = [float(val) for val in y]
        z = [float(val) for val in z]
        speed = [float(val) for val in speed]
        course = [float(val) for val in course]
        runNum = [int(val) for val in runNum]




    print("Creating chart")
    plt.style.use('seaborn-whitegrid')

    np.random.seed(19680801)


    area = [5] * len(x)  # 0 to 15 point radii

    plt.scatter(x, y, s=area, c=colors, alpha=0.5)
    plt.show()

    print("Done")
