#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <math.h>
#include "dithering.h"

int main(int argc, char *argv[]) {
    if (argc != 7) {
        std::cerr << "Got " << argc << " arguments\n";
        std::cerr << "Usage: <input file> <output file> <gradient> <dithering> <bitness> <gamma>\n";
        return EXIT_FAILURE;
    }

    // Guaranteed argv[3] == "0" or "1"
    int const gradient = (strcmp(argv[3], "0") == 0 ? 0 : 1);

    int dithering;
    try {
        dithering = std::stoi(argv[4]);
    } catch (std::invalid_argument const& e) {
        std::cerr << "<dithering> must be an int value\n";
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    } catch (std::out_of_range const& e) {
        std::cerr << "Got too big value in <dithering> to parse as int\n";
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    if (dithering < 0 || dithering > 7) {
        std::cerr << "<dithering> must be an int value in [0..7]\n";
        return EXIT_FAILURE;
    }
    // Guaranteed argv[5] in [0..8]
    int bitness = std::stoi(argv[5]);
    // Guaranteed argv[6] is positive number
    double gamma = std::stod(argv[6]);
    bool sRGB = (strcmp(argv[6], "0") == 0);

    std::string input = argv[1];
    if (input.length() < 4 || input.substr(input.length() - 4, 4) != ".pgm") {
        std::cerr << "Given input file is not pgm\n";
        return EXIT_FAILURE;
    }

    std::string output = argv[2];
    if (output.length() < 4 || output.substr(output.length() - 4, 4) != ".pgm") {
        std::cerr << "Given output file is not pgm\n";
        return EXIT_FAILURE;
    }

    std::ifstream in(argv[1], std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "Couldn't open the file " << argv[1] << '\n';
        return EXIT_FAILURE;
    }

    char p;
    int type;
    in >> p >> type;
    if (p != 'P' || type != 5) {
        std::cerr << "The file " << input << " must have P5 format or couldn't read first line info\n";
        in.close();
        return EXIT_FAILURE;
    }

    int width, height;
    in >> width >> height;
    // Guaranteed height, width are ints and greater than 0

    int brightness;
    in >> brightness;
    char line_separaor = in.get();
    if (brightness != MAX_BRIGHTNESS || line_separaor != '\n') {
        std::cerr << "At the third line should be 255 or couldn't get that info in the file " << input << '\n';
        in.close();
        return EXIT_FAILURE;
    }

    picture pic(height, width, brightness, gradient, in);
    switch (dithering) {
        case 0:
            without_dithering(pic, bitness, sRGB, gamma);
            break;

        case 1:
            ordered_dithering(pic, bitness, sRGB, gamma);
            break;

        case 2:
            random_dithering(pic, bitness, sRGB, gamma);
            break;

        case 3:
            Floyd_Steinberg_dithering(pic, bitness, sRGB, gamma);
            break;

        case 4:
            Jarvis_Judice_Ninke_dithering(pic, bitness, sRGB, gamma);
            break;

        case 5:
            Sierra_3_dithering(pic, bitness, sRGB, gamma);
            break;

        case 6:
            Atkinson_dithering(pic, bitness, sRGB, gamma);
            break;

        case 7:
            Halftone_dithering(pic, bitness, sRGB, gamma);
            break;
    }

    std::ofstream out(output, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Couldn't open the output file " << output << '\n';
        // picture destructor successfully calls clear_array()
        return EXIT_FAILURE;
    }

    out << "P5\n";
    out << width << ' ' << height << '\n';
    out << brightness << '\n';
    double **data = pic.get_all();
    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            if (!out || out.bad()) {
                std::cerr << "Something happened with an output stream while writing in it data\n";
                // picture destructor successfully calls clear_array()
                return EXIT_FAILURE;
            }
            out << static_cast<unsigned char>(floor(data[h][w]));
        }
    }
    out.close();

    return EXIT_SUCCESS;
}