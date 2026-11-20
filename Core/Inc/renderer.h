#pragma once

#include "cube.h"

#include <stdbool.h>
#include <stdint.h>

#define NEAR_PLANE 0.001f

typedef struct {
    float x;
    float y;
} vec2_t;

typedef struct {
    float x;
    float y;
    float z;
} vec3_t;

typedef struct {
    vec3_t position;
    float focal_length;
} camera_t;

void render();

void draw_line(vec2_t p0, vec2_t p1);

bool project_vertex(
    vec3_t world,
    camera_t camera,
    vec2_t *out);

void render_cube(cube_t* cube);

extern camera_t camera;
