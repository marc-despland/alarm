#include "log.h"
#include <jpeglib.h>


Camera::Camera() {
	this->camera_init=false;
	this->data=NULL;
}

void Camera::release() {
	if (this->camera_init) {
		delete[] data;
		this->camera.release();
		this->camera_init=false;
		this->data=NULL;
	}
}

void Camera::init() throw (CameraOpenException){
	if ( !this->camera.open() ) {
        Log::logger->log("CAMERA",ERROR) << "Failed to open the Camera" << endl;
        throw CameraOpenException();
    }
    Log::logger->log("CAMERA",DEBUG) <<"Connected to camera ="<<this->camera.getId() <<" bufs="<<this->camera.getImageBufferSize( )<<endl;
    this->data=new unsigned char[this->camera.getImageBufferSize( )];
	this->camera_init=true;
}



void Camera::capture() throw (CameraOpenException){
	if (!this->camera_init) {
		this->init();
	}
	this->camera.grab();
    this->camera.retrieve ( data );
}

unsigned char * Camera::toJpeg (unsigned long *mem_size,int quality) {

  struct jpeg_compress_struct cinfo;
  unsigned char *mem = NULL;

  struct jpeg_error_mgr jerr;
  JSAMPROW row_pointer[1];
  int row_stride;
  /* Step 1: allocate and initialize JPEG compression object */
  cinfo.err = jpeg_std_error(&jerr);
  /* Now we can initialize the JPEG compression object. */
  jpeg_create_compress(&cinfo);

  /* Step 2: specify data destination (eg, a file) */
  jpeg_mem_dest(&cinfo, &mem, mem_size);

  /* Step 3: set parameters for compression */

  cinfo.image_width = this->camera.getWidth(); 	/* image width and height, in pixels */
  cinfo.image_height = this->camera.getHeight();
  cinfo.input_components = 3;		/* # of color components per pixel */
  cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

  /* Step 4: Start compressor */
  jpeg_start_compress(&cinfo, TRUE);

  /* Step 5: while (scan lines remain to be written) */
  row_stride = this->camera.getWidth() * 3;	/* JSAMPLEs per row in image_buffer */

  while (cinfo.next_scanline < cinfo.image_height) {
    row_pointer[0] = & this->data[cinfo.next_scanline * row_stride];
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  /* Step 6: Finish compression */

  jpeg_finish_compress(&cinfo);
 /* Step 7: release JPEG compression object */
  jpeg_destroy_compress(&cinfo);

}
