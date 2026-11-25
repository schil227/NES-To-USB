# NES-To-USB
Converts NES controller input into a USB HID device using an Arduino Uno

This code is based on the [N64-To-USB](https://github.com/schil227/N64-To-USB) project, which is in turn forked from another project, etc. While the NES_To_USB.ino sketch was written from scratch (with heavy inspiration), the Arduino-joystick stuff is mostly identical, save for one or two changes.

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

# The Code
(Based on the work done by https://tresi.github.io/nes/, which explains it all very well, including a sequence diagram)

The protocol of the NES controller functions rather different from future generations, in that it uses 3 dedicated wires for transmitting signal related things. These are:
- **Latch**: A command sent from the console to the controller, informing the controller to "freeze" the current button states. It's the first thing sent by the controller, which is a high signal for 12us, then low.
- **Pulse**: Also sent from the controller, it consists of 8 high/low pulses, which last for 6us each (12us per pulse). Each high pulse tells the controller to output the next state on the **Data** line.
- **Data**: Sent from the controller to the console. At the signal from the **Latch** command, it outputs the inverted state of the buttons, meaning high when the button is released, and low when the button is pressed. It begins with the 'A' Button at the start of the **Latch** call, and for each **Pulse** it cycles on to the next buttons in order ('A', 'B', 'Select', 'Start', 'Up', 'Down', 'Left', 'Right'). The final pulse tells it that the request is done, and the buttons can be "unlatched".

This protocol is noticeably simpler than the one for the N64 controller, which used 1 two-way data-line, and therefore had to encode high/low on the fidelity of 1us intervals. Fewer buttons and longer intervals also means that the code doesn't need to be nearly as tight, allowing for some quality of life (like a `for` loop!)

### NES_send()

Looking at the NES_To_USB.ino sketch, the `NES_send()` command is very simple; set the latch line high, wait for 12us, and then set it to low. This could've probably been written like

```c++
digitalWrite(NES_LATCH_PIN, high);
delayMicroseconds(12);
digitalWrite(NES_LATCH_PIN, low);
```

But that's lame as shit. Instead, we write to PORTD directly in macros, and grab a handful of `nop`s instead:

```c++
  NES_COMMAND_HIGH;

  // block 12us 
  // each row is 16 NOPs, each NOP use 1 cycle - Arduino Uno operates at 16Mhz, or 16 operations per 1us.
  // therefore, 16 NOPs takes 1us, 12 rows makes 12us.
  asm volatile(
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
    );
    // Beautiful.

    NES_COMMAND_LOW;
```

So to some degree, this runs at a high accuracy. 

### NES_get()

Fundamentally rather dumb, this method reads the 'A' button state immediately (as it's ready before the first pulse), with more `asm` waiting switches the **Pulse** on and off, while querying the **Data** line in between. I say it's dumb, because it's making several accuracy assumptions, taking into account only *some* the cycle costs (like NES_PULSE_HIGH/LOW, NES_DATA_QUERY) but ignoring the fact that it's being executed in a `for` loop. Now that `for` loop is probably getting optimized away, but there's the other dumb part; I'm not actually checking the assembly output to see if these timings are accurate. Now if the NES had a couple dozen buttons, then yes that may be of concern - but I'm lazy, and this worked, and if I measure with my feet then it all comes out to be about right :^). 

### ... and the rest

Outside of the send/get commands, everything can be done with some leisure. The "raw data" is then shifted into `NES_state`, a `struct` which holds the button state and is ultimately written out via serial print. This serial data is interpreted by the ArduinoJoystick code, which is configured to be 8 buttons in the `Descriptors.c` file - I won't get into that nonsense. However, if you want to get into that nonsense, I'll echo what I stuck in that readme.txt here: you're gonna wanna use LUFA version 111009 (https://github.com/abcminiuser/lufa/tree/LUFA-111009), otherwise you'll be in a world of shit trying to get it to compile and run. 

# Conclusion
As I said in so many words at the beginning, I stand on the shoulders of giants; those who broke out their oscilloscope to poke the controllers with, those who wrote other interpreters to handle the high-precision timing of the controller, and those who understand the black-arts of USB devices. Truly my work is nothing special, and surely the number of NES controllers in the world only decreases day-by-day - but it was a fun little thing to do.
