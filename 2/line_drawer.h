#ifndef LINE_DRAWER
#define LINE_DRAWER

double const MAX_BRIGHTNESS = 255.0;
int const AMPLE_SHIFT = 3;
int const STEP = 10;

struct point {
 public:
    explicit point(double const x, double const y);
   //  point(std::initializer_list<double> const& list);
    ~point() = default;

    bool operator<(point const& p) const;

    inline double get_x() const;
    inline double get_y() const;

 private:
    double x;
    double y;
};

struct rectangle {
    explicit rectangle(point const A, point const B, point const C, point const D);
    ~rectangle() = default;

    inline double get_leftest_x() const;
    inline double get_rightest_x() const;
    inline double get_highest_y() const;
    inline double get_lowest_y() const;

    inline point get_A() const;
    inline point get_B() const;
    inline point get_C() const;
    inline point get_D() const;

    double get_height() const;
    double get_width() const;

    double get_square() const;
    double intersect_with(rectangle const& other) const;

 private:
    point A;
    point B;
    point C;
    point D;

    bool is_inside(point const& p) const;
    bool is_entirelly_inside(rectangle const& other) const;
};

struct picture {
 public:
    picture(int h, int w, int m, int **data);
    ~picture();

    int get_h() const;
    int get_w() const;
    int** get_data();

    int get(int const h, int const w) const;
    void set(int const h, int const w, int const val);

 private:
    int height;
    int width;
    int max_brightness;
    int **data = nullptr;
};

struct line_drawer {
 public:
    explicit line_drawer(int b, double t, 
                         double x0, double y0, double x1, double y1,
                         double gamma, bool sRGB,
                         int h, int w, int m, int **data);
    ~line_drawer() = default;

    void draw_line();
    int** get_picture_data();
 private:
    int brightness;
    double thickness;
    point from = point(-1, -1);
    point to = point(-2, -2);
    double gamma;
    bool sRGB;
    picture pic = picture(0, 0, 0, nullptr);

    void fill_pixel(int const x, int const y, double const square);
};

#endif