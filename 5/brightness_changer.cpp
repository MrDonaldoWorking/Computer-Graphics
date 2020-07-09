#include <math.h>
#include <stdexcept>
#include "brightness_changer.h"

void clear_array(int ***arr, int n, int m) {
    if (arr == nullptr) {
        return;
    }

    for (int i = 0; i < n; ++i) {
        if (arr[i] != nullptr) {
            for (int j = 0; j < m; ++j) {
                if (arr[i][j] != nullptr) {
                    delete[] arr[i][j];
                    arr[i][j] = nullptr;
                }
            }
            delete[] arr[i];
            arr[i] = nullptr;
        }
    }
    delete[] arr;
    arr = nullptr;
}

picture::picture(int const h, int const w, int const m, int const c, int ***data)
    : height(h), width(w), max_brightness(m), colors(c), data(data) {}

picture::~picture() {
    clear_array(data, height, width);
}

inline int correct_bounds(double const val) {
    return static_cast<int>(fmin(MAX_BRIGHTNESS, fmax(floor(val), 0)));
}

inline int correct_bounds_with_MB(double const val) {
    return correct_bounds(val * MAX_BRIGHTNESS);
}

int change_brightness(double const val, int const shift, double const factor) {
    return correct_bounds((val - shift) * factor);
}

void picture::manual_RGB(int const shift, double const factor) {
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            for (int color = 0; color < colors; ++color) {
                data[h][w][color] = change_brightness(data[h][w][color], shift, factor);
            }
        }
    }
}

void RGB_to_YCbCr(double *pixel, double const K_R, double const K_G, double const K_B) {
    double R = pixel[0] / MAX_BRIGHTNESS;
    double G = pixel[1] / MAX_BRIGHTNESS;
    double B = pixel[2] / MAX_BRIGHTNESS;

    double Y = K_R * R + K_G * G + K_B * B;
    double Cb = ONE_SECOND * (B - Y) / (1 - K_B);
    double Cr = ONE_SECOND * (R - Y) / (1 - K_R);

    pixel[0] = MAX_BRIGHTNESS * (Y);
    pixel[1] = MAX_BRIGHTNESS * (Cb + ONE_SECOND);
    pixel[2] = MAX_BRIGHTNESS * (Cr + ONE_SECOND);
}

void YCbCr_to_RGB(double *pixel, double const K_R, double const K_G, double const K_B) {
    double Y = pixel[0] / MAX_BRIGHTNESS;
    double Cb = pixel[1] / MAX_BRIGHTNESS - ONE_SECOND;
    double Cr = pixel[2] / MAX_BRIGHTNESS - ONE_SECOND;

    pixel[0] = MAX_BRIGHTNESS * (Y + Cr * (2 - 2 * K_R));
    pixel[1] = MAX_BRIGHTNESS * (Y - (K_B / K_G) * (2 - 2 * K_B) * Cb - (K_R / K_G) * (2 - 2 * K_R) * Cr);
    pixel[2] = MAX_BRIGHTNESS * (Y + (2 - 2 * K_B) * Cb);
}

// delete[] is necassary
double* obtain_pixel(int *pixel, double const colors) {
    double *res = new double[colors];
    for (int color = 0; color < colors; ++color) {
        res[color] = static_cast<double>(pixel[color]);
    }
    return res;
}

void picture::manual_YCbCr(int const shift, double const factor) {
    // idk what to do if P5
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            double *pixel = obtain_pixel(data[h][w], colors);

            RGB_to_YCbCr(pixel, K_R_601, K_G_601, K_B_601);
            pixel[0] = change_brightness(pixel[0], shift, factor);
            YCbCr_to_RGB(pixel, K_R_601, K_G_601, K_B_601);

            for (int color = 0; color < colors; ++color) {
                data[h][w][color] = correct_bounds(pixel[color]);
            }

            delete[] pixel;
        }
    }
}

void picture::switch_brightness(int const change, int const shift, double const factor) {
    if (change % 2 == 1 && colors == MONOCHROME_COLORS) {
        throw std::runtime_error("P5 RGB to YCbCr conversion is not supported yet");
    }
    switch (change) {
        case 0:
            manual_RGB(shift, factor);
            break;

        case 1:
            manual_YCbCr(shift, factor);
            break;
        
        case 2:
            break;
        
        case 3:
            break;
        
        case 4:
            break;
        
        case 5:
            break;
    }
}