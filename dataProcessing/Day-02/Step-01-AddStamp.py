

import datetime
import json
import fileinput
import time

def addEpoch(line):
    epoch = line["epoch"]
    stamp = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(epoch))
    line["stamp"] = stamp
    return line

if __name__ == '__main__':
    for line in fileinput.input():
        line = line.strip()
        try:
            if(len(line) > 0 and line[0] != '#'):
                parsedLine = json.loads(line)
                #print("json: ", end="")
                #print(parsedLine)
                newLine = addEpoch(parsedLine)
                print(json.dumps(newLine))
            else:
                print(line)
        except Exception:
            print("# Line parse fail")
            pass

