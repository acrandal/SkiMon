

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
    currAlt = 0.0
    currLat = 0.0
    currLong = 0.0
    for line in fileinput.input():
        line = line.strip()
        try:
            if(len(line) > 0 and line[0] != '#'):
                parsedLine = json.loads(line)
                #print("json: ", end="")
                #print(parsedLine)
                if(parsedLine["type"] == "environ"):
                    currAlt = parsedLine["altitude"]
                elif(parsedLine["type"] == "GPS"):
                    parsedLine["value"]["altitude"] = currAlt
                    if(currLat != parsedLine["value"]["latitude"] or currLong != parsedLine["value"]["longitude"]):
                        currLat = parsedLine["value"]["latitude"]
                        currLong = parsedLine["value"]["longitude"]
                        print(json.dumps(parsedLine))
                else:
                    pass
                    #print(json.dumps(parsedLine))


            else:
               # print(line)
               pass
        except Exception as e:
            print("# Line parse fail: " + str(e))
            print(" ERR: " + line)
            pass

