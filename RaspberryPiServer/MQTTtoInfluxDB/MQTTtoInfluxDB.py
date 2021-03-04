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
    print(point)
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
    print(point)
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
    #print("message received " ,str(message.payload.decode("utf-8")))
    #print("message topic=",message.topic)
    msg_json = str(message.payload.decode("utf-8"))

    try:
        msg = json.loads(msg_json)
    except Exception:
        print("Count not parse message")
        return
    #print(msg)
    #upload_point(msg["type"], msg["location"], msg["value"])

    message_count += 1
    if( message_count % 100 == 0 ):
        print(".", end = "")
    if( message_count % 10000 == 0 ):
        print()
        message_count = 0


    newPoint = None
    if(msg["type"] == 'accel'):
        newPoint = create_accel_point(msg)
    elif(msg["type"] == 'gyro'):
        newPoint = create_gyro_point(msg)

    if(newPoint):
        influx_client.write_points([newPoint])


if __name__ == "__main__":
    print("Starting logger.")

    influx_client = InfluxDBClient(host='localhost', port=8086, database='skimon')
    print("Connected to Influx")

    # client.write_points(points)

    mqtt_client = mqtt.Client("pyLogger")
    mqtt_client.connect("localhost")
    mqtt_client.subscribe("SkiMon")
    mqtt_client.on_message=on_message
    mqtt_client.loop_start()
    print("Connected to MQTT")

    try:
        while(True):
            sleep(4)
    except KeyboardInterrupt:
        print("Received interrupt - quitting")

    mqtt_client.loop_stop()
    mqtt_client.disconnect()

    print("Logger terminating.")
