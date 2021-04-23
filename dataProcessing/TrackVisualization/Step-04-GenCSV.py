

import datetime
import json
import fileinput
import time

def printHeader():
    headers = [
        "stamp",
        "time",
        "x",
        "y",
        "z",
        "speed",
        "course",
        "runNum",
        "latitude",
        "longitude",
        "altitude",
        "epoch",
        "timeDelta"
    ]
    line = ",".join(headers)
    print(line)


def printLine(dat):
    #print(dat)
    vals = [
        dat["stamp"],
        round(dat["time"], 3),
        round(dat["coordinate"]["x"], 2),
        round(dat["coordinate"]["y"], 2),
        round(dat["coordinate"]["z"], 2),
        dat["value"]["speed"],
        dat["value"]["course"],
        dat["runNum"],
        dat["value"]["latitude"],
        dat["value"]["longitude"],
        dat["value"]["altitude"],
        dat["epoch"],
        round(dat["timeDelta"], 3),
    ]
    vals = [str(x) for x in vals]
    line = ",".join(vals)
    print(line)
    #print()


if __name__ == '__main__':

    # print out header line
    printHeader()

    for line in fileinput.input():
        line = line.strip()
        try:
            parsedLine = json.loads(line)
            printLine(parsedLine)
        except Exception as e:
            print("# Line parse fail: " + str(e))
            pass

