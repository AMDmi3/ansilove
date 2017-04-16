//
//  tundra.c
//  AnsiLove/C
//
//  Copyright (C) 2011-2017 Stefan Vogt, Brian Cassidy, and Frederic Cambus.
//  All rights reserved.
//
//  This source code is licensed under the BSD 2-Clause License.
//  See the file LICENSE for details.
//

#include "tundra.h"

void tundra(unsigned char *inputFileBuffer, int32_t inputFileSize, char *output, char *retinaout, char *font, int32_t bits, bool createRetinaRep)
{
    // some type declarations
    struct fontStruct fontData;
    int32_t columns = 80;
    char tundra_version;
    char tundra_header[8];

    // font selection
    alSelectFont(&fontData, font);

    // libgd image pointers
    gdImagePtr im_Tundra;

    // extract tundra header
    tundra_version = inputFileBuffer[0];
    memcpy(&tundra_header,inputFileBuffer+1,8);

    // need to add check for "TUNDRA24" string in the header
    if (tundra_version != 24)
    {
        fputs ("\nInput file is not a TUNDRA file.\n\n", stderr); exit (4);
    }

    // read tundra file a first time to find the image size
    int32_t character, background = 0, foreground = 0;
    int32_t loop = 0, position_x = 0, position_y = 0;

    loop=9;

    while (loop < inputFileSize)
    {
        if (position_x == 80)
        {
            position_x = 0;
            position_y++;
        }

        character = inputFileBuffer[loop];

        if (character == 1)
        {
            position_y =
                    (inputFileBuffer[loop + 1] << 24) + (inputFileBuffer[loop + 2] << 16) +
                            (inputFileBuffer[loop + 3] << 8) + inputFileBuffer[loop+4];

            position_x =
                    (inputFileBuffer[loop + 5] << 24) + (inputFileBuffer[loop + 6] << 16) +
                            (inputFileBuffer[loop + 7] << 8) + inputFileBuffer[loop+8];

            loop+=8;
        }

        if (character == 2)
        {
            character = inputFileBuffer[loop + 1];

            loop+=5;
        }

        if (character == 4)
        {
            character = inputFileBuffer[loop + 1];

            loop+=5;
        }

        if (character == 6)
        {
            character = inputFileBuffer[loop + 1];

            loop+=9;
        }

        if (character !=1 && character !=2 && character !=4 && character != 6)
        {
            position_x++;
        }

        loop++;
    }
    position_y++;

    // allocate buffer image memory
    im_Tundra = gdImageCreateTrueColor(columns * bits , (position_y) * fontData.font_size_y);

    if (!im_Tundra) {
        perror("Error, can't allocate buffer image memory");
        exit (6);
    }

    // process tundra
    position_x = 0;
    position_y = 0;

    loop = 9;

    while (loop < inputFileSize)
    {
        if (position_x == 80)
        {
            position_x = 0;
            position_y++;
        }

        character = inputFileBuffer[loop];

        if (character == 1)
        {
            position_y =
                    (inputFileBuffer[loop + 1] << 24) + (inputFileBuffer[loop + 2] << 16) +
                            (inputFileBuffer[loop + 3] << 8) + inputFileBuffer[loop + 4];

            position_x =
                    (inputFileBuffer[loop + 5] << 24) + (inputFileBuffer[loop + 6] << 16) +
                            (inputFileBuffer[loop + 7] << 8) + inputFileBuffer[loop + 8];

            loop+=8;
        }

        if (character == 2)
        {
            foreground =
                    (inputFileBuffer[loop + 3] << 16) + (inputFileBuffer[loop + 4] << 8) +
                            inputFileBuffer[loop + 5];

            character = inputFileBuffer[loop+1];

            loop+=5;
        }

        if (character == 4)
        {
            background = (inputFileBuffer[loop + 3] << 16) + (inputFileBuffer[loop + 4] << 8) +
                    inputFileBuffer[loop+5];

            character = inputFileBuffer[loop+1];

            loop+=5;
        }

        if (character==6)
        {
            foreground =
                    (inputFileBuffer[loop + 3] << 16) + (inputFileBuffer[loop + 4] << 8) +
                            inputFileBuffer[loop+5];

            background =
                    (inputFileBuffer[loop + 7] << 16) + (inputFileBuffer[loop + 8] << 8) +
                            inputFileBuffer[loop+9];

            character = inputFileBuffer[loop+1];

            loop+=9;
        }

        if (character !=1 && character !=2 && character !=4 && character !=6)
        {
            alDrawChar(im_Tundra, fontData.font_data, bits, fontData.font_size_y,
                    position_x, position_y, background, foreground, character);

            position_x++;
        }

        loop++;
    }

    // create output image
    FILE *file_Out = fopen(output, "wb");
    gdImagePng(im_Tundra, file_Out);
    fclose(file_Out);

    // in case Retina image output is wanted
    if (createRetinaRep) {
        retina(im_Tundra, retinaout);
    }

    // free memory
    gdImageDestroy(im_Tundra);
}

