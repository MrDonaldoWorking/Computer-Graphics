#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <math.h>
#include "line_drawer.h"

double const EPS = 1e-5;

point::point(double x, double y): x(x), y(y) {}

// point::point(std::initializer_list<double> const& list) {
//     if (list.size() != 2) {
//         throw std::range_error("initializer_list size must be equals to 2");
//     }
//     x = *list.begin();
//     y = *(list.begin() + 1);
// }

double point::get_x() const {
    return x;
}

double point::get_y() const {
    return y;
}

void point::swap_axis() {
    std::swap(x, y);
}

picture::picture(int h, int w, int m, int **data)
    : height(h), width(w), max_brightness(m), data(data) {}

picture::~picture() {
    for (int h = 0; h < height; ++h) {
        delete[] data[h];
    }
    delete[] data;
}

int picture::get_h() const {
    return height;
}

int picture::get_w() const {
    return width;
}

int** picture::get_data() {
    return data;
}

inline double correction(double val, double percentage, int brightness) {
    val *= 1 - percentage;
    val += percentage * brightness / MAX_BRIGHTNESS;
    return val;
}

template <typename T>
inline void do_swap(bool swapped, T &t1, T &t2) {
    if (swapped) {
        std::swap(t1, t2);
    }
}

void picture::fill_pixel(int x, int y, bool const swapped, int const brightness,
                         double const percentage, bool const sRGB, double const gamma) {
    do_swap(swapped, x, y);
    // std::ofstream out("log.txt", std::ios::app);
    // out << x << ' ' << y << ": ";
    if (x < 0 || x >= width || y < 0 || y >= height) {
        // out << "not allowed\n";
        // out.flush();
        return;
    }
    // out << " allowed!\n";
    // out.flush();

    // out << "data[" << x << "][" << y << "] / 255 = ";
    // out.flush();
    double value = data[y][x] / MAX_BRIGHTNESS;
    // out << value << '\n';
    // out.flush();
    if (sRGB) {
        // reverse transformation
        if (value <= 0.04045) {
            value /= 12.92;
        } else {
            // gamma is 2.4 here
            value = pow((value + 0.055) / 1.055, gamma);
        }

        value = correction(value, percentage, brightness);

        // forward transformation
        if (value <= 0.0031308) {
            value *= 12.92;
        } else {
            value = 1.055 * pow(value, 1 / 2.4) - 0.055;
        }

    } else {
        value = pow(value, gamma);
        value = correction(value, percentage, brightness);
        value = pow(value, 1 / gamma);
    }

    value *= MAX_BRIGHTNESS;
    data[y][x] = static_cast<int>(value);
}

line_drawer::line_drawer(int b, double t, double x0, double y0, double x1,
                         double y1, double gamma, bool sRGB, int h, int w,
                         int m, int** data)
    : brightness(b),
      thickness(t),
      gamma(gamma),
      sRGB(sRGB),
      from(x0, y0),
      to(x1, y1),
      pic(h, w, m, data) {}

int** line_drawer::get_picture_data() {
    return pic.get_data();
}

void line_drawer::draw_part_of_line(int x0, int x1, double y, double gradient, bool swapped) {
    int arg1, arg2;
    double fy = y - floor(y);
    for (int x = x0; x <= x1; ++x) {
        // std::cout << x << ' ' << y << '\n';
        arg1 = x;
        arg2 = static_cast<int>(floor(y));
        // do_swap(swapped, arg1, arg2);
        pic.fill_pixel(arg1, arg2, swapped, brightness, 1 - fy, sRGB, gamma);

        arg1 = x;
        arg2 = (int) floor(y) + 1;
        // do_swap(swapped, arg1, arg2);
        pic.fill_pixel(arg1, arg2, swapped, brightness, fy, sRGB, gamma);

        y += gradient;
    }
}

bool check_equals(double a, double b) {
    return abs(a - b) < EPS;
}

void line_drawer::draw_line() {
}