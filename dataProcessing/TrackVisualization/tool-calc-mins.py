

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
    minAlt = 1380.35                        # sub this (m)
    startEpoch = 1618088379.5599353         # sub this (secs)
    baseLat = 47.444633333                  # sub this (deg)
    baseLong = -115.72079                   # sub this (deg)
    minLat = 100
    minLong = 0.0

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
                    if minAlt > currAlt:
                        minAlt = currAlt
                elif(parsedLine["type"] == "GPS"):
                    parsedLine["value"]["altitude"] = currAlt
                    if minLat > parsedLine["value"]["latitude"]:
                        minLat = parsedLine["value"]["latitude"]
                    if minLong > parsedLine["value"]["longitude"]:
                        minLong = parsedLine["value"]["longitude"]

                    if(currLat != parsedLine["value"]["latitude"] and currLong != parsedLine["value"]["longitude"]):
                        currLat = parsedLine["value"]["latitude"]
                        currLong = parsedLine["value"]["longitude"]
                        #print(json.dumps(parsedLine))
                else:
                    pass
                    #print(json.dumps(parsedLine))


            else:
                pass
                #print(line)
        except Exception as e:
            print("# Line parse fail: " + str(e))
            pass
    print(minAlt)
    print(minLat)
    print(minLong)

