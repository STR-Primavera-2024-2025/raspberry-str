# Ubuntu Real Time

> Main reference: https://documentation.ubuntu.com/real-time/en/latest/

## Prerequisites

- A Raspberry Pi 4 or 5, with at least 2GB of RAM.
  - And a USB-C cable or charger for powering it up!
- An SD card of at least 1GB
  - A fast card is recommended, we are using a [Sandisk Extreme Pro (64GB A2 V30 XC1 version 3)](https://amzn.eu/d/088skAH).
- An internet connection for both your PC and the RPi.
- An Ubuntu One/Pro account, though if you don't have one we'll show you how.
  - It's free for up to 5 devices.

### Optional components

- [A micro-HDMI to regular HDMI cable](https://amzn.eu/d/cu1kO97) can be useful to debug certain issues, as the RPi4 and upwards do not have a full-sized HDMI port and you will be left without any output if the ssh doesn't work.
-

## Install steps

- [Download raspberry-pi-imager](https://www.raspberrypi.com/software/)
- Open the imager and flash the system: Other OS > Ubuntu 24.04
  - Remember to set up a recognizable option and check the "ssh" option, to be able to control
    the raspberry pi remotely.
- Once booted, you should be able to ssh into the machine with `ssh <hostname>.local`, being `hostname`
  the one you set up in the flash utility.
  - If not, you can always look into the router's local network to see what IP was assigned.
- Update the system: `sudo apt update && sudo apt install ubuntu-advantage-tools`
- [Create an Ubuntu One account](https://login.ubuntu.com/)
  - Remember to choose the "I don't have an Ubuntu One account" if necessary.
- [Go to the Ubuntu Pro dashboard](https://ubuntu.com/pro/dashboard): Copy the "Command to attach a machine" and paste it into the RPi console.
  - This will activate Ubuntu Pro in the RPi, allowing you to use the real time kernel.
- [Enable the real time kernel](https://documentation.ubuntu.com/pro-client/en/latest/howtoguides/enable_realtime_kernel/): `sudo pro enable realtime-kernel`
- This should be it, reboot the Raspberry Pi and you can get started!

## Using the real-time kernel

### Schedulers

> [Official reference](https://documentation.ubuntu.com/real-time/en/latest/tutorial/first-rt-app/4-schedulers/)

#### Completely Fair Scheduling (CFS)

This scheduler is **NOT** real-time, but it can be useful when running normal programs.
You can try it by copying the [`tasks/cfs.c`](./tasks/cfs.c) file in this repository and compiling it with: `gcc cfs.c -o cfs && ./cfs`

> Remember to install `gcc` first with `sudo apt install build-essentials`.

#### Priority-based scheduling (FIFO)

Tasks with higher priority are provided with much more CPU time.
Copy the file [`tasks/fifo.c`](./tasks/fifo.c) and run: `gcc fifo.c -o fifo && sudo ./fifo`.

> Important, **run it with `sudo`**! Or else the program won't have the necessary permissions to access the real-time kernel.

Example execution:

```shell
~/home/pi/tasks> sudo ./fifo
thread1 priority: 89
thread2 priority: 1
Calls made on thread1: 96075
Calls made on thread2: 3926
```

As you can see, the task with more priority runs a lot more times than the other one.

#### Earliest Deadline First (EDF)

This is the strictest scheduler available, and garantees that tasks will be ordered by the deadline of each task.
It should be used for tasks that must not be skipped.

Copy the file [`tasks/edf.c`](./tasks/edf.c) and run: `gcc edf+wait.c -o edf && ./edf`

The two tasks are configured with:

- Computing time = 10 ms
- Deadline = 11 ms
- Period = 22 ms

You can see the output of the command in the [`tasks/edf.txt`](./tasks/edf.txt) file.

In the output, you can see that the difference between the execution of the two tasks is always less than 11 ms, which proves that the scheduler is working.

## Intel RealSense

- Intel page: `https://dev.intelrealsense.com/docs/using-depth-camera-with-raspberry-pi-3`
- Manual intel: `https://github.com/IntelRealSense/librealsense/blob/development/doc/installation.md`
- `librealsense`: `https://github.com/IntelRealSense/librealsense`

## Other links

- `https://github.com/mathklk/realsense_raspberry_pi4`

RT-TESTS PER RASPBERRY PI
