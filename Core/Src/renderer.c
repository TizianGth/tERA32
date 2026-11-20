#include "renderer.h"
#include "SSD1306.h"

#include <math.h>

camera_t camera = {
    .position = {25.0f, 25.0f, -30.0f},
    .focal_length = 80.0f
};

void render() 
{

    for(int j = 0; j<2;j++) {
    for(int i = 0; i<8;i++) {
        cube_t cube;
        init_cube(&cube, i*4, 20+j*4, j*4, 4);
        render_cube(&cube);
    }
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

void render_cube(cube_t* cube)
{
    bool visible[8];
    vec2_t projected[8];


    // Alle acht Vertices transformieren und projizieren
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

    // Kanten zwischen den projizierten Vertices zeichnen
    for (int i = 0; i < 12; i++) {
        uint8_t a = cube_edges[i][0];
        uint8_t b = cube_edges[i][1];

        // Kante überspringen, wenn ein Endpunkt
        // hinter der Kamera liegt
        if (!visible[a] || !visible[b]) {
            continue;
        }

        float x0 = projected[a].x;
        float y0 = projected[a].y;
        float x1 = projected[b].x;
        float y1 = projected[b].y;

        /*
         * Linien außerhalb des Displays überspringen.
         * Für einen Würfel, der das Display teilweise
         * verlässt, wäre echtes Line-Clipping nötig.
         */
        if (x0 < 0 || x0 >= SSD1306_WIDTH ||
            y0 < 0 || y0 >= SSD1306_HEIGHT ||
            x1 < 0 || x1 >= SSD1306_WIDTH ||
            y1 < 0 || y1 >= SSD1306_HEIGHT) {
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