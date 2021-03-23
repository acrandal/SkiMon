import logging
import json
import asyncio
from hbmqtt.client import MQTTClient, ClientException
from hbmqtt.mqtt.constants import QOS_1, QOS_2

async def uptime_coro():
    C = MQTTClient()
    await C.connect('mqtt://192.168.4.1/')
    await C.subscribe([
            #('$SYS/broker/uptime', QOS_1),
            #('$SYS/broker/load/#', QOS_2),
            ('skimon', QOS_1)
         ])
    try:
        while(True):
            message = await C.deliver_message()
            packet = message.publish_packet
            # print(f"{i}:  {packet.variable_header.topic_name} => {packet.payload.data}")
            msg = (packet.payload.data).decode('utf-8')
            #print(msg)
            outStr = json.loads(msg)
            print(outStr)
        #await C.unsubscribe(['$SYS/broker/uptime', '$SYS/broker/load/#'])
    except ClientException as ce:
        logging.error("Client exception: %s" % ce)
    except KeyboardInterrupt:
        print("Caught keyboard ^C - shutting down")
    finally:
        await C.unsubscribe(['skimon'])
        await C.disconnect()
        
if __name__ == '__main__':
    formatter = "[%(asctime)s] %(name)s {%(filename)s:%(lineno)d} %(levelname)s - %(message)s"
    logging.basicConfig(level=logging.ERROR, format=formatter)
    asyncio.get_event_loop().run_until_complete(uptime_coro())
