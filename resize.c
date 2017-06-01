/**
 * Copies a BMP piece by piece, just because.
 */
       
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "bmp.h"

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        fprintf(stderr, "Usage: ./resize infile outfile [f]\n");
        return 1;
    }

    ///////////////////////////////////////////////////
    /// Basic error checking for the correct inputs ///
    ///////////////////////////////////////////////////
    float factor = atof(argv[3]);
    
    if (factor<=0 || factor>100)
    {
        fprintf(stderr, "Multiplication factor must be in (0.00,100]\n");
        return 2;
    }

    // open input file 
    FILE *infile = fopen(argv[1],"r");
    if (infile==NULL){
        fprintf(stderr,"Could not open %s for reading\n",argv[1]);
        return 3;
    }
    
    FILE *outfile = fopen(argv[2],"w");
    if (outfile==NULL){
        fprintf(stderr,"Could not open %s for writing\n",argv[2]);
        return 4;
    }
    
    ///////////////////////////////////////////////////
    ///          The fun starts here                ///
    ///////////////////////////////////////////////////
    
    // Read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, infile);

    // Read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, infile);

    // Ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 || 
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outfile);
        fclose(infile);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }
    
    // Since we will need to make changes to the header files, just copy over the old into new structs.
    // If it turns out that we get a copy of the pointers (I'm not sure that we will), then just use
    // fread with the resized_ variables 
    BITMAPFILEHEADER resized_bf = bf;
    BITMAPINFOHEADER resized_bi = bi;
    
    // Adjust BITMAPFILEHEADER and BITMAPINFOHEADER relevant fields to reflect resized image
    resized_bi.biWidth  = bi.biWidth*factor;
    resized_bi.biHeight = bi.biHeight*factor;
    resized_bi.biSizeImage = bi.biSizeImage*factor;
    resized_bf.bfSize = resized_bi.biWidth*resized_bi.biHeight + 54; // Full image size
    
    // determine padding for scanlines
    int outpadding = (4 - (resized_bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    // Write outfile's BITMAPFILEHEADER
    fwrite(&resized_bf, sizeof(BITMAPFILEHEADER), 1, outfile);

    // Write outfile's BITMAPINFOHEADER
    fwrite(&resized_bi, sizeof(BITMAPINFOHEADER), 1, outfile);

    // Since our resizing rounds down, the factor f used in calculations has to be based on the actual ratio of pixels
    float f = bi.biHeight/resized_bi.biHeight;
    
    // A few variables we need
    double dx,dy,intpart;
    double fractional = modf(f,&intpart)/2; // Fractional component of FACTOR for calculations
    RGBTRIPLE new_color; // temporary storage

    // Outer x,y loops are destination pixels. Inner i,j loops are source pixels.
    // We get coordinates for a destination pixel and then iterate over the corresponding source pixels, summing
    // the component colors and dividing by f^2 (because there is a f^2:1 ratio of source:destination pixels)
    for (int x=0; x<abs(resized_bi.biHeight); x++){
        for (int y=0; y<resized_bi.biWidth; y++){
            
            // Reset component colors
            new_color.rgbtBlue  = 0;
            new_color.rgbtGreen = 0;
            new_color.rgbtRed   = 0;
        
            // For each of the desination pixels, we iterate over some source pixels:
            for (int i=x*f; i<(int)(x*f+f);i++){

                for (int j=y*f; j<(int)(y*f+f);j++){ 
                    
                    RGBTRIPLE triple; //Temporary storage
                    
                    fseek(infile, j*3+i*bi.biWidth*3 + 54, SEEK_SET); // Set source file position in terms of i and j
                    fread(&triple, sizeof(RGBTRIPLE), 1, infile);     // Read source pixel, store in triple
                     
                    // Destination pixel can be created out of partial source pixels, so we use dx and dy to account for fractions
                    
                    // Cases for dx and dy 
                    /// !!!!!! There is a bug here. When destination width or height can be fully divided by factor F, we !!!!!!
                    //  !!!!!! start and stop at different points depending on the count. For instance 6/1.5 = 4; the first
                    //  !!!!!! destination pixel covers source pixels 0:1.5, but the next covers 1.5:3, etc. How to set up
                    //  !!!!!! to adjust dx and dy dynamically?
                    
                    //  !!!!!! This situation also creates another bug with the i and j FOR loops above. For instance, again,
                    //  !!!!!! if we're going from 6 pixels to 4, the 1st destination pixel needs color from source pixel 0 and 1, 
                    //  !!!!!! the 2nd dest pixel needs color from source 1 and 2, but the 3rd needs color from 3 and 4. It's
                    //  !!!!!! because of the decimal-integer-decimal counting: 0 to 1.5 to 3 to 4.5 to 6 etc. 
                    if (fractional && (bi.biWidth%f)){
                        if (j==0 || j==((int)(y*f+f)-1)){
                            dx=fractional;
                        }
                    }else{
                        dx=1;
                    }
                    if (fractional && (bi.biHeight%f)){
                        if (i==0 || i==((int)(x*f+f)-1)){
                            dy=fractional;
                        }
                    }else{
                        dy=1;
                    }
                    
                    // For each source pixel, add the component destination pixel
                    new_color.rgbtBlue  += triple.rgbtBlue*dx*dy/(f*f);
                    new_color.rgbtGreen += triple.rgbtGreen*dx*dy/(f*f);
                    new_color.rgbtRed   += triple.rgbtRed*dx*dy/(f*f);
                }
            }
            
            // When i and j are done, store composite pixel into output image
            fwrite(&new_color, sizeof(RGBTRIPLE), 1, outfile);
        }
        // Add padding if necessary
        for (int k = 0; k < outpadding; k++)
        {
            fputc(0x00, outfile);
        }
        // Skip over padding, if any
        fseek(outfile, outpadding, SEEK_CUR);

    }


    // close infile
    fclose(infile);

    // close outfile
    fclose(outfile);

    // success
    return 0;
}
