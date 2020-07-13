#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <vector>
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

bool point::operator <(point const& p) const {
    return x < p.x || (x == p.x && y < p.x);
}

point operator -(const point &u, const point &v) {
	return point(u.get_x() - v.get_x(), u.get_y() - v.get_y());
}

bool point::operator ==(point const& p) const {
    return x == p.x && y == p.y;
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
    return sqrt(sqr(A.get_x() - B.get_x()) + sqr(A.get_y() - B.get_y()));
}

double rectangle::get_width() const {
    return sqrt(sqr(B.get_x() - C.get_x()) + sqr(B.get_y() - C.get_y()));
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

inline double vec(point const& v1, point const& v2) {
	return v1.get_x() * v2.get_y() - v2.get_x() * v1.get_y();
}

inline double length(point const& a) {
	return sqrt(sqr(a.get_x()) + sqr(a.get_y()));
}

double square(std::vector<point> & ps) {
    ps.push_back(ps[0]);
    double sum = 0;
    for (size_t i = 1; i < ps.size(); ++i) {
        sum += vec(ps[i - 1], ps[i]);
    }

    return sum / 2;
}

double rectangle::intersect_with(rectangle const& other) const {
    static std::ofstream out("test.log", std::ios::app);
    static int cnt_ = 0;
    ++cnt_;
    out << "=== " << cnt_ << " ===" << std::endl;

    std::vector<point> const others = {other.get_A(), other.get_B(), other.get_C(), other.get_D()};
    std::vector<point> intersections;
    for (point const& p : others) {
        if (is_inside(p)) {
            intersections.push_back(p);
            out << "(" << p.get_x() << ", " << p.get_y() << ") is inside of line\n";
        }
    }

    if (intersections.size() == 4) {
        if (cnt_ >= 0) {
            out << "Pixel is in line, " << other.get_square() << std::endl;
        }
        return other.get_square();
    }
    if (cnt_ >= 0) {
        out << "Pixel is not in line" << std::endl;
    }

    std::vector<point> const rects = {A, B, C, D};
    for (point const& p : rects) {
        if (other.is_inside(p)) {
            intersections.push_back(p);
            out << "(" << p.get_x() << ", " << p.get_y() << ") is inside of pixel\n";
        }
    }
    std::vector<line> const lines = {line(A, B), line(B, C), line(C, D), line(D, A)};
    out << "\nline lines equations:\n";
    for (line const l : lines) {
        out << l.get_a() << ' ' << l.get_b() << ' ' << l.get_c() << '\n';
    }
    std::vector<line> pixel_lines = {line(others.back(), others[0])};
    for (size_t i = 1; i < others.size(); ++i) {
        pixel_lines.push_back(line(others[i - 1], others[i]));
    }
    out << "\npixel line equations:\n";
    for (line const l : pixel_lines) {
        out << l.get_a() << ' ' << l.get_b() << ' ' << l.get_c() << '\n';
    }

    for (line const& drawing : lines) {
        for (line const& squares : pixel_lines) {
            point p = drawing.intersect_with(squares);
            out << "intersection of (" << drawing.get_a() << ' ' << drawing.get_b() << ' ' << drawing.get_c() << ") and (" << squares.get_a() << ' ' << squares.get_b() << ' ' << squares.get_c() << ") is (" << p.get_x() << ", " << p.get_y() << ")\n";
            if (check_equals(squares.get_b(), 0)) { // vertical
                out << "pixel line is vertical\n";
                if (p.get_y() - other.get_lowest_y() > EPS && other.get_highest_y() - p.get_y() > EPS) {
                    intersections.push_back(p);
                    out << "Added to intersections\n";
                }
            } else if (check_equals(squares.get_a(), 0)) { // horisontal
                out << "pixel line is horisontal\n";
                if (p.get_x() - other.get_leftest_x() > EPS && other.get_rightest_x() - p.get_x() > EPS) {
                    intersections.push_back(p);
                    out << "Added to intersection\n";
                }
            } else {
                std::cerr << "SHIT HAPPENS\n";
                std::cerr << "lines\n";
                std::for_each(lines.begin(), lines.end(), [](line const& l) { std::cerr << l.get_a() << ", " << l.get_b() << ", " << l.get_c() << '\n';});
                std::cerr << "pixel_lines\n";
                std::for_each(pixel_lines.begin(), pixel_lines.end(), [](line const& l) { std::cerr << l.get_a() << ", " << l.get_b() << ", " << l.get_c() << '\n';});
                throw std::runtime_error(":(");
            }
        }
    }
    out << "\nIntersections summary:\n";
    for (point const& p : intersections) {
        out << "(" << p.get_x() << ", " << p.get_y() << ")\n";
    }

    if (intersections.empty()) {
        out << "result is " << 0 << std::endl;
        return 0;
    }

    point the_leftest = intersections[0];
    for (point const& p : intersections) {
        the_leftest = std::min(the_leftest, p);
    }
    out << "the leftest is (" << the_leftest.get_x() << ", " << the_leftest.get_y() << ")\n";

    std::sort(intersections.begin(), intersections.end(), [the_leftest](point const& a, point const& b) {
        double vec_prod = vec(a - the_leftest, b - the_leftest);
        return vec_prod > 0 || (vec_prod > 0 && (length(b - the_leftest) - length(a - the_leftest)) > EPS);
    });
    intersections.resize(std::unique(intersections.begin(), intersections.end()) - intersections.begin());

    out << "\nSorted intersections\n";
    for (point const& p : intersections) {
        out << "(" << p.get_x() << ", " << p.get_y() << ")\n";
    }

    out << "result is " << square(intersections) << std::endl;
    return square(intersections);
}

line::line(point const& a, point const& b) {
    this->a = a.get_y() - b.get_y();
    this->b = b.get_x() - a.get_x();
    this->c = -this->a * a.get_x() - this->b * a.get_y();

    double const z = sqrt(sqr(this->a) + sqr(this->b));
    this->a /= z;
    this->b /= z;
    this->c /= z;
}

double line::get_a() const {
    return a;
}

double line::get_b() const {
    return b;
}

double line::get_c() const {
    return c;
}

inline double det(double const a, double const b, double const c, double const d) {
    return a * d - b * c;
}

point line::intersect_with(line const& other) const {
    double const zn = det(a, b, other.a, other.b);
    if (fabs(zn) < EPS) {
        // doesn't intersect
        return point(-1, -1);
    }

    double const x = -det(c, b, other.c, other.b) / zn;
    double const y = -det(a, c, other.a, other.c) / zn;
    return point(x, y);
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

inline double correction(double val, double const percentage, double const brightness) {
    val *= (1 - percentage);
    val += percentage * brightness;
    return val;
}

inline int correct_bounds(double const x) {
    return static_cast<int>(fmin(MAX_BRIGHTNESS, fmax(0, x)));
}

void line_drawer::fill_pixel(int const x, int const y, double const square) {
    static std::ofstream out("test.log", std::ios::app);
    static int cnt = 0;
    ++cnt;

    if (x < 0 || x >= pic.get_w() || y < 0 || y >= pic.get_h()) {
        return;
    }

    double value = pic.get(y, x) / MAX_BRIGHTNESS;
    double curr_brightness = brightness / MAX_BRIGHTNESS;
    if (cnt >= 0) {
        out << "=== " << cnt << " ===\n";
        out << "x = " << x << ", y = " << y << '\n';
        out << "square = " << square << '\n';
        out << "value = " << value << '\n';
        out << "percentage = " << curr_brightness << '\n';
    }
    if (sRGB) {
        // reverse transformation
        value = sRGB_reverse(value);
        curr_brightness = sRGB_reverse(curr_brightness);

        if (cnt >= 0) {
            out << "\nAfter reverse transformation\n";
            out << "value = " << value << '\n';
            out << "percentage = " << curr_brightness << '\n';
        }

        value = correction(value, square, curr_brightness);

        if (cnt >= 0) {
            out << "\nAfter correction\n";
            out << "value = " << value << '\n';
        }

        // forward transformation
        value = sRGB_forward(value);
        if (cnt >= 0) {
            out << "\nAfter forward transformation\n";
            out << "value = " << value << '\n';
        }
    } else {
        value = gamma_reverse(value, gamma);
        curr_brightness = gamma_reverse(curr_brightness, gamma);

        if (cnt >= 0) {
            out << "\nAfter reverse transformation\n";
            out << "value = " << value << '\n';
            out << "percentage = " << curr_brightness << '\n';
        }

        value = correction(value, square, curr_brightness);

        if (cnt >= 0) {
            out << "\nAfter correction\n";
            out << "value = " << value << '\n';
        }

        value = gamma_forward(value, gamma);

        if (cnt >= 0) {
            out << "\nAfter forward transformation\n";
            out << "value = " << value << '\n';
        }
    }

    if (cnt >= 0) {
        out << "data[" << y << "][" << x << "] = " << correct_bounds(value * MAX_BRIGHTNESS) << std::endl;
    }

    pic.set(y, x, correct_bounds(value * MAX_BRIGHTNESS));
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
    for (int x = get_positive(start_x - AMPLE_SHIFT); x < get_positive(end_x + AMPLE_SHIFT); ++x) {
        for (int y = get_positive(start_y - AMPLE_SHIFT); y < get_positive(end_y + AMPLE_SHIFT); ++y) {
            rectangle const curr_pixel = rectangle(point(x, y), point(x + 1, y), point(x + 1, y + 1), point(x, y + 1));
            double const square = line.intersect_with(curr_pixel);
            fill_pixel(x, y, square);
        }
    }
}