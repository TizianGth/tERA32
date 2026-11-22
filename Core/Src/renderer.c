#include "renderer.h"
#include "SSD1306.h"

#include <math.h>

camera_t camera = {
    .position = {-30.0f, 25.0f, -30.0f},
    .focal_length = 80.0f
};

cube_t cubes[8];

void render_init() {
    for(int i = 0; i<8;i++) {
        init_cube(&cubes[i], i*10, 10, 10, 10);
    }
}

void render() 
{
    SSD1306_ClearArea(0, 0, SSD1306_WIDTH, SSD1306_HEIGHT);
    for(int i = 0; i<8;i++) {
        render_cube(&cubes[i]);
    }

    SSD1306_Try_Update();
}

void draw_line(vec2_t p0, vec2_t p1) {
    SSD1306_Draw_Line((uint8_t)p0.x, (uint8_t)p0.y, (uint8_t)p1.x, (uint8_t)p1.y);
}

bool project_vertex(
    vec3_t world,
    camera_t camera,
    vec2_t *out)
{
    float x = world.x - camera.position.x;
    float y = world.y - camera.position.y;
    float z = world.z - camera.position.z;

    if (z <= NEAR_PLANE) {
        return false;
    }

    out->x = (SSD1306_WIDTH / 2.0f)
           + camera.focal_length * x / z;

    out->y = (SSD1306_HEIGHT / 2.0f)
           - camera.focal_length * y / z;

    return true;
}

#include <stdbool.h>

#define CLIP_LEFT   1
#define CLIP_RIGHT  2
#define CLIP_TOP    4
#define CLIP_BOTTOM 8

static int Compute_OutCode(float x, float y)
{
    int code = 0;

    if (x < 0)
        code |= CLIP_LEFT;
    else if (x >= SSD1306_WIDTH)
        code |= CLIP_RIGHT;

    if (y < 0)
        code |= CLIP_TOP;
    else if (y >= SSD1306_HEIGHT)
        code |= CLIP_BOTTOM;

    return code;
}

static bool Clip_Line(float *x0, float *y0,
                      float *x1, float *y1)
{
    int code0 = Compute_OutCode(*x0, *y0);
    int code1 = Compute_OutCode(*x1, *y1);

    while (1) {
        if (!(code0 | code1)) {
            return true; 
        }

        if (code0 & code1) {
            return false;
        }

        int code_out = code0 ? code0 : code1;

        float x, y;

        if (code_out & CLIP_TOP) {
            y = 0;
            x = *x0 + (*x1 - *x0) *
                (y - *y0) / (*y1 - *y0);
        }
        else if (code_out & CLIP_BOTTOM) {
            y = SSD1306_HEIGHT - 1;
            x = *x0 + (*x1 - *x0) *
                (y - *y0) / (*y1 - *y0);
        }
        else if (code_out & CLIP_RIGHT) {
            x = SSD1306_WIDTH - 1;
            y = *y0 + (*y1 - *y0) *
                (x - *x0) / (*x1 - *x0);
        }
        else {
            x = 0;
            y = *y0 + (*y1 - *y0) *
                (x - *x0) / (*x1 - *x0);
        }

        if (code_out == code0) {
            *x0 = x;
            *y0 = y;
            code0 = Compute_OutCode(*x0, *y0);
        } else {
            *x1 = x;
            *y1 = y;
            code1 = Compute_OutCode(*x1, *y1);
        }
    }
}

void render_cube(cube_t* cube)
{
    bool visible[8];
    vec2_t projected[8];

    for (int i = 0; i < 8; i++) {
        vec3_t world = {
            .x = cube->vertices[i * 3],
            .y = cube->vertices[i * 3 + 1],
            .z = cube->vertices[i * 3 + 2]
        };

        visible[i] = project_vertex(
            world,
            camera,
            &projected[i]
        );
    }

for (int i = 0; i < 12; i++) {
    uint8_t a = cube_edges[i][0];
    uint8_t b = cube_edges[i][1];

    if (!visible[a] || !visible[b]) {
        continue;
    }

    float x0 = projected[a].x;
    float y0 = projected[a].y;
    float x1 = projected[b].x;
    float y1 = projected[b].y;

    if (!Clip_Line(&x0, &y0, &x1, &y1)) {
        continue;
    }

    SSD1306_Draw_Line(
        (uint8_t)lroundf(x0),
        (uint8_t)lroundf(y0),
        (uint8_t)lroundf(x1),
        (uint8_t)lroundf(y1)
    );
}
  }