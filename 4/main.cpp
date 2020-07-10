#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include "colour_space.h"

COLOUR_SPACE check_and_get_colour_space(std::string const& s, std::map<std::string, COLOUR_SPACE> &spaces) {
    if (spaces[s] == ERROR) {
        throw std::runtime_error("Colour space " + s + " is not supported\n");
    }
    return spaces[s];
}

int main(int argc, char *argv[]) {
    std::map<std::string, COLOUR_SPACE> colour_spaces = {
        {"RGB", RGB},
        {"HSL", HSL},
        {"HSV", HSV},
        {"YCbCr.601", YCbCr601},
        {"YCbCr.709", YCbCr709},
        {"YCoCg", YCoCg},
        {"CMY", CMY}
    };

    if (argc != 11) {
        std::cerr << "Got " << argc << " arguments\n";
        std::cerr << "Usage: -f <from_color_space> -t <to_color_space> -i <count>"
                     " <input_file_name> -o <count> <output_file_name>";
        return EXIT_FAILURE;
    }

    COLOUR_SPACE colour_space_from = ERROR, colour_space_to = ERROR;
    std::string input_file_name, output_file_name;
    int inputs = 0, outputs = 0;
    try {
        for (int arg = 1; arg < argc; ++arg) {
            if (argv[arg][0] == '-' && strlen(argv[arg]) == 2) {
                switch (argv[arg][1]) {
                    case 'f':
                        colour_space_from = check_and_get_colour_space(argv[arg + 1], colour_spaces);
                        break;

                    case 't':
                        colour_space_to = check_and_get_colour_space(argv[arg + 1], colour_spaces);
                        break;
                    
                    case 'i':
                        inputs = std::stoi(argv[arg + 1]);
                        input_file_name = std::string(argv[arg + 2]);
                        break;

                    case 'o':
                        outputs = std::stoi(argv[arg + 1]);
                        output_file_name = std::string(argv[arg + 2]);
                        break;

                    default:
                        std::cerr << argv[arg] << " is not supported\n";
                        return EXIT_FAILURE;
                }
            }
        }
    } catch (std::runtime_error const& e) {
        std::cerr << e.what();
        return EXIT_FAILURE;
    }

    if (colour_space_from == ERROR) {
        std::cerr << "Input file's colour space info not found\n";
        return EXIT_FAILURE;
    }
    if (colour_space_to == ERROR) {
        std::cerr << "Output file's colour space info not found\n";
        return EXIT_FAILURE;
    }
    if (inputs == 0 || input_file_name.empty()) {
        std::cerr << "Input file or files not found\n";
        return EXIT_FAILURE;
    }
    if (outputs == 0 || output_file_name.empty()) {
        std::cerr << "Output file or files not found\n";
    }

    try {
        picture pic(inputs, input_file_name, colour_space_from);
        pic.to_RGB();
        pic.RGB_to(colour_space_to);
        pic.write(outputs, output_file_name);
    } catch (std::runtime_error const& e) {
        std::cerr << e.what();
        return EXIT_FAILURE;
    }
    // all arguments are correct according to guarantee

    return EXIT_SUCCESS;
}