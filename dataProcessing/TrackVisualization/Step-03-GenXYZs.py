

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
    minAlt = 1262.71                        # sub this (m)
    startEpoch = 1618688205.1110632         # sub this (secs)
    baseLat = 47.4446                       # sub this (deg)
    baseLong = -115.72079                   # sub this (deg)
    GPSDegree = 111122.19769899677          # meters per degree (111 kilometers)
    lastTime = 0.0

    for line in fileinput.input():
        line = line.strip()
        try:
            parsedLine = json.loads(line)
            if(parsedLine["type"] == "GPS"):
                x = 0                       # longitude
                y = 0                       # latitude
                z = 0                       # altitude
                secs = 0                    # time
                runNum = 0                  # which run we're on

                loc = {
                    "coordinate": {
                        'x': GPSDegree * (parsedLine["value"]["longitude"] - baseLong),
                        'y': GPSDegree * (parsedLine["value"]["latitude"] - baseLat),
                        'z': parsedLine["value"]["altitude"] - minAlt
                    },
                    'time': parsedLine["epoch"] - startEpoch,
                    'runNum': parsedLine["runNum"],
                    'speed': parsedLine["value"]["speed"]
                }

                if("course" in parsedLine["value"]):
                    loc['course'] = parsedLine["value"]["course"]
                else:
                    loc['course'] = 0.0

                #print(json.dumps(loc))
                parsedLine["coordinate"] = loc["coordinate"]
                parsedLine["time"] = loc["time"]
                parsedLine["timeDelta"] = parsedLine["time"] - lastTime
                lastTime = parsedLine["time"]
                parsedLine["type"] = "tracking"
                parsedLine["value"]["course"] = loc["course"]
                parsedLine.pop("location", None)
                print(json.dumps(parsedLine))
            else:
                pass
                #print(json.dumps(parsedLine))
        except Exception as e:
            print("# Line parse fail: " + str(e))
            pass

