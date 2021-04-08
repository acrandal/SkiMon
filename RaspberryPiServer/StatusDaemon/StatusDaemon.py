#!/usr/bin/env python3
#
#   IoT MQTT listener and Status Publishing Daemon
#
#   Currently, quite simple for initial data collection
#
#   @author Aaron S. Crandall <crandall@gonzaga.edu>
#   @copyright 2020
#

from platform import release
from time import sleep, time
import json
import logging
import threading

import board
import neopixel


import paho.mqtt.client as mqtt
mqtt.Client._call_socket_register_write = lambda _self: None
mqtt.Client._call_socket_unregister_write = lambda _self, _sock=None: None

global mqtt_client

global debug
debug = False

global done
done = False

global threadLock
threadLock = threading.Lock()

global maxAge
maxAge = 10 

global lastSentEpoch
lastSentEpoch = time()

global stats
stats = {
    "recording": "stop",
    "motes" : {
                "LeftFront": time(),
                "LeftMiddle": time(),
                "LeftBack": time(),
                "RightFront": time(),
                "RightMiddle": time(),
                "RightBack": time()
              }
}

global message_count
message_count = 0;


# Choose an open pin connected to the Data In of the NeoPixel strip, i.e. board.D18
# NeoPixels must be connected to D10, D12, D18 or D21 to work.
pixel_pin = board.D12
 
# The number of NeoPixels
num_pixels = 7
 
# The order of the pixel colors - RGB or GRB. Some NeoPixels have red and green reversed!
# For RGBW NeoPixels, simply change the ORDER to RGBW or GRBW.
ORDER = neopixel.GRB
 
global pixels
pixels = neopixel.NeoPixel(
    pixel_pin, num_pixels, brightness=0.05, auto_write=False, pixel_order=ORDER
)

global pixelIDs
pixelIDs = {
    "recording": 0,
    "LeftFront": 6,
    "LeftMiddle": 5,
    "LeftBack": 4,
    "RightFront": 1,
    "RightMiddle": 2,
    "RightBack": 3
}


# ** ********************************************************
def on_message(client, userdata, message):
    global message_count
    global stats
    global lastSentEpoch
    global threadLock
    global debug

    msg_json = str(message.payload.decode("utf-8"))

    try:
        msg = json.loads(msg_json)
    except Exception:
        logging.warn("Could not parse json message")
        return

    message_count += 1
    if( debug and message_count % 100 == 0 ):
        print(".", end = "", flush=True)
    if( debug and message_count % 10000 == 0 ):
        print("", flush=True)
        message_count = 0

    threadLock.acquire()
    if(msg["type"] == 'accel'):
        stats["motes"][msg["location"]] = time()
    elif(msg["type"] == 'recording'):
        stats["recording"] = msg["value"]
    threadLock.release()

    # Send out an update if it's been more than the timeout (0.5 secs)
    if( lastSentEpoch + 0.5 < time() ):
        sendMsg = {
            "recording": stats["recording"],
            "motes": {}
        }
        for moteName in stats["motes"]:
            sendMsg["motes"][moteName] = (int)(time() - stats["motes"][moteName])

        sendMsg_json = json.dumps(sendMsg)

        mqtt_client.publish("skimon/status", payload=sendMsg_json)
        lastSentEpoch = time()


# **************************************************************************
def on_connect(client, userdata, flags, rc):
    logging.info("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("skimon")


# **************************************************************************
def updateLEDs(currStats):
    global pixels

    red = (255, 0, 0)
    green = (0, 255, 0)
    blue = (0,0,255)
    amber = (255,255,0)
    if( currStats["recording"] == "start" ):
        pixels[pixelIDs["recording"]] = green
    else:
        pixels[pixelIDs["recording"]] = red

    for moteName in currStats["motes"]:
        maxTimeout = 3
        moteTimeout = (int)(time() - currStats["motes"][moteName])
        moteTimeout = min(moteTimeout, maxTimeout)
        moteTimeout = maxTimeout - moteTimeout
        moteColorPct = float(moteTimeout) / maxTimeout
        currColor = blue
        if moteName[0] == 'R':
            currColor = amber
        moteColor = tuple(int(i * moteColorPct) for i in currColor)
        pixels[pixelIDs[moteName]] = moteColor
    pixels.show()


# **************************************************************************
def statusUpdateThreadMain():
    global stats
    global threadLock
    global done

    while not done:
        sleep(0.5)
        threadLock.acquire

        updateLEDs(stats)

        threadLock.release


# **************************************************************************
if __name__ == "__main__":
    formatter = "[%(asctime)s] %(name)s {%(filename)s:%(lineno)d} %(levelname)s - %(message)s"
    logging.basicConfig(level=logging.INFO, format=formatter)
    logging.info("Starting Status Broadcasting Daemon.")

    statusUpdateThread = threading.Thread(target=statusUpdateThreadMain)
    statusUpdateThread.start()

    mqtt_client = mqtt.Client("Status Daemon")
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message
    mqtt_client.enable_logger()

    mqtt_client.connect("localhost")

    try:
        mqtt_client.loop_forever(retry_first_connection=True)
        logging.info("Connected to MQTT - starting processing")
    except KeyboardInterrupt:
        print("")
        logging.info("Received interrupt - quitting")
    finally:
        done = True                 # tell thread to die
        mqtt_client.loop_stop()     # officially end loop
        mqtt_client.disconnect()    # clean disconnect!
        logging.info("Disconnected from MQTT & InfluxDB")
        statusUpdateThread.join()   # Wait for thread to fully die
    logging.info("Logger terminated.")
