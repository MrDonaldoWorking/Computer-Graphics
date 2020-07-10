#include <functional>
#include <utility>
#include <cstdlib>

int const MAX_BRIGHTNESS = 255;
int const MONOCHROME_COLORS = 1;
int const RGB_COLORS = 3;
double const ONE_SECOND = 1.0 / 2;
double const K_R_601 = 0.299;
double const K_G_601 = 0.587;
double const K_B_601 = 0.114;
double const PERCENTAGE = 0.39 / 100;

#define manual_args int const shift, double const factor
typedef std::function<void(int const, double const)> brightness_changer_function_t;

void clear_array(int ***arr, int n, int m);

struct picture {
    picture() = default;
    picture(int const h, int const w, int const m, int const c, int*** d);
    ~picture();

    int get_width() const;
    int get_height() const;
    int get_max_brightness() const;
    int get_data();

    void switch_brightness(int const change, int const shift, double const factor);
 private:
    int width = 0;
    int height = 0;
    int max_brightness = MAX_BRIGHTNESS;
    int colors = 0;
    int ***data = nullptr;

    void auto_changing(brightness_changer_function_t const& manual, double const min_value, double const max_value);
    void auto_changing_with_skip(brightness_changer_function_t const& manual);

    void manual_RGB(int shift, double factor);
    void manual_YCbCr(int const shift, double const factor);
    void auto_RGB();
    void auto_YCbCr();
    void auto_with_skip_RGB();
    void auto_with_skip_YCbCr();
};