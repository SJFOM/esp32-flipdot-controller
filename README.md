# `esp32-flipdot-controller`

**Directly controls a Hanover Displays Ltd. 7020-20-01 dotboard from an R014B-model 20x14 flipdot display.**

![Image of the FDC board](https://damow.net/assets/images/2018/06/assembled-board.jpg)

## ESP-IDF repo setup

### How to build
1. Install the `ESP-IDF-Explorer` VS Code plugin
2. From the command palette, run `ESP-IDF: Build your project`

### How to upload
1. To upload to your ESP32, connect your device and note its COM port (/dev/tty* in Linux/Mac)
2. To set your COM port, run `ESP-IDF: Select port to use` from the command palette
3. Ensure the target board is in program mode - for the flipdot-controller board, this requires pressing both buttons simultaneously for a brief moment.
4. Now, run `ESP-IDF: Flash (UART) your project` to upload your code to the target board

### If path build issues occur
If build issues occur, try adding `C:\Users\USER_NAME\.espressif\tools\idf-python\3.11.2\Scripts` to path. Change **USER_NAME** for your username and check the Python version above matches what is installed by the ESP-IDF-Explorer toolchain.

## Pinout

**GPIO** numbers refer to the ESP32 GPIO pins.
**Pin** numbers refer to the dotboard's 14-pin flex connector.

* **GPIO 21** - Enable Display (**Pin 2**)
* **GPIO 18** - Column Advance (**Pin 7**)
* **GPIO 19** - Reset (**Pin 8**)
* **GPIO 17** - Set/Unset (**Pin 9**)
* **GPIO 5** - Row Advance (**Pin 10**)

In addition, **GPIO 4** controls the coil drive on **Pin 12** via a _FET/BJT_.

## Acknowledgement
This PCB design and firmware was originally created by Damo (a.k.a. [themainframe](https://github.com/themainframe)) as part of their exploration into controlling FlipDots - see the associated [blog post on damow.net](https://damow.net/fun-with-flip-dot-displays/).