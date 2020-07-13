#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <fstream>
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

int picture::get(int const h, int const w) const {
    return data[h][w];
}

void picture::set(int const h, int const w, int const val) {
    data[h][w] = val;
}

rectangle::rectangle(point const A, point const B, point const C, point const D) : A(A), B(B), C(C), D(D) {}

point rectangle::get_A() const {
    return A;
}

point rectangle::get_B() const {
    return B;
}

point rectangle::get_C() const {
    return C;
}

point rectangle::get_D() const {
    return D;
}

inline double sqr(double const x) {
    return x * x;
}

double rectangle::get_height() const {
    return sqr(A.get_x() - B.get_x()) + sqr(A.get_y() - B.get_y());
}

double rectangle::get_width() const {
    return sqr(B.get_x() - C.get_x()) + sqr(B.get_y() - C.get_y());
}

double rectangle::get_leftest_x() const {
    return fmin(fmin(A.get_x(), B.get_x()), fmin(C.get_x(), D.get_x()));
}

double rectangle::get_rightest_x() const {
    return fmax(fmax(A.get_x(), B.get_x()), fmax(C.get_x(), D.get_x()));
}

double rectangle::get_highest_y() const {
    return fmax(fmax(A.get_y(), B.get_y()), fmax(C.get_y(), D.get_y()));
}

double rectangle::get_lowest_y() const {
    return fmin(fmin(A.get_y(), B.get_y()), fmin(C.get_y(), D.get_y()));
}

double triangle_square(point const A, point const B, point const C) {
    return fabs((B.get_x() - A.get_x()) * (C.get_y() - A.get_y()) - (C.get_x() - A.get_x()) * (B.get_y() - A.get_y())) / 2;
}

bool check_equals(double const a, double const b) {
    return abs(a - b) < EPS;
}

double rectangle::get_square() const {
    return get_height() * get_width();
}

// checking if 4 triangles sum is equals to rectangle square
bool rectangle::is_inside(point const& p) const {
    double const S1 = triangle_square(A, B, p);
    double const S2 = triangle_square(B, C, p);
    double const S3 = triangle_square(C, D, p);
    double const S4 = triangle_square(D, A, p);

    return check_equals(get_square(), S1 + S2 + S3 + S4);
}

bool rectangle::is_entirelly_inside(rectangle const& other) const {
    return is_inside(other.get_A()) && is_inside(other.get_B()) && is_inside(other.get_C()) && is_inside(other.get_D());
}

double rectangle::intersect_with(rectangle const& other) const {
    if (is_entirelly_inside(other)) {
        return other.get_square();
    }

    double x_step = other.get_width() / STEP;
    double y_step = other.get_height() / STEP;

    // split pixel into 100 smaller squares
    int cnt = 0;
    for (double x = other.get_leftest_x() + x_step; x <= other.get_rightest_x(); x += x_step) {
        for (double y = other.get_lowest_y() + y_step; y <= other.get_highest_y(); y += y_step) {
            if (is_inside(point(x, y))) {
                ++cnt;
            }
        }
    }

    return cnt / sqr(STEP);
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

double sRGB_reverse(double const val) {
    if (val <= 0.04045) {
        return val / 12.92;
    } else {
        return pow((val + 0.055) / 1.055, 2.4);
    }
}

double sRGB_forward(double const val) {
    if (val <= 0.0031308) {
        return val * 12.92;
    } else {
        return 1.055 * pow(val, 1 / 2.4) - 0.055;
    }
}

double gamma_reverse(double const val, double const gamma) {
    return pow(val, gamma);
}

double gamma_forward(double const val, double const gamma) {
    return pow(val, 1 / gamma);
}

inline double correction(double const val, double const percentage, int const brightness) {
    return val * (1 - percentage) + percentage * brightness;
}

void line_drawer::fill_pixel(int const x, int const y, double const square) {
    if (x < 0 || x >= pic.get_w() || y < 0 || y >= pic.get_h()) {
        return;
    }

    double value = pic.get(y, x) / MAX_BRIGHTNESS;
    double percentage = brightness / MAX_BRIGHTNESS;
    if (sRGB) {
        // reverse transformation
        value = sRGB_reverse(value);
        percentage = sRGB_reverse(percentage);

        value = correction(value, percentage, brightness);

        // forward transformation
        value = sRGB_forward(value);
    } else {
        value = gamma_reverse(value, gamma);
        percentage = gamma_reverse(percentage, gamma);

        value = correction(value, percentage, brightness);

        value = gamma_forward(value, gamma);
    }

    pic.set(y, x, value * MAX_BRIGHTNESS);
}

inline int get_positive(double const x) {
    return static_cast<int>(fmax(0, x));
}

void line_drawer::draw_line() {
    std::ofstream out("test.log");
    out.precision(5);
    out << std::fixed;

    point const line_vec = point(to.get_x() - from.get_x(), to.get_y() - from.get_y());
    out << "line_vec = (" << line_vec.get_x() << ", " << line_vec.get_y() << ")\n";

    point const thick_vec = (check_equals(line_vec.get_x(), 0) ? point(1, 0) : point(-line_vec.get_y() / line_vec.get_x(), 1));
    double const wideness = sqrt(sqr(thickness / 2) / (sqr(thick_vec.get_x()) + sqr(thick_vec.get_y())));
    out << "thick_vec = (" << thick_vec.get_x() << ", " << thick_vec.get_y() << ")\n";
    out << "wideness = " << wideness << '\n';

    double const shift_x = wideness * thick_vec.get_x();
    double const shift_y = wideness * thick_vec.get_y();

    point const A = point(from.get_x() + shift_x, from.get_y() + shift_y);
    point const B = point(from.get_x() - shift_x, from.get_y() - shift_y);
    point const C = point(to.get_x() - shift_x, to.get_y() - shift_y);
    point const D = point(to.get_x() + shift_x, to.get_y() + shift_y);
    rectangle const line = rectangle(A, B, C, D);
    out << "A = (" << A.get_x() << ", " << A.get_y() << ")\n";
    out << "B = (" << B.get_x() << ", " << B.get_y() << ")\n";
    out << "C = (" << C.get_x() << ", " << C.get_y() << ")\n";
    out << "D = (" << D.get_x() << ", " << D.get_y() << ")\n";

    double const start_x = line.get_leftest_x();
    out << "start_x = " << get_positive(start_x - AMPLE_SHIFT) << '\n';
    double const end_x = line.get_rightest_x();
    out << "end_x = " << get_positive(end_x + AMPLE_SHIFT) << '\n';
    double const start_y = line.get_lowest_y();
    out << "start_y = " << get_positive(start_y - AMPLE_SHIFT) << '\n';
    double const end_y = line.get_highest_y();
    out << "end_y = " << get_positive(end_y + AMPLE_SHIFT) << std::endl;
    for (int x = get_positive(start_x - AMPLE_SHIFT); x <= get_positive(end_x + AMPLE_SHIFT); ++x) {
        for (int y = get_positive(start_y - AMPLE_SHIFT); y <= get_positive(end_y + AMPLE_SHIFT); ++y) {
            rectangle const curr_pixel = rectangle(point(x, y), point(x + 1, y), point(x, y + 1), point(x + 1, y + 1));
            double const square = line.intersect_with(curr_pixel);
            fill_pixel(x, y, square);
        }
    }
}