// **********************************************************************************
//
// BSD License.
//
// Copyright (c) 2019, Bruno Keymolen, email: bruno.keymolen@gmail.com
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this
// list of conditions and the following disclaimer in the documentation and/or other
// materials provided with the distribution.
// Neither the name of "Bruno Keymolen" nor the names of its contributors may be
// used to endorse or promote products derived from this software without specific
// prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// **********************************************************************************


#include <stdlib.h>
#include <sys/stat.h> 
#include <fcntl.h>


extern "C" {
  #include "libavutil/common.h"
  #include "libavcodec/avcodec.h"
  #include "libswscale/swscale.h"
}
 
#include <opencv2/highgui/highgui_c.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"


#include "vdecoder.hpp"



const char* CW_IMG_FRAMES 	= "Decoded";

int _fd = 0;


static int read_cb(unsigned char* buf, int size, void *usr)
{
  //we can read from a stream, internal memory and such
  //for this demo we read from a file _fd
  return read(_fd, buf, size);
}


//https://timvanoosterhout.wordpress.com/2015/07/02/converting-an-ffmpeg-avframe-to-and-opencv-mat/
void avframeToMat(const AVFrame * frame, cv::Mat& image)
{
  int width = frame->width;
  int height = frame->height;

  // Allocate the opencv mat and store its stride in a 1-element array
  if (image.rows != height || image.cols != width || image.type() != CV_8UC3) image = cv::Mat(height, width, CV_8UC3);
  int cvLinesizes[1];
  cvLinesizes[0] = image.step1();

  // Convert the colour format and write directly to the opencv matrix
  SwsContext* conversion = sws_getContext(width, height, (AVPixelFormat) frame->format, width, height, AV_PIX_FMT_BGR24, SWS_FAST_BILINEAR, NULL, NULL, NULL);
  sws_scale(conversion, frame->data, frame->linesize, 0, height, &image.data, cvLinesizes);
  sws_freeContext(conversion);
}



static void frame_cb(AVFrame *frame, void *usr)
{
  LOG_DBG("frame: " << frame->coded_picture_number << ", "  << frame->width << " x " << frame->height << " " << frame->key_frame);
   
  cv::Mat cv_mat(frame->height, frame->width, CV_8UC3);//, frame->data[0], frame->linesize[0]);
  avframeToMat(frame, cv_mat);
  imshow(CW_IMG_FRAMES, cv_mat);
  cv::waitKey(1);
}




int process_file(keymolen::VDecoder& decoder, const char *path)
{
  _fd = open(path, O_RDONLY);
  if (_fd <= 0)
  {
    LOG_ERR("could not open " << path);
    return -1;
  }

  LOG_DBG("_fd: " << _fd);

  if(decoder.setup(AV_CODEC_ID_H264, &read_cb, 0) !=0)
  {
    return -1;
  }

  decoder.decode(&frame_cb, 0);

  decoder.reset();

  close(_fd);

  return 0;
}


int main(int argc, char *argv[])
{

  cv::namedWindow(CW_IMG_FRAMES, cv::WINDOW_AUTOSIZE);
  cv::moveWindow(CW_IMG_FRAMES, 10, 10);

  keymolen::VDecoder decoder;
  process_file(decoder, "my_first_video.mkv");
  process_file(decoder, "my_second_video.mkv"); 

  LOG_DBG("done...");

  return 0;
}

