/**
 * Copies a BMP piece by piece, just because.
 */
       
#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        fprintf(stderr, "Usage: ./copy scale infile outfile\n");
        return 1;
    }

    // remember filenames
    char *infile = argv[2];
    char *outfile = argv[3];
    
    //scale
    float scale=atoi(argv[1]);

    // open input file 
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 3;
    }
    
    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER in_bf;
    fread(&in_bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER in_bi;
    fread(&in_bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (in_bf.bfType != 0x4d42 || in_bf.bfOffBits != 54 || in_bi.biSize != 40 || 
        in_bi.biBitCount != 24 || in_bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }
    
    //incorporating header changes
    DWORD new_bfSize;
    DWORD new_biSize;
    LONG new_biWidth,new_biHeight;
    int padding_new;
    
    new_biWidth=in_bi.biWidth*scale;
    new_biHeight=in_bi.biHeight*scale;
    
    padding_new = (4 - (new_biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    
    new_biSize=((sizeof(RGBTRIPLE)*new_biWidth)+padding)*abs(new_biHeight);
    new_bfSize=new_biSize+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);

    // write outfile's bfType
    fwrite(&bf, sizeof(WORD), 1, outptr);
    
    // write outfile's bfSize
    fwrite(&new_bfSize, sizeof(DWORD), 1, outptr);
    
    // write outfile's (rest of)BITMAPFILEHEADER
    fwrite(&in_bf+6, 2*sizeof(WORD)+sizeof(DWORD), 1, outptr);

    // write outfile's biSize
    fwrite(&new_biSize, sizeof(DWORD), 1, outptr);
    
    // write outfile's biWidth
    fwrite(&new_biWidth, sizeof(LONG), 1, outptr);
    
    // write outfile's biHeight
    fwrite(&new_biHeight, sizeof(LONG), 1, outptr);
    
    // write outfile's (rest of)BITMAPFILEHEADER
    fwrite(&bf+12, sizeof(BITMAPFILEHEADER)-12, 1, outptr);

    // determine padding for scanlines
    int padding_old = (4 - (in_bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(in_bi.biHeight); i < biHeight; i++)
    {
        // iterate over pixels in scanline
        for (int j = 0; j < in_bi.biWidth; j++)
        {
            // temporary storage
            RGBTRIPLE triple;

            // read RGB triple from infile
            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

            // write RGB triple to outfile
            fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
        }

        // skip over padding, if any
        fseek(inptr, padding_old, SEEK_CUR);

        // then add it back (to demonstrate how)
        for (int k = 0; k < padding_new; k++)
        {
            fputc(0x00, outptr);
        }
    }

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // success
    return 0;
}
