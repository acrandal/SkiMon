#!/usr/bin/env python3
#
#   IoT MQTT listener and InfluxDB logger
#
#   Currently, quite simple for initial data collection
#
#   @author Aaron S. Crandall <acrandal@gmail.com>
#   @copyright 2020
#

import paho.mqtt.client as mqtt
from influxdb import InfluxDBClient
from time import sleep
import json
import logging

global influx_client
global message_count
message_count = 0;


def upload_point(measurement, tag_location, field_value):
    point = {
        "measurement": measurement,
        "tags": {
            "deviceLocation": tag_location,
        },
        "fields": {
            "value": field_value
        }
    }
    # print(point)
    influx_client.write_points([point])


# ** Create a InfluxDB data point from an gyro message **
def create_gyro_point(msg):
    point = {
        "measurement": "gyroscope",
        "tags": {
            "deviceLocation": msg["location"],
        },
        "fields": {
            "x": msg["value"]["x"],
            "y": msg["value"]["y"],
            "z": msg["value"]["z"]
        }
    }
    # print(point)
    return point


# ** Create a InfluxDB data point from an accel message **
def create_accel_point(msg):
    point = {
        "measurement": "accelerometer",
        "tags": {
            "deviceLocation": msg["location"],
        },
        "fields": {
            "x": msg["value"]["x"],
            "y": msg["value"]["y"],
            "z": msg["value"]["z"]
        }
    }
    return point


def on_message(client, userdata, message):
    global message_count
    msg_json = str(message.payload.decode("utf-8"))

    try:
        msg = json.loads(msg_json)
    except Exception:
        print("Could not parse message")
        return
    #print(msg)
    #upload_point(msg["type"], msg["location"], msg["value"])

    message_count += 1
    if( message_count % 100 == 0 ):
        print(".", end = "", flush=True)
    if( message_count % 10000 == 0 ):
        print("", flush=True)
        message_count = 0


    newPoint = None
    if(msg["type"] == 'accel'):
        newPoint = create_accel_point(msg)
    elif(msg["type"] == 'gyro'):
        newPoint = create_gyro_point(msg)

    if(newPoint):
        influx_client.write_points([newPoint])

def on_connect(client, userdata, flags, rc):
    logging.info("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("skimon")

if __name__ == "__main__":
    formatter = "[%(asctime)s] %(name)s {%(filename)s:%(lineno)d} %(levelname)s - %(message)s"
    logging.basicConfig(level=logging.INFO, format=formatter)
    logging.info("Starting logger.")

    influx_client = InfluxDBClient(host='localhost', port=8086, database='skimon')
    logging.info("Connected to InfluxDB")

    # client.write_points(points)

    mqtt_client = mqtt.Client("pyLogger")
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message
    mqtt_client.enable_logger()

    mqtt_client.connect("localhost")

    try:
        mqtt_client.loop_forever()
        logging.info("Connected to MQTT - starting processing")
    except KeyboardInterrupt:
        print("")
        logging.info("Received interrupt - quitting")
    finally:
        mqtt_client.loop_stop()
        mqtt_client.disconnect()
        influx_client.close()
        logging.info("Disconnected from MQTT & InfluxDB")
    logging.info("Logger terminated.")
