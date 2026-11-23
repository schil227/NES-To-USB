# NES-To-USB
Converts NES controller input into a USB HID device using an Arduino Uno

This code is based on the [N64-To-USB](https://github.com/schil227/N64-To-USB) project, which is in turn forked from another project, etc. While the NES_To_USB.ino sketch was written from scratch (with heavy inspiration), the Arduino-joystick stuff is mostly identical, save for one or two changes.

# Info

## What it does
- Supports 1 NES controller as a Game Controller USB HID (Human Interface Device)

## Required
- NES Controller
- Arduino Uno (others will probably work, use at your own knowledge/risk)
- USB type B connector
- 5 Wires
- (Recommended) a NES Controller 7-pin Female port
    - You can probably just jam some pins into your NES controller, but careful buddy those things are antiques now, you know.

In addition, you'll need some software:
- [dfu-programmer](https://github.com/dfu-programmer/dfu-programmer/releases)
- (and maybe) [zadig](https://zadig.akeo.ie/)

## How to set it up (short)
```
NES Controller Pins
          +----> Power  (white)
          |
5 +---------+  7
  | x  x  o   \
  | o  o  o  o |
4 +------------+ 1
    |  |  |  |
    |  |  |  +-> Ground (brown)
    |  |  +----> Pulse  (red)
    |  +-------> Latch  (orange)
    +----------> Data   (yellow)
```

(Copied from the much better repo https://tresi.github.io/nes/)

- Download repo
- Upload Arduino sketch to Arduino Uno
- Connect the controller:
    - Pin 7 (Power) to 5v
    - Pin 4 (Data) to Digital Pin 4
    - Pin 3 (Latch) to Digital Pin 2
    - Pin 2 (Pulse) to Digital Pin 7
    - Pin 1 (Ground) to Ground
- Boot Arduino into Device Mode (google it)
- Download DFU Programmer, run the following (in this case, I'm in the same directory as the dfu executable)
	- `.\dfu-programmer.exe atmega16u2 erase`
	- `.\dfu-programmer.exe atmega16u2 flash path_to_repo_code\NES-To-USB\NES_To_USB_Atmega16u2\Arduino-joystick.hex`
	- `.\dfu-programmer.exe atmega16u2 reset`
		- To turn your Arduino back into an Arduino, go and find `Arduino-usbserial-atmega16u2-Uno-Rev3.hex` wherever your Arduino stuff is installed, and repeat the above (google it)
- Observe your NES controller is now a USB game controller\* (e.g. in Windows, check out "setup game controller", find one called "Arduino Joystick")

\*You _may_ have to change the device into a WinUSB device, which would require something like zadig
