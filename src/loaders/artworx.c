//
//  artworx.c
//  AnsiLove/C
//
//  Copyright (C) 2011-2016 Stefan Vogt, Brian Cassidy, Frederic Cambus.
//  All rights reserved.
//
//  This source code is licensed under the BSD 3-Clause License.
//  See the file LICENSE for details.
//

#include "artworx.h"

void artworx(char *input, char *output, char *retinaout, bool createRetinaRep)
{
    const unsigned char *font_data;
    unsigned char *font_data_adf;

    // load input file
    FILE *input_file = fopen(input, "r");
    if (input_file == NULL) {
        fputs("\nFile error.\n\n", stderr); exit (1);
    }

    // get the file size (bytes)
    size_t get_file_size = filesize(input);
    int32_t input_file_size = (int32_t)get_file_size;

    // next up is loading our file into a dynamically allocated memory buffer
    unsigned char *input_file_buffer;
    int32_t result;

    // allocate memory to contain the whole file
    input_file_buffer = (unsigned char *) malloc(sizeof(unsigned char)*input_file_size);
    if (input_file_buffer == NULL) {
        fputs ("\nMemory error.\n\n", stderr); exit (2);
    }

    // copy the file into the buffer
    result = fread(input_file_buffer, 1, input_file_size, input_file);
    if (result != input_file_size) {
        fputs ("\nReading error.\n\n", stderr); exit (3);
    } // whole file is now loaded into input_file_buffer

    // close input file, we don't need it anymore
    fclose(input_file);

    // libgd image pointers
    gdImagePtr im_ADF;

    // create ADF instance
    im_ADF = gdImageCreate(640,(((input_file_size - 192 - 4096 -1) / 2) / 80) * 16);

    // error output
    if (!im_ADF) {
        fputs ("\nCan't allocate buffer image memory.\n\n", stderr); exit (7);
    }

    // ADF color palette array
    int32_t adf_colors[16] = { 0, 1, 2, 3, 4, 5, 20, 7, 56, 57, 58, 59, 60, 61, 62, 63 };

    int32_t loop;
    int32_t index;

    // process ADF font
    font_data_adf = (unsigned char *) malloc(sizeof(unsigned char)*4096);
    if (font_data_adf == NULL) {
        fputs ("\nMemory error.\n\n", stderr); exit (7);
    }
    memcpy(font_data_adf,input_file_buffer+193,4096);

    font_data=font_data_adf;

    // process ADF palette
    for (loop = 0; loop < 16; loop++)
    {
        index = (adf_colors[loop] * 3) + 1;
        gdImageColorAllocate(im_ADF, (input_file_buffer[index] << 2 | input_file_buffer[index] >> 4),
                                            (input_file_buffer[index + 1] << 2 | input_file_buffer[index + 1] >> 4),
                                            (input_file_buffer[index + 2] << 2 | input_file_buffer[index + 2] >> 4));
    }

    gdImageColorAllocate(im_ADF, 0, 0, 0);

    // process ADF
    int32_t position_x = 0, position_y = 0;
    int32_t character, attribute, color_foreground, color_background;
    loop = 192 + 4096 + 1;

    while(loop < input_file_size)
    {
        if (position_x == 80)
        {
            position_x = 0;
            position_y++;
        }

        character = input_file_buffer[loop];
        attribute = input_file_buffer[loop+1];

        color_background = (attribute & 240) >> 4;
        color_foreground = attribute & 15;

        alDrawChar(im_ADF, font_data, 8, 16, position_x, position_y, color_background, color_foreground, character);

        position_x++;
        loop+=2;
    }

    // create output file
    FILE *file_Out = fopen(output, "wb");
    gdImagePng(im_ADF, file_Out);
    fclose(file_Out);

    // in case Retina image output is wanted
    if (createRetinaRep)
    {
        gdImagePtr im_RetinaANSi;

        // make the Retina image @2x as large as im_ADF
        im_RetinaANSi = gdImageCreate(im_ADF->sx * 2, im_ADF->sy * 2);

        gdImageCopyResized(im_RetinaANSi, im_ADF, 0, 0, 0, 0,
                           im_RetinaANSi->sx, im_RetinaANSi->sy,
                           im_ADF->sx, im_ADF->sy);

        // create retina output image
        FILE *file_RetinaOut = fopen(retinaout, "wb");
        gdImagePng(im_RetinaANSi, file_RetinaOut);
        fclose(file_RetinaOut);

        gdImageDestroy(im_RetinaANSi);
    }

    // nuke garbage
    gdImageDestroy(im_ADF);
}
