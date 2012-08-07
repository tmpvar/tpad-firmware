
# tpad firmware

So you want to hack on the tpad's firmware! No? well then you are probably just trying to flash the latest firmware.  You can do that with the `tpad` command. See: http://github.com/tmpvar/tpad/ for instructions.

It's really a painless process, I promise!

## Prerequisites

you will need avrdude and avr-gcc

### Debian

    sudo apt-get install avrdude gcc-avr libusb-1.0-0

### OSX & Brew

    sudo brew install avrdude avr-gcc

### Windows

install [winavr](http://winavr.sourceforge.net/index.html)

## Flashing the latest firmware for your device

First off you are going to want to clone this project

    git clone https://github.com/tmpvar/tpad-firmware.git
    cd tpad-firmware/tpad/tpad4-mega

If you are running a device other than the `tpad4-mega` you'll want to `cd` into the proper directory.

Grab yourself a writing stick (a pen works fine). We'll be using it to reset the tpad into "bootloader mode" so we can flash the new firmware. Press the reset button on the bottom of the tpad.  (there should be a hole for you to stick the pen into) and run:

    AVRDUDE_PORT=`ls /dev/tty.usbmodem*` make avrdude

If the programmer times out, you will need to repeat this process again (faster this time!).

## License

The majority of this code is LUFA, you can find the licensing info [here](https://github.com/tmpvar/tpad-firmware/blob/master/LUFA/License.txt)

The `tpad/**` directory is MIT licensed, copyright Elijah Insua 2012 (unless otherwise noted)
