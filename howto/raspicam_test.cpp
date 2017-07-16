/**********************************************************
 Software developed by AVA ( Ava Group of the University of Cordoba, ava  at uco dot es)
 Main author Rafael Munoz Salinas (rmsalinas at uco dot es)
 This software is released under BSD license as expressed below
-------------------------------------------------------------------
Copyright (c) 2013, AVA ( Ava Group University of Cordoba, ava  at uco dot es)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software
   must display the following acknowledgement:

   This product includes software developed by the Ava group of the University of Cordoba.

4. Neither the name of the University nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY AVA ''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL AVA BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****************************************************************/

#include <iostream>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <sys/timeb.h>
#include <png.h>
#include "raspicam.h"
#include <jpeglib.h>
#include <stdio.h>

using namespace std;
bool doTestSpeedOnly=false;
size_t nFramesCaptured=100;
//parse command line
//returns the index of a command line param in argv. If not found, return -1



void write_JPEG_file (string filename,unsigned char *data,raspicam::RaspiCam &Camera, int quality) {

  struct jpeg_compress_struct cinfo;

  struct jpeg_error_mgr jerr;
  FILE * outfile;
  JSAMPROW row_pointer[1];
  int row_stride;

  /* Step 1: allocate and initialize JPEG compression object */
  cinfo.err = jpeg_std_error(&jerr);
  /* Now we can initialize the JPEG compression object. */
  jpeg_create_compress(&cinfo);

  /* Step 2: specify data destination (eg, a file) */
  /* Note: steps 2 and 3 can be done in either order. */

  if ((outfile = fopen(filename.c_str(), "wb")) == NULL) {
    cout << "can't open "<<filename<<endl;
    exit(1);
  }
  jpeg_stdio_dest(&cinfo, outfile);

  /* Step 3: set parameters for compression */

  /* First we supply a description of the input image.
   * Four fields of the cinfo struct must be filled in:
   */
  cinfo.image_width = Camera.getWidth(); 	/* image width and height, in pixels */
  cinfo.image_height = Camera.getHeight();
  cinfo.input_components = 3;		/* # of color components per pixel */
  cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

  /* Step 4: Start compressor */

  /* TRUE ensures that we will write a complete interchange-JPEG file.
   * Pass TRUE unless you are very sure of what you're doing.
   */
  jpeg_start_compress(&cinfo, TRUE);

  /* Step 5: while (scan lines remain to be written) */
  /*           jpeg_write_scanlines(...); */

  /* Here we use the library's state variable cinfo.next_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   * To keep things simple, we pass one scanline per call; you can pass
   * more if you wish, though.
   */
  row_stride = Camera.getWidth() * 3;	/* JSAMPLEs per row in image_buffer */

  while (cinfo.next_scanline < cinfo.image_height) {
    /* jpeg_write_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could pass
     * more than one scanline at a time if that's more convenient.
     */
    row_pointer[0] = & data[cinfo.next_scanline * row_stride];
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  /* Step 6: Finish compression */

  jpeg_finish_compress(&cinfo);
  /* After finish_compress, we can close the output file. */
  fclose(outfile);

  /* Step 7: release JPEG compression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_compress(&cinfo);

  /* And we're done! */
}


void write_png_file(string filepath,unsigned char *data,raspicam::RaspiCam &Camera)
{
	int width=Camera.getWidth();
	int height=Camera.getHeight();
png_byte color_type = PNG_COLOR_TYPE_RGBA;
png_byte bit_depth = 8;
	png_structp png_ptr;
png_infop info_ptr;
        /* create file */
        FILE *fp = fopen(filepath.c_str(), "wb");
        if (!fp)
                cout <<"[write_png_file] File "<<filepath<<" could not be opened for writing"<<endl;


        /* initialize stuff */
        png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

        if (!png_ptr)
                cout <<"[write_png_file] png_create_write_struct failed"<<endl;

        info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr)
                cout << "[write_png_file] png_create_info_struct failed"<<endl;

        if (setjmp(png_jmpbuf(png_ptr)))
                cout << "[write_png_file] Error during init_io"<<endl;

        png_init_io(png_ptr, fp);


        /* write header */
        if (setjmp(png_jmpbuf(png_ptr)))
                cout <<"[write_png_file] Error during writing header"<<endl;

        png_set_IHDR(png_ptr, info_ptr, width, height,
                     bit_depth, color_type, PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

        png_write_info(png_ptr, info_ptr);


        /* write bytes */
        if (setjmp(png_jmpbuf(png_ptr)))
                cout <<"[write_png_file] Error during writing bytes"<<endl;

png_bytep * row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
        for (int y=0; y<height; y++) {
                row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));
		png_byte* row = row_pointers[y];
		for (int x=0;x<width;x++) {
			png_byte* ptr = &(row[x*4]);
			ptr[0]=data[(y*width*3)+x*3];
			ptr[1]=data[(y*width*3)+(x*3)+1];
			ptr[2]=data[(y*width*3)+(x*3)+2];
			ptr[3]=255;
		}
	}
        png_write_image(png_ptr, row_pointers);


        /* end write */
        if (setjmp(png_jmpbuf(png_ptr)))
                cout << "[write_png_file] Error during end of write"<<endl;

        png_write_end(png_ptr, NULL);

        /* cleanup heap allocation */
        /*for (y=0; y<height; y++)
                free(row_pointers[y]);
        free(row_pointers);*/

        fclose(fp);
}



int findParam ( string param,int argc,char **argv ) {
    int idx=-1;
    for ( int i=0; i<argc && idx==-1; i++ )
        if ( string ( argv[i] ) ==param ) idx=i;
    return idx;

}
//parse command line
//returns the value of a command line param. If not found, defvalue is returned
float getParamVal ( string param,int argc,char **argv,float defvalue=-1 ) {
    int idx=-1;
    for ( int i=0; i<argc && idx==-1; i++ )
        if ( string ( argv[i] ) ==param ) idx=i;
    if ( idx==-1 ) return defvalue;
    else return atof ( argv[  idx+1] );
}

raspicam::RASPICAM_EXPOSURE getExposureFromString ( string str ) {
    if ( str=="OFF" ) return raspicam::RASPICAM_EXPOSURE_OFF;
    if ( str=="AUTO" ) return raspicam::RASPICAM_EXPOSURE_AUTO;
    if ( str=="NIGHT" ) return raspicam::RASPICAM_EXPOSURE_NIGHT;
    if ( str=="NIGHTPREVIEW" ) return raspicam::RASPICAM_EXPOSURE_NIGHTPREVIEW;
    if ( str=="BACKLIGHT" ) return raspicam::RASPICAM_EXPOSURE_BACKLIGHT;
    if ( str=="SPOTLIGHT" ) return raspicam::RASPICAM_EXPOSURE_SPOTLIGHT;
    if ( str=="SPORTS" ) return raspicam::RASPICAM_EXPOSURE_SPORTS;
    if ( str=="SNOW" ) return raspicam::RASPICAM_EXPOSURE_SNOW;
    if ( str=="BEACH" ) return raspicam::RASPICAM_EXPOSURE_BEACH;
    if ( str=="VERYLONG" ) return raspicam::RASPICAM_EXPOSURE_VERYLONG;
    if ( str=="FIXEDFPS" ) return raspicam::RASPICAM_EXPOSURE_FIXEDFPS;
    if ( str=="ANTISHAKE" ) return raspicam::RASPICAM_EXPOSURE_ANTISHAKE;
    if ( str=="FIREWORKS" ) return raspicam::RASPICAM_EXPOSURE_FIREWORKS;
    return raspicam::RASPICAM_EXPOSURE_AUTO;
}

raspicam::RASPICAM_AWB getAwbFromString ( string str ) {
if ( str=="OFF" ) return raspicam::RASPICAM_AWB_OFF;
if ( str=="AUTO" ) return raspicam::RASPICAM_AWB_AUTO;
if ( str=="SUNLIGHT" ) return raspicam::RASPICAM_AWB_SUNLIGHT;
if ( str=="CLOUDY" ) return raspicam::RASPICAM_AWB_CLOUDY;
if ( str=="SHADE" ) return raspicam::RASPICAM_AWB_SHADE;
if ( str=="TUNGSTEN" ) return raspicam::RASPICAM_AWB_TUNGSTEN;
if ( str=="FLUORESCENT" ) return raspicam::RASPICAM_AWB_FLUORESCENT;
if ( str=="INCANDESCENT" ) return raspicam::RASPICAM_AWB_INCANDESCENT;
if ( str=="FLASH" ) return raspicam::RASPICAM_AWB_FLASH;
if ( str=="HORIZON" ) return raspicam::RASPICAM_AWB_HORIZON;
return raspicam::RASPICAM_AWB_AUTO;
}
void processCommandLine ( int argc,char **argv,raspicam::RaspiCam &Camera ) {
    Camera.setWidth ( getParamVal ( "-w",argc,argv,1280 ) );
    Camera.setHeight ( getParamVal ( "-h",argc,argv,960 ) );
    Camera.setBrightness ( getParamVal ( "-br",argc,argv,50 ) );

    Camera.setSharpness ( getParamVal ( "-sh",argc,argv,0 ) );
    Camera.setContrast ( getParamVal ( "-co",argc,argv,0 ) );
    Camera.setSaturation ( getParamVal ( "-sa",argc,argv,0 ) );
    Camera.setShutterSpeed( getParamVal ( "-ss",argc,argv,0 ) );
    Camera.setISO ( getParamVal ( "-iso",argc,argv ,400 ) );
    if ( findParam ( "-vs",argc,argv ) !=-1 )
        Camera.setVideoStabilization ( true );
    Camera.setExposureCompensation ( getParamVal ( "-ec",argc,argv ,0 ) );

    if ( findParam ( "-gr",argc,argv ) !=-1 )
      Camera.setFormat(raspicam::RASPICAM_FORMAT_GRAY);
    if ( findParam ( "-yuv",argc,argv ) !=-1 ) 
      Camera.setFormat(raspicam::RASPICAM_FORMAT_YUV420);
    if ( findParam ( "-test_speed",argc,argv ) !=-1 )
        doTestSpeedOnly=true;
    int idx;
    if ( ( idx=findParam ( "-ex",argc,argv ) ) !=-1 )
        Camera.setExposure ( getExposureFromString ( argv[idx+1] ) );
    if ( ( idx=findParam ( "-awb",argc,argv ) ) !=-1 )
        Camera.setAWB( getAwbFromString ( argv[idx+1] ) );
    nFramesCaptured=getParamVal("-nframes",argc,argv,100);
    Camera.setAWB_RB(getParamVal("-awb_b",argc,argv ,1), getParamVal("-awb_g",argc,argv ,1));

}
void showUsage() {
    cout<<"Usage: "<<endl;
    cout<<"[-help shows this help]\n"<<endl;
    cout<<"[-gr sets gray color mode]\n"<<endl;
    cout<<"[-test_speed use for test speed and no images will be saved]\n";
    cout<<"[-yuv sets yuv420 color mode]\n"<<endl;
    cout<<"[-w width] [-h height] \n[-br brightness_val(0,100)]\n[-sh  sharpness_val (-100 to 100)]\n";
    cout<<"[-co contrast_val (-100 to 100)]\n[-sa saturation_val (-100 to 100)]\n";
    cout<<"[-iso ISO_val  (100 to 800)]\n[-vs turns on video stabilisation]\n[-ec exposure_compensation_value(-10,10)]\n";
    cout<<"[-ss shutter_speed (value in microsecs (max 330000)]\n[-ec exposure_compensation_value(-10,10)]\n";
    cout<<"[-exp mode (OFF,AUTO,NIGHT,NIGHTPREVIEW,BACKLIGHT,SPOTLIGHT,SPORTS,SNOW,BEACH,VERYLONG,FIXEDFPS,ANTISHAKE,FIREWORKS)]"<<endl;
    cout<<"[-awb (OFF,AUTO,SUNLIGHT,CLOUDY,TUNGSTEN,FLUORESCENT,INCANDESCENT,FLASH,HORIZON)]"<<endl;
    cout<<"[-nframes val: number of frames captured (100 default). 0 == Infinite lopp]\n";
    cout<<"[-awb_r val:(0,8):set the value for the red component of white balance]"<<endl;
    cout<<"[-awb_g val:(0,8):set the value for the green component of white balance]"<<endl;

    cout<<endl;
}

//timer functions
#include <sys/time.h>
#include <unistd.h>
class Timer{
    private:
    struct timeval _start, _end;

public:
    Timer(){}
    void start(){
        gettimeofday(&_start, NULL);
    }
    void end(){
        gettimeofday(&_end, NULL);
    }
    double getSecs(){
    return double(((_end.tv_sec  - _start.tv_sec) * 1000 + (_end.tv_usec - _start.tv_usec)/1000.0) + 0.5)/1000.;
    }

};

void saveImage ( string filepath,unsigned char *data,raspicam::RaspiCam &Camera ) {
    std::ofstream outFile ( filepath.c_str(),std::ios::binary );
    if ( Camera.getFormat()==raspicam::RASPICAM_FORMAT_BGR ||  Camera.getFormat()==raspicam::RASPICAM_FORMAT_RGB ) {
        outFile<<"P6\n";
    } else if ( Camera.getFormat()==raspicam::RASPICAM_FORMAT_GRAY ) {
        outFile<<"P5\n";
    } else if ( Camera.getFormat()==raspicam::RASPICAM_FORMAT_YUV420 ) { //made up format
        outFile<<"P7\n";
    }
    outFile<<Camera.getWidth() <<" "<<Camera.getHeight() <<" 255\n";
    outFile.write ( ( char* ) data,Camera.getImageBufferSize() );
}


int main ( int argc,char **argv ) {
    if ( argc==1 ) {
        cerr<<"Usage (-help for help)"<<endl;
    }

    if ( findParam ( "-help",argc,argv ) !=-1) {
        showUsage();
        return -1;
    }

  
    raspicam::RaspiCam Camera;
    processCommandLine ( argc,argv,Camera );
    cout<<"Connecting to camera"<<endl;
    
    if ( !Camera.open() ) {
        cerr<<"Error opening camera"<<endl;
        return -1;
    }
    cout<<"Connected to camera ="<<Camera.getId() <<" bufs="<<Camera.getImageBufferSize( )<<endl;
    unsigned char *data=new unsigned char[  Camera.getImageBufferSize( )];
    Timer timer;


    cout<<"Capturing...."<<Camera.getWidth()<<"x"<<Camera.getHeight()<<" "<<Camera.getImageBufferSize( )<<endl;
    size_t i=0;
    timer.start();
 do{
        Camera.grab();
        Camera.retrieve ( data );
        if ( !doTestSpeedOnly ) {
            if ( i%5==0 ) 	  cout<<"\r capturing ..."<<i<<"/"<<nFramesCaptured<<std::flush;
            if ( i%30==0 && i!=0  && nFramesCaptured>0 ) { //save image if not in inifite loop
                std::stringstream fn;
                fn<<"image";
		if (i<10) fn<<"0";
		fn<<i<<".jpg";
		//write_png_file( fn.str(),data,Camera );
		write_JPEG_file( fn.str(),data,Camera,80 );
                //saveImage ( fn.str(),data,Camera );
		cerr<<"Saving "<<fn.str()<<endl;
            }
        }
    }while(++i<nFramesCaptured || nFramesCaptured==0);//stops when nFrames captured or at infinity lpif nFramesCaptured<0

    timer.end();
    if ( !doTestSpeedOnly )    cout<<endl<<"Images saved in imagexx.ppm"<<endl;



    cerr<< timer.getSecs()<< " seconds for "<< nFramesCaptured<< "  frames : FPS " << ( ( float ) ( nFramesCaptured ) / timer.getSecs() ) <<endl;

    Camera.release();


}
