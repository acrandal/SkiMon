#!/usr/bin/env python3
#
#   IoT MQTT listener and InfluxDB logger
#
#   Currently, quite simple for initial data collection
#
#   @author Aaron S. Crandall <acrandal@gmail.com>
#   @copyright 2020
#

from socket import MSG_ERRQUEUE
import paho.mqtt.client as mqtt
#from influxdb import InfluxDBClient
from time import sleep, time
import json
import logging
import logging.handlers as handlers
from datetime import datetime
import threading
import queue

#global logger

global message_count
message_count = 0

global log_file_name
global log_file

global done
done = False

global msgQueue
msgQueue = queue.Queue()


# ** ********************************************************
def writerThreadMain():
    global done
    global msgQueue
    global log_file
    global message_count

    while not done:
        try:
            msg = msgQueue.get(block=True, timeout=1)
        except queue.Empty:
            continue

        #print(msg)
        #logger.info(msg + "\n")
        log_file.write(msg + "\n")

        message_count += 1
        if( message_count % 100 == 0 ):
            print(".", end = "", flush=True)
        if( message_count % 10000 == 0 ):
            print("", flush=True)
            message_count = 0

        print(msgQueue.qsize())

    logging.info("Thread ending")


# ** ********************************************************
def on_message(client, userdata, message):
    global message_count
    global log_file
    global msgQueue

    msg_json = str(message.payload.decode("utf-8"))
    try:
        msg = json.loads(msg_json)
        msg["epoch"] = time()
        msg_json = json.dumps(msg)
    except Exception:
        pass

    try:
        msgQueue.put(msg_json)          # Enqueue for buffered write
    except Exception as e:
        logging.error("Could not put to queue: " + str(e))


# ** ********************************************************
def on_connect(client, userdata, flags, rc):
    logging.info("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("skimon")


if __name__ == "__main__":
    formatter = "[%(asctime)s] %(name)s {%(filename)s:%(lineno)d} %(levelname)s - %(message)s"
    logging.basicConfig(level=logging.INFO, format=formatter)
    logging.info("Starting MQTT File Scribe.")

    #logger = logging.getLogger('skimon')
    #logger.setLevel(logging.INFO)

    #logHandler = handlers.TimedRotatingFileHandler(file_path + "/" + 'skimon.log', when='M', interval=15)
    #logHandler.setLevel(logging.INFO)
    #logger.addHandler(logHandler)

    # create new log file for this run
    now = datetime.now()
    file_path = "/var/lib/influxdb/skimon/"
    log_file_name = now.strftime("skimon-log-%Y-%m-%d %H:%M:%S.dat")
    try:
        log_file = open(file_path + "/" + log_file_name, 'w')
    except Exception as e:
        logging.error("Problem opening log file: " + str(e))

    # Create buffered writing thread
    writerThread = threading.Thread(target=writerThreadMain)
    writerThread.start()

    # Create and setup MQTT client
    mqtt_client = mqtt.Client("FileScribe")
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message
    mqtt_client.enable_logger()

    mqtt_client.connect("localhost")

    # Main loop & operations - wait for messages via MQTT
    try:
        mqtt_client.loop_forever()
        logging.info("Connected to MQTT - starting processing")
    except KeyboardInterrupt:
        print("")
        logging.info("Received interrupt - quitting")
    finally:
        done = True
        mqtt_client.loop_stop()
        mqtt_client.disconnect()
        logging.info("Disconnected from MQTT")
        writerThread.join()
        logging.info("Writing thread stopped")
        log_file.flush()
        log_file.close()
        logging.info("Log file flushed and closed")

    logging.info("Logger terminated.")