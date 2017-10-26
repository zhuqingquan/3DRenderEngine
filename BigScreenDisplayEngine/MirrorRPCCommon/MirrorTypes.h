#pragma once

#ifndef _MIRROR_TYPES_H_
#define _MIRROR_TYPES_H_

namespace SOA{
	namespace Mirror{

		//像素枚举类型。
		enum PIXFormat
		{
			PIXFMT_UNKOWN = 0,
			PIXFMT_YUV420P,
			PIXFMT_YUY2,
			PIXFMT_A8R8G8B8,
			PIXFMT_R8G8B8,
			PIXFMT_NV12,
			PIXFMT_X8R8G8B8,

			PIXFMT_PCM = 100
		};

		const float SizeOfPixel[] = {0, 1.5, 2, 4, 3, 1.5, 4};

		/*
		 * 编解码，媒体流的枚举类型。
		 */
		enum Codec
		{
			CODEC_UNKOWN = 0,

			//video codec		//码流的编码格式。
			CODEC_H264 = 1,			//H264格式的码流类型
			CODEC_MPEG4,		//MPEG4格式的码流类型
			CODEC_MJPEG,		//MJPEG格式的码流类型
			CODEC_MPEG2_VIDEO = 100,//MPEG2视频的码流类型。
			CODEC_HIK,           //海康的码流类型

			//source codec		//媒体流数据源的类型。
			CODEC_H264_NAL = 4,		//NAL数据源类型
			CODEC_H264_RTP,		//H264格式的码流、RTP数据源类型
			CODEC_RTSP,			//RTSP数据源类型
			CODEC_MPEG4_RTP,	//MPEG4格式的码流、RTP数据源类型
			CODEC_MJPEG_RTP,	//MJPEG格式的码流、RTP数据源类型
			CODEC_MPEG2_RTP,	//MPEG2格式的码流、RTP数据源类型，此类型与MPEG2_ES不一样，有封装的格式。
			CODEC_MPEG2_ES_RTP,	//MP2格式的码流、RTP数据源类型，直接的MPEG2码流，没有封装格式。
			CODEC_HIK_AV,       //海康的媒体流类型，复合流

			//audio codec
			CODEC_G711_ALAW = 1000,
			CODEC_G711_ULAW,
			CODEC_MPEG2_AUDIO,	//MPEG2音频的码流类型。
			CODEC_AAC,
		};

		/*
		 * 颜色结构体
		 */
		struct Color
		{
			unsigned char a;
			unsigned char r;
			unsigned char g;
			unsigned char b;
		};

		/*
		 * 将32为整数值转换为Color对象
		 */
		inline Color ColorFrom(unsigned int color)
		{
			Color bigColor;
			bigColor.a = (unsigned char)((color & 0xff000000) >> 24);
			bigColor.r = (unsigned char)((color & 0x00ff0000) >> 16);
			bigColor.g = (unsigned char)((color & 0x0000ff00) >> 8);
			bigColor.b = (unsigned char)(color & 0x000000ff);
			return bigColor;
		}

		/*
		 * 将Color对象转换为32为整数值
		 */
		inline unsigned int ColorTo(const Color& color)
		{
			unsigned int result = 0;
			//memcpy(&result, &color, sizeof(result));
			unsigned int tmp = color.a;
			result = (result | (tmp << 24));
			tmp = color.r;
			result = (result | (tmp << 16));
			tmp = color.g;
			result = (result | (tmp << 8));
			tmp = color.b;
			result = (result | (tmp));
			return result;
		}

		/*
		 * 获取与sampleColor颜色差别最大的颜色
		 */
		inline Color getMaxDistinctionColor(const Color& sampleColor)
		{
			Color result;
			result.a = sampleColor.a;
			result.r = sampleColor.r>=128 ? 0x00 : 0xFF;
			result.g = sampleColor.g>=128 ? 0x00 : 0xFF;
			result.b = sampleColor.b>=128 ? 0x00 : 0xFF;
			return result;
		}

		/*
		 * RTSP媒体数据源的连接方式。
		 */
		enum ConnectionType
		{
			UnicastRtp = 0,
			MulticastRtp,
			OverHttp,
			Tcp
		};

		/*
		 * 数据流的类型。
		 */
		enum StreamType
		{
			STREAM_T_UNKOWN = 0,
			STREAM_T_VIDEO,
			STREAM_T_AUDIO,
			STREAM_T_MIXED//音视频混合
		};

		/*
		 * 控制帧率显示的定时器类型
		 */
		enum TimerType
		{
			TIMER_T_UNKOWN = 0,//让系统确定定时器的类型。
			TIMER_T_VIDEOCARD,//显卡定时器。
			TIMER_T_MEDIATIMER,//媒体软件定时器。
		};

		/*
		 * 绘画类型
		 */
		enum DrawType
		{
			DT_T_UNKOWN = 0,//
			DT_T_TEXTURE_ONLY,//只采用纹理绘画
			DT_T_SURFACE_ONLY,//只采用表面
			DT_T_SURFACE_TO_TEXTURE,//采用表面到纹理的绘画
		};

		struct BigScreenConfig
		{
			BigScreenConfig():pixFormatOfView(PIXFormat::PIXFMT_A8R8G8B8)
				,drawType(DrawType::DT_T_TEXTURE_ONLY)
				,bScaleForDraw(true)
				,timeType(TimerType::TIMER_T_VIDEOCARD)
				,refreshRate(60)
			{

			}
			PIXFormat pixFormatOfView;
			DrawType drawType;

			bool bScaleForDraw;
			TimerType timeType;
			int refreshRate;
		};

		/*
		 *	电视墙背景底图的显示模式
		 */
		enum BigScreenBackgroundMode
		{
			BS_BACKGROUNDMODE_Independent   = 0,		//独立模式
			BS_BACKGROUNDMODE_Merged		= 1,		//合并模式
		};
	}
}

#endif //_MIRROR_TYPES_H_