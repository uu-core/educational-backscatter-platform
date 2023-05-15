# Pico-Backscatter
An educational project on backscatter using Raspberry Pi Pico

## --> Updates <--
- 08.05.2023: Comment about the baud-rate: Notice that the baud-rate of the CC1352 is specified from 20-1000 kBaud (we tested it working as low as 30 kBaud with the backscatter setup). The CC2500 would allow for lower baud-rate configurations (specified down-to 1.2 kBaud). The timing of the state-machine is precise (no significant error or clock-difference needs to be considered).
- 27.04.2023: The Python script (generating the PIO) generated state machines which did not compile due to a mistake by splitting the required delay over a number of instructions. The issue has been resolved. The C-implementation was not affected by this issue.
- 29.03.2023: We added a new library `project_pico_libs/backscatter.c`. The `baseband` example demonstrates how to generate a PIO-file (python script) and assemble it using `pioasm`. This has the disadvantage that testing different baseband settings require a re-compilation for each setting. Therefore, we implemented a automatic generation of the state-machine binary in C (`project_pico_libs/backscatter.c`) and demonstrate its use in `carrier-receiver-baseband`. This enables changing the baseband settings on-the-fly for a convenient automation of tests.

## Repo Organization
- `hardware` contains the hardware design with further description and explanation (generating the PIO using the python script).
- `project_pico_libs` contains the libs for all projects using the RPI Pico (carrier/receiver/baseband).
- `baseband` contains a 2-FSK baseband code using Pico PIO together with a generator script, its description and some exercise questions.
- `carrier-CC2500` contains a carrier generator using the Mikroe-1435 (CC2500) on the Pico.
- `receiver-CC2500` contains a receiver using the Mikroe-1435 (CC2500) on the Pico.
- `carrier-Firefly` contains the configuration guidance for home setup with zolertia firefly as carrier.
- `carrier-characteristics` contains a measurement to estimate the typical carrier bandwidth.
- `carrier_receiver-CC1352` contains the configuration guidance for lab setup with CC1352 as carrier and/or receiver.
- `carrier-receiver-baseband` integrates all components into one setup: the Pico generates the baseband, uses one Mikroe-1435 (CC2500) to generate a carrier and a second Mikroe-1435 (CC2500) to receive the backscattered signal. _This setup generates the state-machine code at run-time, such that the baseband settings can be changed without re-compilation._
- `stats` contains the system evaluation script.

## Installation
A number of pre-requisites are needed to work with this repo:
Raspberry Pi Pico SDK, cmake and arm-none-eabi-gcc have to be installed for building and flashing the application code.
<br>Please follow the installation guidance in [Getting started with Rasberry Pi Pico](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf).
<br>Using Visual Studio Code is recommended.
