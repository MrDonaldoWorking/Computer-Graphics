#ifndef LINE_DRAWER
#define LINE_DRAWER

double const MAX_BRIGHTNESS = 255.0;

struct point {
 public:
    explicit point(double x, double y);
   //  point(std::initializer_list<double> const& list);
    ~point() = default;

    double get_x() const;
    double get_y() const;

    void swap_axis();

 private:
    double x;
    double y;
};

struct picture {
 public:
    picture(int h, int w, int m, int **data);
    ~picture();
    int get_h() const;
    int get_w() const;
    int** get_data();

    void fill_pixel(int x, int y, bool swapped, int brightness, double percentage, bool sRGB, double gamma);
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

    void draw_part_of_line(int x0, int x1, double y, double gradient, bool swapped);
};

#endif