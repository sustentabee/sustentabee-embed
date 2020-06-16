import os, pika


url = os.environ.get('CLOUDAMQP_URL', """amqp://kdtloxga:2QKIDpHEaorsmgYusaeUnVO04reAw-y1@buck.rmq.cloudamqp.com/kdtloxga""")
params = pika.URLParameters(url)
params.socket_timeout = 5
connection = pika.BlockingConnection(params)

channel = connection.channel()
channel.queue_bind(exchange="amq.topic", queue="test")

def callback(ch, method, properties, body):
    print("Received: %r" % body)

channel.basic_consume(queue="test", on_message_callback=callback, auto_ack=True)

print("Waiting for messages...")
channel.start_consuming ()
