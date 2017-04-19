# Intro

This project was created as a way for me to show off some of the things Azure can do using non-IaaS services. The goal was to collect and monitor
information from various sensors and display them in some type of dashboard, basically an IoT type thing.

# Components

There are a few different parts that make up this project, each with their own repo:

* The [sensors](https://git.esheavyindustries.com/esell/homebase)
* The [backend](https://git.esheavyindustries.com/esell/homebase-be)
* The [frontend](https://git.esheavyindustries.com/esell/homebase-fe)

## Sensors

This repo has not only the code for the sensors I used, but also any detail on the hardware. I am a novice hardware hacker so I used this opportunity
to practice creating my own PCBs to help simplify the sensor creation process. Most of the sensors are based off of the ESP8266.

## Backend

This repo has the code to handle any of the backend functions used by the dashboard, basically the API layer for everything. Most of this was done via 
Azure Functions but there are other behind the scenes components are also used.

## Frontend

This repo has the code to handle the dashboard. I am horrible with GUI stuff (as well as Javascript) so be warned :)

# Resources

Some of this code is either full on copies of existing projects, or code that started off as a copy and was slightly modified:

* D3 gauges came from http://bl.ocks.org/brattonc/5e5ce9beee483220e2f6 
* Azure IoT hub inspired by https://github.com/gloveboxes/Arduino-ESP8266-Secure-Azure-IoT-Hub-Client
* base64 code basis https://github.com/adamvr/arduino-base64
* sha256 code basis https://github.com/Cathedrow/Cryptosuite

# License

Except where noted this project is licensed under the GNU GPLv3 license.
