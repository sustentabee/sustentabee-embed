# Sustentabee Embed Repository

Repository of Sustentabee's embed system codes, including Arduino files and messaging integrations with python


## embed-arduino

Embed arduino codes. The file sustentabee.ino represents the current version of the prototype device firmware.

In order to build this environment, you need to have assembled correctly the current version of the prototype device.

If you want to see a detailed description of it's components and assemble, send an e-mail to germano.c.lodi@gmail.com


## rabbitmq-queue-watcher

Python environment used to consume the messaging queue, which is where the arduino is sending the measured data.

In order to build this environment, you need to have Docker and docker-compose installed in your machine. Then, run the following commands
1) docker-compose build
2) docker-compose up
3) docker exec -it -d pyenv bash