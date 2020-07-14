#include <stdexcept>
#include "buffered_reader.h"

reader::reader() : curr_pos(0), end_pos(0), read(0) {
    buffer = new uchar[BUF_LEN];
}

reader::reader(char *file) {
    reader();

    in.open(file);
    if (!in.is_open()) {
        throw std::runtime_error("Couldn't open " + std::string(file));
    }
}

reader::reader(std::string const& file) {
    reader();

    in.open(file);
    if (!in.is_open()) {
        throw std::runtime_error("Couldn't open" + file);
    }
}

reader::~reader() {
    delete[] buffer;
}

char reader::get_next() {
    if (curr_pos == end_pos) {
        in.read(reinterpret_cast<char *>(buffer), BUF_LEN);
        end_pos = in.gcount();

        if (end_pos == 0) {
            throw std::runtime_error("An input error occurred while reading file");
        }

        curr_pos = 0;
    }
    ++read;

    return buffer[curr_pos++];
}

void reader::get_next(int *pixel, size_t const pos, int const type) {
    switch (type) {
        case 5:
            pixel[pos] = get_next();
            break;

        case 6:
            for (int i = 0; i < RGB_BYTES; ++i) {
                pixel[pos + i] = get_next();
            }
            break;

        default:
            throw std::runtime_error("P" + std::to_string(type) + " is not supported");
    }
}

int get_colors(int type) {
    switch (type) {
        case 5:
            return MONOCHROME_BYTES;
        
        case 6:
            return RGB_BYTES;
        
        default:
            throw std::runtime_error("P" + std::to_string(type) + " is not supported");
    }
}

picture reader::get_pnm_data() {
    char p;
    int type, width, height, max_brightness;
    in >> p >> type >> width >> height >> max_brightness;
    get_next();

    int const colors = get_colors(type);
    size_t const picture_size = width * height * colors;
    int *data = new int[width * height * colors];
    for (size_t i = 0; i < picture_size; i += colors) {
        get_next(data, i, type);
    }

    return picture(type, width, height, max_brightness, colors, data);
}