#include <fstream>

int const MAX_BRIGHTNESS = 255;
double const S_RGB_BARRIER = 0.04045;
double const S_RGB_DIVISOR = 12.92;
double const S_RGB_DEGREE = 2.4;
double const S_RGB_BASE_TERM = 0.055;
double const S_RGB_BASE_DIVISOR = 1.055;

template <typename T>
void clear_array(T **arr, int const n);

struct picture {
    picture() = default;
    picture(int h, int w, int m, int g, std::ifstream &in);
    ~picture();

    int get_width() const;
    int get_height() const;
    int get_max_brightness() const;
    double** get_all();
    double get(int const h, int const w);
    void set(int const h, int const w, double const x);

 private:
    int width = 0;
    int height = 0;
    int max_brightness = MAX_BRIGHTNESS;
    double **data = nullptr;

    int counter = 0;
};

int const ORDERED_SIZE = 8;
int const ORDERED[8][8] = {
    {0,  48, 12, 60, 3,  51, 15, 63},
    {32, 16, 44, 28, 35, 19, 47, 31},
    {8,  56, 4,  52, 11, 59, 7,  55},
    {40, 24, 36, 20, 43, 27, 39, 23},
    {2,  50, 14, 62, 1,  49, 13, 61},
    {34, 18, 46, 30, 33, 17, 45, 29},
    {10, 58, 6,  54, 9,  57, 5,  53},
    {42, 26, 38, 22, 41, 25, 37, 21}
};

int const HALFTONE[4][4] = {
    {6,  12, 10, 3},
    {11, 15, 13, 7},
    {9,  14, 5,  1},
    {4,  8,  2,  0}
};

double const ONE_SIXTEENTH = 1.0 / 16;

void without_dithering(picture &pic, int const bitness, bool const sRGB, double const gamma);
void ordered_dithering(picture &pic, int const bitness, bool const sRGB, double const gamma);
void random_dithering(picture &pic, int const bitness, bool const sRGB, double const gamma);
void Floyd_Steinberg_dithering(picture &pic, int const bitness, bool const sRGB, double const gamma);
void Jarvis_Judice_Ninke_dithering(picture &pic, int const bitness, bool const sRGB, double const gamma);
void Sierra_3_dithering(picture &pic, int const bitness, bool const sRGB, double const gamma);
void Atkinson_dithering(picture &pic, int const bitness, bool const sRGB, double const gamma);
void Halftone_dithering(picture &pic, int const bitness, bool const sRGB, double const gamma);