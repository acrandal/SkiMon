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

def on_message(client, userdata, message):
    #print("message received " ,str(message.payload.decode("utf-8")))
    #print("message topic=",message.topic)
    msg_json = str(message.payload.decode("utf-8"))

    try:
        msg = json.loads(msg_json)
    except Exception:
        print("Count not parse message")
        return
    #print(msg)
    upload_point(msg["type"], msg["location"], msg["value"])


if __name__ == "__main__":
    print("Starting logger.")

    influx_client = InfluxDBClient(host='bug.local', port=8086, database='IoTData')
    print("Connected to Influx")

    # client.write_points(points)

    mqtt_client = mqtt.Client("pyLogger")
    mqtt_client.connect("10.0.0.192")
    mqtt_client.subscribe("IoT/Data")
    mqtt_client.on_message=on_message
    mqtt_client.loop_start()

    try:
        while(True):
            sleep(4)
    except KeyboardInterrupt:
        print("Received interrupt - quitting")

    mqtt_client.loop_stop()
    mqtt_client.disconnect()

    print("Logger terminating.")
