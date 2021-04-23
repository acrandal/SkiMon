#!/usr/bin/env python3

import datetime
import json
import fileinput
import time
import sys
import os
import logging


runs = [0, 6, 7, 8, 9, 10, 11, 12, 13, 14]
#runs = [0]
motes = [ "LeftFront", "LeftMiddle", "LeftBack", "RightFront", "RightMiddle", "RightBack" ]
sensorTypes = [ "accel", "gyro", "mag" ]
currGPS = { "latitude" : 0.0, "longitude" : 0.0, "speed" : 0.0, "course" : 0.0, "isFresh" : False }

totalExecs = len(runs) * len(motes) * len(sensorTypes)
currExec = 0


def doSingleFile(currRunNum, currMote, currSensorType, inFileName, outFileName):
    inFile = open(inFileName)
    logging.debug("Opened input file: " + inFileName)
    logging.debug("currRunNum: " + str(currRunNum))
    logging.debug("currMote: " + str(currMote))
    logging.debug("currSensorType: " + str(currSensorType))
    with open(outFileName, 'w') as outFile:
        #print(outFileName + " Created / opened")
        outFile.seek(0)
        outFile.truncate()

        csvFields = [
            "stamp", "runNum", "location", "sensor", "x", "y", "z", "epoch",
            "GPSlatitude", "GPSlongitude", "GPSspeed", "GPScourse"
        ]
        outFile.write(",".join(csvFields) + "\n")


        for line in inFile:
            line = line.strip()
            try:
                if(len(line) > 0 and line[0] != '#'):
                    parsedLine = json.loads(line)
                    if(parsedLine["runNum"] == currRunNum):
                        #print(json.dumps(parsedLine))

                        # Check for and snag GPS data
                        if(parsedLine["type"] == "GPS" and parsedLine["value"]["latitude"] != currGPS["latitude"]):
                            #print(json.dumps(parsedLine))
                            currGPS["latitude"] = parsedLine["value"]["latitude"]
                            currGPS["longitude"] = parsedLine["value"]["longitude"]
                            if("speed" in parsedLine["value"]):
                                currGPS["speed"] = parsedLine["value"]["speed"]
                            else:
                                currGPS["speed"] = 0.0
                            if("course" in parsedLine["value"]):
                                currGPS["course"] = parsedLine["value"]["course"]
                            else:
                                currGPS["course"] = 0.0
                            currGPS["isFresh"] = True
                        elif(parsedLine["type"] == currSensorType and parsedLine["location"] == currMote):
                            # Create empty GPS data fields
                            parsedLine["GPSlatitude"] = ""
                            parsedLine["GPSlongitude"] = ""
                            parsedLine["GPSspeed"] = ""
                            parsedLine["GPScourse"] = ""

                            # *IF* following latest GPS data, add to ONE data sample
                            if(currGPS["isFresh"]):
                                parsedLine["GPSlatitude"] = currGPS["latitude"]
                                parsedLine["GPSlongitude"] = currGPS["longitude"]
                                parsedLine["GPSspeed"] = currGPS["speed"]
                                parsedLine["GPScourse"] = currGPS["course"]
                                currGPS["isFresh"] = False
                            # print(json.dumps(parsedLine))

                            outVals = [
                                parsedLine["stamp"],
                                parsedLine["runNum"],
                                parsedLine["location"],
                                parsedLine["type"],
                                parsedLine["value"]["x"],
                                parsedLine["value"]["y"],
                                parsedLine["value"]["z"],
                                parsedLine["epoch"],
                                parsedLine["GPSlatitude"],
                                parsedLine["GPSlongitude"],
                                parsedLine["GPSspeed"],
                                parsedLine["GPScourse"]
                            ]
                            outVals = [str(x) for x in outVals]     # make em strings!
                            outFile.write(",".join(outVals) + "\n")
                else:
                    pass
                    #print(line)
            except KeyboardInterrupt as e:
                print("# Keyboard interrupt: " + str(e))
                pass
                raise KeyboardInterrupt

    inFile.close()


if __name__ == '__main__':
    inFileName = sys.argv[1]
    inFile = open(inFileName)
    outDir = "PerRunData/"
    #startStops = json.load(f)
    #print(startStops)

    for currRunNum in runs:

        currDir = outDir + "RunNum0" + str(currRunNum)
        cmd = "mkdir -p " + currDir
        os.system(cmd)

        for currMote in motes:
            for currSensorType in sensorTypes:
                currExec += 1
                currFileName = "SkiMon-Day02-Run"
                if(currRunNum) < 10:
                    print("Adding zero t: " + str(currRunNum))
                    currFileName += "0"
                currFileName += str(currRunNum) + "-" + currMote + "-" + currSensorType + ".csv"
                print("Doing work unit {0}/{1} : {2}".format(currExec, totalExecs, currFileName))
                #print(currFileName)

                outFileName = currDir + "/" + currFileName

                #doSingleFile(currRunNum, currMote, currSensorType, inFileName, outFileName)

