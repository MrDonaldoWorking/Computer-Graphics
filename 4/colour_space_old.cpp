#include <iostream>
#include <fstream>
#include <stdexcept>
#include <math.h>
#include "colour_space.h"

// picture initialization

picture::picture(int cnt, std::string const& name, COLOUR_PALETTE palette) : palette(palette) {
    switch (cnt) {
        case 1:
            if (name.substr(name.length() - 4, 4) != ".ppm") {
                force_close("Expected ppm file\n");
            }

            read_data_P6(name);
            break;

        case 3:
            if (name.substr(name.length() - 4, 4) != ".pgm") {
                force_close("Expected ppm file\n");
            }

            read_data_P5(name);
            break;
    }
}

void picture::clear_array() {
    if (data != nullptr) {
        for (int h = 0; h < height; ++h) {
            for (int w = 0; w < width; ++w) {
                delete[] data[h][w];
            }
            delete[] data[h];
        }
        delete[] data;
        data = nullptr;
    }
}

picture::~picture() {
    clear_array();
}

int picture::get_width() const {
    return width;
}

int picture::get_height() const {
    return height;
}

int picture::get_max_brightness() const {
    return max_brightness;
}

int*** picture::get_data() {
    return data;
}

void picture::read_data_P5(std::string const& file) {
    int dot_pos = file.find_last_of('.');
    std::string name = file.substr(0, dot_pos);
    std::string extension = file.substr(dot_pos);
    for (int num = 0; num < 3; ++num) {
        std::string file_name = name + "_" + std::to_string(num + 1) + extension;
        std::ifstream in(file_name);
        read_meta_data(in, file_name, 5);

        // File contains width * height bytes of data
        for (int h = 0; h < height; ++h) {
            for (int w = 0; w < width; ++w) {
                data[h][w][num] = in.get();
            }
        }
    }
}

void picture::read_data_P6(std::string const& name) {
    std::ifstream in(name);
    read_meta_data(in, name, 6);

    // File contains width * height * 3 bytes of data
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            for (int color = 0; color < 3; ++color) {
                data[h][w][color] = in.get();
            }
        }
    }
}

void picture::update_int(std::string const& name, int &x, int curr_value) {
    if (x == 0) {
        x = curr_value;
    } else if (x != curr_value) {
        force_close(name + " in 2 files are different\n");
    }
}

void picture::read_meta_data(std::ifstream &in, std::string const& name, int encoding) {
    if (!in.is_open()) {
        force_close("Couldn't find file" + name + '\n');
    }

    char p, line_separator;
    int type;
    in >> p >> type;
    line_separator = in.get();
    if (p != 'P' || type != encoding || line_separator != '\n') {
        force_close("Expected P" + std::to_string(encoding) +  " encoding in " + name);
    }

    int this_height, this_width;
    in >> this_width >> this_height;
    line_separator = in.get();
    // guaranteed that width > 0 && height > 0 && they are ints
    if (line_separator != '\n') {
        force_close("Couldn't read width and height\n");
    }
    update_int("Heights", height, this_height);
    update_int("Width", width, this_width);

    in >> max_brightness;
    line_separator = in.get();
    if (max_brightness != 255 || line_separator != '\n') {
        force_close("Couldn't read max brightness or max brightness is not 255\n");
    }

    if (data == nullptr) {
        data = new int**[height];
        for (int h = 0; h < height; ++h) {
            data[h] = new int*[width];
            for (int w = 0; w < width; ++w) {
                data[h][w] = new int[3];
            }
        }
    }
}

void picture::force_close(std::string const& message) {
    clear_array();
    throw std::runtime_error(message);
}

// end of picture initialization

// from ? to RGB

// a < b
inline bool less(double a, double b) {
    return (b - a) > ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * EPS);
}

inline int correction(double val) {
    return static_cast<int>(floor(MAX_BRIGHTNESS * fmax(0, fmin(1, val))));
}

inline void update(int *pixel, double x, double y, double z) {
    pixel[0] = correction(x);
    pixel[1] = correction(y);
    pixel[2] = correction(z);
}

void HSL_to_RGB(int *pixel) {
    double H = (pixel[0] / static_cast<double>(MAX_BRIGHTNESS)) * TWO_PIES;
    double S = pixel[1] / static_cast<double>(MAX_BRIGHTNESS);
    double L = pixel[2] / static_cast<double>(MAX_BRIGHTNESS);

    double H_ = H / ONE_THIRD_PI;
    double C = (1 - fabs(2 * L - 1)) * S;
    double X = C * (1 - fabs(fmod(H_, 2) - 1));
    double m = L - C / 2;

    double R = 0, G = 0, B = 0;
    switch (static_cast<int>(floor(H_))) {
        case 0:
            R = C;
            G = X;
            B = 0;
            break;

        case 1:
            R = X;
            G = C;
            B = 0;
            break;

        case 2:
            R = 0;
            G = C;
            B = X;
            break;

        case 3:
            R = 0;
            G = X;
            B = C;
            break;

        case 4:
            R = X;
            G = 0;
            B = C;
            break;

        case 5:
            R = C;
            G = 0;
            B = X;
            break;
    }

    update(pixel, R + m, G + m, B + m);
}

void HSV_to_RGB(int *pixel) {
    double H = (pixel[0] / static_cast<double>(MAX_BRIGHTNESS)) * TWO_PIES;
    double S = pixel[1] / static_cast<double>(MAX_BRIGHTNESS);
    double V = pixel[2] / static_cast<double>(MAX_BRIGHTNESS);

    double C = V * S;
    double H_ = H / ONE_THIRD_PI;
    double X = C * (1 - fabs(fmod(H_, 2) - 1));
    double m = V - C;

    double R = 0, G = 0, B = 0;
    // doesn't work if H_ > 0 :(
    if (H_ >= 0 && H_ <= 1) {
        R = C;
        G = X;
        B = 0;
    } else if (H_ > 1 && H_ <= 2) {
        R = X;
        G = C;
        B = 0;
    } else if (H_ > 2 && H_ <= 3) {
        R = 0;
        G = C;
        B = X;
    } else if (H_ > 3 && H_ <= 4) {
        R = 0;
        G = X;
        B = C;
    } else if (H_ > 4 && H_ <= 5) {
        R = X;
        G = 0;
        B = C;
    } else if (H_ > 5 && H_ <= 6) {
        R = C;
        G = 0;
        B = X;
    }

    update(pixel, R + m, G + m, B + m);
}

// Not working :(
void HSL_V_to_RGB(int *pixel, COLOUR_PALETTE palette) {
    double H = (pixel[0] / static_cast<double>(MAX_BRIGHTNESS)) * TWO_PIES;
    double S = pixel[1] / static_cast<double>(MAX_BRIGHTNESS);
    double L = pixel[2] / static_cast<double>(MAX_BRIGHTNESS);

    double H_ = H / ONE_THIRD_PI;
    double C, m;
    switch (palette) {
        case HSL:
            C = (1 - fabs(2 * L - 1)) * S;
            m = L - C / 2;    
            break;
        
        case HSV:
            C = S * L;
            m = L - C;
            break;
    }
    double X = C * (1 - fabs(fmod(H_, 2) - 1));

    double R = 0, G = 0, B = 0;
    if (less(H_, 1)) {
        R = (C + m);
        G = (X + m);
        B = (m);
    } else if (less(H_, 2)) {
        R = (X + m);
        G = (C + m);
        B = (m);
    } else if (less(H_, 3)) {
        R = (m);
        G = (C + m);
        B = (X + m);
    } else if (less(H_, 4)) {
        R = (m);
        G = (X + m);
        B = (C + m);
    } else if (less(H_, 5)) {
        R = (X + m);
        G = (m);
        B = (C + m);
    } else if (less(H_, 6)) {
        R = (C + m);
        G = (m);
        B = (X + m);
    }

    update(pixel, R, G, B);
}

void YCbCr_to_RGB(int *pixel, double K_R, double K_G, double K_B) {
    double Y = pixel[0] / static_cast<double>(MAX_BRIGHTNESS);
    double Cb = pixel[1] / static_cast<double>(MAX_BRIGHTNESS) - ONE_SECOND;
    double Cr = pixel[2] / static_cast<double>(MAX_BRIGHTNESS) - ONE_SECOND;

    double R = Y + Cr * (2 - 2 * K_R);
    double G = Y - (K_B / K_G) * (2 - 2 * K_B) * Cb - (K_R / K_G) * (2 - 2 * K_R) * Cr;
    double B = Y + (2 - 2 * K_B) * Cb;

    update(pixel, R, G, B);
}

void YCoCg_to_RGB(int *pixel) {
    double Y = pixel[0] / static_cast<double>(MAX_BRIGHTNESS);
    double Co = pixel[1] / static_cast<double>(MAX_BRIGHTNESS) - ONE_SECOND;
    double Cg = pixel[2] / static_cast<double>(MAX_BRIGHTNESS) - ONE_SECOND;

    double R = Y + Co - Cg;
    double G = Y + Cg;
    double B = Y - Co - Cg;

    update(pixel, R, G, B);
}

void picture::to_RGB() {
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            switch (palette) {
                case RGB:
                    return;

                case HSL:
                    HSL_to_RGB(data[h][w]);
                    break;

                case HSV:
                    HSV_to_RGB(data[h][w]);
                    break;

                case YCbCr601:
                    YCbCr_to_RGB(data[h][w], K_R_601, K_G_601, K_B_601);
                    break;
                
                case YCbCr709:
                    YCbCr_to_RGB(data[h][w], K_R_709, K_G_709, K_B_709);
                    break;

                case YCoCg:
                    YCoCg_to_RGB(data[h][w]);
                    break;

                case CMY:
                    data[h][w][0] = MAX_BRIGHTNESS - data[h][w][0];
                    data[h][w][1] = MAX_BRIGHTNESS - data[h][w][1];
                    data[h][w][2] = MAX_BRIGHTNESS - data[h][w][2];
                    break;
            }
        }
    }
}

// end of ? to RGB

// RGB to ?

void RGB_to_HSL(int *pixel) {
    int max_i = std::max(pixel[0], std::max(pixel[1], pixel[2]));
    int min_i = std::min(pixel[0], std::min(pixel[1], pixel[2]));

    double R = pixel[0] / static_cast<double>(MAX_BRIGHTNESS);
    double G = pixel[1] / static_cast<double>(MAX_BRIGHTNESS);
    double B = pixel[2] / static_cast<double>(MAX_BRIGHTNESS);

    double max_d = fmax(R, fmax(G, B));
    double min_d = fmin(R, fmin(G, B));
    double V = max_d;
    double C = max_d - min_d;
    double L = V - C / 2;

    double H = 0;
    if (max_i != min_i) {
        if (max_d == R) {
            H = ONE_THIRD_PI * ((G - B) / C);
        } else if (max_d == G) {
            H = ONE_THIRD_PI * (2 + (B - R) / C);
        } else if (max_d == B) {
            H = ONE_THIRD_PI * (4 + (R - G) / C);
        }
    }
    if (H < 0) {
        H += TWO_PIES;
    }

    double S = 0;
    // V != 0 && V != 1
    if (max_i != 0 && max_i != 2 * MAX_BRIGHTNESS - min_i) {
        S = (V - L) / std::min(L, 1 - L);
    }

    update(pixel, H / TWO_PIES, S, L);
}

void RGB_to_HSV(int *pixel) {
    double R = pixel[0] / static_cast<double>(MAX_BRIGHTNESS);
    double G = pixel[1] / static_cast<double>(MAX_BRIGHTNESS);
    double B = pixel[2] / static_cast<double>(MAX_BRIGHTNESS);

    double max_d = fmax(R, fmax(G, B));
    double min_d = fmin(R, fmin(G, B));
    double C = max_d - min_d;
    double V = max_d;

    double H = 0;
    if (C != 0) {
        if (max_d == R) {
            H = ONE_THIRD_PI * (G - B) / C;
        } else if (max_d == G) {
            H = ONE_THIRD_PI * (2 + (B - R) / C);
        } else if (max_d == B) {
            H = ONE_THIRD_PI * (4 + (R - G) / C);
        }
    }
    if (H < 0) {
        H += TWO_PIES;
    }

    double S = 0;
    if (V != 0) {
        S = C / V;
    }

    update(pixel, H / TWO_PIES, S, V);
}

// Not working :(
void RGB_to_HSL_V(int *pixel, COLOUR_PALETTE palette) {
    int max_i = std::max(pixel[0], std::max(pixel[1], pixel[2]));
    int min_i = std::min(pixel[0], std::min(pixel[1], pixel[2]));
    double max_d = max_i / static_cast<double>(MAX_BRIGHTNESS);
    double min_d = min_i / static_cast<double>(MAX_BRIGHTNESS);

    double V = max_d;
    double C = max_d - min_d;
    double L = V - C / 2;

    double H = 0;
    if (max_i != min_i) {
        if (max_i == pixel[0]) { // R
            H = ONE_THIRD_PI * (pixel[1] - pixel[2]) / C / MAX_BRIGHTNESS;
        } else if (max_i == pixel[1]) { // G
            H = ONE_THIRD_PI * (2 + (pixel[2] - pixel[0]) / C / MAX_BRIGHTNESS);
        } else { // B
            H = ONE_THIRD_PI * (4 + (pixel[0] - pixel[1]) / C / MAX_BRIGHTNESS);
        }
    }

    double S = 0;
    switch (palette) {
        case HSL:
            // L != 0 && L != 1
            if (max_i != 0 && max_i != 2 * MAX_BRIGHTNESS - min_i) {
                S = (V - L) / std::min(L, 1 - L);
            }
            pixel[2] = correction(L);
            break;

        case HSV:
            // V != 0
            if (max_i != 0) {
                S = C / V;
            }
            pixel[2] = correction(V);
            break;
    }
    pixel[0] = correction(H / TWO_PIES);
    pixel[1] = correction(S);
}

void RGB_to_YCbCr(int *pixel, double K_R, double K_G, double K_B) {
    double R = pixel[0] / static_cast<double>(MAX_BRIGHTNESS);
    double G = pixel[1] / static_cast<double>(MAX_BRIGHTNESS);
    double B = pixel[2] / static_cast<double>(MAX_BRIGHTNESS);

    double Y = K_R * R + K_G * G + K_B * B;
    double Cb = ONE_SECOND * (B - Y) / (1 - K_B);
    double Cr = ONE_SECOND * (R - Y) / (1 - K_R);

    pixel[0] = correction(Y);
    pixel[1] = correction(Cb + ONE_SECOND);
    pixel[2] = correction(Cr + ONE_SECOND);
}

void RGB_to_YCoCg(int *pixel) {
    double R = pixel[0] / static_cast<double>(MAX_BRIGHTNESS);
    double G = pixel[1] / static_cast<double>(MAX_BRIGHTNESS);
    double B = pixel[2] / static_cast<double>(MAX_BRIGHTNESS);

    double Y = R / 4 + G / 2 + B / 4;
    double Co = R / 2 - B / 2;
    double Cg = -R / 4 + G / 2 - B / 4;

    pixel[0] = correction(Y);
    pixel[1] = correction(Co + ONE_SECOND);
    pixel[2] = correction(Cg + ONE_SECOND);
}

void picture::RGB_to(COLOUR_PALETTE palette) {
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            switch (palette) {
                case RGB:
                    return;

                case HSL:
                    RGB_to_HSL(data[h][w]);
                    break;

                case HSV:
                    RGB_to_HSV(data[h][w]);
                    break;

                case YCbCr601:
                    RGB_to_YCbCr(data[h][w], K_R_601, K_G_601, K_B_601);
                    break;
                
                case YCbCr709:
                    RGB_to_YCbCr(data[h][w], K_R_709, K_G_709, K_B_709);
                    break;

                case YCoCg:
                    RGB_to_YCoCg(data[h][w]);
                    break;
                
                case CMY:
                    data[h][w][0] = MAX_BRIGHTNESS - data[h][w][0];
                    data[h][w][1] = MAX_BRIGHTNESS - data[h][w][1];
                    data[h][w][2] = MAX_BRIGHTNESS - data[h][w][2];
                    break;
            }
        }
    }
}

// end of RGB to ?

// write info to file

void picture::write(int cnt, std::string const& name) {
    switch (cnt) {
        case 1:
            if (name.substr(name.length() - 4, 4) != ".ppm") {
                force_close("Output file must be ppm\n");
            }

            write_to_one_P6(name);
            break;

        case 3:
            if (name.substr(name.length() - 4, 4) != ".pgm") {
                force_close("Output file must be pgm\n");
            }

            write_to_three_P5(name);
            break;
    }
}

void picture::write_to_one_P6(std::string const& file) {
    std::ofstream out(file);
    if (!out.is_open()) {
        force_close("Couldn't open file " + file + " to write\n");
    }

    write_meta_data(out, file, 6);

    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            for (int color = 0; color < 3; ++color) {
                if (!out || out.bad()) {
                    force_close("Output stream was damaged\n");
                }
                out << static_cast<unsigned char>(data[h][w][color]);
            }
        }
    }

    out.close();
}

void picture::write_to_three_P5(std::string const& file) {
    int dot_pos = file.find_last_of('.');
    std::string name = file.substr(0, dot_pos);
    std::string extension = file.substr(dot_pos);
    for (int num = 0; num < 3; ++num) {
        std::string file_name = name + "_" + std::to_string(num + 1) + extension;
        // std::cout << "Writing info into " << file_name << '\n';
        std::ofstream out(file_name);
        if (!out.is_open()) {
            force_close("Could't open file " + file_name + " to write\n");
        }

        write_meta_data(out, file_name, 5);
        
        for (int h = 0; h < height; ++h) {
            for (int w = 0; w < width; ++w) {
                if (!out || out.bad()) {
                    force_close("Output stream was damaged\n");
                }
                out << static_cast<unsigned char>(data[h][w][num]);
            }
        }

        out.close();
    }
}

void picture::write_meta_data(std::ofstream &out, std::string const& name, int encoding) {
    out << 'P' << encoding << '\n';
    out << width << ' ' << height << '\n';
    out << max_brightness << '\n';
}