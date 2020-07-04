#include <fstream>
#include <string>
#include <stdio.h>
#include <vector>

// int const BUF_LEN = 5;

typedef unsigned char uchar;
int const BYTE_MAX = 1 << 8 - 1;

namespace reader {
    int const BUF_LEN = 1 << 8, RGB_BYTES = 3;
    uchar *buffer = new uchar[BUF_LEN];
    char p;
    int type;
    int height, width;
    unsigned int max_code;

    std::ifstream in;

    int curr_pos = 0, end_pos = 0, read = 0;

    char get_next() {
        if (curr_pos == end_pos) {
            in.read((char *) buffer, BUF_LEN);
            end_pos = in.gcount();

            if (end_pos == 0) {
                throw std::runtime_error("An input error occurred while reading file");
            }

            curr_pos = 0;
        }
        ++read;

        return buffer[curr_pos++];
    }
}

struct pixel {
    std::vector<uchar> info;

    pixel() {
        try {
            if (reader::type == 5) {
                info.resize(1);
            } else {
                info.resize(3);
            }
        } catch (...) {
            throw std::runtime_error("No free memory to allocate pixel");
        }
    }
};

namespace reader {
    std::vector<std::vector<pixel>> data;

    void get_next(pixel &to) {
        if (type == 5) {
            to.info[0] = get_next();
        } else {
            // for (int &i : to.info) {
            //     i = get_next();
            // }
            for (int i = 0; i < RGB_BYTES; ++i) {
                to.info[i] = get_next();
            }
        }
    }
}

namespace writer {
    int height, width;

    std::ofstream out;

    std::vector<std::vector<pixel>> data;

    void set_data(int a, int b) {
        height = a;
        width = b;
        try {
            data.resize(a, std::vector<pixel>(b));
        } catch (...) {
            throw std::runtime_error("No free memory to allocate writer::data");
        }
    }

    void write_meta() {
        try {
            out << reader::p << reader::type << '\n';
            out << width << ' ' << height << '\n';
            out << reader::max_code << '\n';
        } catch (...) {
            throw std::runtime_error("An error occurred while writing meta info");
        }
    }

    void write_data() {
        for (auto &i : data) {
            for (auto &j : i) {
                for (uchar k : j.info) {
                    try {
                        out.write((char *) &k, 1);
                    } catch (...) {
                        throw std::runtime_error("An error occurred while writing data");
                    }
                }
            }
        }
    }
}

namespace edit {
    void inverse() {
        printf("inverse\n");

        writer::set_data(reader::height, reader::width);

        for (int i = 0; i < reader::height; ++i) {
            for (int j = 0; j < reader::width; ++j) {
                for (int k = 0; k < reader::data[i][j].info.size(); ++k) {
                    writer::data[i][j].info[k] = reader::max_code - reader::data[i][j].info[k];
                }
            }
        }
    }

    namespace mirror {
        void vertical() {
            printf("vertical mirror\n");

            writer::set_data(reader::height, reader::width);

            // for (int i = 0; i < reader::height; ++i) {
            //     writer::data[i] = reader::data[reader::height - 1 - i];
            // }

            for (int i = 0; i < writer::height; ++i) {
                for (int j = 0; j < writer::width; ++j) {
                    writer::data[i][j] = reader::data[reader::height - i - 1][j];
                }
            }
        }

        void horisontal() {
            printf("horizontal mirror\n");

            writer::set_data(reader::height, reader::width);

            for (int i = 0; i < writer::height; ++i) {
                for (int j = 0; j < writer::width; ++j) {
                    writer::data[i][j] = reader::data[i][reader::width - 1 - j];
                }
            }
        }
    }

    namespace rotate {
        /* 1 2 3    4 1  1 1 <- 2 1   1 2 <- 1 1
           4 5 6 -> 5 2  2 1 <- 2 2   2 2 <- 1 2
                    6 3  3 1 <- 2 3   3 2 <- 1 3
        */
        void clockwise() {
            printf("clockwise rotate\n");

            writer::set_data(reader::width, reader::height);

            for (int i = 0; i < writer::height; ++i) {
                for (int j = 0; j < writer::width; ++j) {
                    writer::data[i][j] = reader::data[reader::height - j - 1][i];
                }
            }
        }

        /* 1 2 3    3 6  1 1 <- 1 3   1 2 <- 2 3
           4 5 6 -> 2 5  2 1 <- 1 2   2 2 <- 2 2
                    1 4  3 1 <- 1 1   3 2 <- 2 1
        */
        void counterclockwise() {
            printf("counterclockwise\n");

            writer::set_data(reader::width, reader::height);

            for (int i = 0; i < writer::height; ++i) {
                for (int j = 0; j < writer::width; ++j) {
                    writer::data[i][j] = reader::data[j][reader::width - 1 - i];
                }
            }
        }
    }
}

void run(int argc, char *argv[], bool debug) {
    if (argc != 4) {
        throw std::runtime_error("Expected 3 arguments: input file, output file, command");
    }

    reader::in.open(argv[1], std::ios::in | std::ios::binary);
    if (!reader::in.is_open()) {
        throw std::runtime_error("Input file can't be opened");
    }

    {
        using namespace reader;
        in >> p >> type;
        printf("%c %d\n", p, type);

        if (p != 'P' || (type > 6 || type < 5)) {
            throw std::runtime_error("Expected P5 or P6 in input file");
        }

        in >> width >> height >> max_code;
        get_next();

        try {
            data.resize(height, std::vector<pixel>(width));
        } catch (...) {
            throw std::runtime_error("No free memory to allocate reader::data");
        }
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                get_next(data[i][j]);
            }
        }

        if (debug) {
            std::string s = argv[1];
            std::ofstream fout(s + "decoded_from_input.pnm");
            fout << p << type - 3 << '\n' << width << ' ' << height << '\n' << max_code << '\n';

            for (int i = 0; i < height; ++i) {
                for (int j = 0; j < width; ++j) {
                    for (auto &k : data[i][j].info) {
                        fout << k << ' ';
                    }
                }
            }

            fout.close();
        }
    }

    printf("h = %d, w = %d, max = %d\n", reader::height, reader::width, reader::max_code);

    try {
    reader::in.close();
    } catch (...) {
        throw std::runtime_error("Couldn't close an input file");
    }
    
    switch (argv[3][0]) {
        case '0':
            edit::inverse();
            break;
        case '1':
            edit::mirror::horisontal();
            break;
        case '2':
            edit::mirror::vertical();
            break;
        case '3':
            edit::rotate::clockwise();
            break;
        case '4':
            edit::rotate::counterclockwise();
            break;
        default:
            throw std::runtime_error("Expected only 0, 1, 2, 3, 4 in command");
            break;
    }

    writer::out.open(argv[2], std::ios::out | std::ios::binary);
    if (!writer::out.is_open()) {
        throw std::runtime_error("An output file can't be opened");
    }

    writer::write_meta();
    writer::write_data();

    try {
        writer::out.close();
    } catch (...) {
        throw std::runtime_error("Couldn't close an output file");
    }

    if (debug) {
        using namespace writer;

        std::string s = argv[2];
        std::ofstream fout(s + "decoded_from_output.pnm");
        fout << reader::p << reader::type - 3 << '\n' << width << ' ' << height << '\n' << reader::max_code << '\n';
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                for (auto &k : data[i][j].info) {
                    fout << k << ' ';
                }
            }
        }
        fout.close();
    }
}

int main(int argc, char* argv[]) {
    try {
        run(argc, argv, false);
    } catch (std::runtime_error const& e) {
        printf("An error occured\n");
        printf(e.what());
        printf("\n");
    } catch (...) {
        printf("An unexpected error occurred\n");
    }

    delete[] reader::buffer;

    return 0;
}