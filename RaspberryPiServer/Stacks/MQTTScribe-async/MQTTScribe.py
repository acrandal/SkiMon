#!/usr/bin/env python3
##
#   MQTT to InfluxDB Scribe for the SkiMon Project
#   
#   @author Aaron S. Crandall <crandall@gonzaga.edu>
#   @copyright 2021
#
#

import logging
import json
import asyncio
from hbmqtt.client import MQTTClient, ClientException, ProtocolException
from hbmqtt.mqtt.constants import QOS_1, QOS_2
from influxdb import InfluxDBClient


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


def handle_packet(influx_client, msg_json):
    try:
        msg = json.loads(msg_json)
    except Exception:
        logging.error("Could not parse packet: " + msg_json)
        return

    newPoint = None
    if(msg["type"] == 'accel'):
        newPoint = create_accel_point(msg)
    elif(msg["type"] == 'gyro'):
        newPoint = create_gyro_point(msg)

    if(newPoint):
        influx_client.write_points([newPoint])


async def uptime_coro(mqtt_host_uri, influx_client):
    C = MQTTClient()
    await C.connect(mqtt_host_uri)
    await C.subscribe([
            #('$SYS/broker/uptime', QOS_1),
            #('$SYS/broker/load/#', QOS_2),
            ('skimon', QOS_1)
         ])
    try:
        while(True):
            try:        # Wanna see a hack? Keep reading
                message = await C.deliver_message()
                packet = message.publish_packet
                msg = (packet.payload.data).decode('utf-8')
                handle_packet(influx_client, msg)
            except ClientException as ce:
                logging.error("Caught and ignoring client exception")
                pass
            except ProtocolException:
                logging.error("Caught and ignoring client protocol handler")
    except ClientException as ce:
        logging.error("Client exception: %s" % ce)
    except KeyboardInterrupt:           # NOTE: This doesn't catch, and I don't know why
        print("Caught keyboard ^C - shutting down")
    finally:
        await C.unsubscribe(['skimon'])
        await C.disconnect()
        

# Main function / code
if __name__ == '__main__':
    mqtt_host_uri = "mqtt://192.168.4.1/"

    influx_host = 'localhost'
    influx_port = 8086
    influx_db = 'skimon'

    formatter = "[%(asctime)s] %(name)s {%(filename)s:%(lineno)d} %(levelname)s - %(message)s"
    logging.basicConfig(level=logging.INFO, format=formatter)

    logging.info("Starting scribe logger")

    influx_client = InfluxDBClient(host=influx_host, port=influx_port, database=influx_db)
    logging.info("Connected to Influx")

    try:
        asyncio.get_event_loop().run_until_complete(uptime_coro(mqtt_host_uri, influx_client))
    except KeyboardInterrupt:
        logging.info("Caught ^C in main function - quitting")
    finally:
        influx_client.close()
        logging.info("InfluxDB connection closed")
    logging.info("Shutting down complete - done.")
