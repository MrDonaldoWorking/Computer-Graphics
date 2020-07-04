#include <iostream>
#include <fstream>
#include <string>
#include "line_drawer.h"

void clear_array(int **data, int height) {
    for (int h = 0; h < height; ++h) {
        delete[] data[h];
    }
    delete[] data;
}

int show_message(std::ifstream &in, std::string const &message) {
    std::cerr << message << '\n';
    in.close();
    return 1;
}

int show_message(std::ifstream &in, char const *message, char const *file) {
    std::cerr << message << file << '\n';
    in.close();
    return 1;
}

int show_message(std::ifstream &in, char const *message, char const *file,
                 int **data, int height) {
    clear_array(data, height);
    return show_message(in, message, file);
}

int main(int argc, char* argv[]) {
    if (argc != 9 && argc != 10) {
        std::cerr << "Got " << argc << " arguments\n";
        std::cerr << "Usage: <input filename> <output filename> <line brightness> <line thickness> <x0> <y0> <x1> <y1> <gamma?>\n";
        return 1;
    }

    std::ifstream in(argv[1]);
    if (!in.is_open()) {
        std::cerr << "Couldn't open " << argv[1];
        return 1;
    }

    int brightness;
    double thickness, x0, y0, x1, y1, gamma = 2.4;
    bool sRGB = true;
    try {
        brightness = std::stoi(argv[3]);
        if (brightness < 0) {
            return show_message(in, "Brightness must be a positive integer");
        }

        std::string positive_error = " must be a positive number";
        thickness = std::stod(argv[4]);
        if (thickness < 0) {
            return show_message(in, "Thickness" + positive_error);
        }

        std::string coord = " coordinate";
        x0 = std::stod(argv[5]);
        if (x0 < 0) {
            return show_message(in, "x0" + coord + positive_error);
        }
        y0 = std::stod(argv[6]);
        if (y0 < 0) {
            return show_message(in, "y0" + coord + positive_error);
        }
        x1 = std::stod(argv[7]);
        if (x0 < 0) {
            return show_message(in, "x1" + coord + positive_error);
        }
        y1 = std::stod(argv[8]);
        if (y1 < 0) {
            return show_message(in, "y1" + coord + positive_error);
        }

        if (argc == 10) {
            gamma = std::stod(argv[9]);
            sRGB = false;
        }
    } catch (std::invalid_argument const& e) {
        std::cerr << "Got not a number value\n";
        std::cerr << e.what();
        return 1;
    } catch (std::out_of_range const& e) {
        std::cerr << "Brightness value is too big\n";
        std::cerr << e.what();
        return 1;
    }
    std::cout << "Succsessfully read all data from console\n";

    char p, line_separator;
    int type;
    in >> p >> type;
    line_separator = in.get();
    if (p != 'P' || (type < 5 && type > 6) || line_separator != '\n') {
        return show_message(in, "Expected P5 or P6 at a first line of ", argv[1]);
    }

    int width, height;
    in >> width >> height;
    line_separator = in.get();
    if (width <= 0 || height <= 0 || line_separator != '\n') {
        return show_message(in, "Expected two positive integer values at a second line of ", argv[1]);
    }
    if (x0 >= width || x1 >= width || y0 >= height || y1 >= height) {
        return show_message(in, "Line coordinates are out of bounds: [0.." + std::to_string(width) + ")*[0.." + std::to_string(height) + ")");
    }

    int max_brightness;
    in >> max_brightness;
    line_separator = in.get();
    if (max_brightness != MAX_BRIGHTNESS || line_separator != '\n') {
        return show_message(in, "Expected 255 at a third line of ", argv[1]);
    }
    if (brightness > max_brightness) {
        return show_message(in, "Brightness given in arguments is larger than max brightness in file ", argv[1]);
    }

    int **data;
    data = new int*[height];
    for (int h = 0; h < height; ++h) {
        data[h] = new int[width];
    }

    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            data[h][w] = in.get();
            if (data[h][w] == -1 || in.gcount() == 0 || in.eof()) {
                return show_message(in, "Unexpected data size: not enough bytes in ", argv[1], data, height);
            }
        }
    }
    if (!(in.get() == -1) || !in.eof()) {
        return show_message(in, "Unexpected data size: redundant bytes in ", argv[1], data, height);
    }
    std::cout << "Succsessfully read all data from file\n";

    line_drawer drawer(brightness, thickness, x0, y0, x1, y1, gamma, sRGB, height, width, max_brightness, data);
    drawer.draw_line();
    std::cout << "Line was successfully drawn\n";

    std::ofstream out(argv[2]);
    if (!out.is_open()) {
        std::cerr << "Couldn't open " << argv[2];
        in.close();
        return 1;
    }

    out << "P5\n";
    out << width << ' ' << height << '\n';
    out << max_brightness << '\n';

    data = drawer.get_picture_data();
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            if (!out || out.bad()) {
                std::cerr << "An error occurred while writing data\n";
                out.close();
                // array data will be cleared in pictore destructor (in line_drawer)
                return 1;
            }
            out << static_cast<unsigned char>(data[h][w]);
        }
    }
    std::cout << "Successfully written picture with line data to file\n";

    out.close();

    return 0;
}