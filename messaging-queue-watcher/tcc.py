from flask import Flask, request, jsonify
import json
from multiprocessing import Process
import pika
import requests

app = Flask(__name__)

processes = []

@app.route("/watcher", methods=["POST"])
def watcher_post():
    print("main process after api call")                                                                                                                              
    data = request.get_json()

    p = Process(target=start_watcher, args=(data,))
    p.start()

    print("main process after subprocess spawn")
    return "end"

@app.route("/watcher", methods=["DELETE"])
def watcher_delete():                                                                                                                              
    for process in processes:
        process.terminate()
        del(process)

    return "end"

def start_watcher(data):
    url = data["url"]
    params = pika.URLParameters(url)
    params.socket_timeout = 5
    connection = pika.BlockingConnection(params)

    queue = data["queue"]
    channel = connection.channel()
    channel.queue_bind(exchange="amq.topic", queue=queue)

    def callback(ch, method, properties, body):
        payload_data = json.loads(body)
        request_confirmation = False
        request_url = ""
        request_data = {}

        if payload_data["type"] == "door":
            request_confirmation = True
            request_url = "https://sustentabee-webapi.herokuapp.com/door-opening"
            request_data = {
                "sensorSerialNo": str(payload_data["serial_no"]),
                "date": str(payload_data["timestamp"].split()[0]),
                "time": str(payload_data["timestamp"].split()[1]),
                "open": payload_data["door_state"] > 0,
            } 
        elif payload_data["type"] == "curr_temp":
            request_confirmation = True
            request_url = "https://sustentabee-webapi.herokuapp.com/measurement"
            request_data = {
                "sensorSerialNo": str(payload_data["serial_no"]),
                "date": str(payload_data["timestamp"].split()[0]),
                "time": str(payload_data["timestamp"].split()[1]),
                "temperature": payload_data["temperature"],
                "current": float(payload_data["current"]),
                "voltage": float(127),
                "power": float(float(payload_data["current"]) * float(127)),
                "consumption": float(float(float(payload_data["current"]) * float(127))*30),
            }

        print(request_data)
           
        if request_confirmation:
            requests.post(request_url, data=json.dumps(request_data), headers={"content-type": "application/json"})

    channel.basic_consume(queue=queue, on_message_callback=callback, auto_ack=True)

    print("Waiting for messages...")
    channel.start_consuming()