#include <math.h>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <iostream>
#include <fstream>
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

int change_brightness(double const val, double const shift, double const factor) {
    return correct_bounds((val - shift) * factor);
}

// void run_through_all(int const n, int const m, int const shift, double const factor,
//                      brightness_changer_function_t const& transform) {
//     for (int i = 0; i < n; ++i) {
//         for (int j = 0; j < m; ++j) {
//             transform(i, j, shift, factor);
//         }
//     }
// }

void picture::manual_RGB(int shift, double factor) {
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

    pixel[0] = fmin(MAX_BRIGHTNESS, fmax(MAX_BRIGHTNESS * (Y + Cr * (2 - 2 * K_R)), 0));
    pixel[1] = fmin(MAX_BRIGHTNESS, fmax(MAX_BRIGHTNESS * (Y - (K_B / K_G) * (2 - 2 * K_B) * Cb - (K_R / K_G) * (2 - 2 * K_R) * Cr), 0));
    pixel[2] = fmin(MAX_BRIGHTNESS, fmax(MAX_BRIGHTNESS * (Y + (2 - 2 * K_B) * Cb), 0));
}

// delete[] is necassary
double* obtain_pixel(int *pixel, int const colors) {
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

// std::pair<int const, int const> picture::find_min_max(bool is_YCbCr) {
//     double max_value = 0, min_value = MAX_BRIGHTNESS;
//     for (int h = 0; h < height; ++h) {
//         for (int w = 0; w < width; ++w) {
//             for (int color = 0; color < colors; ++color) {
//                 max_value = fmax(max_value, data[h][w][color]);
//                 min_value = fmin(min_value, data[h][w][color]);
//             }
//         }
//     }

//     return std::make_pair(min_value, max_value);
// }

// void picture::auto_changing(brightness_changer_function_t const& manual,
//                             double const min_value, double const max_value) {
//     manual(min_value, MAX_BRIGHTNESS / static_cast<double>(max_value - min_value));
// }

// std::pair<int const, int const> picture::find_min_max_with_skip() {
//     long long brightnesses[MAX_BRIGHTNESS + 1];
//     for (int br = 0; br <= MAX_BRIGHTNESS; ++br) {
//         brightnesses[br] = 0;
//     }

//     for (int h = 0; h < height; ++h) {
//         for (int w = 0; w < width; ++w) {
//             for (int color = 0; color < colors; ++color) {
//                 ++brightnesses[data[h][w][color]];
//             }
//         }
//     }

//     int max_value = 0;
//     long long picture_size = height * width * colors;
//     double skip = 0;
//     for (; max_value <= MAX_BRIGHTNESS; ++max_value) {
//         skip += brightnesses[max_value] / static_cast<double>(picture_size);
//         if (skip > PERCENTAGE) {
//             break;
//         }
//     }

//     int min_value = MAX_BRIGHTNESS;
//     skip = 0;
//     for (; min_value >= 0 && skip < PERCENTAGE; --min_value) {
//         skip += brightnesses[max_value] / static_cast<double>(picture_size);
//         if (skip > PERCENTAGE) {
//             break;
//         }
//     }

//     return std::make_pair(min_value, max_value);
// }

// void picture::auto_changing_with_skip(brightness_changer_function_t const& manual) {
//     auto min_max = find_min_max_with_skip();
//     manual(min_max.first, MAX_BRIGHTNESS / static_cast<double>(min_max.second - min_max.first));
// }



void picture::auto_RGB() {
    int max_value = 0, min_value = MAX_BRIGHTNESS;
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            for (int color = 0; color < colors; ++color) {
                max_value = std::max(max_value, data[h][w][color]);
                min_value = std::min(min_value, data[h][w][color]);
            }
        }
    }
    // auto_changing([this] (manual_args) { manual_RGB(shift, factor); }, min_value, max_value);
    int const shift = min_value;
    double const factor = MAX_BRIGHTNESS / static_cast<double>(max_value - min_value);
    std::cout << shift << ' ' << factor << '\n';
    manual_RGB(shift, factor);
}

void picture::auto_YCbCr() {
    double min_value = MAX_BRIGHTNESS, max_value = 0;
    int cnt = 0;
    std::ofstream out("test.log");
    out.precision(1);
    out << std::fixed;
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            double *pixel = obtain_pixel(data[h][w], colors);

            RGB_to_YCbCr(pixel, K_R_601, K_G_601, K_B_601);
            min_value = fmin(min_value, pixel[0]);
            max_value = fmax(max_value, pixel[0]);
            if (++cnt % 65536 <= 10) {
                out << "=== " << cnt << " ===\n";
                out << data[h][w][0] << ' ' << data[h][w][1] << ' ' << data[h][w][2] << '\n';
                out << min_value << ' ' << max_value << ' ' << pixel[0] << ' ' << pixel[1] << ' ' << pixel[2] << std::endl;
            }

            delete[] pixel;
        }
    }
    std::cout << min_value << ' ' << max_value << '\n';

    // auto_changing([this] (manual_args) { manual_YCbCr(shift, factor); }, min_value, max_value);
    double const shift = min_value;
    double const factor = MAX_BRIGHTNESS / (max_value - min_value);
    std::cout << shift << ' ' << factor << '\n';
    manual_YCbCr(shift, factor);
}

void picture::auto_with_skip_RGB() {
    std::vector<int> brightnesses;
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            for (int color = 0; color < colors; ++color) {
                brightnesses.push_back(data[h][w][color]);
            }
        }
    }

    int pos = static_cast<int>(PERCENTAGE * height * width * colors);
    std::nth_element(brightnesses.begin(), brightnesses.begin() + pos, brightnesses.end());
    int min_value = brightnesses[pos];
    std::nth_element(brightnesses.begin(), brightnesses.begin() + pos, brightnesses.end(), std::greater<int>());
    int max_value = brightnesses[pos];
    // auto_changing_with_skip([this] (manual_args) { manual_RGB(shift, factor); });
    std::cout << min_value << ' ' << max_value << '\n';

    int const shift = min_value;
    double const factor = MAX_BRIGHTNESS / static_cast<double>(max_value - min_value);
    std::cout << shift << ' ' << factor << '\n';

    manual_RGB(shift, factor);
}

void picture::auto_with_skip_YCbCr() {
    std::vector<double> brightnesses;
    std::ofstream out("test.log");
    int cnt = 0;
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            double *pixel = obtain_pixel(data[h][w], colors);

            RGB_to_YCbCr(pixel, K_R_601, K_G_601, K_B_601);
            brightnesses.push_back(pixel[0]);
            if (++cnt % 65536 <= 10) {
                out << "=== " << cnt << " ===\n";
                out << brightnesses.back() << std::endl;
            }

            delete[] pixel;
        }
    }

    double pos = static_cast<int>(PERCENTAGE * height * width * colors);
    std::cout << pos << '\n';
    std::nth_element(brightnesses.begin(), brightnesses.begin() + pos, brightnesses.end());
    double min_value = brightnesses[pos];
    std::nth_element(brightnesses.begin(), brightnesses.begin() + pos, brightnesses.end(), std::greater<double>());
    int max_value = brightnesses[pos];
    std::cout << min_value << ' ' << max_value << '\n';

    // auto_changing_with_skip([this] (manual_args) { manual_YCbCr(shift, factor); });
    double const shift = min_value;
    double const factor = MAX_BRIGHTNESS / (max_value - min_value);
    std::cout << shift << ' ' << factor << '\n';

    manual_YCbCr(shift, factor);
}

void picture::switch_brightness(int const change, int const shift, double const factor) {
    if (change % 2 == 1 && colors == MONOCHROME_COLORS) {
        throw std::runtime_error("P5 RGB to YCbCr conversion is not supported yet");
    }
    std::cout.precision(5) << '\n';
    std::cout << std::fixed;

    switch (change) {
        case 0:
            manual_RGB(shift, factor);
            break;

        case 1:
            manual_YCbCr(shift, factor);
            break;
        
        case 2:
            auto_RGB();
            break;
        
        case 3:
            auto_YCbCr();
            break;
        
        case 4:
            auto_with_skip_RGB();
            break;
        
        case 5:
            auto_with_skip_YCbCr();
            break;
    }
}