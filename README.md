# CPBoy

CPBoy is a work in progress DMG GameBoy emulator for the fx-CP400. It based on the [Peanut-GB emulator by deltabeard](https://github.com/deltabeard/Peanut-GB).

Since the CAS has a rather slow processor, a high speed emulator core is mandatory to run emulation at an acceptable speed. Unfortunately, the accuracy of the emulation suffers a bit, which causes some games to be unplayable or have bugs. 


## Getting Started

You will need to have the [Hollyhock-2 CFW](https://github.com/SnailMath/hollyhock-2/) installed. After that just copy the most recent CPBoy.bin from the releases tab into the root directory of your calculator. 

To load your Gameboy ROM, create a directory named "CPBoy" and in there another directory "roms" and copy it to that directory on your calculator. The program should automatically detect the roms and create a selection menu.


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

## Games That Don't Work

 - Gameboy Colour games that are not backwards compatible with the Gameboy
 - Pokemon Pinball
 - Pokemon Red
 - Metroid 2 (Samus goes invisible in some areas)
 - Turok 2 (Crashes upon death)

## Building

If you want to build the emulator from source you will need the [Hollyhock-2 SDK + Newlib](https://github.com/SnailMath/hollyhock-2/). Then, run ´make´ in your terminal to build it.


## License

This project is licensed under the MIT License.
