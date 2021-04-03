#!/usr/bin/env python3
#
#   IoT MQTT Hearbeat Daemon
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


import paho.mqtt.client as mqtt

global mqtt_client


# **************************************************************************
def on_connect(client, userdata, flags, rc):
    logging.info("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    # client.subscribe("skimon")


# **************************************************************************
if __name__ == "__main__":
    formatter = "[%(asctime)s] %(name)s {%(filename)s:%(lineno)d} %(levelname)s - %(message)s"
    logging.basicConfig(level=logging.INFO, format=formatter)
    logging.info("Starting MQTT Heartbeat Daemon.")

    mqtt_client = mqtt.Client("Hearbeat Daemon")
    mqtt_client.on_connect = on_connect
    #mqtt_client.on_message = on_message
    mqtt_client.enable_logger()

    mqtt_client.connect("localhost")
    msg = {
        "type": "heartbeat",
        "location": "backpack",
        "value": "beat"
    }
    msg_json = json.dumps(msg)

    try:
        logging.info("Connected to MQTT - starting heartbeat")
        while(True):
            mqtt_client.publish("skimon", payload=msg_json)
            sleep(0.5)
    except KeyboardInterrupt:
        print("")
        logging.info("Received interrupt - quitting")
    finally:
        mqtt_client.loop_stop()     # officially end loop
        mqtt_client.disconnect()    # clean disconnect!
        logging.info("Disconnected from MQTT & InfluxDB")
    logging.info("Heartbeat daemon terminated.")
