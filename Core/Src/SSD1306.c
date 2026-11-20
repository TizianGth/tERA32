#include "SSD1306.h"
#include "FreeSans10pt7b.h"

#include "cube.h"

#include <math.h>

#define ADDRESS 0x3C << 1 // 7-bit address shifted for HAL functions

cube_t cube;

uint8_t buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

I2C_HandleTypeDef* SSD1306_HI2C1 = NULL;

const GFXfont *font = &FreeSans10pt7b;

#define NEAR_PLANE 0.01f
bool project_vertex(
    vec3_t world,
    vec3_t camera,
    float focal_length,
    screen_point_t *out)
{
    // Weltkoordinaten -> Kamerakoordinaten
    float x = world.x - camera.x;
    float y = world.y - camera.y;
    float z = world.z - camera.z;

    // Punkt liegt auf oder hinter der Kamera
    if (z <= NEAR_PLANE) {
        return false;
    }

    // Perspektivische Projektion
    out->x = (SSD1306_WIDTH / 2.0f)
           + focal_length * x / z;

    out->y = (SSD1306_HEIGHT / 2.0f)
           - focal_length * y / z;

    return true;
}

void SSD1306_Draw_String(int x, int y, const char* str)
{
    while (*str)
    {
        char c = *str++;

        const GFXglyph *glyph =
            &font->glyph[c - font->first];

        SSD1306_Draw_Char(x, y, c);

        x += glyph->xAdvance;
    }
}

void SSD1306_Draw_Char(int x, int y, char c) {
    y += (int)(font->yAdvance+1)/2; // Adjust y to vertically center the text; rounding up
    if (c < FreeSans10pt7b.first || c > FreeSans10pt7b.last)
        return;

    const GFXglyph *glyph =
        &FreeSans10pt7b.glyph[c - FreeSans10pt7b.first];

    const uint8_t *bitmap =
        FreeSans10pt7b.bitmap + glyph->bitmapOffset;

    uint8_t w = glyph->width;
    uint8_t h = glyph->height;

    uint8_t bit = 0;
    uint8_t bits = 0;

    for (uint8_t yy = 0; yy < h; yy++)
    {
        for (uint8_t xx = 0; xx < w; xx++)
        {
            if ((bit++ & 7) == 0)
            {
                bits = *bitmap++;
            }

            if (bits & 0x80)
            {
                SSD1306_Set_Pixel(
                    x + glyph->xOffset + xx,
                    y + glyph->yOffset + yy
                );
            }

            bits <<= 1;
        }
    }
}

void SSD1306_Init(I2C_HandleTypeDef* hi2c)
{
    
  SSD1306_HI2C1 = hi2c;
  uint8_t init[] = {
    0x00,
    0xAE,
    0x20, 0x00,
    0xB0,
    0xC8,
    0x00,
    0x10,
    0x40,
    0x81, 0xFF,
    0xA1,
    0xA6,
    0xA8, 0x3F,
    0xA4,
    0xD3, 0x00,
    0xD5, 0xF0,
    0xD9, 0x22,
    0xDA, 0x12,
    0xDB, 0x20,
    0x8D, 0x14,
    0xAF
  };

  HAL_I2C_Master_Transmit(SSD1306_HI2C1, ADDRESS, init, sizeof(init), 100);

  init_cube(&cube, 20, 20, 0, 10);
  

  {
    vec3_t camera = {
        .x = 25.0f,
        .y = 25.0f,
        .z = -30.0f
    };
    float focal_length = 80.0f;
    screen_point_t projected[8];
    bool visible[8];

    // Alle acht Vertices transformieren und projizieren
    for (int i = 0; i < 8; i++) {
        vec3_t world = {
            .x = cube.vertices[i * 3],
            .y = cube.vertices[i * 3 + 1],
            .z = cube.vertices[i * 3 + 2]
        };

        visible[i] = project_vertex(
            world,
            camera,
            focal_length,
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

  SSD1306_Try_Update();
}

void SSD1306_Set_Pixel(uint8_t x, uint8_t y)
{
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;
    buffer[(y / 8) * SSD1306_WIDTH + x] |= (1u << (y & 7));
}

bool SSD1306_Try_Update()
{
    if (SSD1306_HI2C1 &&
        HAL_I2C_IsDeviceReady(SSD1306_HI2C1, ADDRESS, 1, 100) != HAL_OK)
            return false;

    for (uint8_t page = 0; page < 8; page++)
    {
        uint8_t cmd[] = {
            0x00,
            (uint8_t)(0xB0 + page),
            0x00,
            0x10
        };

        HAL_I2C_Master_Transmit(SSD1306_HI2C1, ADDRESS, cmd, 4, 100);

        uint8_t data[129];
        data[0] = 0x40;

        for (uint8_t i = 0; i < 128; i++)
        {
            data[i + 1] = buffer[page * 128 + i];
        }

        HAL_I2C_Master_Transmit(SSD1306_HI2C1, ADDRESS, data, 129, 100);
    }
    return true;
}

int abs(int x) {
    return (x < 0) ? -x : x;
}

void SSD1306_Draw_Line(uint8_t x0, uint8_t y0,
                       uint8_t x1, uint8_t y1)
{
    int16_t dx = abs((int)x1 - (int)x0);
    int16_t dy = abs((int)y1 - (int)y0);

    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;

    int16_t err = dx - dy;

    while (1)
    {
        SSD1306_Set_Pixel(x0, y0);

        if (x0 == x1 && y0 == y1)
            break;

        int16_t e2 = 2 * err;

        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }

        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void SSD1306_ClearArea(uint8_t x0, uint8_t y0,
                       uint8_t x1, uint8_t y1)
{
    for (uint8_t y = y0; y < y1; y++)
    {
        for (uint8_t x = x0; x < x1; x++)
        {
            uint16_t index = x + (y / 8) * 128;
            uint8_t bit = 1 << (y & 7);

            buffer[index] &= ~bit;
        }
    }
}