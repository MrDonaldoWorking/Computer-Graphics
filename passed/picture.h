#ifndef PICTURE_H
#define PICTURE_H

#include "buffered_reader.h"

struct picture {
    picture(int const t, int const w, int const h, int const m, int const c, int *data);
    ~picture();

    int get_type() const;
    int get_width() const;
    int get_height() const;

    int get(int const h, int const w) const;
    void set(int const h, int const w, int const val);

 private:
    int const type;
    int const width;
    int const height;
    int const max_brightness;
    int const colors;
    int *data;
};

#endif