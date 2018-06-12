#include <iostream>
#include <png.h>

int x, y;

int width, height;
png_byte color_type;
png_byte bit_depth;

png_structp png_ptr;
png_infop info_ptr;
int number_of_passes;
png_bytep *row_pointers;

void abort_(const char *s, ...)
{
    va_list args;
    va_start(args, s);
    vfprintf(stderr, s, args);
    fprintf(stderr, "\n");
    va_end(args);
    abort();
}

void set_pixel(int x, int y, uint32_t color) {
    row_pointers[y][x*4+0] = (png_byte)((color & 0xff000000) >> 24);
    row_pointers[y][x*4+1] = (png_byte)((color & 0x00ff0000) >> 16);
    row_pointers[y][x*4+2] = (png_byte)((color & 0x0000ff00) >> 8);
    row_pointers[y][x*4+3] = (png_byte)((color & 0x000000ff) >> 0);
}

uint8_t glyphs[11520];

void load_font_16(char *file_name) {
    FILE *fp = fopen(file_name, "rb");
    fread(glyphs, sizeof(uint8_t), 11520, fp);
    fclose(fp);
}

void put_glyph_32(int x, int y, uint8_t glyph[32]) {
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            if (glyph[i << 1] & (0x80 >> j)) {
                set_pixel(x + j, y + i, 0x000000ff);
            }
        }
        for (int j = 0; j < 8; j++) {
            if (glyph[(i << 1) + 1] & (0x80 >> j)) {
                set_pixel(x + 8 + j, y + i, 0x000000ff);
            }
        }
    }
}

void write_png_file(char *file_name)
{
    width = 1024;
    height = 1024;
    color_type = PNG_COLOR_TYPE_RGBA;
    bit_depth = 8;

    /* create file */
    FILE *fp = fopen(file_name, "wb");
    if (!fp)
        abort_("[write_png_file] File %s could not be opened for writing", file_name);

    /* initialize stuff */
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
        abort_("[write_png_file] png_create_write_struct failed");

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        abort_("[write_png_file] png_create_info_struct failed");

    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[write_png_file] Error during init_io");

    png_init_io(png_ptr, fp);

    /* write header */
    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[write_png_file] Error during writing header");

    png_set_IHDR(png_ptr, info_ptr, width, height,
                 bit_depth, color_type, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
    for (y=0; y<height; y++) {
        row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));
        memset(row_pointers[y], 0, png_get_rowbytes(png_ptr,info_ptr));
    }

    png_write_info(png_ptr, info_ptr);

    /* write bytes */
    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[write_png_file] Error during writing bytes");

    load_font_16("dosfonts/IYG.HAN");
    for (int i = 0; i < 32; i++) {
        for(int j = 0; j < 32; j++) {
            int glyphIndex = i * 32 + j;
            if (glyphIndex >= 360) break;
            put_glyph_32(j * 16, i * 16, &glyphs[glyphIndex * 32]);
        }
    }

    png_write_image(png_ptr, row_pointers);

    /* end write */
    if (setjmp(png_jmpbuf(png_ptr)))
        abort_("[write_png_file] Error during end of write");

    png_write_end(png_ptr, NULL);

    /* cleanup heap allocation */
    for (y = 0; y < height; y++)
        free(row_pointers[y]);
    free(row_pointers);

    fclose(fp);
}

int main()
{
    write_png_file("gogo.png");
    return 0;
}