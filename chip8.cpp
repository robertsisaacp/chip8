#include "chip8.h"
#include <iostream>
using namespace std;

void chip8::initialize()
{
    // initialize registers/memory
    programCount        = 0x200;
    opcode              = 0;
    indexReg            = 0;
    stackPointer        = 0;

    for(int i = 0; i < 2048; ++i) gfx[i] = 0;                       // clear display
    for(int i = 0; i < 16; ++i) stack[i] = 0;                       // clear stack
    for(int i = 0; i < 16; ++i) keypad[i] = V[i] = 0;               // clear keypad
    for(int i = 0; i < 4096; ++i) memory[i] = 0;                    // clear memory
    for(int i = 0; i < 80; ++i){ memory[i] = chip8_fontset[i]; }    // load fontset

    // reset timers
    delay_timer = sound_timer = 0;

    drawFlag = true;
    srand(time(NULL));
}

void chip8::emulateCycle()
{
    // fetch opcode
    opcode = memory[programCount] << 8 | memory[programCount + 1];
    printf("%X\n", opcode);
    programCount+=2;

    // process (decode and execute) opcode
    decode(opcode);

    // update timers
    if(delay_timer > 0){
        --delay_timer;
    }

    if(sound_timer > 0) {
        if(sound_timer == 1) {
            printf("BEEP!\n");
        }
        --sound_timer;
    }
}

bool chip8::loadGame(std::string filename)
{
    // open file, check if appropriate size
    ifstream input(filename, std::ios::binary | std::ios::ate);
    if(input){
        if(input.tellg() > (4096 - 512)){
            cout << "File too large; not enough memory available." << endl;
            return false;
        }
        else {
            input.seekg(0, input.beg);
        }
    }

    // copy data into buffer, then buffer into system memory
    vector<char> buffer((istreambuf_iterator<char>(input)), (istreambuf_iterator<char>()));
    for(int i = 0; i < buffer.size(); ++i){
        memory[i + 512] = buffer.at(i); // memory starts at 0x200
    }

    input.close();
    return true;
}

void chip8::setKeypad()
{

}

void chip8::decode(unsigned short op)
{
    switch(op & 0xF000) {
    case 0x0000:
        switch(opcode & 0x000F) {
        case 0x0000: // [00E0] clears the screen
            for(int i = 0; i < 2048; ++i) {
                gfx[i] = 0;
            }
            drawFlag = true;
            break;
        case 0x000E: // [00EE] returns from subroutine
            --stackPointer;                         // 16 levels of stack, decrease to avoid overwrite
            programCount = stack[stackPointer];     // put stored return address from stack into programCount
            programCount += 2;                      // increase programCount
            break;
        default:
            printf("Unknown opcode: 0x%X\n", opcode);
        }
        break;

    case 0x1000: // [1xxx] jumps to address xxx
        programCount = opcode & 0x0FFF;
        break;

    case 0x2000: // [2xxx] calls the subroutine at address xxx
        stack[stackPointer] = programCount;
        ++stackPointer;
        programCount = opcode & 0x0FFF;
        break;

    case 0x3000: // [3Xxx] skips next instruction if VX doesn't equal xx
        if(V[(opcode & 0x0f00) >> 8] == (opcode & 0x00FF)) {
            programCount += 4;
        }
        else {
            programCount += 2;
        }
        break;

    case 0x4000: // [4Xxx] skips the next instruction if VX doesn't equal xx
        if(V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
            programCount += 4;
        else
            programCount += 2;
        break;

    case 0x5000: // [5XY0] skips the next instruction if VX equals VY.
        if(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
            programCount += 4;
        else
            programCount += 2;
        break;

    case 0x6000: // [6Xxx] Sets VX to xx.
        V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
        programCount += 2;
        break;

    case 0x7000: // [7Xxx] Adds xx to VX.
        V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
        programCount += 2;
        break;

    case 0x8000:
        switch(opcode & 0x000F) {
        case 0x0000: // [8XY0] sets VX to the value of VY
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
            programCount += 2;
            break;

        case 0x0001: // [8XY1] sets VX to "VX OR VY"
            V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
            programCount += 2;
            break;

        case 0x0002: // [8XY2] sets VX to "VX AND VY"
            V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
            programCount += 2;
            break;

        case 0x0003: // [8XY3] sets VX to "VX XOR VY"
            V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
            programCount += 2;
            break;

        case 0x0004: // [8XY4] adds VY to VX. VF is set to 1 for carry, else 0
            if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])) {
                V[0xF] = 1; //carry
            }
            else {
                V[0xF] = 0;
            }
            V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
            programCount += 2;
            break;

        case 0x0005: // [8XY5] VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't
            if(V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8]) {
                V[0xF] = 0; // there is a borrow
            }
            else {
                V[0xF] = 1;
            }
            V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
            programCount += 2;
            break;

        case 0x0006: // [8XY6] shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift
            V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
            V[(opcode & 0x0F00) >> 8] >>= 1;
            programCount += 2;
            break;

        case 0x0007: // [8XY7] sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
            if(V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4]) {   // VY-VX
                V[0xF] = 0; // there is a borrow
            }
            else {
                V[0xF] = 1;
            }
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
            programCount += 2;
            break;

        case 0x000E: // [8XYE] shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift
            V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
            V[(opcode & 0x0F00) >> 8] <<= 1;
            programCount += 2;
            break;

        default:
            printf("Unknown opcode [0x8000]: 0x%X\n", opcode);
        }
        break;

    case 0x9000: // [9XY0] skips the next instruction if VX doesn't equal VY
        if(V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]){
            programCount += 4;
        }
        else {
            programCount += 2;
        }
        break;

    case 0xA000: // [Axxx] sets indexReg to the address xxx
        indexReg = opcode & 0x0FFF;
        programCount += 2;
        break;

    case 0xB000: // [Bxxx] jumps to the address xxx plus V0
        programCount = (opcode & 0x0FFF) + V[0];
        break;

    case 0xC000: // CXxx: Sets VX to a random number and xx
        V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
        programCount += 2;
        break;

    case 0xD000:  // draws sprite at corrdinate (VX, VY) that has width of 8 pixels and height
    {             // of n pixels.
        unsigned short x = V[(opcode & 0x0F00) >> 8];
        unsigned short y = V[(opcode & 0x00F0) >> 4];
        unsigned short height = opcode & 0x000F;
        unsigned short pixel;

        V[0xF] = 0;
        for(int yline = 0; yline < height; yline++) {
            pixel = memory[indexReg + yline];
            for(int xline = 0; xline < 8; xline++) {
                if((pixel & (0x80 >> xline)) != 0) {
                    if(gfx[(x + xline + ((y + yline) * 64))] == 1) {
                        V[0xF] = 1;
                    }
                    gfx[x + xline + ((y + yline) * 64)] ^= 1;
                }
            }
        }

        drawFlag = true;
        programCount += 2;
    }
        break;

    case 0xE000:
        switch(opcode & 0x00FF)
        {
        case 0x009E: // EX9E: Skips the next instruction if the key stored in VX is pressed
            if(keypad[V[(opcode & 0x0F00) >> 8]] != 0){
                programCount += 4;
            }
            else{
                programCount += 2;
            }
            break;

        case 0x00A1: // EXA1: Skips the next instruction if the key stored in VX isn't pressed
            if(keypad[V[(opcode & 0x0F00) >> 8]] == 0){
                programCount += 4;
            }
            else{
                programCount += 2;
            }
            break;

        default:
            printf ("Unknown opcode [0xE000]: 0x%X\n", opcode);
        }
        break;

    case 0xF000:
        switch(opcode & 0x00FF)
        {
        case 0x0007: // [FX07] sets VX to the value of the delay timer
            V[(opcode & 0x0F00) >> 8] = delay_timer;
            programCount += 2;
            break;

        case 0x000A: // [FX0A] a key press is awaited, and then stored in VX
        {
            bool keyPress = false;
            for(int i = 0; i < 16; ++i) {
                if(keypad[i] != 0) {
                    V[(opcode & 0x0F00) >> 8] = i;
                    keyPress = true;
                }
            }

            // no keyPress, skip cycle
            if(!keyPress) return;

            programCount += 2;
        }
            break;

        case 0x0015: // [FX15] sets the delay timer to VX
            delay_timer = V[(opcode & 0x0F00) >> 8];
            programCount += 2;
            break;

        case 0x0018: // [FX18] sets the sound timer to VX
            sound_timer = V[(opcode & 0x0F00) >> 8];
            programCount += 2;
            break;

        case 0x001E: // [FX1E] adds VX to indexReg
            if(indexReg + V[(opcode & 0x0F00) >> 8] > 0xFFF){    // VF is set to 1 when range overflow (indexReg+VX>0xFFF), and 0 when there isn't.
                V[0xF] = 1;
            }
            else{
                V[0xF] = 0;
            }
            indexReg += V[(opcode & 0x0F00) >> 8];
            programCount += 2;
            break;

        case 0x0029: // [FX29] sets indexReg to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font
            indexReg = V[(opcode & 0x0F00) >> 8] * 0x5;
            programCount += 2;
            break;

        case 0x0033: // [FX33] stores the Binary-coded decimal representation of VX at the addresses indexReg, indexReg plus 1, and indexReg plus 2
            memory[indexReg]     = V[(opcode & 0x0F00) >> 8] / 100;
            memory[indexReg + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
            memory[indexReg + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
            programCount += 2;
            break;

        case 0x0055: // [FX55] stores V0 to VX in memory starting at address indexReg
            for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i){
                memory[indexReg + i] = V[i];
            }

            // on the original interpreter, when the operation is done, indexReg = indexReg + X + 1.
            indexReg += ((opcode & 0x0F00) >> 8) + 1;
            programCount += 2;
            break;

        case 0x0065: // [FX65] Fills V0 to VX with values from memory starting at address indexReg
            for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i){
                V[i] = memory[indexReg + i];
            }

            // on the original interpreter, when the operation is done, indexReg = indexReg + X + 1.
            indexReg += ((opcode & 0x0F00) >> 8) + 1;
            programCount += 2;
            break;

        default:
            printf ("Unknown opcode [F000]: 0x%X\n", opcode);
        }
        break;

    default:
        printf("Unknown opcode: %X\n", opcode);
    }
}

