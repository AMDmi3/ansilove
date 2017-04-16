//
//  binary.c
//  AnsiLove/C
//
//  Copyright (C) 2011-2017 Stefan Vogt, Brian Cassidy, and Frederic Cambus.
//  All rights reserved.
//
//  This source code is licensed under the BSD 2-Clause License.
//  See the file LICENSE for details.
//

#include "binary.h"

void binary(unsigned char *inputFileBuffer, int32_t inputFileSize, char *output, char *retinaout, int32_t columns, char *font, int32_t bits, bool icecolors, bool createRetinaRep)
{
    // some type declarations
    struct fontStruct fontData;

    // font selection
    alSelectFont(&fontData, font);

    // libgd image pointers
    gdImagePtr im_Binary;

    // allocate buffer image memory
    im_Binary = gdImageCreate(columns * bits,
                              ((inputFileSize / 2) / columns * fontData.height));

    if (!im_Binary) {
        perror("Error, can't allocate buffer image memory");
        exit (6);
    }

    // allocate black color
    gdImageColorAllocate(im_Binary, 0, 0, 0);

    // allocate color palette
    int32_t colors[16];

    colors[0] = gdImageColorAllocate(im_Binary, 0, 0, 0);
    colors[1] = gdImageColorAllocate(im_Binary, 0, 0, 170);
    colors[2] = gdImageColorAllocate(im_Binary, 0, 170, 0);
    colors[3] = gdImageColorAllocate(im_Binary, 0, 170, 170);
    colors[4] = gdImageColorAllocate(im_Binary, 170, 0, 0);
    colors[5] = gdImageColorAllocate(im_Binary, 170, 0, 170);
    colors[6] = gdImageColorAllocate(im_Binary, 170, 85, 0);
    colors[7] = gdImageColorAllocate(im_Binary, 170, 170, 170);
    colors[8] = gdImageColorAllocate(im_Binary, 85, 85, 85);
    colors[9] = gdImageColorAllocate(im_Binary, 85, 85, 255);
    colors[10] = gdImageColorAllocate(im_Binary, 85, 255, 85);
    colors[11] = gdImageColorAllocate(im_Binary, 85, 255, 255);
    colors[12] = gdImageColorAllocate(im_Binary, 255, 85, 85);
    colors[13] = gdImageColorAllocate(im_Binary, 255, 85, 255);
    colors[14] = gdImageColorAllocate(im_Binary, 255, 255, 85);
    colors[15] = gdImageColorAllocate(im_Binary, 255, 255, 255);

    // process binary
    int32_t character, attribute, background, foreground;
    int32_t loop = 0, position_x = 0, position_y = 0;

    while (loop < inputFileSize)
    {
        if (position_x == columns)
        {
            position_x = 0;
            position_y++;
        }

        character = inputFileBuffer[loop];
        attribute = inputFileBuffer[loop+1];

        background = (attribute & 240) >> 4;
        foreground = (attribute & 15);

        if (background > 8 && !icecolors)
        {
            background -= 8;
        }

        alDrawChar(im_Binary, fontData.font_data, bits, fontData.height,
                   position_x, position_y, colors[background], colors[foreground], character);

        position_x++;
        loop+=2;
    }

    // create output image
    FILE *file_Out = fopen(output, "wb");
    gdImagePng(im_Binary, file_Out);
    fclose(file_Out);

    // in case Retina image output is wanted
    if (createRetinaRep) {
        retina(im_Binary, retinaout);
    }

    // free memory
    gdImageDestroy(im_Binary);
}
