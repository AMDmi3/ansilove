//
//  main.c
//  AnsiLove/C
//
//  Copyright (C) 2011-2015 Stefan Vogt, Brian Cassidy, Frederic Cambus.
//  All rights reserved.
//
//  This source code is licensed under the BSD 3-Clause License.
//  See the file LICENSE for details.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include "alconfig.h"
#include "strtolower.h"
#include "substr.h"
#include "ansilove.h"

// prototypes
void showHelp(void);
void listExamples(void);
void versionInfo(void);
void synopsis(void);

void showHelp(void)
{
    printf("\nSUPPORTED FILE TYPES:\n"
           "  ANS  BIN  ADF  IDF  XB  PCB  TND  ASC  NFO  DIZ\n"
           "  Files with custom suffix default to the ANSi renderer.\n\n"
           "PC FONTS:\n"
           "  80x25              icelandic\n"
           "  80x50              latin1\n"
           "  baltic             latin2\n"
           "  cyrillic           nordic\n"
           "  french-canadian    portuguese\n"
           "  greek              russian\n"
           "  greek-869          terminus\n"
           "  hebrew             turkish\n\n"
           "AMIGA FONTS:\n"
           "  amiga              topaz\n"
           "  microknight        topaz+\n"
           "  microknight+       topaz500\n"
           "  mosoul             topaz500+\n"
           "  pot-noodle\n\n"
           "DOCUMENTATION:\n"
           "  Detailed help is available at the AnsiLove/C repository on GitHub.\n"
           "  <https://github.com/ansilove/AnsiLove-C>\n\n");
}

void listExamples(void)
{
    printf("\nEXAMPLES:\n"); 
    printf("  ansilove file.ans (output path/name identical to input, no options)\n"
           "  ansilove -i file.ans (enable iCE colors)\n"
           "  ansilove -r file.ans (adds Retina @2x output file)\n"
           "  ansilove -o dir/file file.ans (custom path/name for output)\n"
           "  ansilove -s file.bin (just display SAUCE record, don't generate output)\n"
           "  ansilove -m transparent file.ans (render with transparent background)\n"
           "  ansilove -f amiga -b 9 -c 202 file.bin (set font, bits, columns)\n"
           "\n");
}

void versionInfo(void)
{
    printf("All rights reserved.\n"
           "\nFork me on GitHub: <https://github.com/ansilove/AnsiLove-C>\n"
           "Bug reports: <https://github.com/ansilove/AnsiLove-C/issues>\n\n"
           "This is free software, released under the 3-Clause BSD license.\n"
           "<https://github.com/ansilove/AnsiLove-C/blob/master/LICENSE>\n\n");
}

// following the IEEE Std 1003.1 for utility conventions
void synopsis(void)
{
    printf("\nSYNOPSIS:\n"
           "  ansilove [options] file\n"
           "  ansilove -e | -h | -v\n\n"
           "OPTIONS:\n"
           "  -b bits     set to 9 to render 9th column of block characters (default: 8)\n"
           "  -c columns  adjust number of columns for BIN files (default: 160)\n"
           "  -e          print a list of examples\n"
           "  -f font     select font (default: 80x25)\n"
           "  -h          show help\n"
           "  -i          enable iCE colors\n"
           "  -m mode     set rendering mode for ANS files:\n"
           "                ced            black on gray, with 78 columns\n"
           "                transparent    render with transparent background\n"
           "                workbench      use Amiga Workbench palette\n"
           "  -o file     specify output filename/path\n"
           "  -r          creates additional Retina @2x output file\n"
           "  -s          show SAUCE record without generating output\n"
           "  -v          show version information\n"
           "\n");
}

int main(int argc, char *argv[])
{
    printf("AnsiLove/C %s - ANSi / ASCII art to PNG converter\n"\
           "Copyright (C) 2011-2015 Stefan Vogt, Brian Cassidy, Frederic Cambus.\n", VERSION);
    
    // SAUCE record related bool types
    bool justDisplaySAUCE = false;
    bool fileHasSAUCE = false;
    
    // retina output bool type
    bool createRetinaRep = false;
    
    // iCE colors bool type
    bool icecolors = false;

    // analyze options and do what has to be done
    bool fileIsBinary = false;
    bool fileIsANSi = false;
    bool fileIsPCBoard = false;
    bool fileIsTundra = false;
    
    if (argc == 1 || argc >= 9) {
        synopsis();
        return EXIT_SUCCESS;
    }

    int getoptFlag;
    char *bits = NULL;
    char *mode = NULL;
    char *columns = NULL;
    char *font = NULL;

    char *input = NULL, *output = NULL;
    char *retinaout = NULL;

    while ((getoptFlag = getopt(argc, argv, "b:c:ef:him:o:rsv")) != -1) {
        switch(getoptFlag) {
        case 'b':
            bits = optarg;
            break;
        case 'c':
            columns = optarg;
            break;
        case 'e':
            listExamples();
            return EXIT_SUCCESS;
        case 'f':
            font = optarg;
            break;
        case 'h':
            showHelp();
            return EXIT_SUCCESS;
        case 'i':
            icecolors = true;
            break;
        case 'm':
            mode = optarg;
            break;
        case 'o':
            output = optarg;
            break;
        case 'r':
            createRetinaRep = true;
            break;
        case 's':
            justDisplaySAUCE = true;
            break;
        case 'v':
            versionInfo();
            return EXIT_SUCCESS;
        }
    }

    input = argv[optind];

    argc -= optind; 
    argv += optind;

    // let's check the file for a valid SAUCE record
    sauce *record = sauceReadFileName(input);
    
    // record == NULL also means there is no file, we can stop here
    if (record == NULL) {
        printf("\nFile %s not found.\n\n", input);
        return EXIT_FAILURE;
    }
    else {
        // if we find a SAUCE record, update bool flag
        if (!strcmp(record->ID, SAUCE_ID)) {
            fileHasSAUCE = true;
        }
    }

    if (!justDisplaySAUCE) 
    {
        // create output file name if output is not specified
        if (!output) {
            int outputLen = strlen(input) + 5;
            output = malloc(outputLen);
            snprintf(output, outputLen, "%s%s", input, ".png");
        }

        if (createRetinaRep) {
            int retinaLen = strlen(input) + 8;
            retinaout = malloc(retinaLen);
            snprintf(retinaout, retinaLen, "%s%s", input, "@2x.png");        
        }

        // default to 8 if bits option is not specified
        if (!bits) {
            bits = "8";
        }

        // default to empty string if mode option is not specified
        if (!mode) {
            mode = "";
        }

        // convert numeric command line flags to integer values
        int32_t int_bits = atoi(bits);

        // now set bits to 8 if not already value 8 or 9
        if (int_bits != 8 && int_bits != 9) {
            int_bits = 8;
        }

        // default to 160 if columns option is not specified
        if (!columns) {
            columns = "160";
        }

        // default to 80x25 font if font option is not specified
        if (!font) {
            font = "80x25";
        }

        // get file extension
        char *fext = strrchr(input, '.');
        fext = fext ? strtolower(fext) : "none";

        // create the output file by invoking the appropiate function
        if (!strcmp(fext, ".pcb")) {
            // params: input, output, font, bits, icecolors
            alPcBoardLoader(input, output, retinaout, font, int_bits, createRetinaRep);
            fileIsPCBoard = true;
        }
        else if (!strcmp(fext, ".bin")) {
            // params: input, output, columns, font, bits, icecolors
            alBinaryLoader(input, output, retinaout, columns, font, int_bits, icecolors, createRetinaRep);
            fileIsBinary = true;
        }
        else if (!strcmp(fext, ".adf")) {
            // params: input, output, bits
            alArtworxLoader(input, output, retinaout, createRetinaRep);
        }
        else if (!strcmp(fext, ".idf")) {
            // params: input, output, bits
            alIcedrawLoader(input, output, retinaout, fileHasSAUCE, createRetinaRep);
        }
        else if (!strcmp(fext, ".tnd")) {
            alTundraLoader(input, output, retinaout, font, int_bits, fileHasSAUCE, createRetinaRep);
            fileIsTundra = true;
        }
        else if (!strcmp(fext, ".xb")) {
            // params: input, output, bits
            alXbinLoader(input, output, retinaout, createRetinaRep);
        }
        else {
            // params: input, output, font, bits, icecolors, fext
            alAnsiLoader(input, output, retinaout, font, int_bits, mode, icecolors, fext, createRetinaRep);
            fileIsANSi = true;
        }

        // gather information and report to the command line
        printf("\nInput File: %s\n", input);
        printf("Output File: %s\n", output);
        if (createRetinaRep) {
            printf("Retina Output File: %s\n", retinaout);
        }
        if (fileIsANSi || fileIsBinary || 
            fileIsPCBoard || fileIsTundra) {
            printf("Font: %s\n", font);
        }
        if (fileIsANSi || fileIsBinary || 
            fileIsPCBoard || fileIsTundra) {
            printf("Bits: %d\n", int_bits);
        }
        if (icecolors && (fileIsANSi || fileIsBinary)) {
            printf("iCE Colors: enabled");
        }
        if (fileIsBinary) {
            printf("Columns: %s\n", columns);
        }
    }

    // either display SAUCE or tell us if there is no record
    if (!fileHasSAUCE) {
        printf("\nFile %s does not have a SAUCE record.\n", input);
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

    return EXIT_SUCCESS;
}
