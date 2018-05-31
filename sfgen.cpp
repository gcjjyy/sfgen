#include <iostream>
#include <png.h>

png_structp png_ptr;

int main() {
    std::cout << "Hello" << std::endl;
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    return 0;
}