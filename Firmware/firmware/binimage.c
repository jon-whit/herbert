///////////////////////////////////////////////////////////////////////////////////
//
//  Creates a flashable, bootloadable image from an ELF file
//
//  The resulting binary image is in the
//  following format:
//
//  Location  Field
//   0   - 3       Image Size - not including header and CRC
//   4   - 7       Image End - Zero from end of file to this address (bss section)
//   8   - 11      Entry point - Beginning of execution (_start1)
//   12  - 51      Exception Vectors
//   52  - n       Firmware Image
//   n+1 - n+4     32-bit CRC of whole image (0-n)
//
//  Note: Image Size, Entry Point, Image End & CRC are in network byte order
//


#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <common/crc32.h>


#define VERSION "1.0"


#define FILENAME_LEN 4096
#define CMD_LINE_LEN 4096
#define BUFFER_LEN   1024

#define VECTORS_LEN  40

void usage();



int main(int argc, char** argv)
{
    FILE*         cmdFile;
    FILE*         vectorFile;
    FILE*         imageFile;
    FILE*         outputFile;
    char          vectorFileName[FILENAME_LEN];
    char          imageFileName[FILENAME_LEN];
    char          cmdLine[CMD_LINE_LEN];
    char          buf[BUFFER_LEN];
    char*         pos;
    int           count;
    int           rc;
    size_t        vectorsSize;
    size_t        imageSize;
    unsigned long entryPoint;
    unsigned long imageEnd;
    CRC32         crcValue = INITIAL_CRC32_VALUE;
    


    // Check command line parameters
    if(argc < 2)
    {
        fprintf(stderr, "Error: Missing input and output file names\n\n");
        usage();
        return -1;
    }
    
    if(argc == 2 && strcmp(argv[1], "-h") == 0)
    {
        usage();
        return 0;
    }
    
    if(argc != 3)
    {
        fprintf(stderr, "Error: Invalid parameters\n\n");
        usage();
        return -1;
    }
    
    

    // Find end of image (end of bss section)
    snprintf(cmdLine, CMD_LINE_LEN, "objdump -t %s | grep __bss_end", argv[1]);

    if(!(cmdFile = popen(cmdLine, "r")))
    {
        fprintf(stderr, "Error processing input file '%s'.\n", argv[1]);
        return rc;
    }
    
    fscanf(cmdFile, "%lx", &imageEnd);
    
    pclose(cmdFile);



    // Get Entry Point
    snprintf(cmdLine, CMD_LINE_LEN, "objdump -t %s | grep _start1", argv[1]);

    if(!(cmdFile = popen(cmdLine, "r")))
    {
        fprintf(stderr, "Error processing input file '%s'.\n", argv[1]);
        return rc;
    }
    
    fscanf(cmdFile, "%lx", &entryPoint);

    pclose(cmdFile);



    // Determine binary image file names
    pos = strrchr(argv[1], '.');
    
    if((pos = strrchr(argv[1], '.')))
    {
        *pos = 0;
        snprintf(vectorFileName, FILENAME_LEN, "%s_vectors.bin", argv[1]);
        snprintf(imageFileName,  FILENAME_LEN, "%s_image.bin", argv[1]);
        *pos = '.';
    }
    else
    {
        fprintf(stderr, "Error processing input file '%s'.  Invalid file name.\n", argv[1]);
        return rc;
    }



    // Create exception vectors binary file
    snprintf(cmdLine, CMD_LINE_LEN, "objcopy -I elf32-big -O binary -R .text -R .init "
                                    "-R .fini -R .rodata -R .data -R .sbss2 -R .sdata2 "
                                    "-R .ctors -R .dtors -R .eh_frame -R .jcr -R .bss "
                                    "-R .sbss -R .sdata -R .tdata --gap-fill 0 %s %s",
                                    argv[1], vectorFileName);

    if((rc = system(cmdLine)))
    {
        fprintf(stderr, "Error processing input file '%s'.\n", argv[1]);
        return rc;
    }



    // Create SDRAM binary file
    snprintf(cmdLine, CMD_LINE_LEN, "objcopy -I elf32-big -O binary -R .vectors.reset "
                                    "-R .vectors.sw_exception -R .vectors.interrupt "
                                    "-R .vectors.hw_exception --gap-fill 0 %s %s",
                                    argv[1], imageFileName);

    if((rc = system(cmdLine)))
    {
        fprintf(stderr, "Error processing input file '%s'.\n", argv[1]);
        return rc;
    }



    // Open Input Vector file and get length
    vectorFile = fopen(vectorFileName, "rb");

    if(!vectorFile)
    {
        fprintf(stderr, "Error: Unable to open vector file '%s' %s\n", vectorFileName, strerror(errno));
        return -1;
    }

    fseek(vectorFile, 0, SEEK_END);
    vectorsSize = ftell(vectorFile);
    rewind(vectorFile);

    if(vectorsSize != VECTORS_LEN)
    {
        fclose(vectorFile);
        fprintf(stderr, "Error: Invalid vector file '%s'.  Incorrect length.\n", vectorFileName);
        return -1;
    }



    // Open Input SDRAM image file and get length
    imageFile = fopen(imageFileName, "rb");

    if(!imageFile)
    {
        fclose(vectorFile);
        fprintf(stderr, "Error: Unable to open SDRAM file '%s' %s\n", imageFileName, strerror(errno));
        return -1;
    }

    fseek(imageFile, 0, SEEK_END);
    imageSize = ftell(imageFile);
    rewind(imageFile);



    // Open output file
    outputFile = fopen(argv[2], "wb");

    if(!outputFile)
    {
        fclose(vectorFile);
        fclose(imageFile);
        fprintf(stderr, "Error: Unable to open output file '%s' %s\n", argv[2], strerror(errno));
        return -1;
    }



    // Write data to output file
    imageSize  = ntohl(imageSize);
    imageEnd   = ntohl(imageEnd);
    entryPoint = ntohl(entryPoint);

    crcValue = calcCRC32(crcValue, (char*)&imageSize,  sizeof(imageSize));
    crcValue = calcCRC32(crcValue, (char*)&imageEnd,   sizeof(imageEnd));
    crcValue = calcCRC32(crcValue, (char*)&entryPoint, sizeof(entryPoint));

    fwrite(&imageSize,  sizeof(imageSize),  1, outputFile);
    fwrite(&imageEnd,   sizeof(imageEnd),   1, outputFile);
    fwrite(&entryPoint, sizeof(entryPoint), 1, outputFile);


    if((count = fread(buf, 1, VECTORS_LEN, vectorFile)) != VECTORS_LEN)
    {
        fclose(vectorFile);
        fclose(imageFile);
        fclose(outputFile);
        fprintf(stderr, "Error: Error reading vector file '%s'\n", vectorFileName);
        return -1;
    }
    
    crcValue = calcCRC32(crcValue, buf, VECTORS_LEN);

    fwrite(buf, 1, VECTORS_LEN, outputFile);


    while((count = fread(buf, 1, BUFFER_LEN, imageFile)) > 0)
    {
        crcValue = calcCRC32(crcValue, buf, count);
        fwrite(buf, 1, count, outputFile);
    }
    
    if(!feof(imageFile))
    {
        fclose(vectorFile);
        fclose(imageFile);
        fclose(outputFile);
        fprintf(stderr, "Error: processing SDRAM image file '%s'\n", imageFileName);
        return -1;
    }


    crcValue = ntohl(crcValue);
            
    fwrite(&crcValue, sizeof(crcValue), 1, outputFile);

    fclose(vectorFile);
    fclose(imageFile);
    fclose(outputFile);
    
    return 0;
}


void usage()
{
    printf("binimage - Binary Image Creator\n");
    printf("  Version %s  Built %s, %s\n", VERSION, __TIME__, __DATE__);
    
    printf("Usage:\n");
    printf("  binimage <elf file> <output binary file>  - Creates a Binary Image\n");
    printf("  binimage -h                               - Shows help\n");
}

