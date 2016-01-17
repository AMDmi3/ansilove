//
//  ansi.c
//  AnsiLove/C
//
//  Copyright (C) 2011-2016 Stefan Vogt, Brian Cassidy, Frederic Cambus.
//  All rights reserved.
//
//  This source code is licensed under the BSD 3-Clause License.
//  See the file LICENSE for details.
//

#include "ansilove.h"
#include "ansi.h"

void ansi(char *input, char *output, char *retinaout, char *font, int32_t int_bits, char *mode, bool icecolors, char *fext, bool createRetinaRep)
{
    // ladies and gentlemen, it's type declaration time
    struct fontStruct fontData;

    int32_t columns = 80;
    
    bool isDizFile = false;
    bool ced = false;
    bool transparent = false;
    bool workbench = false;

    // font selection
    alSelectFont(&fontData, font);
        
    // to deal with the bits flag, we declared handy bool types   
    if (!strcmp(mode, "ced")) {
        ced = true;
    }
    else if (!strcmp(mode, "transparent")) {
        transparent = true;
    }
    else if (!strcmp(mode, "workbench")) {
        workbench = true;
    }
    else if (!strcmp(mode, "workbench-transparent")) {
        workbench = true;
        transparent = true;
    }
    
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

    // check if current file has a .diz extension
    if (!strcmp(fext, ".diz")) {
        isDizFile = true;
    }

    // libgd image pointers
    gdImagePtr im_ANSi;

    // ANSi processing loops
    int32_t loop = 0, ansi_sequence_loop, seq_graphics_loop; 
    
    // character definitions
    int32_t current_character, next_character, character; 
    unsigned char ansi_sequence_character;
    
    // default color values
    int32_t color_background = 0, color_foreground = 7; 
    
    // text attributes
    bool bold = false, underline = false, italics = false, blink = false;
    
    // positions
    int32_t position_x = 0, position_y = 0, position_x_max = 0, position_y_max = 0;
    int32_t saved_position_y = 0, saved_position_x = 0;
    
    // sequence parsing variables
    int32_t seqValue, seqArrayCount, seq_line, seq_column;
    char *seqGrab;
    char **seqArray;
    
    // ANSi buffer structure array definition
    int32_t structIndex = 0;
    struct ansiChar *ansi_buffer, *temp;
    
    // ANSi buffer dynamic memory allocation
    ansi_buffer = malloc(sizeof(struct ansiChar));
    
    // background canvas
    int32_t background_canvas = 0;
    
    // ANSi interpreter
    while (loop < input_file_size)
    {
        current_character = input_file_buffer[loop];
        next_character = input_file_buffer[loop + 1];

        if (position_x==80 && !strcmp(WRAP_COLUMN_80, "1"))
        {
            position_y++;
            position_x=0;
        }
        
        // CR + LF
        if (current_character == 13 && next_character == 10) {
            position_y++;
            position_x = 0;
            loop++;
        }

        // LF
        if (current_character == 10)
        {
            position_y++;
            position_x = 0;
        }
        
        // tab
        if (current_character == 9)
        {
            position_x += 8;
        }
        
        // sub
        if (current_character == 26 && !strcmp(SUBSTITUTE_BREAK, "1"))
        {
            break;
        }
        
        // ANSi sequence
        if (current_character == 27 && next_character == 91)
        {            
            for (ansi_sequence_loop = 0; ansi_sequence_loop < 12; ansi_sequence_loop++)
            {
                ansi_sequence_character = input_file_buffer[loop + 2 + ansi_sequence_loop];
                
                // cursor position
                if (ansi_sequence_character == 'H' || ansi_sequence_character == 'f')
                {
                    // create substring from the sequence's content
                    seqGrab = substr((char *)input_file_buffer, loop+2, ansi_sequence_loop);
                    
                    // create sequence content array
                    seqArrayCount = explode(&seqArray, ';', seqGrab);
                    
                    if (seqArrayCount > 1) {
                        // convert grabbed sequence content to integers
                        seq_line = atoi(seqArray[0]);
                        seq_column = atoi(seqArray[1]);
                        
                        // finally set the positions
                        position_y = seq_line-1;
                        position_x = seq_column-1;
                    }
                    else {
                        // no coordinates specified? we move to the home position
                        position_y = 0;
                        position_x = 0;
                    }
                    loop+=ansi_sequence_loop+2;
                    break;
                }
                
                // cursor up
                if (ansi_sequence_character=='A')
                {
                    // create substring from the sequence's content
                    seqGrab = substr((char *)input_file_buffer, loop+2, ansi_sequence_loop);
                    
                    // now get escape sequence's position value
                    int32_t seq_line = atoi(seqGrab);
                    
                    if (seq_line == 0) {
                        seq_line = 1;
                    }
                    
                    position_y = position_y - seq_line;
                    
                    loop+=ansi_sequence_loop+2;
                    break;
                }
                
                // cursor down
                if (ansi_sequence_character=='B')
                {
                    // create substring from the sequence's content
                    seqGrab = substr((char *)input_file_buffer, loop+2, ansi_sequence_loop);
                    
                    // now get escape sequence's position value
                    int32_t seq_line = atoi(seqGrab);
                    
                    if (seq_line == 0) {
                        seq_line = 1;
                    }
                    
                    position_y = position_y + seq_line;
                     
                    loop+=ansi_sequence_loop+2;
                    break;
                }

                // cursor forward
                if (ansi_sequence_character=='C')
                {
                    // create substring from the sequence's content                    
                    seqGrab = substr((char *)input_file_buffer, loop+2, ansi_sequence_loop);

                    // now get escape sequence's position value
                    int32_t seq_column = atoi(seqGrab);
                    
                    if (seq_column == 0) {
                        seq_column = 1;
                    }
                    
                    position_x = position_x + seq_column;
                    
                    if (position_x>80)
                    {
                        position_x=80;
                    }
                    
                    loop+=ansi_sequence_loop+2;
                    break;
                }
                
                // cursor backward
                if (ansi_sequence_character=='D')
                {
                    // create substring from the sequence's content                    
                    seqGrab = substr((char *)input_file_buffer, loop+2, ansi_sequence_loop);

                    // now get escape sequence's content length
                    int32_t seq_column = atoi(seqGrab);
                    
                    if (seq_column == 0) {
                        seq_column = 1;
                    }
                    
                    position_x = position_x - seq_column;
                    
                    if (position_x < 0)
                    {
                        position_x = 0;
                    }
                    
                    loop+=ansi_sequence_loop+2;
                    break;
                }
                
                // save cursor position
                if (ansi_sequence_character=='s')
                {
                    saved_position_y = position_y;
                    saved_position_x = position_x;
                    
                    loop+=ansi_sequence_loop+2;
                    break;
                }
                
                // restore cursor position
                if (ansi_sequence_character=='u')
                {
                    position_y = saved_position_y;
                    position_x = saved_position_x;
                    
                    loop+=ansi_sequence_loop+2;
                    break;
                }
                
                // erase display
                if (ansi_sequence_character=='J')
                {
                    // create substring from the sequence's content                    
                    seqGrab = substr((char *)input_file_buffer, loop+2, ansi_sequence_loop);
                        
                    // convert grab to an integer
                    int32_t eraseDisplayInt = atoi(seqGrab);
                        
                    if (eraseDisplayInt == 2)
                    {    
                        position_x=0;
                        position_y=0;
                            
                        position_x_max=0;
                        position_y_max=0;
                        
                        // reset ansi buffer
                        free(ansi_buffer);
                        ansi_buffer = malloc(sizeof(struct ansiChar));
                        structIndex=0;
                    }
                    loop+=ansi_sequence_loop+2;
                    break;
                }
                
                // set graphics mode
                if (ansi_sequence_character=='m')
                {                    
                        // create substring from the sequence's content
                        seqGrab = substr((char *)input_file_buffer, loop+2, ansi_sequence_loop);
                        
                        // create sequence content array
                        seqArrayCount = explode(&seqArray, ';', seqGrab);
                        
                        // a loophole in limbo
                        for (seq_graphics_loop = 0; seq_graphics_loop < seqArrayCount; seq_graphics_loop++)
                        {
                            // convert split content value to integer
                            seqValue = atoi(seqArray[seq_graphics_loop]);
                            
                            if (seqValue == 0)
                            {
                                color_background = 0;
                                color_foreground = 7;
                                bold = false;
                                underline = false;
                                italics = false;
                                blink = false;
                            }
                            
                            if (seqValue == 1)
                            {
                                if (!workbench)
                                {
                                    color_foreground+=8;
                                }
                                bold = true;
                            }
                            
                            if (seqValue == 3)
                            {
                                italics = true;
                            }
                            
                            if (seqValue == 4)
                            {
                                underline = true;
                            }
                            
                            if (seqValue == 5)
                            {
                                if (!workbench)
                                {
                                    color_background+=8;
                                }
                                blink = true;
                            }
                            
                            if (seqValue > 29 && seqValue < 38)
                            {
                                color_foreground = seqValue - 30;
                                
                                if (bold)
                                {
                                    color_foreground+=8;
                                }
                            }
                            
                            if (seqValue > 39 && seqValue < 48)
                            {
                                color_background = seqValue - 40;
                                
                                if (blink && icecolors)
                                {
                                    color_background+=8;
                                }
                            }
                        }
                    
                    loop+=ansi_sequence_loop+2;
                    break;
                }
                
                // cursor (de)activation (Amiga ANSi)
                if (ansi_sequence_character == 'p')
                {
                    loop+=ansi_sequence_loop+2;
                    break;
                }

                // skipping set mode and reset mode sequences
                if (ansi_sequence_character == 'h' || ansi_sequence_character == 'l')
                {
                    loop+=ansi_sequence_loop+2;
                    break;
                }
            }
        }
        else if (current_character!=10 && current_character!=13 && current_character!=9)
        {
            // record number of columns and lines used
            if (position_x>position_x_max)
            {
                position_x_max=position_x;
            }
            
            if (position_y>position_y_max)
            {
                position_y_max=position_y;
            }
            
            // write current character in ansiChar structure 
            if (!fontData.isAmigaFont || (current_character != 12 && current_character != 13))
            {
                // reallocate structure array memory
                temp = realloc(ansi_buffer, (structIndex + 1) * sizeof(struct ansiChar));
                ansi_buffer = temp;
                
                ansi_buffer[structIndex].color_background = color_background;
                ansi_buffer[structIndex].color_foreground = color_foreground;
                ansi_buffer[structIndex].current_character = current_character;
                ansi_buffer[structIndex].bold = bold;
                ansi_buffer[structIndex].italics = italics;
                ansi_buffer[structIndex].underline = underline;
                ansi_buffer[structIndex].position_x = position_x;
                ansi_buffer[structIndex].position_y = position_y;
                                
                structIndex++;
                position_x++;
            }
        }
        loop++;
    }
    
    // allocate image buffer memory
    position_x_max++;
    position_y_max++;
    
    if (ced)
    {
        columns = 78;
    }
    
    if (isDizFile) {
        columns = fmin(position_x_max,80);
    }
        
    // create that damn thingy
    im_ANSi = gdImageCreate(columns * int_bits,(position_y_max)*fontData.font_size_y);
    
    if (!im_ANSi) {
        fputs ("\nCan't allocate ANSi buffer image memory.\n\n", stderr); exit (6);
    }
    
    int32_t colors[21];
    
    if (ced)
    {
        colors[0]=gdImageColorAllocate(im_ANSi, 170, 170, 170);

        for (loop=1; loop<16; loop++)
        {     
            colors[loop]=gdImageColorAllocate(im_ANSi, 0, 0, 0);
        }
    }
    else if (workbench)
    {        
        gdImageFill(im_ANSi, 0, 0, 0);
        colors[0] = gdImageColorAllocate(im_ANSi, 170, 170, 170);
        colors[1] = gdImageColorAllocate(im_ANSi, 0, 0, 255);
        colors[2] = gdImageColorAllocate(im_ANSi, 255, 255, 255);
        colors[3] = gdImageColorAllocate(im_ANSi, 0, 255, 255);
        colors[4] = gdImageColorAllocate(im_ANSi, 0, 0, 0);
        colors[5] = gdImageColorAllocate(im_ANSi, 255, 0, 255);
        colors[6] = gdImageColorAllocate(im_ANSi, 102, 136, 187);
        colors[7] = gdImageColorAllocate(im_ANSi, 255, 255, 255);
        colors[8] = gdImageColorAllocate(im_ANSi, 170, 170, 170);
        colors[9] = gdImageColorAllocate(im_ANSi, 0, 0, 255);
        colors[10] = gdImageColorAllocate(im_ANSi, 255, 255, 255);
        colors[11] = gdImageColorAllocate(im_ANSi, 0, 255, 255);
        colors[12] = gdImageColorAllocate(im_ANSi, 0, 0, 0);
        colors[13] = gdImageColorAllocate(im_ANSi, 255, 0, 255);
        colors[14] = gdImageColorAllocate(im_ANSi, 102, 136, 187);
        colors[15] = gdImageColorAllocate(im_ANSi, 255, 255, 255);
    }

    else
    {
        // Allocate standard ANSi color palette
        
        colors[0] = gdImageColorAllocate(im_ANSi, 0, 0, 0);
        colors[1] = gdImageColorAllocate(im_ANSi, 170, 0, 0);
        colors[2] = gdImageColorAllocate(im_ANSi, 0, 170, 0);
        colors[3] = gdImageColorAllocate(im_ANSi, 170, 85, 0);
        colors[4] = gdImageColorAllocate(im_ANSi, 0, 0, 170);
        colors[5] = gdImageColorAllocate(im_ANSi, 170, 0, 170);
        colors[6] = gdImageColorAllocate(im_ANSi, 0, 170, 170);
        colors[7] = gdImageColorAllocate(im_ANSi, 170, 170, 170);
        colors[8] = gdImageColorAllocate(im_ANSi, 85, 85, 85);
        colors[9] = gdImageColorAllocate(im_ANSi, 255, 85, 85);
        colors[10] = gdImageColorAllocate(im_ANSi, 85, 255, 85);
        colors[11] = gdImageColorAllocate(im_ANSi, 255, 255, 85);
        colors[12] = gdImageColorAllocate(im_ANSi, 85, 85, 255);
        colors[13] = gdImageColorAllocate(im_ANSi, 255, 85, 255);
        colors[14] = gdImageColorAllocate(im_ANSi, 85, 255, 255);
        colors[15] = gdImageColorAllocate(im_ANSi, 255, 255, 255);
        colors[20] = gdImageColorAllocate(im_ANSi, 200, 220, 169);

        background_canvas = gdImageColorAllocate(im_ANSi, 0, 0, 0);
    }

    // even more definitions, sigh
    int32_t ansiBufferItems = structIndex;
    
    // render ANSi
    for (loop = 0; loop < ansiBufferItems; loop++)
    {
        // grab ANSi char from our structure array
        color_background = ansi_buffer[loop].color_background;
        color_foreground = ansi_buffer[loop].color_foreground;
        character = ansi_buffer[loop].current_character;
        bold = ansi_buffer[loop].bold;
        italics = ansi_buffer[loop].italics;
        underline = ansi_buffer[loop].underline;
        position_x = ansi_buffer[loop].position_x;
        position_y = ansi_buffer[loop].position_y;
        
        alDrawChar(im_ANSi, fontData.font_data, int_bits, fontData.font_size_y, 
                   position_x, position_y, colors[color_background], colors[color_foreground], character);

    }
    
    // transparent flag used?
    if (transparent)
    {
        gdImageColorTransparent(im_ANSi, background_canvas);
    }

    // create output image
    FILE *file_Out = fopen(output, "wb");
    gdImagePng(im_ANSi, file_Out);
    fclose(file_Out);
    
    // in case Retina image output is wanted
    if (createRetinaRep)
    {
        gdImagePtr im_RetinaANSi;
        
        // make the Retina image @2x as large as im_ANSi
        im_RetinaANSi = gdImageCreate(im_ANSi->sx * 2, im_ANSi->sy * 2);
        
        gdImageCopyResized(im_RetinaANSi, im_ANSi, 0, 0, 0, 0,
                           im_RetinaANSi->sx, im_RetinaANSi->sy,
                           im_ANSi->sx, im_ANSi->sy);
        
        // create retina output image
        FILE *file_RetinaOut = fopen(retinaout, "wb");
        gdImagePng(im_RetinaANSi, file_RetinaOut);
        fclose(file_RetinaOut);
        
        gdImageDestroy(im_RetinaANSi);
    }
    
    // free memory
    free(ansi_buffer);
    
    gdImageDestroy(im_ANSi);
}
