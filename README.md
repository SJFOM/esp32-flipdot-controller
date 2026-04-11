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

## MacOS: On first setup
ESP-IDF requires some packages be installed beforehand - these can be installed using Homebrew with the following command
```shell
brew update; brew install cmake dfu-util
```

## Testing Animations (Without Hardware)

A standalone C test harness is included to visualize and test animations without physical hardware. Currently supports testing the **Firework Animation**.

### Quick Start

**Using Make (Recommended):**
```bash
cd /workspaces/esp32-flipdot-controller/firework_animation_testing

# Build and run for 30 seconds (default)
make -f Makefile.firework run

# Build and run for a custom duration
make -f Makefile.firework run-60    # 60 seconds
```

**Direct Compilation:**
```bash
cd /workspaces/esp32-flipdot-controller/firework_animation_testing

gcc -std=c99 -Wall -Wextra -O2 -o firework_test firework_test.c \
    ../main/firework.c ../main/fill.c -I../main/include -Imock_headers -lm

./firework_test [DURATION_SECONDS]
```

### What You'll See

- Real-time ASCII art visualization of the 20×14 dot matrix in landscape orientation
- `█` = LED on, `·` = LED off  
- Frame counter, elapsed time, next firework scale (1–10), and countdown to next trigger
- Fireworks fire at random scale and random 1–10 second intervals
- Particle animations with gravity simulation — larger scale = more particles, higher speed, longer lifespan
- Press **Ctrl+C** to stop at any time
- Final statistics: total frames and average FPS

### How It Works

The test harness (`firework_test.c`) provides:
- **Complete hardware mocking** - FreeRTOS, ESP-IDF logging, and display functions
- **Direct compilation** - Compiles your actual animation code without modifications
- **Terminal rendering** - Visual feedback of the animation in real-time

For detailed information, see [firework_animation_testing/README_FIREWORK_TEST.md](firework_animation_testing/README_FIREWORK_TEST.md).

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