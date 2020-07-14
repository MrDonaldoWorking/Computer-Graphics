#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include "buffered_reader.h"

int const MAX_BRIGHTNESS = 255;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Got " << argc << " arguments\n";
        std::cerr << "Usage: <input file>\n";
        return EXIT_FAILURE;
    }

    std::ifstream in(argv[1], std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "Couldn't open " << argv[1] << '\n';
        return EXIT_FAILURE;
    }

    char p;
    int type, width, height, max_brightness;
    in >> p >> type >> width >> height >> max_brightness;
    if (width < height) {
        std::cerr << "Not supported yet\n";
        return EXIT_FAILURE;
    }
    if (width == height) {
        std::cerr << "This picture is already square\n";
        return EXIT_FAILURE;
    }
    std::cout << p << type << '\n' << width << ' ' << height << '\n' << max_brightness << std::endl;
    in.get();

    int const colors = (type == 5 ? 1 : (type == 6 ? 3 : 0));
    size_t picture_size = width * height * colors;
    int *data = new int[picture_size];
    for (size_t i = 0; i < picture_size; ++i) {
        data[i] = in.get();
    }
    std::cout << "Successfully read a data" << std::endl;
    in.close();

    std::ofstream fout("p3_new.ppm");
    fout << p << type - 3 << '\n' << width << ' ' << height << '\n' << max_brightness << '\n';
    // for (size_t i = 0; i < picture_size; ++i) {
    //     fout << data[i] << ' ';
    //     if (i % width == width - 1) {
    //         fout << '\n';
    //     }
    // }
    for (size_t i = 0; i < picture_size; i += colors) {
        for (size_t j = i; j < picture_size && j < i + colors; ++j) {
            fout << data[j] << '\t';
        }
        fout << '\n';
    }
    fout.close();

    size_t const cnt_size = (MAX_BRIGHTNESS + 1) * colors;
    std::cout << "cnt_size = " << cnt_size << std::endl;
    int *cnt = new int[cnt_size];
    std::cout << "Successfully allocated memory for cnt" << std::endl;
    for (size_t i = 0; i < cnt_size; ++i) {
        cnt[i] = 0;
    }
    std::cout << "Successfully initialized cnt" << std::endl;
    for (size_t i = 0; i < static_cast<size_t>(width) * colors; i += colors) {
        for (size_t j = i; j < static_cast<size_t>(width) * colors && j < i + colors; ++j) {
            ++cnt[(j - i) * (MAX_BRIGHTNESS + 1) + data[j]];
        }
    }
    std::cout << "Counted maximum in the top" << std::endl;
    for (size_t i = picture_size - width * colors; i < picture_size; i += colors) {
        for (size_t j = i; j < picture_size && j < i + colors; ++j) {
            ++cnt[(j - i) * (MAX_BRIGHTNESS + 1) + data[j]];
        }
    }
    std::cout << "Counted maximum in the bottom" << std::endl;

    int max_cnt[colors];
    for (int &i : max_cnt) {
        i = 0;
    }
    int max_color[colors];
    for (int &i : max_color) {
        i = 0;
    }
    for (int br = 0; br <= MAX_BRIGHTNESS; ++br) {
        for (int color = 0; color < colors; ++color) {
            if (max_cnt[color] < cnt[color * (MAX_BRIGHTNESS + 1) + br]) {
                max_cnt[color] = cnt[color * (MAX_BRIGHTNESS + 1) + br];
                max_color[color] = br;
            }
        }
    }
    delete[] cnt;
    std::cout << "Calculated the most encountered color: " << std::endl;
    for (int &i : max_color) {
        std::cout << i << ' ';
    }
    std::cout << std::endl;

    size_t const to_add = width * (static_cast<size_t>(width - height) / 2) * colors;
    std::cout << "to_add = " << to_add << std::endl;
    int *new_data = new int[width * width * colors];
    size_t min_j = 1e9 + 7, max_j = 0;
    for (size_t i = 0; i < to_add; i += colors) {
        for (size_t j = i; j < i + colors && j < to_add; ++j) {
            min_j = std::min(min_j, j);
            max_j = std::max(max_j, j);
            new_data[j] = max_color[j - i];
        }
    }
    std::cout << "min = " << min_j << ", max = " << max_j << std::endl;
    std::cout << "Filled upper frame of new picture" << std::endl;
    min_j = 1e9 + 7, max_j = 0;
    for (size_t i = to_add; i < to_add + picture_size; i += colors) {
        for (size_t j = i; j < i + colors && j < to_add + picture_size; ++j) {
            min_j = std::min(min_j, j);
            max_j = std::max(max_j, j);
            new_data[j] = data[j - to_add];
        }
    }
    std::cout << "min = " << min_j << ", max = " << max_j << std::endl;
    std::cout << "Moved picture data into new data" << std::endl;
    delete[] data;
    min_j = 1e9 + 7, max_j = 0;
    for (size_t i = to_add + picture_size; i < static_cast<size_t>(width) * width * colors; i += colors) {
        for (size_t j = i; j < i + colors && j < static_cast<size_t>(width) * width * colors; ++j) {
            min_j = std::min(min_j, j);
            max_j = std::max(max_j, j);
            new_data[j] = max_color[j - i];
        }
    }
    std::cout << "min = " << min_j << ", max = " << max_j << std::endl;
    std::cout << "Filled lower frame of new picture" << std::endl;
    std::cout << "Created new picture data" << std::endl;

    std::string const input(argv[1]);
    std::string const output = input.substr(0, input.length() - 4) + "_squared" + input.substr(input.length() - 4, 4);
    std::cout << "Writing into new file " << output << std::endl;
    std::ofstream out(output, std::ios::binary);
    out << p << type << '\n' << width << ' ' << width << '\n' << max_brightness << '\n';
    for (size_t i = 0; i < static_cast<size_t>(width) * width * colors; ++i) {
        out << static_cast<unsigned char>(new_data[i]);
    }
    delete[] new_data;
    std::cout << "Successfully wrote all data into " << output << std::endl;

    return 0;
}