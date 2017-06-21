/**
 * Copies a BMP piece by piece, just because.
 */
       
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "bmp.h"

BITMAPFILEHEADER resized_bf; // global scope for functions
BITMAPINFOHEADER resized_bi;
int outpadding; // same with outpadding
float factor;

void row_copy(BYTE outfile_buffer[resized_bi.biHeight][resized_bi.biWidth*3+outpadding], int row);

int main(int argc, char *argv[])
{
    
    ///////////////////////////////////////////////////
    /// Basic error checking for the correct inputs ///
    ///////////////////////////////////////////////////
    if (argc != 4)
    {
        fprintf(stderr, "Usage: ./resize [f] infile outfile\n");
        return 1;
    }

    float f = atof(argv[1]);
    if(modff(f,&factor)){
        fprintf(stderr, "Multiplication factor can only be an integer\n");
        return 2;
    }
    
    if (factor<1 || factor>100)
    {
        fprintf(stderr, "Multiplication factor must be in [1,100]\n");
        return 3;
    }
    
    // open input file 
    FILE *infile = fopen(argv[2],"r");
    if (infile==NULL){
        fprintf(stderr,"Could not open %s for reading\n",argv[1]);
        return 4;
    }
    
    // open output file
    FILE *outfile = fopen(argv[3],"w");
    if (outfile==NULL){
        fprintf(stderr,"Could not open %s for writing\n",argv[2]);
        return 5;
    }
    
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
    
    ///////////////////////////////////////////////////
    ///          The fun starts here                ///
    ///////////////////////////////////////////////////
    
    // We will need to make changes to the header files, just copy over old into new (declared above globally)
    resized_bf = bf;
    resized_bi = bi;
    
    // Adjust relevant fields to reflect resized image.
    resized_bi.biWidth     = bi.biWidth*factor;
    resized_bi.biHeight    = bi.biHeight*factor;
    resized_bi.biSizeImage = bi.biSizeImage*factor;
    resized_bf.bfSize      = resized_bi.biWidth*abs(resized_bi.biHeight)*3 + 54; // Full image size. width/height are in pixels, so *3
    
    // determine padding for both buffers
    outpadding = (4 - (resized_bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    int inpadding =  (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    // buffer of 8-bit datawords. 3x columns since each pixel is 3 bytes
    BYTE infile_buffer[abs(bi.biHeight)][bi.biWidth*3+inpadding];
    
    // buffer for outfile, taking into account scaling factor
    BYTE outfile_buffer[abs(resized_bi.biHeight)][resized_bi.biWidth*3+outpadding];
   
    // read input file into RAM
    fread(&infile_buffer, sizeof(infile_buffer), 1, infile); // Infile_buffer now contains all pixels from infile
    fclose(infile);                                          // Done with input file at this point. SPEEEEEEEEEEEEED

    // TEST STEP - Copy infile buffer to outfile buffer
    // memcpy(outfile_buffer,infile_buffer,sizeof(outfile_buffer));

    // write output image's headers
    fwrite(&resized_bf, sizeof(BITMAPFILEHEADER), 1, outfile);
    fwrite(&resized_bi, sizeof(BITMAPINFOHEADER), 1, outfile);
    
    
    // now we need to do some adjustments on the pixels to properly resize the image
    // a few variables
    int outcol=0;
    int incol=0;
    int outrow=0;
    int inrow=0;


    // for each row: multiply the columns correctly based on f first,
    // then duplicate the rows downward to fill the buffer
    for (int i=0;i<abs(bi.biHeight);i++){     // loop through rows
        for (int j=0;j<bi.biWidth;j++){       // loop through cols on each row
            for (int n=0;n<factor;n++){
                
                outfile_buffer[outrow][outcol]   = infile_buffer[inrow][incol];  // Copy over pixels in 1byte chunks
                outfile_buffer[outrow][outcol+1] = infile_buffer[inrow][incol+1];
                outfile_buffer[outrow][outcol+2] = infile_buffer[inrow][incol+2];
                outcol+=3; // next pixel in outfile
            }
            incol+=3; // next pixel in infile
        }
        
        // add padding
        for (int k=0; k<outpadding;k++)
        {
            outfile_buffer[outrow][resized_bi.biWidth*3+k] = 0x00;
        }
        
        // call row copy function if we need to duplicate pixels when factor>1
        if(factor>1){
            row_copy(outfile_buffer,outrow); // copy current row (factor) times
        }
        
        // set up variables for next round
        outrow+=factor;
        inrow+=1;
        outcol=0;
        incol=0;
    }
    
    // write the entire output file in 1 access
    fwrite(&outfile_buffer, sizeof(outfile_buffer), 1, outfile);

    // close outfile
    fclose(outfile);

    // success
    return 0;
}

// row copy function. copies row from outfile_buffer (factor) times into subsequent row spaces
void row_copy(BYTE outfile_buffer[abs(resized_bi.biHeight)][resized_bi.biWidth*3+outpadding], int row){

    // copy for f>=2
    for (int n=1;n<factor;n++){
        for (int i=0;i<(resized_bi.biWidth*3+outpadding);i++){ // want to include padding during row copy
            outfile_buffer[row+n][i] = outfile_buffer[row][i]; // copy the row 1 col at a time
        }
    }
}