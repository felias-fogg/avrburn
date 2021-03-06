# avrburn

This sketch turns your Gambuino META into a mobile AVR programmer.

## Features

This programmer lets you program most of the AVR ATmega and ATtiny MCUs, at least all of those
that support ISP programming and are listed in Atmel Studio 7.0. Just put the HEX files in the
folder BURN on your SD card and off you go.

## Usage

Any file with a **HEX** extension will be recognized as an a file to be programmed to flash memory, any file with an **EEP** extension as a file to be programmed into EEPROM, any file with a **FUS** extension will be used as a fuse file, and any file with a **LCK** extension as a lockbits file. The former two file types use the familiar ihex format, which you get, for instance, when you request to **Export Compiled Binary** in Arduino's **Sketch** menu. Fuse and lockbit files simply list the fuses or lockbits in hexadecimal, one per line, starting with the low fuse. There are some examples in the BURN folder.

The menus should be pretty much self-explanatory. In the **Settings** menu, you can set whether you want to have automatic verification and automatic erasure before programming, both of which is enabled by default. The **Safe Fuse Editting** option makes sure that you cannot edit fuses that make the MCU unresponsive. Finally, you can choose the SPI clock frequency. 100 kHz is a good compromise between speed and robustness. However, in general, you can chose any clock that is not higher than a quarter of the MCU clock frequency. 

Navigating through the menus is done with the **up** and **down** buttons as well as the **A** button to select an entry. Pressing the **B** button exits the current menu. Pressing the **Menu** button restarts the sketch.

## Electrical connection

Currently, the following Gamebuino-Meta pins are used:
* Port D2 (PA14): Reset
* Port D3 (PA09): MOSI
* Port D4 (PA08): SCK
* Port D5 (PA15): MISO

In addition, Vcc and GND are connected. You have to connect
these pins with the corresponding pins on the target system. It is, of course, possible to use other pins. But then you have to change the corresponding definitions in avrburn.ino. 

**BEWARE**: If you connect the pins directly to the target system, make sure that you connect only to
3.3 Volt systems. Because the Gamebuino Meta is not 5 Volt tolerant, it will get damaged if
connected to a 5 Volt system. If you have a chip in a programming socket and the Gambuino supplies the current, then there won't be any problem (not possible anymore!). 

There exists now a PCB that can be plugged into the Meta, which does the level shifting if necessary. In other words, you can now also program 5 volt systems!

## Planned additions

The following things are planned for the future:
* Porting the program to ODROID GO (not so sure anymore)

The software is published under the [LGPL](http://www.gnu.org/licenses/lgpl-3.0.html).

