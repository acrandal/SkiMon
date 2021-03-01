#!/usr/bin/env python3
#
#   GPSD listener and InfluxDB logger
#
#   Currently, quite simple for initial data collection
#
#   @author Aaron S. Crandall <crandall@gonzaga.edu>
#   @copyright 2021
#

from influxdb import InfluxDBClient
from gps3.agps3threaded import AGPS3mechanism
from time import sleep

# ** Create a InfluxDB data point from gps data point **
def create_gps_point(location, latitude, longitude, speed, course):
    is_valid = False
    point = {
        "measurement": "GPS",
        "tags": {
            "deviceLocation": location,
        },
        "fields": {
        }
    }

    try:
        latitude = float(latitude)
        is_valid = True
    except ValueError:
        latitude = 0.0
    point["fields"]["latitude"] = latitude

    try:
        longitude = float(longitude)
        is_valid = True
    except ValueError:
        longitude = 0.0
    point["fields"]["longitude"] = longitude

    try:
        speed = float(speed)
        point["fields"]["speed"] = speed
    except ValueError:
        speed = 0.0

    try:
        course = float(course)
        point["fields"]["course"] = course
    except ValueError:
        course = -1000.0

   # print(point)
    if is_valid:
        return point
    else:
        return None


if __name__ == "__main__":

    print("Connecting to InfluxDB")
    influx_client = InfluxDBClient(host='localhost', port=8086, database='skimon')
    print("Connected to Influx")


    agps_thread = AGPS3mechanism()  # Instantiate AGPS3 Mechanisms
    agps_thread.stream_data()  # From localhost (), or other hosts, by example, (host='gps.ddns.net')
    agps_thread.run_thread()  # Throttle time to sleep after an empty lookup, default '()' 0.2 two tenths of a second

    while True:  # All data is available via instantiated thread data stream attribute.
        # line #140-ff of /usr/local/lib/python3.5/dist-packages/gps3/agps.py
        print('---------------------')
        print(                   agps_thread.data_stream.time)
        print('Lat:{}   '.format(agps_thread.data_stream.lat))
        print('Lon:{}   '.format(agps_thread.data_stream.lon))
        print('Speed:{} '.format(agps_thread.data_stream.speed))
        print('Course:{}'.format(agps_thread.data_stream.track))
        print('---------------------')
        newPoint = create_gps_point("backpack", agps_thread.data_stream.lat, agps_thread.data_stream.lon, agps_thread.data_stream.speed, agps_thread.data_stream.track)
        if(newPoint):
            influx_client.write_points([newPoint])
        sleep(0.5) # Sleep, or do other things for as long as you like.
