/*
    NOAA-APT - Lightweight APT decoder for NOAA weather satellites
    Copyright (C) 2024 Patrick O'Brien-Seitz

    NOAA-APT is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    NOAA-APT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <sndfile.h>
#include <stdlib.h>
#include <fftw3.h>
#include <math.h>
#include "algebra.h"
#include "utils.h"
#include "image.h"

/*
Create full image from demodulated normalized APT audio
Can only handle 11025Hz audio files. This will be changed in a future update
*/
int create_image(int width)
{
    SF_INFO sfinfo_input;
    SNDFILE *sndfile_input;
    sfinfo_input.format = 0;

    const char *file_path = "./documentation/samples/audio/20210720111842.wav";
    // Opening input audio file.
    sndfile_input = sf_open(file_path, SFM_READ, &sfinfo_input);
    if (!sndfile_input)
    {
        printf("Failed to open file: %s\n", sf_strerror(NULL));
        return -1;
    }

    sf_count_t frames = sfinfo_input.frames;
    // Calculates the height of images based on total frames and width
    int height = ceil((double)(frames / 5512));
    sf_count_t count = 0;
    sf_count_t buffer_length = 11025;

    FILE *image;
    image = fopen("output/images/apt_image.bmp", "w+");

    // Build file header
    BitMapFileHeader header = {
        .signature = 0x424D,
        .file_size = sizeof(BitMapFileHeader) + sizeof(BitMapInfoHeader) + (256 * sizeof(BitMapColorTable)) + (width * height),
        .reserved = 0,
        .data_offset = sizeof(BitMapFileHeader) + sizeof(BitMapInfoHeader) + (256 * sizeof(BitMapColorTable)),
    };
    BitMapFileHeader *header_ptr = &header;

    // Write file header to image
    write_file_header(image, header_ptr);

    // Build info header
    BitMapInfoHeader InfoHeader = {
        .size = sizeof(BitMapInfoHeader),
        .width = width,
        .height = height,
        .planes = 1,
        .bits_per_pixel = 8,
        .compression = 0,
        .image_size = width * height,
        .x_pixels_per_m = 0,
        .y_pixels_per_m = 0,
        .colors_used = 256,
        .important_colors = 256,
    };
    BitMapInfoHeader *InfoHeader_ptr = &InfoHeader;

    // Write info header to image
    write_info_header(image, InfoHeader_ptr);

    // Write color table to image
    write_color_table(image);

    // Get lines of demodulated APT data and write to image
    while (count < frames)
    {
        printf("count: %d\n", count);
        sf_count_t start_frame = sf_seek(sndfile_input, count, SEEK_SET);
        double *input_buffer = (double *)fftw_malloc(sizeof(double) * 11025);
        sf_count_t frames_requested = sf_readf_double(sndfile_input, input_buffer, 11025);
        printf("Frames read: %ld\n", frames_requested);

        // Getting the demodulated buffer
        double *intermediate_buffer = am_demodulate(input_buffer, 11025);

        // Write pixel data to image
        for (int i = 0; i < 11024; i++)
        {
            uint32_t pixel_data = intermediate_buffer[i] * 255;
            fputc(pixel_data, image);
        }

        count += frames_requested;
        free(input_buffer);
        free(intermediate_buffer);
    }

    printf("=================\n");
    printf("Finished!\n");
    sf_close(sndfile_input);
    fclose(image);
    return 0;
}

/*
Test, takes 1 second of audio and tries to output an image
use bit masking and bit shifting to get the correct byte.
BMP is using little endian
*/
void create_test_image(double *buffer, int width, int height)
{
    FILE *image;
    image = fopen("documentation/images/test_11025_image.bmp", "w+");

    // Build file header
    BitMapFileHeader header = {
        .signature = 0x424D,
        .file_size = sizeof(BitMapFileHeader) + sizeof(BitMapInfoHeader) + (256 * sizeof(BitMapColorTable)) + (width * height),
        .reserved = 0,
        .data_offset = sizeof(BitMapFileHeader) + sizeof(BitMapInfoHeader) + (256 * sizeof(BitMapColorTable)),
    };
    BitMapFileHeader *header_ptr = &header;

    // Write file header to image
    write_file_header(image, header_ptr);

    // Build info header
    BitMapInfoHeader InfoHeader = {
        .size = sizeof(BitMapInfoHeader),
        .width = width,
        .height = height,
        .planes = 1,
        .bits_per_pixel = 8,
        .compression = 0,
        .image_size = width * height,
        .x_pixels_per_m = 0,
        .y_pixels_per_m = 0,
        .colors_used = 256,
        .important_colors = 256,
    };
    BitMapInfoHeader *InfoHeader_ptr = &InfoHeader;

    // Write info header to image
    write_info_header(image, InfoHeader_ptr);

    // Write color table to image
    write_color_table(image);

    // Write pixel data to image
    for (int i = 0; i < 11024; i++)
    {
        uint32_t pixel_value = buffer[i] * 255;
        printf("%d: %d\n", i, pixel_value);
        fputc(pixel_value, image);
    }

    fclose(image);
}

/* Test to see if I can create a sample bmp image*/
void create_color_test_image()
{
    char header[54] = {
        0x42,
        0x4D,
        0xE6,
        0x71,
        0x0B,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x36,
        0x00,
        0x00,
        0x00,
        0x28,
        0x00,
        0x00,
        0x00,
        0xF4,
        0x01,
        0x00,
        0x00,
        0xF4,
        0x01,
        0x00,
        0x00,
        0x01,
        0x00,
        0x18,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0xB0,
        0x71,
        0x0B,
        0x00,
        0xC4,
        0x0E,
        0x00,
        0x00,
        0xC4,
        0x0E,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    };
    FILE *image;
    image = fopen("documentation/images/test_image.bmp", "w+");

    // Writing header to image
    for (int i = 0; i < 54; i++)
    {
        fputc(header[i], image);
    }

    // Writing colors to image
    for (int i = 0; i < (100 * 500); i++)
    {
        fputc(255, image);
        fputc(0, image);
        fputc(0, image);
    }
    for (int i = 0; i < (100 * 500); i++)
    {
        fputc(0, image);
        fputc(0, image);
        fputc(255, image);
    }
    for (int i = 0; i < (100 * 500); i++)
    {
        fputc(160, image);
        fputc(0, image);
        fputc(0, image);
    }
    for (int i = 0; i < (100 * 500); i++)
    {
        fputc(0, image);
        fputc(120, image);
        fputc(120, image);
    }
    for (int i = 0; i < (100 * 500); i++)
    {
        fputc(120, image);
        fputc(120, image);
        fputc(0, image);
    }
    fclose(image);
}

void write_file_header(FILE *image, BitMapFileHeader *file_header)
{
    fputc((file_header->signature & 0xFF00) >> (2 * 4), image);
    fputc((file_header->signature & 0x00FF), image);

    parse_dword(image, file_header->file_size);
    parse_dword(image, file_header->reserved);
    parse_dword(image, file_header->data_offset);
}

void write_info_header(FILE *image, BitMapInfoHeader *InfoHeader)
{
    parse_dword(image, InfoHeader->size);
    parse_dword(image, InfoHeader->width);
    parse_dword(image, InfoHeader->height);

    parse_word(image, InfoHeader->planes);
    parse_word(image, InfoHeader->bits_per_pixel);

    parse_dword(image, InfoHeader->compression);
    parse_dword(image, InfoHeader->image_size);
    parse_dword(image, InfoHeader->x_pixels_per_m);
    parse_dword(image, InfoHeader->y_pixels_per_m);
    parse_dword(image, InfoHeader->colors_used);
    parse_dword(image, InfoHeader->important_colors);
}

void write_color_table(FILE *image)
{
    for (int i = 0; i < 256; i++)
    {
        fputc(i, image);
        fputc(i, image);
        fputc(i, image);
        fputc(0, image);
    }
}

void parse_word(FILE *image, uint16_t value)
{
    fputc((value & 0x00FF), image);
    fputc((value & 0xFF00) >> (2 * 4), image);
}

void parse_dword(FILE *image, uint32_t value)
{
    fputc((value & 0x000000FF), image);
    fputc((value & 0x0000FF00) >> (2 * 4), image);
    fputc((value & 0x00FF0000) >> (4 * 4), image);
    fputc((value & 0xFF000000) >> (6 * 4), image);
}