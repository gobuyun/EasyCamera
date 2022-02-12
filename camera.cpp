#include "camera.h"

extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libavutil/time.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "SDL2/SDL.h"
}

#include <cstdio>
#include <malloc.h>
#include <iostream>

using namespace std;

#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)


int sfp_refresh_thread(void* opaque)
{
	while (1) {
		SDL_Event event;
		event.type = SFM_REFRESH_EVENT;
		SDL_PushEvent(&event);
		SDL_Delay(66); // 我的摄像头15fps
	}
	return 0;
}

camera::camera(std::string&& cameraName)
{
	avdevice_register_all();

	AVFormatContext* pFmtCtx = nullptr;
	AVInputFormat* ifmt = av_find_input_format("dshow");
	int errCode;
	std::string cameraUrl{"video="+ cameraName };
	if ((errCode = avformat_open_input(&pFmtCtx, cameraUrl.c_str(), ifmt, nullptr)) != 0)
	{
		std::cout << "avformat_open_input failed" << std::endl;
		return;
	}

	if ((errCode = avformat_find_stream_info(pFmtCtx, nullptr)) < 0)
	{
		std::cout << "avformat_find_stream_info" << endl;
		return;
	}

	av_dump_format(pFmtCtx, -1, nullptr, 0);

	// find video stream
	int videoInx = -1;
	AVCodecParameters* pVideoCodecParam = nullptr;
	for (int i = 0; i < pFmtCtx->nb_streams; ++i)
	{
		if (pFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			pVideoCodecParam = pFmtCtx->streams[i]->codecpar;
			videoInx = i;
			break;
		}
	}
	if (videoInx == -1)
	{
		std::cout << "not found video stream" << endl;
		return;
	}

	AVPacket *pReadPacket = av_packet_alloc();
	pReadPacket->buf = nullptr;
	pReadPacket->size = 0;
	AVFrame* pReadFrame = av_frame_alloc();
	av_image_alloc(pReadFrame->data, pReadFrame->linesize,
		pVideoCodecParam->width, pVideoCodecParam->height,
		(AVPixelFormat)pVideoCodecParam->format, 1);
	AVFrame* pOutFrame = av_frame_alloc();
	av_image_alloc(pOutFrame->data, pOutFrame->linesize, 
		pVideoCodecParam->width, pVideoCodecParam->height,
		(AVPixelFormat)pVideoCodecParam->format, 1);

	// SDL
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	SDL_Window* win = SDL_CreateWindow("Camera", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, pVideoCodecParam->width, pVideoCodecParam->height, SDL_WINDOW_OPENGL);
	SDL_Renderer* render = SDL_CreateRenderer(win, -1, 0);
	SDL_Texture* texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_YUY2, SDL_TEXTUREACCESS_STREAMING, pVideoCodecParam->width, pVideoCodecParam->height);
	SDL_Rect rect = {0, 0, pVideoCodecParam->width, pVideoCodecParam->height};
	SDL_Thread* video_tid = SDL_CreateThread(sfp_refresh_thread, "delayThread", NULL);
	SDL_Event event;

	struct SwsContext* img_convert_ctx;
	img_convert_ctx = sws_getContext(pVideoCodecParam->width, pVideoCodecParam->height,
		(AVPixelFormat)pVideoCodecParam->format,
		pVideoCodecParam->width, pVideoCodecParam->height,
		AV_PIX_FMT_YUYV422, 0, NULL, NULL, NULL);
	
	while (true)
	{
		SDL_WaitEvent(&event);
		if (event.type != (int)SFM_REFRESH_EVENT)
		{
			continue; 
		}
		if (av_read_frame(pFmtCtx, pReadPacket) < 0)
		{
			break;
		}
		if (pReadPacket->stream_index != videoInx)
		{
			continue;
		}
		pReadFrame->data[0] = pReadPacket->data;

		sws_scale(img_convert_ctx, (const unsigned char* const*)pReadFrame->data, pReadFrame->linesize, 0, pVideoCodecParam->height, pOutFrame->data, pOutFrame->linesize);
		SDL_RenderClear(render);
		SDL_UpdateTexture(texture, NULL, pOutFrame->data[0], pOutFrame->linesize[0]);
		SDL_RenderCopy(render, texture, NULL, &rect);
		SDL_RenderPresent(render);
	}	
}