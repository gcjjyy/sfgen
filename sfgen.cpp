#include <iostream>
#include <vector>
#include <png.h>
#include <stdarg.h>
#include <memory.h>
#include <arpa/inet.h>

#define JOONG_INDEX (160)
#define JONG_INDEX (160 + 88)

int x, y;

int width, height;
png_byte color_type;
png_byte bit_depth;

png_structp png_ptr;
png_infop info_ptr;
int number_of_passes;
png_bytep *row_pointers;

uint8_t font_eng[4096];
uint8_t font_kor[11520];
uint8_t *ksc5601;

uint32_t font_color = 0x000000ff;
uint32_t background_color = 0x00000000;

std::vector<uint16_t> kor;
std::vector<uint8_t> eng;

const char g_choseongType[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 3, 1, 2, 4, 4, 4, 2, 1, 3, 0};
const char g_choseongTypeJongseongExist[] = {0, 5, 5, 5, 5, 5, 5, 5, 5, 6, 7, 7, 7, 6, 6, 7, 7, 7, 6, 6, 7, 5};
const char g_jongseongType[] = {0, 0, 2, 0, 2, 1, 2, 1, 2, 3, 0, 2, 1, 3, 3, 1, 2, 1, 3, 3, 1, 1};
unsigned char g_jamoTable[] = {1, 2, 0, 3, 0, 0,  4,  5,  6,  0,  0,  0,  0,  0,  0,
							   0, 7, 8, 9, 0, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};

void abort_(const char *s, ...)
{
	va_list args;
	va_start(args, s);
	vfprintf(stderr, s, args);
	fprintf(stderr, "\n");
	va_end(args);
	abort();
}

void set_pixel(int x, int y, uint32_t color)
{
	row_pointers[y][x * 4 + 0] = (png_byte)((color & 0xff000000) >> 24);
	row_pointers[y][x * 4 + 1] = (png_byte)((color & 0x00ff0000) >> 16);
	row_pointers[y][x * 4 + 2] = (png_byte)((color & 0x0000ff00) >> 8);
	row_pointers[y][x * 4 + 3] = (png_byte)((color & 0x000000ff) >> 0);
}

void load_ksc5601(char *file_name)
{
	uint8_t hibyte = 0;
	uint8_t lobyte = 0;

	FILE *fp = fopen(file_name, "r");
	fseek(fp, 0L, SEEK_END);
	long fileSize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	ksc5601 = new uint8_t[fileSize];
	fread(ksc5601, sizeof(uint8_t), fileSize, fp);
	fclose(fp);

	for (int i = 0; i < fileSize;) {
		// ENG
		if (ksc5601[i] >= 0x20 && ksc5601[i] < 0x80) {
			eng.push_back((uint8_t)(ksc5601[i] & 0x7f));
			i++;
		}
		// KOR
		else if (((ksc5601[i] & 0xe0) == 0xe0) && ((ksc5601[i + 1] & 0x80) == 0x80) &&
				 ((ksc5601[i + 2] & 0x80) == 0x80)) {
			hibyte = 0;
			lobyte = 0;
			hibyte |= (ksc5601[i] & 0x0f) << 4;
			hibyte |= (ksc5601[i + 1] & 0x3c) >> 2;
			lobyte |= (ksc5601[i + 1] & 0x03) << 6;
			lobyte |= (ksc5601[i + 2] & 0x3f);

			kor.push_back((uint16_t)(hibyte << 8) + lobyte);

			i += 3;
		} else {
			i++;
		}
	}

	delete[] ksc5601;
}

void load_font_eng(char *file_name)
{
	FILE *fp = fopen(file_name, "rb");
	fread(font_eng, sizeof(uint8_t), 4096, fp);
	fclose(fp);
}

void load_font_kor(char *file_name)
{
	FILE *fp = fopen(file_name, "rb");
	fread(font_kor, sizeof(uint8_t), 11520, fp);
	fclose(fp);
}

void put_glyph_eng(int x, int y, uint8_t glyph[16])
{
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 8; j++) {
			if (glyph[i] & (0x80 >> j)) {
				set_pixel(x + j, y + i, font_color);
			}
		}
	}
}

void put_glyph_kor(int x, int y, uint8_t glyph[32])
{
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 8; j++) {
			if (glyph[i << 1] & (0x80 >> j)) {
				set_pixel(x + j, y + i, font_color);
			}
		}
		for (int j = 0; j < 8; j++) {
			if (glyph[(i << 1) + 1] & (0x80 >> j)) {
				set_pixel(x + 8 + j, y + i, font_color);
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

	png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
				 PNG_FILTER_TYPE_BASE);

	row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
	for (y = 0; y < height; y++) {
		size_t size = png_get_rowbytes(png_ptr, info_ptr);
		row_pointers[y] = (png_byte *)malloc(size);

		for (int i = 0; i < size / sizeof(uint32_t); i++) {
			((uint32_t *)row_pointers[y])[i] = ntohl(background_color);
		}
	}

	png_write_info(png_ptr, info_ptr);

	/* write bytes */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[write_png_file] Error during writing bytes");

	x = 0;
	y = 0;

	FILE *spacing = fopen("Spacing.txt", "w");

	printf("Total English Character Size: %lu\n", eng.size());
	fprintf(spacing, "[[8, \"");
	for (size_t i = 0; i < eng.size(); i++) {
		put_glyph_eng(x, y, &font_eng[eng[i] * 16]);

		x += 16;
		if (x >= 1024) {
			x = 0;
			y += 16;
		}

		if (eng[i] == '\\' || eng[i] == '\"') {
			fprintf(spacing, "\\%c", eng[i]);
		} else {
			fprintf(spacing, "%c", eng[i]);
		}
	}
	fprintf(spacing, "\"]]");
	fclose(spacing);

	printf("Total Korean Character Size: %lu\n", kor.size());
	for (size_t i = 0; i < kor.size(); i++) {
		if (kor[i] >= 0x3130 && kor[i] <= 0x318f) {
			uint16_t code = kor[i] - 0x3130 - 1;
			put_glyph_kor(x, y, &font_kor[(g_jamoTable[code] + (20 * 3)) * 32]);
		} else if (kor[i] >= 0xac00 && kor[i] <= 0xd7af) {
			uint16_t index = kor[i] - 0xac00;

			uint16_t choseong = ((index / 28) / 21) + 1;
			uint16_t joongseong = ((index / 28) % 21) + 1;
			uint16_t jongseong = index % 28;

			int32_t cho_type = (jongseong) ? g_choseongTypeJongseongExist[joongseong] : g_choseongType[joongseong];
			int32_t joong_type = ((choseong == 1 || choseong == 16) ? 0 : 1) + (jongseong ? 2 : 0);
			int32_t jong_type = g_jongseongType[joongseong];

			put_glyph_kor(x, y, &font_kor[(cho_type * 20 + choseong) * 32]);
			put_glyph_kor(x, y, &font_kor[(JOONG_INDEX + (joong_type * 22 + joongseong)) * 32]);

			if (jongseong) {
				put_glyph_kor(x, y, &font_kor[(JONG_INDEX + (jong_type * 28 + jongseong)) * 32]);
			}
		}

		x += 16;
		if (x >= 1024) {
			x = 0;
			y += 16;
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

void set_font_color(uint32_t color)
{
	font_color = color;
}

void set_background_color(uint32_t color)
{
	background_color = color;
}

int main(int argc, char *argv[])
{
	if (argc < 3) {
		printf("Usage: %s eng_font_file kor_font_file [font_color] [background_color]\n", argv[0]);
		printf("   ex) %s font.eng font.kor ffffffff 00000000\n");
		return 0;
	}

	load_ksc5601("CHARLIST.txt");
	load_font_eng(argv[1]);
	load_font_kor(argv[2]);

	if (argc >= 4) {
		set_font_color((uint32_t)strtol(argv[3], NULL, 16));
		if (argc >= 5) {
			set_background_color((uint32_t)strtol(argv[4], NULL, 16));
		}
	}

	write_png_file("result.png");

	return 0;
}
