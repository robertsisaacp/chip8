# chip-8

## recreating and emulating the chip-8 virtual machine

Using Laurence Muller's excellent guide (listed below) I'm attempting to recreate and emulate the Chip-8 architecture in C++. Whereas Laurence's guide makes heavy use of C-style primitives; and logic (switch cases.. switch cases everywhere), my own goal is to modernize the code trading some C-level speed for the conveniences and flexibility of C++11's (and 14) features.

There will be lambdas.

References used:
https://en.wikipedia.org/wiki/CHIP-8
http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/
http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
