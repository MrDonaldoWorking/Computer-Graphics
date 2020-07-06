#include <map>
#include <string>

int const MAX_BRIGHTNESS = 255;
int const TWO_PIES = 360;
int const ONE_THIRD_PI = 60;
double const EPS = 1e-5;
double const ONE_SECOND = 0.5;
double const K_R_601 = 0.299;
double const K_G_601 = 0.587;
double const K_B_601 = 0.114;
double const K_R_709 = 0.0722;
double const K_G_709 = 0.2126;
double const K_B_709 = 0.7152;

enum COLOUR_PALETTE {
    ERROR,
    RGB,
    HSL,
    HSV,
    YCbCr601,
    YCbCr709,
    YCoCg,
    CMY
};

struct picture {
    picture() = default;
    picture(int cnt, std::string const& name, COLOUR_PALETTE palette);
    ~picture();

    int get_width() const;
    int get_height() const;
    int get_max_brightness() const;
    int*** get_data();

    void to_RGB();
    void RGB_to(COLOUR_PALETTE);

    void write(int cnt, std::string const& name);

 private:
    int width = 0;
    int height = 0;
    int max_brightness = 255;
    int ***data = nullptr;
    COLOUR_PALETTE palette = ERROR;

    void update_int(std::string const& name, int& x, int curr_value);
    void read_meta_data(std::ifstream &in, std::string const& name, int encoding);
    void read_data_P5(std::string const& file);
    void read_data_P6(std::string const& file);

    void force_close(std::string const& message);
    void clear_array();

    void write_to_one_P6(std::string const& file);
    void write_to_three_P5(std::string const& file);
    void write_meta_data(std::ofstream &out, std::string const& name, int encoding);
};