#include <iostream>
#include <OPENGL/gl.h>
#include <GLUT/glut.h>
#include "chip8.h"

#define PATH "/Users/isaacroberts/ClionProjects/chip8/c8games/PONG"
using namespace std;

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

// setup window
int scale = 10;
int display_width = SCREEN_WIDTH * scale;
int display_height = SCREEN_HEIGHT * scale;

// chip8
chip8 localChip8;

void display();
void reshape_window(GLsizei w, GLsizei h);
void keypadboardUp(unsigned char keypad, int x, int y);
void keypadboardDown(unsigned char keypad, int x, int y);

// Use new drawing method
#define DRAWWITHTEXTURE
typedef unsigned char BYTE;
BYTE screenData[SCREEN_HEIGHT][SCREEN_WIDTH][3];
void setupTexture();

int main(int argc, char **argv)
{

    localChip8.loadGame(PATH);

    // Setup OpenGL
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

    glutInitWindowSize(display_width, display_height);
    glutInitWindowPosition(320, 320);
    glutCreateWindow("chip8");

    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutReshapeFunc(reshape_window);
    glutKeyboardFunc(keypadboardDown);
    glutKeyboardUpFunc(keypadboardUp);

//#ifdef DRAWWITHTEXTURE
//    setupTexture();
//#endif

    glutMainLoop();

    return 0;
}

// Setup Texture
void setupTexture()
{
    // Clear screen
    for(int y = 0; y < SCREEN_HEIGHT; ++y)
        for(int x = 0; x < SCREEN_WIDTH; ++x)
            screenData[y][x][0] = screenData[y][x][1] = screenData[y][x][2] = 0;

    // Create a texture 
    glTexImage2D(GL_TEXTURE_2D, 0, 3, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)screenData);

    // Set up the texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    // Enable textures
    glEnable(GL_TEXTURE_2D);
}

void updateTexture(const chip8& c8)
{
    // Update pixels
    for(int y = 0; y < 32; ++y)
        for(int x = 0; x < 64; ++x)
            if(c8.gfx[(y * 64) + x] == 0)
                screenData[y][x][0] = screenData[y][x][1] = screenData[y][x][2] = 0;	// Disabled
            else
                screenData[y][x][0] = screenData[y][x][1] = screenData[y][x][2] = 255;  // Enabled

    // Update Texture
    glTexSubImage2D(GL_TEXTURE_2D, 0 ,0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)screenData);

    glBegin( GL_QUADS );
    glTexCoord2d(0.0, 0.0);		glVertex2d(0.0,			  0.0);
    glTexCoord2d(1.0, 0.0); 	glVertex2d(display_width, 0.0);
    glTexCoord2d(1.0, 1.0); 	glVertex2d(display_width, display_height);
    glTexCoord2d(0.0, 1.0); 	glVertex2d(0.0,			  display_height);
    glEnd();
}

// Old gfx code
void drawPixel(int x, int y)
{
    glBegin(GL_QUADS);
    glVertex3f((x * scale) + 0.0f,     (y * scale) + 0.0f,	 0.0f);
    glVertex3f((x * scale) + 0.0f,     (y * scale) + scale, 0.0f);
    glVertex3f((x * scale) + scale, (y * scale) + scale, 0.0f);
    glVertex3f((x * scale) + scale, (y * scale) + 0.0f,	 0.0f);
    glEnd();
}

void updateQuads(const chip8& c8)
{
    // Draw
    for(int y = 0; y < 32; ++y)
        for(int x = 0; x < 64; ++x)
        {
            if(c8.gfx[(y*64) + x] == 0)
                glColor3f(0.0f,0.0f,0.0f);
            else
                glColor3f(1.0f,1.0f,1.0f);

            drawPixel(x, y);
        }
}

void display()
{
    localChip8.emulateCycle();

    if(localChip8.drawFlag)
    {
        // Clear framebuffer
        glClear(GL_COLOR_BUFFER_BIT);

#ifdef DRAWWITHTEXTURE
        updateTexture(localChip8);
#else
        updateQuads(localChip8);
#endif

        // Swap buffers!
        glutSwapBuffers();

        // Processed frame
        localChip8.drawFlag = false;
    }
}

void reshape_window(GLsizei w, GLsizei h)
{
    glClearColor(0.0f, 0.0f, 0.5f, 0.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, h, 0);
    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, w, h);

    // Resize quad
    display_width = w;
    display_height = h;
}

void keypadboardDown(unsigned char keypad, int x, int y)
{
    if(keypad == 27)    // esc
        exit(0);

    if(keypad == '1')		localChip8.keypad[0x1] = 1;
    else if(keypad == '2')	localChip8.keypad[0x2] = 1;
    else if(keypad == '3')	localChip8.keypad[0x3] = 1;
    else if(keypad == '4')	localChip8.keypad[0xC] = 1;

    else if(keypad == 'q')	localChip8.keypad[0x4] = 1;
    else if(keypad == 'w')	localChip8.keypad[0x5] = 1;
    else if(keypad == 'e')	localChip8.keypad[0x6] = 1;
    else if(keypad == 'r')	localChip8.keypad[0xD] = 1;

    else if(keypad == 'a')	localChip8.keypad[0x7] = 1;
    else if(keypad == 's')	localChip8.keypad[0x8] = 1;
    else if(keypad == 'd')	localChip8.keypad[0x9] = 1;
    else if(keypad == 'f')	localChip8.keypad[0xE] = 1;

    else if(keypad == 'z')	localChip8.keypad[0xA] = 1;
    else if(keypad == 'x')	localChip8.keypad[0x0] = 1;
    else if(keypad == 'c')	localChip8.keypad[0xB] = 1;
    else if(keypad == 'v')	localChip8.keypad[0xF] = 1;

    //printf("Press keypad %c\n", keypad);
}

void keypadboardUp(unsigned char keypad, int x, int y)
{
    if (keypad == '1') localChip8.keypad[0x1] = 0;
    else if (keypad == '2') localChip8.keypad[0x2] = 0;
    else if (keypad == '3') localChip8.keypad[0x3] = 0;
    else if (keypad == '4') localChip8.keypad[0xC] = 0;

    else if (keypad == 'q') localChip8.keypad[0x4] = 0;
    else if (keypad == 'w') localChip8.keypad[0x5] = 0;
    else if (keypad == 'e') localChip8.keypad[0x6] = 0;
    else if (keypad == 'r') localChip8.keypad[0xD] = 0;

    else if (keypad == 'a') localChip8.keypad[0x7] = 0;
    else if (keypad == 's') localChip8.keypad[0x8] = 0;
    else if (keypad == 'd') localChip8.keypad[0x9] = 0;
    else if (keypad == 'f') localChip8.keypad[0xE] = 0;

    else if (keypad == 'z') localChip8.keypad[0xA] = 0;
    else if (keypad == 'x') localChip8.keypad[0x0] = 0;
    else if (keypad == 'c') localChip8.keypad[0xB] = 0;
    else if (keypad == 'v') localChip8.keypad[0xF] = 0;
}