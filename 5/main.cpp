#include <iostream>
#include <string>
#include <fstream>
#include "brightness_changer.h"

int main(int argc, char* argv[]) {
    if (argc != 4 && argc != 6) {
        std::cerr << "Got " << argc << " arguments\n";
        std::cerr << "Usage: <input file> <output file> <change>\n if change is 0 or 1 then you should put also <shift> <factor>\n";
        return 1;
    }

    std::string const input(argv[1]);
    if (input.length() < 4 || input.substr(input.length() - 4, 4) != ".pnm") {
        std::cerr << "Input file must have pnm extension\n";
        return 1;
    }

    std::string const output(argv[2]);
    if (output.length() < 4 || output.substr(output.length() - 4, 4) != ".pnm") {
        std::cerr << "Output file must have pnm extension\n";
        return 1;
    }

    int change;
    try {
        change = std::stoi(argv[3]);
    } catch (std::invalid_argument const& e) {
        std::cerr << "<change> must be an int value\n";
        std::cerr << e.what() << '\n';
        return 1;
    } catch (std::out_of_range const& e) {
        std::cerr << "<change> must be in [0..5]\n";
        std::cerr << e.what() << '\n';
        return 1;
    }

    int shift;
    double factor;
    switch (change) {
        case 0:
        case 1:
            if (argc == 4) {
                std::cerr << "Not ample arguments: sought <shift> <factor>, but couldn't find them\n";
                return 1;
            }
            try {
                shift = std::stoi(argv[4]);
            } catch (std::invalid_argument const& e) {
                std::cerr << "<shift> must be an int value\n";
                std::cerr << e.what() << '\n';
                return 1;
            } catch (std::out_of_range const& e) {
                std::cerr << "<shift> must be in [-255..255]\n";
                std::cerr << e.what() << '\n';
                return 1;
            }
            if (shift < -255 || shift > 255) {
                std::cerr << "<shift> must be in [-255..255]\n";
                return 1;
            }
            try {
                factor = std::stod(argv[5]);
            } catch (std::invalid_argument const& e) {
                std::cerr << "<factor> must be a positive number\n";
                std::cerr << e.what() << '\n';
            }
            if (factor < 1.0 / 255 || factor > 255) {
                std::cerr << "<factor> must be in [1/255..255]\n";
                return 1;
            }
            break;

        case 2:
        case 3:
        case 4:
        case 5:
            if (argc == 6) {
                std::cerr << "Redundant arguments: " << argv[4] << ' ' << argv[5] << " are not needed\n";
                return 1;
            }
            break;
        
        default:
            std::cerr << "<change> must be in [0..5]\n";
            return 1;
    }

    std::ifstream in(input);
    if (!in.is_open()) {
        std::cerr << "Couldn't open " << input << '\n';
        return 1;
    }

    char p;
    int type;
    in >> p >> type;
    char line_separator = in.get();
    if (p != 'P' || type < 5 || type > 6 || line_separator != '\n') {
        std::cerr << "Couldn't read first line in the file or got unextected encoding, different from P5 or P6 in " << input << '\n';
        in.close();
        return 1;
    }

    int width, height;
    in >> width >> height;
    line_separator = in.get();
    if (width <= 0 || height <= 0 || line_separator != '\n') {
        std::cerr << "Couldn't get width an height info or they are not positive in " << input << '\n';
        in.close();
        return 1;
    }

    int brightness;
    in >> brightness;
    line_separator = in.get();
    if (brightness != MAX_BRIGHTNESS || line_separator != '\n') {
        std::cerr << "Couldn't get the highest pixel brightness or it is not equals " << MAX_BRIGHTNESS << " in " << input << '\n';
        in.close();
        return 1;
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
                    return 1;
                }
            }
        }
    }
    if (!(in.get() == -1) || !in.eof()) {
        std::cerr << "Unexpected data size: redundant bytes in " << input << '\n';
        in.close();
        clear_array(data, height, width);
        return 1;
    }

    picture pic(height, width, brightness, colors, data);

    std::cout << "OK\n";

    std::ofstream out(output);
    if (!out.is_open()) {
        std::cerr << "Couldn't open " << output << '\n';
        return 1;
    }
    clear_array(data, height, width);

    return 0;
}