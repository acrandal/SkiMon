

import json


vals = {}
vals["type"] = "accelerometer"
vals["location"] = "Left-Front"
vals["values"] = {"x": 23.7, "y": -11.8, "z": 0.23}

outStr = json.dumps(vals)

print(outStr)

testStr = "{\"type\":\"accel\", \"location\": \"Left-Front\", \"values\": {\"x\": 0.435494, \"y\": -1.057628, \"z\": 9.931411}}"

inStr = json.loads(testStr)

print(inStr)
