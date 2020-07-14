#ifndef BUFFERED_READER_H
#define BUFFERED_READER_H

#include <string>
#include <fstream>
#include "picture.h"

typedef unsigned char uchar;

int const BUF_LEN = 1 << 8;
int const MONOCHROME_BYTES = 1;
int const RGB_BYTES = 3;

struct reader {
    reader(char *file);
    reader(std::string const& file);
    ~reader();

    char get_next();
    void get_next(int *pixel, size_t const pos, int const type);
    picture get_pnm_data();

 private:
    reader();

    std::ifstream in;
    uchar *buffer;
    int curr_pos;
    int end_pos;
    int read;
};

#endif