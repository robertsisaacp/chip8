
#include "chip8.h"
#include <iostream> /////////
void chip8::initialize()
{
    // initialize registers/memory

    programCount        = 0x200;
    opcode              = 0;
    indexReg            = 0;
    stackPointer        = 0;

    for(int i = 0; i < 2048; ++i) gfx[i] = 0;           // clear display
    for(int i = 0; i < 16; ++i) stack[i] = 0;           // clear stack
    for(int i = 0; i < 16; ++i) keypad[i] = V[i] = 0;   // clear keypad
    for(int i = 0; i < 4096; ++i) memory[i] = 0;        // clear memory

    // load fontset
    for(int i = 0; i < 80; ++i){
        memory[i] = chip8_fontset[i];
    }

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

    // decode and execute opcode
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

void chip8::loadGame(std::string file)
{
    std::ifstream input(file, std::ios::binary);
    // copy data into buffer
    std::vector<char> buffer((std::istreambuf_iterator<char>(input)),
            (std::istreambuf_iterator<char>()));

    for(int i = 0; i < buffer.size(); ++i){
        // NOTE: recall memory starts at 0x200
        memory[i + 512] = buffer[i];
        // printf("%X\n", buffer[i]); // test if memory is loading properly
    }
}

void chip8::setKeypad()
{

}



void chip8::decode(unsigned short op)
{
    switch(op & 0xF000)
    {
        case 0x0000: //
            switch(opcode & 0x000F)
            {
            case 0x0000: // 0x00E0 - clears the screen
                break;

            }

        case 0x00E0: // clears the screen
            break;

        case 0x00EE: // returns from a subroutine
            break;

        case 0x1000: // jumps to address xxx
            break;

        case 0x2000: // calls the subroutine at address xxx
            stack[stackPointer] = programCount;
            ++stackPointer;
            programCount = opcode & 0x0FFF;
            break;

        case 0x0004: // adds Vx to Vy; Register VF is set to 1 when there is a carry
            if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
                V[0xF] = 1; //carry
            else
                V[0xF] = 0;
            V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
            programCount += 2;
            break;

        case 0x0033: /* Stores the BCD representation of Vx at addresses indexReg,
                      * indexReg + 1, and indexReg + 2 */
            memory[indexReg]     = V[(opcode & 0x0F00) >> 8] / 100;
            memory[indexReg + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
            memory[indexReg + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
            programCount += 2;
            break;

        case 0xA000: // Axxx - MEM - Sets indexReg to the address xxx
            indexReg = op & 0x0FFF;
            programCount += 2;
            break;

        case 0xE000: // EX9E: Skips the next instruction if the key stored in VX is pressed
            switch(opcode & 0x00FF) {
            case 0x009E:
                if (keypad[V[(opcode & 0x0F00) >> 8]] != 0)
                    programCount += 4;
                else
                    programCount += 2;
                break;
            }
                // TODO: find better way...
            case 0xD000:
            {
                unsigned short x = V[(opcode & 0x0F00) >> 8];
                unsigned short y = V[(opcode & 0x00F0) >> 4];
                unsigned short height = opcode & 0x000F;
                unsigned short pixel;

                V[0xF] = 0;
                for (int yline = 0; yline < height; yline++)
                {
                    pixel = memory[indexReg + yline];
                    for(int xline = 0; xline < 8; xline++)
                    {
                        if((pixel & (0x80 >> xline)) != 0)
                        {
                            if(gfx[(x + xline + ((y + yline) * 64))] == 1)
                                V[0xF] = 1;
                            gfx[x + xline + ((y + yline) * 64)] ^= 1;
                        }
                    }
                }

                drawFlag = true;
                programCount += 2;
            }
                break;
            // more opcodes to come...

    default:
        printf("Unknown opcode: %X\n", opcode);
    }
}

