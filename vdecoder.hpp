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

#ifndef KEYMOLEN_VDECODER_HPP
#define KEYMOLEN_VDECODER_HPP

#include "common.hpp"


#include <istream> 
#include <iostream>


extern "C" {
  #include "libavcodec/avcodec.h"
  #include "libavutil/common.h"
  #include "libavutil/imgutils.h"
  #include "libavutil/mathematics.h"
  #include "libavformat/avformat.h"
  #include "libavformat/avio.h"
}


namespace keymolen
{
  
  class VDecoder
  {
  private:
    const static int FILE_STREAM_BUFFER_SIZE = 8192;
  public:
    VDecoder();
    virtual ~VDecoder();
  public:
    int setup(enum AVCodecID codec_id, int (*read_cb)(unsigned char* data, int size, void *usr), void* usr);
    int decode(void (frame_cb)(AVFrame *frame, void *usr), void* usr);
    int reset();
  private:
    unsigned char* file_stream_buffer_;
    AVIOContext* io_context_;
    AVFormatContext* format_context_;
  private:
    static int _file_stream_read(void *usr, uint8_t *buf, int buf_size);
    int file_stream_read(uint8_t *buf, int buf_size);
    int decode(AVPacket *pkt);
    int setup();
  private:
    enum AVCodecID codec_id_;
    const AVCodec *codec_;
    AVCodecParserContext *parser_;
    AVCodecContext *codec_ctx_;
    AVFrame *frame_;
    struct
    {
      void (*cb)(AVFrame *frame, void *usr);
      void* usr;
    } decode_cb_;
    struct
    {
      int (*cb)(unsigned char* data, int size, void *usr);
      void* usr;
    } read_cb_;
  };


}


#endif


