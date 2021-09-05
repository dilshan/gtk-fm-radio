# GTK FM Radio

GTK FM Radio is a lightweight, GUI base FM radio tuner application for [Raspberry Pi OS](https://www.raspberrypi.org/software/). This application uses GTK to provide the UI of the project, as the name implies.

The current version of the GTK FM Radio supports tuners based on the *Quintic* [QN8035](https://datasheetspdf.com/pdf-down/Q/N/8/QN8035_Quintic.pdf), which connects to the Raspberry Pi through the I2C bus.

The tuner application provided in this release supports the following features:

 - Manual and automatic station scanning.
 - Decode RDS PS (program service) data.
 - Volume control.
 - Display RSSI and SNR readings receive from the tuner.

The QN8035 driver base on [github.com/dilshan/qn8035-rpi-fm-radio](https://github.com/dilshan/qn8035-rpi-fm-radio), and it uses the *[WiringPi](http://wiringpi.com/)* library to communicate with the tuner.

The GTK FM Radio is released under the terms of the [MIT License](LICENSE).
