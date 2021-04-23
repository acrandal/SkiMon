

import datetime
import json
import fileinput
import time

def addRun(line, startStops):
    epoch = line["epoch"]
    currRun = 0
    for run in startStops:
        if epoch >= run['start_epoch'] and epoch <= run['stop_epoch']:
            currRun = run['run']
            break
    line['runNum'] = currRun
    return line

if __name__ == '__main__':
    f = open('startStop.json')
    startStops = json.load(f)
    #print(startStops)

    for line in fileinput.input():
        line = line.strip()
        try:
            if(len(line) > 0 and line[0] != '#'):
                parsedLine = json.loads(line)
                #print("json: ", end="")
                #print(parsedLine)
                newLine = addRun(parsedLine, startStops)
                print(json.dumps(newLine))
            else:
                print(line)
        except Exception as e:
            print("# Line parse fail: " + str(e))
            pass

