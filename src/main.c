#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../lib/imagelib/imagelib.h"

float **read_kernel(char *kernel_path, size_t *H, size_t *W) {
        FILE *file = fopen(kernel_path, "r");
        char *line = NULL;
        size_t length = 0;
        ssize_t read;

        // Get width
        read = getline(&line, &length, file);
        size_t width = atoi(line);

        // Get height
        read = getline(&line, &length, file);
        size_t height = atoi(line);

        // Create kernel matrix
        float **K = calloc(height, sizeof(float *));

        // Populate kernel matrix
        size_t row = 0;
        while ((read = getline(&line, &length, file)) != -1) {
                K[row] = calloc(width, sizeof(float));

                size_t col = 0;
                char *pch = strtok(line, " ");
                while (pch != NULL) {
                        K[row][col] = atof(pch);
                        col += 1;
                        pch = strtok(NULL, " ");
                }
                row += 1;
        }

        // Free stuff
        fclose(file);
        free(line);

        // Return values
        (*H) = height;
        (*W) = width;
        return K;
}

int main(int argc, char *argv[]) {
        char *img_in_path = argv[1];
        char *kernel_path = argv[2];
        char *img_out_path = argv[3];

        size_t k_height = 0;
        size_t k_width = 0;
        float **K = read_kernel(kernel_path, &k_height, &k_width);

        size_t k_width_center = floor(k_width / 2);
        size_t k_height_center = floor(k_height / 2);
        size_t k_width_radius = k_width - k_width_center - 1;
        size_t k_height_radius = k_height - k_height_center - 1;

        Image *src = img_png_read_from_file(img_in_path);
        Image *out = img_png_read_from_file(img_in_path);
        for (size_t row = 0; row < src->height; row++) {
                for (size_t col = 0; col < src->width; col++) {
                        Color *acc = malloc(sizeof(Color));

                        size_t i = 0;
                        for (ssize_t row_start = row - k_width_radius; row_start <= row + k_width_radius; row_start++) {
                                size_t j = 0;
                                for (ssize_t col_start = col - k_height_radius; col_start <= col + k_height_radius; col_start++) {
                                        if (row_start < 0 || row_start >= src->width) {
                                                continue;
                                        }
                                        else if (col_start < 0 || col_start >= src->height) {
                                                continue;
                                        }
                                        else {
                                                float K_ij = K[i][j];
                                                Color I_ij = src->pixels[row][col];
                                                acc->R = K_ij * I_ij.R;
                                                acc->G = K_ij * I_ij.G;
                                                acc->B = K_ij * I_ij.B;
                                        }
                                        j += 1;
                                }
                                i += 1;
                        }
                        size_t sum = k_width * k_height;
                        out->pixels[row][col].R = acc->R / sum;
                        out->pixels[row][col].G = acc->G / sum;
                        out->pixels[row][col].B = acc->B / sum;
                }
        }
        img_png_write_to_file(out, img_out_path);

        img_destroy(out);
        img_destroy(src);

        return 0;
}
