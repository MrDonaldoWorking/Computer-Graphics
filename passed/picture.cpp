#include "picture.h"

picture::picture(int const t, int const w, int const h, int const m, int const c, int *data)
    : type(t), width(w), height(h),  max_brightness(m), colors(c), data(data) {}

picture::~picture() {
    delete[] data;
}

int picture::get_type() const {
    return type;
}

int picture::get_width() const {
    return width;
}

int picture::get_height() const {
    return height;
}

int picture::get(int const h, int const w) const {
    return data[width * h + w];
}

void picture::set(int const h, int const w, int const val) {
    data[width * h + w] = val;
}