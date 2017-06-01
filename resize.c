/**
 * Copies a BMP piece by piece, just because.
 */
       
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

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
    
    // Read infile's BITMAPFILEHEADER
    //BITMAPFILEHEADER bf;
    //fread(&bf, sizeof(BITMAPFILEHEADER), 1, infile);

    // Read infile's BITMAPINFOHEADER
    //BITMAPINFOHEADER bi;
    //fread(&bi, sizeof(BITMAPINFOHEADER), 1, infile);

    // Ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 || 
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outfile);
        fclose(infile);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }
    
    ///////////////////////////////////////////////////
    ///          The fun starts here                ///
    ///////////////////////////////////////////////////
    
    
    // Since we will need to make changes to the header files, just copy over the old into new structs.
    //BITMAPFILEHEADER resized_bf = bf;
    //BITMAPINFOHEADER resized_bi = bi;
    
    // Adjust BITMAPFILEHEADER and BITMAPINFOHEADER relevant fields to reflect resized image
    //resized_bi.biWidth  = bi.biWidth*factor;
    //resized_bi.biHeight = bi.biHeight*factor;
    //resized_bi.biSizeImage = bi.biSizeImage*factor;
    //resized_bf.bfSize = resized_bi.biWidth*resized_bi.biHeight + 54; // Full image size
    
    // determine padding for scanlines
    //int outpadding = (4 - (resized_bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    // Write outfile's BITMAPFILEHEADER
    //fwrite(&resized_bf, sizeof(BITMAPFILEHEADER), 1, outfile);

    // Write outfile's BITMAPINFOHEADER
    //fwrite(&resized_bi, sizeof(BITMAPINFOHEADER), 1, outfile);

    // Since our resizing rounds down, the factor f used in calculations has to be based on the actual ratio of pixels
    //float f = bi.biHeight/(float)resized_bi.biHeight;
    //float f = 1.0/factor;
    
    // A few variables we need
    //double dx,dy,intpart;
    //double fractional = modf(f,&intpart)/2; // Fractional component of FACTOR for calculations
    //RGBTRIPLE new_color; // temporary storage
    
    // Buffer of RGBTRIPLEs to store the input file
    RGBTRIPLE infile_buffer[(bf.bfSize)/3];
    
    // New array for output file, taking into account scaling factor
    int new_size = (bf.bfSize-54)*factor+54;
    RGBTRIPLE outfile_buffer[new_size/3];
   
    // Read input file pixels into memory
    fread(&infile_buffer, bf.bfSize, 1, infile);
    fclose(infile);     // Done with input file at this point
    
    // Copy infile buffer to outfile buffer
    memcpy(outfile_buffer,infile_buffer,sizeof(outfile_buffer));
    
    // Now we need to do some adjustments on the outfile_buffer to resize our image
    // Need to adjust the headers => width, height, and size for bi and bf
    
    
    
    // fwrite(&outfile_buffer, bf.bfSize,1,outfile); // it works!
    
    
    
    
    
    
    






    // close outfile
    fclose(outfile);

    // success
    return 0;
}
