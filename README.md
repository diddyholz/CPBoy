# CPBoy

CPBoy is a work in progress DMG GameBoy emulator for the fx-CP400. It based on the [Peanut-GB emulator by deltabeard](https://github.com/deltabeard/Peanut-GB).

Since the CAS has a rather slow processor, a high speed emulator core is mandatory to run emulation at an acceptable speed. Unfortunately, the accuracy of the emulation suffers a bit, which causes some games to be unplayable or have bugs. 


## Getting Started

You will need to have the [Hollyhock-2 CFW](https://github.com/SnailMath/hollyhock-2/) installed. After that just copy the most recent CPBoy.hkk from the releases tab into the root directory of your calculator. 

To load your Gameboy ROM, create a directory named "roms" and copy it to that to "usr/CPBoy" directory on your calculator. (if the folders "usr" and "usr/CPBoy" do not exist, create them).

The program should automatically detect the roms and create a selection menu.


## Controls

| Calculator key | GB key |
| -------------- | ------ |
| EXE						 | A      |
| +   					 | B      |
| SHIFT 				 | SELECT |
| CLEAR					 | START  |
| UP	 					 | UP		  |
| DOWN					 | DOWN	  |
| LEFT					 | LEFT	  |
| RIGHT					 | RIGHT  |

**MISC:**
 - (-): Exit emulator
 - KEYBOARD: Toggle frameskipping (When enabled, only render every second frame, which speeds up the emulator a bit but lowers its FPS)
 - BACKSPACE: Enable Interlacing (When enabled, only render every second line and switch every frame, which speeds up the emulator a bit but makes visuals worse)


## Building

If you want to build the emulator from source you will need the [Hollyhock-2 SDK](https://github.com/SnailMath/hollyhock-2/). Then, run ´make´ in your terminal to build it.


## License

This project is licensed under the MIT License.
