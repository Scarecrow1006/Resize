/**
 * Copies a BMP piece by piece, just because.
 */
       
#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

int main(int argc, char *argv[]){
    // ensure proper usage
    if (argc != 4){
        fprintf(stderr, "Usage: ./copy scale infile outfile\n");
        return 1;
    }

    // remember filenames
    char *infile = argv[2];
    char *outfile = argv[3];
    
    //scale
    float scale=atof(argv[1]);

    // open input file 
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL){
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL){
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
        in_bi.biBitCount != 24 || in_bi.biCompression != 0){
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }
    
    //header data for output file
    BITMAPFILEHEADER out_bf=in_bf;
    BITMAPINFOHEADER out_bi=in_bi;
    
    //new width and height
    out_bi.biWidth=in_bi.biWidth*scale;
    out_bi.biHeight=in_bi.biHeight*scale;
    
    //new padding
    int padding_new = (4 - (out_bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    
    out_bi.biSizeImage=((sizeof(RGBTRIPLE)*out_bi.biWidth)+padding_new)*abs(out_bi.biHeight);
    out_bf.bfSize=out_bi.biSize+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);

    // write outfile's BITMAPFILEHEADER
    fwrite(&out_bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&out_bi, sizeof(BITMAPINFOHEADER), 1, outptr);

    // determine padding for scanlines
    int padding_old = (4 - (in_bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    
    //counters
    int i,j,k,l=4,m;
    
    if(scale==1.0){
        // iterate over infile's scanlines
        for (i = 0; i < abs(in_bi.biHeight); i++){
            // iterate over pixels in scanline
            for (j = 0; j < in_bi.biWidth; j++){
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
            for (k = 0; k < padding_new; k++){
                fputc(0x00, outptr);
            }
        }
    }
    else{
        if(scale>1.0){
            // iterate over infile's scanlines
            for (i = 0; i < abs(in_bi.biHeight); i++){
                // iterate over pixels in scanline
                for (j = 0; j < in_bi.biWidth; j++){
                    // temporary storage
                    RGBTRIPLE triple;
                    
                    // read RGB triple from infile
                    fread(&triple, sizeof(RGBTRIPLE), 1, inptr);
                    
                    // write RGB triple to outfile
                    for(m=0;m<scale;m++){
                        fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
                    }
                }
                
                // skip over padding, if any
                fseek(inptr, padding_old, SEEK_CUR);
                
                // then add it back (to demonstrate how)
                for (k = 0; k < padding_new; k++){
                    fputc(0x00, outptr);
                }
                
                l--;
                if(l>0){
                    fseek(inptr, -(padding_old+in_bi.biWidth), SEEK_CUR);
                    i--;
                }
                if(l==0) l=4;
            }
        }
        else{
            float temp=1.0/scale;
            int mult=temp;
            
            // iterate over infile's scanlines
            for (i = 0; i < abs(out_bi.biHeight); i++){
                // iterate over pixels in scanline
                for (j = 0; j < out_bi.biWidth; j++){
                    // temporary storage
                    RGBTRIPLE triple[mult];
                    RGBTRIPLE scanline_pixel[mult];
                    RGBTRIPLE pixel;
                    
                    for(m=0;m<mult;m++){
                        // read RGB triple from infile
                        fread(triple, sizeof(RGBTRIPLE), mult, inptr);
                        
                        //next scanline
                        fseek(inptr, in_bi.biWidth-mult+padding_old, SEEK_CUR);
                        
                        //reset scanline_pixel
                        scanline_pixel[m].rgbtBlue=0x00;
                        scanline_pixel[m].rgbtGreen=0x00;
                        scanline_pixel[m].rgbtRed=0x00;
                        
                        for(l=0;l<mult;l++){
                            triple[l].rgbtBlue*=scale;
                            scanline_pixel[m].rgbtBlue+=triple[l].rgbtBlue;
                            triple[l].rgbtGreen*=scale;
                            scanline_pixel[m].rgbtGreen+=triple[l].rgbtGreen;
                            triple[l].rgbtRed*=scale;
                            scanline_pixel[m].rgbtRed+=triple[l].rgbtRed;
                        }
                    }
                    
                    //combining vertical pixels
                    for(l=0;l<mult;l++){
                        scanline_pixel[l].rgbtBlue*=scale;
                        pixel.rgbtBlue+=scanline_pixel[l].rgbtBlue;
                        scanline_pixel[l].rgbtGreen*=scale;
                        pixel.rgbtGreen+=scanline_pixel[l].rgbtGreen;
                        scanline_pixel[l].rgbtRed*=scale;
                        pixel.rgbtRed+=scanline_pixel[l].rgbtRed;
                    }
                    
                    // write RGB triple to outfile
                    fwrite(&pixel, sizeof(RGBTRIPLE), 1, outptr);
                    
                    //reset pixel
                    pixel.rgbtBlue=0x00;
                    pixel.rgbtGreen=0x00;
                    pixel.rgbtRed=0x00;
                    
                    fseek(inptr, -(mult*(in_bi.biWidth+padding_old)), SEEK_CUR);
                }
                
                // skip over padding, if any
                fseek(inptr, padding_old, SEEK_CUR);
                
                // then add it back (to demonstrate how)
                for (k = 0; k < padding_new; k++){
                    fputc(0x00, outptr);
                }
            }
        }
    }
    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // success
    return 0;
}
