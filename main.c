#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <limits.h>

#define WIDTH 50
#define HEIGHT 50
#define PPM_SCALER 25
#define SAMPLE_SIZE 10

typedef float Layer[HEIGHT][WIDTH];

static inline int clampi(int x, int low, int high)
{
    if (x < low)  x = low;
    if (x > high) x = high;
    return x;
}

void layer_fill_rect(Layer layer, int x, int y, int w, int h, float value)
{
    assert(w > 0);
    assert(h > 0);
    int x0 = clampi(x, 0, WIDTH-1);
    int y0 = clampi(y, 0, HEIGHT-1);
    int x1 = clampi(x0 + w - 1, 0, WIDTH-1);
    int y1 = clampi(y0 + h - 1, 0, HEIGHT-1);
    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            layer[y][x] = value;
        }
    }
}

void layer_fill_circle(Layer layer, int cx, int cy, int r, float value)
{
    assert(r > 0);
    int x0 = clampi(cx - r, 0, WIDTH-1);
    int y0 = clampi(cy - r, 0, HEIGHT-1);
    int x1 = clampi(cx + r, 0, WIDTH-1);
    int y1 = clampi(cy + r, 0, HEIGHT-1);
    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            int dx = x - cx;
            int dy = y - cy;
            if (dx*dx + dy*dy <= r*r) {
                layer[y][x] = value;
            }
        }
    }
}

void layer_save_as_ppm(Layer layer, const char *file_path)
{
    FILE *f = fopen(file_path, "wb");
    if (f == NULL) {
        fprintf(stderr, "ERROR: could not open file %s: %m\n",
                file_path);
        exit(1);
    }

    fprintf(f, "P6\n%d %d 255\n", WIDTH * PPM_SCALER, HEIGHT * PPM_SCALER);

    for (int y = 0; y < HEIGHT * PPM_SCALER; ++y) {
        for (int x = 0; x < WIDTH * PPM_SCALER; ++x) {
            float s = layer[y / PPM_SCALER][x / PPM_SCALER];
            char pixel[3] = {
                (char) floorf(255 * s),
                0,
                0
            };

            fwrite(pixel, sizeof(pixel), 1, f);
        }
    }

    fclose(f);
}

void layer_save_as_bin(Layer layer, const char *file_path)
{
    FILE *f = fopen(file_path, "wb");
    if (f == NULL) {
        fprintf(stderr, "ERROR: could not open file %s: %m", file_path);
        exit(1);
    }
    fwrite(layer, sizeof(Layer), 1, f);
    fclose(f);
}

void layer_load_from_bin(Layer layer, const char *file_path)
{
    assert(0 && "TODO: layer_load_from_bin is not implemented yet!");
}

float feed_forward(Layer inputs, Layer weights)
{
    float output = 0.0f;

    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            output += inputs[y][x] * weights[y][x];
        }
    }

    return output;
}

int rand_range(int low, int high)
{
    assert(low < high);
    return rand() % (high - low) + low;
}

void layer_random_rect(Layer layer)
{
    layer_fill_rect(layer, 0, 0, WIDTH, HEIGHT, 0.0f);
    int x = rand_range(0, WIDTH);
    int y = rand_range(0, HEIGHT);

    int w = WIDTH - x;
    if (w < 2) w = 2;
    w = rand_range(1, w);

    int h = HEIGHT - x;
    if (h < 2) h = 2;
    h = rand_range(1, h);

    layer_fill_rect(layer, x, y, w, h, 1.0f);
}

void layer_random_circle(Layer layer)
{
    layer_fill_rect(layer, 0, 0, WIDTH, HEIGHT, 0.0f);
    int cx = rand_range(0, WIDTH);
    int cy = rand_range(0, HEIGHT);
    int r = INT_MAX;
    if (r > cx) r = cx;
    if (r > cy) r = cy;
    if (r > WIDTH - cx) r = WIDTH - cx;
    if (r > HEIGHT - cy) r = HEIGHT - cy;
    if (r < 2) r = 2;
    r = rand_range(1, r);
    layer_fill_circle(layer, cx, cy, r, 1.0f);
}

static Layer inputs;
static Layer weights;

int main(void)
{
    char file_path[256];

#define PREFIX "rect"
    for (int i = 0; i < SAMPLE_SIZE; ++i) {
        printf("[INFO] generating "PREFIX" %d\n", i);

        layer_random_rect(inputs);

        snprintf(file_path, sizeof(file_path), PREFIX"-%02d.bin", i);
        layer_save_as_bin(inputs, file_path);
        snprintf(file_path, sizeof(file_path), PREFIX"-%02d.ppm", i);
        layer_save_as_ppm(inputs, file_path);
    }

    return 0;
}
