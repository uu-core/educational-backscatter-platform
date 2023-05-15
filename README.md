# Pico-Backscatter
An educational project on backscatter using Raspberry Pi Pico

## Repo Organization
- `hardware` contains the hardware design with further description and explanation (generating the PIO using the python script).
- `project_pico_libs` contains the libs for all projects using the RPI Pico (carrier/receiver/baseband).
- `baseband` contains a 2-FSK baseband code using Pico PIO together with a generator script, its description and some exercise questions.
- `carrier-CC2500` contains a carrier generator using the Mikroe-1435 (CC2500) on the Pico.
- `receiver-CC2500` contains a receiver using the Mikroe-1435 (CC2500) on the Pico.
- `carrier-Firefly` contains the configuration guidance for home setup with zolertia firefly as carrier.
- `carrier-characteristics` contains a measurement to estimate the typical carrier bandwidth.
- `carrier_receiver-CC1352` contains the configuration guidance for lab setup with CC1352 as carrier and/or receiver.
- `carrier-receiver-baseband` integrates all components into one setup: the Pico generates the baseband, uses one Mikroe-1435 (CC2500) to generate a carrier and a second Mikroe-1435 (CC2500) to receive the backscattered signal.
- `stats` contains the system evaluation script.

## Installation
A number of pre-requisites are needed to work with this repo:
Raspberry Pi Pico SDK, cmake and arm-none-eabi-gcc have to be installed for building and flashing the application code.
<br>Please follow the installation guidance in [Getting started with Rasberry Pi Pico](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf).
<br>Using Visual Studio Code is recommended.
