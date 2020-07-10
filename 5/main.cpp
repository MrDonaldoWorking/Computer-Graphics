#include <iostream>
#include <string>
#include <fstream>
#include "brightness_changer.h"

int main(int argc, char* argv[]) {
    if (argc != 4 && argc != 6) {
        std::cerr << "Got " << argc << " arguments\n";
        std::cerr << "Usage: <input file> <output file> <change>\n if change is 0 or 1 then you should put also <shift> <factor>\n";
        return EXIT_FAILURE;
    }

    std::string const input(argv[1]);
    if (input.length() < 4 || input.substr(input.length() - 4, 4) != ".pnm") {
        std::cerr << "Input file must have pnm extension\n";
        return EXIT_FAILURE;
    }

    std::string const output(argv[2]);
    if (output.length() < 4 || output.substr(output.length() - 4, 4) != ".pnm") {
        std::cerr << "Output file must have pnm extension\n";
        return EXIT_FAILURE;
    }

    int change;
    try {
        change = std::stoi(argv[3]);
    } catch (std::invalid_argument const& e) {
        std::cerr << "<change> must be an int value\n";
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    } catch (std::out_of_range const& e) {
        std::cerr << "<change> must be in [0..5]\n";
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    int shift;
    double factor;
    switch (change) {
        case 0:
        case 1:
            if (argc == 4) {
                std::cerr << "Not ample arguments: sought <shift> <factor>, but couldn't find them\n";
                return EXIT_FAILURE;
            }
            try {
                shift = std::stoi(argv[4]);
            } catch (std::invalid_argument const& e) {
                std::cerr << "<shift> must be an int value\n";
                std::cerr << e.what() << '\n';
                return EXIT_FAILURE;
            } catch (std::out_of_range const& e) {
                std::cerr << "<shift> must be in [-255..255]\n";
                std::cerr << e.what() << '\n';
                return EXIT_FAILURE;
            }
            if (shift < -255 || shift > 255) {
                std::cerr << "<shift> must be in [-255..255]\n";
                return EXIT_FAILURE;
            }
            try {
                factor = std::stod(argv[5]);
            } catch (std::invalid_argument const& e) {
                std::cerr << "<factor> must be a positive number\n";
                std::cerr << e.what() << '\n';
            }
            if (factor < 1.0 / 255 || factor > 255) {
                std::cerr << "<factor> must be in [1/255..255]\n";
                return EXIT_FAILURE;
            }
            break;

        case 2:
        case 3:
        case 4:
        case 5:
            if (argc == 6) {
                std::cerr << "Redundant arguments: " << argv[4] << ' ' << argv[5] << " are not needed\n";
                return EXIT_FAILURE;
            }
            break;
        
        default:
            std::cerr << "<change> must be in [0..5]\n";
            return EXIT_FAILURE;
    }

    std::ifstream in(input);
    if (!in.is_open()) {
        std::cerr << "Couldn't open " << input << '\n';
        return EXIT_FAILURE;
    }

    char p;
    int type;
    in >> p >> type;
    if (p != 'P' || type < 5 || type > 6) {
        std::cerr << "Couldn't read first line in the file or got unextected encoding, different from P5 or P6 in " << input << '\n';
        in.close();
        return EXIT_FAILURE;
    }

    int width, height;
    in >> width >> height;
    if (width <= 0 || height <= 0) {
        std::cerr << "Couldn't get width an height info or they are not positive in " << input << '\n';
        in.close();
        return EXIT_FAILURE;
    }

    int brightness;
    in >> brightness;
    char line_separator = in.get();
    if (brightness != MAX_BRIGHTNESS || line_separator != '\n') {
        std::cerr << "Couldn't get the highest pixel brightness or it is not equals " << MAX_BRIGHTNESS << " in " << input << '\n';
        in.close();
        return EXIT_FAILURE;
    }

    int colors = (type == 5 ? MONOCHROME_COLORS : RGB_COLORS);
    int ***data = new int**[height];
    for (int h = 0; h < height; ++h) {
        data[h] = new int*[width];
        for (int w = 0; w < width; ++w) {
            data[h][w] = new int[colors];
        }
    }

    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            for (int color = 0; color < colors; ++color) {
                data[h][w][color] = in.get();

                if (data[h][w][color] == -1 || in.gcount() == 0 || in.eof()) {
                    std::cerr << "Unexpected data size: not ample bytes in " << input << '\n';
                    in.close();
                    clear_array(data, height, width);
                    return EXIT_FAILURE;
                }
            }
        }
    }
    if (!(in.get() == -1) || !in.eof()) {
        std::cerr << "Unexpected data size: redundant bytes in " << input << '\n';
        in.close();
        clear_array(data, height, width);
        return EXIT_FAILURE;
    }

    picture pic(height, width, brightness, colors, data);
    try {
        pic.switch_brightness(change, shift, factor);
    } catch (std::runtime_error const& e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    std::ofstream out(output);
    if (!out.is_open()) {
        std::cerr << "Couldn't open " << output << '\n';
        return EXIT_FAILURE;
    }

    out << p << type << line_separator;
    out << width << ' ' << height << line_separator;
    out << brightness << line_separator;
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            for (int color = 0; color < colors; ++color) {
                if (!out || out.bad()) {
                    std::cerr << "An error occurred while writing data\n";
                    out.close();
                    // array data will be cleared in picture destructor
                    return EXIT_FAILURE;
                }
                out << static_cast<unsigned char>(data[h][w][color]);
            }
        }
    }

    return EXIT_SUCCESS;
}