#include "Cube.h"

void init_cube(cube_t* cube, uint8_t x, uint8_t y, uint8_t z, uint8_t size)
{
    cube->vertices[0]  = x;
    cube->vertices[1]  = y;
    cube->vertices[2]  = z;

    cube->vertices[3]  = x + size;
    cube->vertices[4]  = y;
    cube->vertices[5]  = z;

    cube->vertices[6]  = x + size;
    cube->vertices[7]  = y + size;
    cube->vertices[8]  = z;

    cube->vertices[9]  = x;
    cube->vertices[10] = y + size;
    cube->vertices[11] = z;

    // Rückseite
    cube->vertices[12] = x;
    cube->vertices[13] = y;
    cube->vertices[14] = z + size;

    cube->vertices[15] = x + size;
    cube->vertices[16] = y;
    cube->vertices[17] = z + size;

    cube->vertices[18] = x + size;
    cube->vertices[19] = y + size;
    cube->vertices[20] = z + size;

    cube->vertices[21] = x;
    cube->vertices[22] = y + size;
    cube->vertices[23] = z + size;
}