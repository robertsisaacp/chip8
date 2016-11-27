//
// Created by Isaac Roberts on 11/27/16.
//

#ifndef CHIP8_CHIP8_H
#define CHIP8_CHIP8_H

#include <string>
#include <fstream>
#include <vector>

class chip8 {
public:
    chip8(){}
    ~chip8(){}

    bool loadGame(std::string);
    void emulateCycle();
    void decode(unsigned short);
    void setKeypad();

    bool drawFlag;

    /* The graphics system: The chip 8 has one instruction that
     * draws sprite to the screen. Drawing is done in XOR mode
     * and if a pixel is turned off as a result of drawing, the
     * VF register is set. This is used for collision detection.
     * The graphics of the Chip 8 are black and white and the screen
     * has a total of 2048 pixels (64 x 32). This can easily be
     * implemented using an array that hold the pixel state (1 or 0) */
    unsigned char gfx[64 * 32];

    /* Chip 8 has hexadecimal-based keypad (0x0-0xf)
     * array keypad[16] to store current state */
    unsigned char keypad[16];

private:
    void initialize();
    /* 35 opcodes */
    unsigned short opcode;

    /* 4K memory */
    /* 0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
     * 0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
     * 0x200-0xFFF - Program ROM and work RAM */
    unsigned char memory[4096];

    /* 15 8-bit general purpose registers V0 -> VE
     * . VF (register 16) is reserved for a "carry
     * flag." */
    unsigned char V[16];

    /* index register i and program counter PC */
    unsigned short indexReg;
    unsigned short programCount;


    /* Interrupts and hardware registers. The Chip 8 has none,
     * but there are two timer registers that count at 60 Hz.
     * When set above zero they will count down to zero. */
    unsigned char delay_timer;
    unsigned char sound_timer;

    /* stack pointer has 16 levels, stack pointer sp for remembering
     * which level of stack used */
    unsigned short stack[16];
    unsigned short stackPointer;

    /* This is the Chip 8 font set. Each number or
     * character is 4 pixels wide and 5 pixel high */
    unsigned char chip8_fontset[80] =
            {
                    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
                    0x20, 0x60, 0x20, 0x20, 0x70, // 1
                    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
                    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
                    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
                    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
                    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
                    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
                    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
                    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
                    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
                    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
                    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
                    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
                    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
                    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
            };
};

#endif //CHIP8_CHIP8_H
