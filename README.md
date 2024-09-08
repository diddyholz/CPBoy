# CPBoy

CPBoy is a work in progress DMG GameBoy emulator for the fx-CP400. It is based on the [Peanut-GB emulator by deltabeard](https://github.com/deltabeard/Peanut-GB).

Many games run with fullspeed with only one frame being skipped. Unfortunately, to make the emulator as fast as it is on such a low performing hardware, some accuracy had to be sacrificied which may cause some unexpected bugs with some roms.

## Getting Started

You will need to have the [Hollyhock-2 CFW](https://github.com/SnailMath/hollyhock-2/) installed. After that, extract the CPBoy.zip file from the [latest release](https://github.com/diddyholz/CPBoy/releases) to the root of your calculator. You should then have the following file structure on your calculator:
```
├── run.bin (Hollyhock launcher)
├── CPBoy.bin (Main CPBoy executable)
└── CPBoy/
    ├── bin/
    |   ├── il.bin
    |   └── y.bin
    └── roms/
        └── ** Put your roms in here **
``` 

To load your Gameboy ROMs, put them into the `/CPBoy/roms/` directory. They should have the file ending `.gbc`. CPBoy should then automatically detect the roms.


## Controls

The controls can be changed in the "Settings" tab in CPBoy.

These are the default controls:

| GB Action | Calculator key |
| --------- | -------------- |
| A         | EXE            |
| B         | +              |
| SELECT    | SHIFT          |
| START     | CLEAR          |
| UP        | UP             |
| DOWN      | DOWN           |
| LEFT      | LEFT           |
| RIGHT     | RIGHT          |
| Open Menu | (-)          |


## Games That Don't Work

 - Gameboy Colour games that are not backwards compatible with the Gameboy
 - Pokemon Pinball
 - Metroid 2 (Samus goes invisible in some areas)
 - Turok 2 (Crashes upon death)

If you encounter any issues with games, I will add them to this list.


## Building

If you want to build the emulator from source you will need the [Hollyhock-2 SDK + Newlib](https://github.com/SnailMath/hollyhock-2/). Then, run ´make´ in your terminal.
Or you can just push to your own fork of the repo and it will compile it and publish it as a release.


## License

This project is licensed under the MIT License.
