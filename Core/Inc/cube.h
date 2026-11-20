#pragma once

#include <stdint.h>

static const uint8_t cube_edges[12][2] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0},
    {4, 5}, {5, 6}, {6, 7}, {7, 4},
    {0, 4}, {1, 5}, {2, 6}, {3, 7}
};

typedef struct {
    uint8_t vertices[8*3];
} cube_t;

void init_cube(cube_t* cube, uint8_t x, uint8_t y, uint8_t z, uint8_t size);