#include <vector>
#include <algorithm>
#include <numeric>
#include <math.h>
#include <iostream>
#include <random>
#include <chrono>
#include "dithering.h"

template <typename T>
void clear_array(T **arr, int n) {
    if (arr == nullptr) {
        return;
    }
    for (int i = 0; i < n; ++i) {
        if (arr[i] != nullptr) {
            delete[] arr[i];
        }
        arr[i] = nullptr;
    }
    delete[] arr;
    arr = nullptr;
}

picture::picture(int h, int w, int m, int g, std::ifstream &in) : height(h), width(w), max_brightness(max_brightness) {
    data = new double*[height];
    for (int h = 0; h < height; ++h) {
        data[h] = new double[width];
    }

    // std::cout << "Initializing picture: " << width << " * " << height << '\n';

    if (g == 1) {
        // std::cout << "Gradient mode chosen\n";
        double gradient_coefficient = static_cast<double>(MAX_BRIGHTNESS) / (width - 1);
        for (int h = 0; h < height; ++h) {
            for (int w = 0; w < width; ++w) {
                data[h][w] = gradient_coefficient * w;
            }
        }
    } else {
        // std::cout << "Picture mode chosen\n";
        for (int h = 0; h < height; ++h) {
            for (int w = 0; w < width; ++w) {
                data[h][w] = in.get();

                // Guaranteed file has width * height bytes
                if (in.gcount() == 0 || in.bad()) {
                    for (int i = 0; i < h; ++i) {
                        delete[] data[i];
                    }
                    delete[] data;
                    in.close();
                    throw std::runtime_error("Something happened with an input stream while reading info\n");
                }
            }
        }
    }
    in.close();
    // std::cout << "Created\n";
    // std::cout << "data[0][0] = " << data[0][0] << '\n';
}

picture::~picture() {
    clear_array(data, height);
}

int picture::get_height() const {
    return height;
}

int picture::get_width() const {
    return width;
}

int picture::get_max_brightness() const {
    return max_brightness;
}

double** picture::get_all() {
    return data;
}

double picture::get(int const h, int const w) {
    // std::cout << "Getting data[" << h << "][" << w << "]: ";
    return data[h][w];
}

void picture::set(int const h, int const w, double const x) {
    // std::cout << "Setting data[" << h << "][" << w << "] = " << x << std::endl;
    data[h][w] = x;
}

std::vector<int> get_even_scatter(int bitness) {
    std::vector<int> res(1 << bitness);
    if (bitness == 8) {
        std::iota(res.begin(), res.end(), 0);
        return res;
    }

    int mid_vals = res.size() - 2;
    int sum_up = MAX_BRIGHTNESS - (1 << bitness) + 1;
    int stride = sum_up / (mid_vals + 1), remnant = sum_up % (mid_vals + 1);
    int pos = 1, val = 1;
    while (mid_vals--) {
        val += stride;
        if (remnant-- > 0) {
            ++val;
        }
        res[pos++] = val;
    }
    res.back() = MAX_BRIGHTNESS;

    return res;
}

int get_less_or_equal(std::vector<int> const& arr, int const x) {
    std::vector<int>::const_iterator pos = std::lower_bound(arr.begin(), arr.end(), x);
    if (*pos == x) {
        // equals
        return x;
    }
    return *(pos - 1);
}

int get_greater_or_equal(std::vector<int> const& arr, int const x) {
    std::vector<int>::const_iterator pos = std::upper_bound(arr.begin(), arr.end(), x);
    if (pos == arr.end()) {
        return *arr.rbegin();
    }
    return *pos;
}

double correction(double val, bool const sRGB, double const gamma) {
    val /= MAX_BRIGHTNESS;
    if (sRGB) {
        if (val <= S_RGB_BARRIER) {
            val /= S_RGB_DIVISOR;
        } else {
            val = pow((val + S_RGB_BASE_TERM) / S_RGB_BASE_DIVISOR, S_RGB_DEGREE);
        }
    } else {
        val = pow(val, gamma);
    }
    val *= MAX_BRIGHTNESS;

    return val;
}

inline int correct_bounds(double val) {
    return static_cast<int>(floor(fmin(MAX_BRIGHTNESS, fmax(0, val))));
}

double get_nearest(double const val, int const bitness, bool const sRGB, double const gamma,
                   bool const without_dist, bool const with_dist, double const add) {
    static std::vector<int> const scatter = get_even_scatter(bitness);
    // static int cnt = 0;
    // static std::ofstream out("test.log", std::ios::app);

    double left = correction(get_less_or_equal(scatter, val), sRGB, gamma);
    // std::cout << "Got left = " << left << std::endl;
    double right = correction(get_greater_or_equal(scatter, val), sRGB, gamma);
    // std::cout << "Got right = " << right << std::endl;
    double middle = correction(val, sRGB, gamma);
    // std::cout << "Got middle = " << middle << std::endl;
    // if (++cnt % 65536 <= 10) {
    //     out << "=== " << cnt << " ===" << std::endl;
    //     out << "val = " << val << std::endl;
    //     out << "left = " << left << ", right = " << right << std::endl << "middle = " << middle;
    // }

    if (without_dist) {
        middle += add;
        // if (cnt % 65536 <= 10) {
        //     out << ", add = " << add << std::endl;
        // }
    }
    if (with_dist) {
        middle += add * (right - left);
        // if (cnt % 65536 <= 10) {
        //     out << ", add = " << add << std::endl;
        // }
    }
    // if (cnt % 65536 <= 10) {
    //     out << "middle + add = " << middle << std::endl;
    //     out.flush();
    // }

    double res;
    if (fabs(middle - left) < fabs(middle - right)) {
        res = left;
    } else {
        res = right;
    }

    // if (cnt % 65536 <= 10) {
    //     out << "correct_bounds(res) = " << correct_bounds(res) << '\n';
    //     out.flush();
    // }
    return res;
}

int round_pixel(double const val, int const bitness, bool const sRGB, double const gamma,
                bool const without_dist, bool const with_dist, double const add) {
    return correct_bounds(get_nearest(val, bitness, sRGB, gamma, without_dist, with_dist, add));
}

void without_dithering(picture &pic, int const bitness, bool const sRGB, double const gamma) {
    for (int h = 0; h < pic.get_height(); ++h) {
        for (int w = 0; w < pic.get_width(); ++w) {
            pic.set(h, w, round_pixel(pic.get(h, w), bitness, sRGB, gamma, false, false, 0));
        }
    }
}

void ordered_dithering(picture &pic, int const bitness, bool const sRGB, double const gamma) {
    for (int h = 0; h < pic.get_height(); ++h) {
        for (int w = 0; w < pic.get_width(); ++w) {
            double add = (ORDERED[h % ORDERED_SIZE][w % ORDERED_SIZE] + 0.5) / ORDERED_SIZE / ORDERED_SIZE - 0.5;
            pic.set(h, w, round_pixel(pic.get(h, w), bitness, sRGB, gamma, false, true, add));
        }
    }
}

void random_dithering(picture &pic, int const bitness, bool const sRGB, double const gamma) {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 rnd(seed);
    int bound = static_cast<int>(1e7);
    for (int h = 0; h < pic.get_height(); ++h) {
        for (int w = 0; w < pic.get_width(); ++w) {
            long long curr = rnd();
            double add = ((curr % (2 * bound + 1)) - bound) / static_cast<double>(bound);
            pic.set(h, w, round_pixel(pic.get(h, w), bitness, sRGB, gamma, false, true, add / 2));
        }
    }
}

void update_error(std::vector<double> &error, std::vector<int> const& coefficients,
                  int const pos, int const barrier, double const error_val) {
    int shift = (coefficients.size() == 2 ? 3 : 0);
    if (pos + 1 < barrier) {
        error[pos + 1] += error_val * coefficients[3 - shift];
    }
    if (pos + 2 < barrier) {
        error[pos + 2] += error_val * coefficients[4 - shift];
    }
    if (coefficients.size() == 2) {
        return;
    }
    if (pos - 2  >= 0) {
        error[pos - 2] += error_val * coefficients[0];
    }
    if (pos - 1 >= 0) {
        error[pos - 1] += error_val * coefficients[1];
    }
    error[pos] += error_val * coefficients[2];
}

void dithering_with_errors(picture &pic, int const bitness, bool const sRGB, double const gamma,
                           double const fraction, std::vector<int> const& prev,
                           std::vector<int> const& curr, std::vector<int> const& next) {
    std::vector<double> prev_error(pic.get_width());
    std::vector<double> curr_error(pic.get_width());
    std::vector<double> next_error(pic.get_width());
    if (prev.size() != 2) {
        throw std::runtime_error("prev.size() must be 2");
    }
    if (curr.size() != 5) {
        throw std::runtime_error("curr.size() must be 5");
    }
    if (next.size() != 5) {
        throw std::runtime_error("next.size() must be 5");
    }
    for (int h = 0; h < pic.get_height(); ++h) {
        for (int w = 0; w < pic.get_width(); ++w) {
            double curr_val = get_nearest(pic.get(h, w), bitness, sRGB, gamma, true, false, prev_error[w]);
            double middle = correction(pic.get(h, w), sRGB, gamma) + prev_error[w];

            double error = (middle - curr_val) * fraction;
            if (w + 1 < pic.get_width()) {
                prev_error[w + 1] += error * prev[0];
            }
            if (w + 2 < pic.get_width()) {
                prev_error[w + 2] += error * prev[1];
            }
            // if (h + 1 < pic.get_height()) {
                if (w - 2 >= 0) {
                    curr_error[w - 2] += error * curr[0];
                }
                if (w - 1 >= 0) {
                    curr_error[w - 1] += error * curr[1];
                }
                curr_error[w] += error * curr[2];
                if (w + 1 < pic.get_width()) {
                    curr_error[w + 1] += error * curr[3];
                }
                if (w + 2 < pic.get_width()) {
                    curr_error[w + 2] += error * curr[4];
                }
            // }
            // if (h + 2 < pic.get_height()) {
                if (w - 2 >= 0) {
                    next_error[w - 2] += error * next[0];
                }
                if (w - 1 >= 0) {
                    next_error[w - 1] += error * next[1];
                }
                next_error[w] += error * next[2];
                if (w + 1 < pic.get_width()) {
                    next_error[w + 1] += error * next[3];
                }
                if (w + 2 < pic.get_width()) {
                    next_error[w + 2] += error * next[4];
                }

            pic.set(h, w, curr_val);
        }

        prev_error.swap(curr_error);
        curr_error.swap(next_error);
        next_error.assign(pic.get_width(), 0);
    }
}

void Floyd_Steinberg_dithering(picture &pic, int const bitness, bool const sRGB, double const gamma) {
    dithering_with_errors(pic, bitness, sRGB, gamma, ONE_SIXTEENTH, {7, 0}, {0, 3, 5, 1, 0}, {0, 0, 0, 0, 0});
    return;

    // std::vector<double> prev_error(pic.get_width());
    // std::vector<double> curr_error(pic.get_width());
    // // int cnt = 0;
    // // std::ofstream out("test.log", std::ios::app);
    // for (int h = 0; h < pic.get_height(); ++h) {
    //     for (int w = 0; w < pic.get_width(); ++w) {
    //         double curr_val = get_nearest(pic.get(h, w), bitness, sRGB, gamma, true, false, prev_error[w]);
    //         double middle = correction(pic.get(h, w), sRGB, gamma) + prev_error[w];

    //         // if (++cnt % 65536 <= 10) {
    //         //     out << "curr_val = " << curr_val << std::endl;
    //         // }

    //         double error = (middle - curr_val) * ONE_SIXTEENTH;
    //         // if (cnt % 65536 <= 10) {
    //         //     out << "error = " << error << std::endl;
    //         // }
    
    //         if (w + 1 < pic.get_width()) {
    //             prev_error[w + 1] += error * 7;
    //             // if (cnt % 65536 <= 10) {
    //             //     out << "curr_error[" << w + 1 << "] += " << error * 7 << " = " << curr_error[w + 1] << std::endl;
    //             // }
    //         }
    //         // if (h + 1 < pic.get_height()) {
    //             if (w - 1 >= 0) {
    //                 curr_error[w - 1] += error * 3;
    //                 // if (cnt % 65536 <= 10) {
    //                 //     out << "next_error[" << w - 1 << "] += " << error * 3 << " = " << next_error[w - 1] << std::endl;
    //                 // }
    //             }
    //             curr_error[w] += error * 5;
    //             // if (cnt % 65536 <= 10) {
    //             //     out << "next_error[" << w << "] += " << error * 5 << " = " << next_error[w] << std::endl;
    //             // }
    //             if (w + 1 < pic.get_width()) {
    //                 curr_error[w + 1] += error;
    //                 // if (cnt % 65536 <= 10) {
    //                 //     out << "next_error[" << w + 1 << "] += " << error << " = " << next_error[w + 1] << std::endl;
    //                 // }
    //             }
    //         // }
    //         pic.set(h, w, curr_val);
    //     }

    //     curr_error.swap(prev_error);
    //     curr_error.assign(pic.get_width(), 0);
    // }
}

void Jarvis_Judice_Ninke_dithering(picture &pic, int const bitness, bool const sRGB, double const gamma) {
    dithering_with_errors(pic, bitness, sRGB, gamma, ONE_FORTY_EIGHTH, {7, 5}, {3, 5, 7, 5, 3}, {1, 3, 5, 3, 1});
    return;
    // std::vector<double> prev_error(pic.get_width());
    // std::vector<double> curr_error(pic.get_width());
    // std::vector<double> next_error(pic.get_width());
    // for (int h = 0; h < pic.get_height(); ++h) {
    //     for (int w = 0; w < pic.get_width(); ++w) {
    //         double curr_val = get_nearest(pic.get(h, w), bitness, sRGB, gamma, true, false, prev_error[w]);
    //         double middle = correction(pic.get(h, w), sRGB, gamma) + prev_error[w];

    //         double error = (middle - curr_val) * ONE_FORTY_EIGHTH;
    //         if (w + 1 < pic.get_width()) {
    //             prev_error[w + 1] += error * 7;
    //         }
    //         if (w + 2 < pic.get_width()) {
    //             prev_error[w + 2] += error * 5;
    //         }
    //         // if (h + 1 < pic.get_height()) {
    //             if (w - 2 >= 0) {
    //                 curr_error[w - 2] += error * 3;
    //             }
    //             if (w - 1 >= 0) {
    //                 curr_error[w - 1] += error * 5;
    //             }
    //             curr_error[w] += error * 7;
    //             if (w + 1 < pic.get_width()) {
    //                 curr_error[w + 1] += error * 5;
    //             }
    //             if (w + 2 < pic.get_width()) {
    //                 curr_error[w + 2] += error * 3;
    //             }
    //         // }
    //         // if (h + 2 < pic.get_height()) {
    //             if (w - 2 >= 0) {
    //                 next_error[w - 2] += error;
    //             }
    //             if (w - 1 >= 0) {
    //                 next_error[w - 1] += error * 3;
    //             }
    //             next_error[w] += error * 5;
    //             if (w + 1 < pic.get_width()) {
    //                 next_error[w + 1] += error * 3;
    //             }
    //             if (w + 2 < pic.get_width()) {
    //                 next_error[w + 2] += error;
    //             }
    //         // }

    //         pic.set(h, w, curr_val);
    //     }

    //     prev_error.swap(curr_error);
    //     curr_error.swap(next_error);
    //     next_error.assign(pic.get_width(), 0);
    // }
}

void Sierra_3_dithering(picture &pic, int const bitness, bool const sRGB, double const gamma) {
    dithering_with_errors(pic, bitness, sRGB, gamma, ONE_THIRTY_SECOND, {5, 3}, {2, 4, 5, 4, 2}, {0, 2, 3, 2, 0});
    // std::vector<double> prev_error(pic.get_width());
    // std::vector<double> curr_error(pic.get_width());
    // std::vector<double> next_error(pic.get_width());
    // for (int h = 0; h < pic.get_height(); ++h) {
    //     for (int w = 0; w < pic.get_width(); ++w) {
    //         double curr_val = get_nearest(pic.get(h, w), bitness, sRGB, gamma, true, false, prev_error[w]);
    //         double middle = correction(pic.get(h, w), sRGB, gamma) + prev_error[w];

    //         double error = (middle - curr_val) * ONE_THIRTY_SECOND;
    //         if (w + 1 < pic.get_width()) {
    //             prev_error[w + 1] += error * 5;
    //         }
    //         if (w + 2 < pic.get_width()) {
    //             prev_error[w + 2] += error * 3;
    //         }
    //         // if (h + 1 < pic.get_height()) {
    //             if (w - 2 >= 0) {
    //                 curr_error[w - 2] += error * 2;
    //             }
    //             if (w - 1 >= 0) {
    //                 curr_error[w - 1] += error * 4;
    //             }
    //             curr_error[w] += error * 5;
    //             if (w + 1 < pic.get_width()) {
    //                 curr_error[w + 1] += error * 4;
    //             }
    //             if (w + 2 < pic.get_width()) {
    //                 curr_error[w + 2] += error * 2;
    //             }
    //         // }
    //         // if (h + 2 < pic.get_height()) {
    //             if (w - 1 >= 0) {
    //                 next_error[w - 1] += error * 2;
    //             }
    //             next_error[w] += error * 3;
    //             if (w + 1 < pic.get_width()) {
    //                 next_error[w + 1] += error * 2;
    //             }
    //         // }

    //         pic.set(h, w, curr_val);
    //     }

    //     prev_error.swap(curr_error);
    //     curr_error.swap(next_error);
    //     next_error.assign(pic.get_width(), 0);
    // }
}

void Atkinson_dithering(picture &pic, int const bitness, bool const sRGB, double const gamma) {

}

void Halftone_dithering(picture &pic, int const bitness, bool const sRGB, double const gamma) {

}
