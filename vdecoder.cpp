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

//
// resources: 
// 
// demux containers (like mkv):
//  https://riptutorial.com/ffmpeg/example/30955/reading-from-memory
// decode audio/video stream:
//  https://libav.org/documentation/doxygen/master/decode_video_8c-example.html
//


#include <stdlib.h>


#include "vdecoder.hpp"


extern "C" {
  #include "libavutil/error.h"
}
 




namespace keymolen
{

  VDecoder::VDecoder() :
    io_context_(NULL),
    format_context_(NULL),
    codec_ctx_(NULL),
    frame_(NULL)
  {
      //muxer/demuxers
      av_register_all();
      //decoders
      avcodec_register_all();
  }

  VDecoder::~VDecoder()
  {
    reset();
  }

  int VDecoder::decode(AVPacket *pkt)
  {
    if (pkt->size)
    {
      //we have at least one frame, decode now
      int rs = avcodec_send_packet(codec_ctx_, pkt);
      if(rs < 0)
      {
        LOG_ERR("error sending packet for decoding. " << rs);
        return -1;
      }

      while(true)
      {
        int r = avcodec_receive_frame(codec_ctx_, frame_);
        if (r == AVERROR(EAGAIN))
        {
          //need more data, just return
          return 0;
        }
        else if (r == AVERROR_EOF)
        {
          char errstr[1024];
          av_strerror(r, errstr, 1024);

          LOG_ERR("avcodec_receive_frame error: " << r << " " << errstr);

          return 0;
        }
        else if (r < 0)
        {
          LOG_ERR("decode error");
          return -1;
        }

        decode_cb_.cb(frame_, decode_cb_.usr);
      }

    }
    else
    {
      LOG_DBG("need data!");
    }

    return 0;
  }




  int64_t _file_stream_seek(void *opaque, int64_t offset, int whence)
  {
    LOG_DBG("offset: " << offset << " whence: " << whence);
    return 0;
  }


  int VDecoder::_file_stream_read(void *usr, uint8_t *buf, int buf_size)
  {
    return ((VDecoder*)usr)->file_stream_read(buf, buf_size);
  }


  int VDecoder::file_stream_read(uint8_t *buf, int buf_size)
  {
    return read_cb_.cb(buf, buf_size, read_cb_.usr);
  }


  int VDecoder::setup()
  {
    codec_ = avcodec_find_decoder(codec_id_);
    if (!codec_)
    {
      LOG_ERR("codec not found!");
      return -1;
    }
    LOG_DBG("codec: " << codec_->id);
    parser_ = av_parser_init(codec_->id);
    if (!parser_)
    {
      LOG_ERR("parser not found!");
      return -1;
    }
    codec_ctx_ = avcodec_alloc_context3(codec_);
   
    if (avcodec_open2(codec_ctx_, codec_, NULL) < 0) 
    {
      LOG_ERR("Could not open codec");
      return -1; 
    }

    frame_ = av_frame_alloc();
  
    return 0;
  }
  
  int VDecoder::setup(enum AVCodecID codec_id, int (read_cb)(unsigned char* data, int size, void *usr), void* usr)
  {
    reset();

    codec_id_ = codec_id;

    if(setup() != 0)
    {
      return -1;
    }

    read_cb_.usr = usr;
    read_cb_.cb = read_cb;


    if ((file_stream_buffer_ = (unsigned char*)av_malloc(FILE_STREAM_BUFFER_SIZE)) == NULL)
    {
      LOG_ERR("out of memory");
      return -1;
    }
    io_context_ = avio_alloc_context(
        file_stream_buffer_,
        FILE_STREAM_BUFFER_SIZE,
        0,
        (void*)this,
        &_file_stream_read,
        NULL,
        &_file_stream_seek);


    if (io_context_ == NULL)
    {
      LOG_ERR("out of memory");
      return -2;
    }
    format_context_ = avformat_alloc_context();
    format_context_->pb = io_context_;
    format_context_->flags |= AVFMT_FLAG_CUSTOM_IO;

    return 0;
  }

  int VDecoder::reset()
  {
    //container
    if (format_context_)
    {
      avformat_close_input(&format_context_);
      format_context_ = NULL;
    }
    if  (io_context_)
    {
      av_free(io_context_);
      io_context_ = NULL;
    }

    //codec
    if (codec_ctx_)
    {
      avcodec_free_context(&codec_ctx_);
      codec_ctx_ = NULL;
    }
    if (frame_)
    {
      av_frame_free(&frame_);
      frame_ = NULL;
    }

    return 0;
  }


  //
  //demux the container
  //
  int VDecoder::decode(void (*frame_cb)(AVFrame *frame, void *usr), void* usr)
  {
    decode_cb_.usr = usr;
    decode_cb_.cb = frame_cb;


    int r = 0;

    //the filename can be empty since we have custom IO
    if ((r = avformat_open_input(&format_context_, "", NULL, NULL)) < 0)
    {
      char errstr[1024];
      av_strerror(r, errstr, 1024);
      LOG_ERR("could not open_input, (r: " << r << ") " << errstr);
      return -1;
    }

    AVPacket av_packet;
    av_init_packet(&av_packet); 
    av_packet.data = NULL;


    r = 0;
    while (r == 0)
    {
      r = av_read_frame(format_context_, &av_packet);
      if (r != 0)
      {
        char errstr[1024];
        av_strerror(r, errstr, 1024);
        LOG_ERR("frame != 0  (r: " << r << ") " << errstr);
        break;
      }
   
      if (av_packet.size > 0)
      {
        decode(&av_packet);
      }
    }


    av_packet_unref(&av_packet);


    return 0;
  }




}


