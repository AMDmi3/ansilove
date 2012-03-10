//
//  main.c
//  AnsiLove/C
//
//  Copyright (C) 2011-2012 Stefan Vogt, Brian Cassidy, Frederic Cambus.
//  All rights reserved.
//  https://github.com/ByteProject/AnsiLove-C/
//
//  Use of this source code is governed by a MIT-style license.
//  See the file LICENSE for details.
//

#if defined(__APPLE__) && defined(__MACH__)
#import <Foundation/Foundation.h>
#import "alconfig.h"
#import "strtolower.h"
#import "substr.h"
#import "ansilove.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "alconfig.h"
#include "strtolower.h"
#include "substr.h"
#include "ansilove.h"
#endif

// prototypes
void showHelp(void);
void showExamples(void);
void showVersion(void);
void showSynopsis(void);

void showHelp(void)
{
    printf("helpmeplease\n\n");
}

void showExamples(void)
{
    printf("EXAMPLES:\n"); 
    printf("ansilove ansi.ans ansi.png\n");
    printf("ansilove ansi.ans ansi.png 80x25 9 1 (80x25 font, 9-bit, iCE colors)\n");
    printf("ansilove ansi.ans -s 80x25 (80x25 font)\n");
    printf("ansilove ansi.ans ansi.png 80x50 9 (80x50 font, 9-bit)\n");
    printf("ansilove ansi.ans -s russian 9 (Russian font, 9-bit)\n");
    printf("ansilove ansi.ans ansi.png amiga (Amiga font)\n");
    printf("ansilove pcboard.pcb pcboard.png\n");
    printf("ansilove pcboard.pcb pcboard.png 80x25 9 (80x25 font, 9-bit)\n");
    printf("ansilove binary.bin -s 160\n");
    printf("ansilove binary.bin binary.png 160 80x25 9 1 (80x25 font, 9-bit, iCE colors)\n");
    printf("ansilove binary.bin binary.png 160 80x50 9 (80x50 font, 9-bit)\n");
    printf("ansilove tundra.tnd tundra.png\n");
    printf("ansilove tundra.tnd -s 80x25 9 (80x25 font, 9-bit)\n");
    printf("ansilove xbin.xb xbin.png\n");
    printf("ansilove ansiwithsauce.ans -r (just display SAUCE record)\n\n");
}

void showVersion(void)
{
    printf("version fooobazbar\n\n");
}

// following the IEEE Std 1003.1 for utility conventions
void showSynopsis(void)
{
    printf("SYNOPSIS:\n"
           "  ansilove file -i [operands]\n"
           "  ansilove file -o file.png [operands]\n"
           "  ansilove file -s\n"
           "  ansilove -vhe\n\n"
           "OPTIONS:\n"
           "  -i  output identical to input with .png suffix added\n"
           "  -o  specify custom file name / path for output\n"
           "  -s  display SAUCE record without generating output\n"
           "  -v  version information, equivalent to --version\n"
           "  -h  show help, equivalent to --help\n"
           "  -e  print a list of examples\n\n"
           "OPERANDS:\n"
           "  font bits icecolors columns\n\n");
    //printf("ansilove <inputfile> <outputfile> columns (.BIN only) font bits icecolors\n\n");
}

int main(int argc, char *argv[])
{
    printf("AnsiLove/C %s - ANSi / ASCII art to PNG converter\n"\
           "Copyright (C) 2011-2012 Stefan Vogt, Brian Cassidy, Frederic Cambus.\n\n", VERSION);
    
    // we do this before anything else
    if (argc <= 2) {
        showSynopsis();
        return EXIT_SUCCESS;
    }
    
    // SAUCE record related bool types
    bool justDisplaySAUCE = false;
    bool fileHasSAUCE = false;
    
    // in case the SAUCE flag is set we set our bool type to 'true'
    if (strcmp(argv[2], "-r") == 0) {
        justDisplaySAUCE = true;
    }
    
    // let's check the file for a valid SAUCE record
    sauce *record = sauceReadFileName(argv[1]);
    
    // record == NULL also means there is no file, we can stop here
    if (record == NULL) {
        printf("\nFile %s not found.\n\n", argv[1]);
        return EXIT_FAILURE;
    }
    else {
        // if we find a SAUCE record, update bool flag
        if (strcmp(record->ID, SAUCE_ID) == 0) {
            fileHasSAUCE = true;
        }
    }
    
    // this should be self-explanatory
    if (justDisplaySAUCE == false) 
    {
        // declaration of types we pass to ansilove.c
        char *input = argv[1];
        char output[1000] = { 0 };
        char columns[1000] = { 0 };
        char font[1000] = { 0 };
        char bits[1000] = { 0 };
        char icecolors[1000] = { 0 };
        char *fext;
        
        // find last position of char '.' so we can determine the file extension
        size_t index = strrchr(input, '.') - input;
        
        // calculate size of the input string
        size_t inpSize = strlen(input);
        
        // generate size_t result we can pass to our substr() implementation
        size_t result = inpSize - index;
        
        // finally create the file extension string
        fext = substr(input, inpSize - result, result);
        fext = strtolower(fext);
        if (fext == NULL) {
            fext = "none";
        }
        
        // in case we got arguments for input, output, and the '-s' flag is set
        if (strcmp(argv[2], "-s") == 0) 
        {
            // append .png suffix to file name
            sprintf(output, "%s.png", input);
        }
        else {
            // so the user provided an alternate path / file name
            sprintf(output, "%s", argv[2]);
        }
        
        // check flags and apply them based on the file extension
        if (strcmp(fext, ".bin") == 0)
        {
            // columns
            if (argc >= 4) {
                sprintf(columns, "%s", argv[3]);
            }
            else {
                sprintf(columns, "%s", "160");
            }        
            // font
            if (argc >= 5) {
                sprintf(font, "%s", argv[4]);
            }
            else {
                sprintf(font, "%s", "80x25");
            }
            // bits
            if (argc >= 6) {
                sprintf(bits, "%s", argv[5]);
            }
            else {
                sprintf(bits, "%s", "8");
                
            }
            // iCE colors
            if (argc >= 7) {
                sprintf(icecolors, "%s", argv[6]);
            }
            else {
                sprintf(icecolors, "%s", "0");
            }
        }
        else {
            // font
            if (argc >= 4) {
                sprintf(font, "%s", argv[3]);
            }
            else {
                sprintf(font, "%s", "80x25");
            }
            // bits
            if (argc >= 5) {
                sprintf(bits, "%s", argv[4]);
            }
            else {
                sprintf(bits, "%s", "8");
            }
            // iCE colors
            if (argc >= 6) {
                sprintf(icecolors, "%s", argv[5]);
            }
            else {
                sprintf(icecolors, "%s", "0");
            }
        }
        
        // report all flags to the command line
        printf("\nInput File: %s\n", input);
        printf("Output File: %s\n", output);
        printf("Columns (.BIN only): %s\n", columns);
        printf("Font (.ANS/.BIN only): %s\n", font);
        printf("Bits (.ANS/.BIN only): %s\n", bits);
        printf("iCE Colors (.ANS/.BIN only): %s\n", icecolors);
        
        // create the output file by invoking the appropiate function
        if (strcmp(fext, ".pcb") == 0) {
            // params: input, output, font, bits, icecolors
            alPcBoardLoader(input, output, font, bits);
        }
        else if (strcmp(fext, ".bin") == 0) {
            // params: input, output, columns, font, bits, icecolors
            alBinaryLoader(input, output, columns, font, bits, icecolors);
        }
        else if (strcmp(fext, ".adf") == 0) {
            // params: input, output, bits
            alArtworxLoader(input, output, bits);
        }
        else if (strcmp(fext, ".idf") == 0) {
            // params: input, output, bits
            alIcedrawLoader(input, output, bits, fileHasSAUCE);
        }
        else if (strcmp(fext, ".tnd") == 0) {
            alTundraLoader(input, output, font, bits);
        }
        else if (strcmp(fext, ".xb") == 0) {
            // params: input, output, bits
            alXbinLoader(input, output, bits);
        }
        else {
            // params: input, output, font, bits, icecolors, fext
            alAnsiLoader(input, output, font, bits, icecolors, fext);
        }
    }
    
    // either display SAUCE or tell us if there is no record
    if (fileHasSAUCE == false) {
        printf("\nFile %s does not have a SAUCE record.\n", argv[1]);
    }
    else {
        printf( "\n%s: %s v%s\n", "Id", record->ID, record->version);
        printf( "%s: %s\n", "Title", record->title );
        printf( "%s: %s\n", "Author", record->author);
        printf( "%s: %s\n", "Group", record->group);
        printf( "%s: %s\n", "Date", record->date);
        printf( "%s: %d\n", "Datatype", record->dataType);
        printf( "%s: %d\n", "Filetype", record->fileType);
        if (record->flags != 0) {
            printf( "%s: %d\n", "Flags", record->flags);
        }
        if (record->tinfo1 != 0) {
            printf( "%s: %d\n", "Tinfo1", record->tinfo1);
        }
        if (record->tinfo2 != 0) {
            printf( "%s: %d\n", "Tinfo2", record->tinfo2);
        }
        if (record->tinfo3 != 0) {
            printf( "%s: %d\n", "Tinfo3", record->tinfo3);
        }
        if (record->tinfo4 != 0) {
            printf( "%s: %d\n", "Tinfo4", record->tinfo4);
        }
        if (record->comments > 0) {
            int32_t i;
            printf( "Comments: ");
            for(i = 0; i < record->comments; i++) {
                printf( "%s\n", record->comment_lines[i] );
            }
        }
    }
    
    // post a message when the output file is created (in case we created output)
    if (justDisplaySAUCE == false) {
        printf("\nSuccessfully created output file.\n\n");
    }
    else {
        printf("\n");
    }
    
    return EXIT_SUCCESS;
}
