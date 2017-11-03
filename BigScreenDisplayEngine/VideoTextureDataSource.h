#pragma once
#ifndef _SOA_MIRROR_RENDER_VIDEO_TEXTURE_DATASOURCE_H_
#define _SOA_MIRROR_RENDER_VIDEO_TEXTURE_DATASOURCE_H_

#include "IDisplayContentProvider.h"
#include "DxRenderCommon.h"
#include <fstream>
#include <assert.h>

namespace zRender
{
	class IRawFrameTexture;
}

namespace SOA
{
namespace Mirror
{
namespace Render
{
	class VideoTextureDataSource : public zRender::TextureDataSource
	{
	public:
		VideoTextureDataSource(const char* fileName, zRender::PIXFormat pixFmt, int width, int height, int pitch)
			: m_isUpdatedIdentify(0)
			, m_frameHeight(0), m_frameWidth(0), m_framePixFmt(zRender::PIXFMT_UNKNOW), m_framePitch(0)
			, m_curFrameIndex(-1)
		{
			if(pixFmt<=0 || width<=0 || height<=0 || pitch<=0 || NULL==fileName)
				return;
			std::ifstream srcFileStream(fileName, std::ios::binary | std::ios::in);
			if(!srcFileStream)
			{
				assert(false);
				return;
			}
			int frameDataLen = pitch * height;
			unsigned char* frameData = NULL;
			while(srcFileStream)
			{
				frameData = (unsigned char*)malloc(frameDataLen);
				assert(frameData);
				srcFileStream.read((char*)frameData, frameDataLen);
				m_frameVector.push_back(frameData);
			}
			srcFileStream.close();
			if(m_frameVector.size()<=0)
			{
				free(frameData);
				return;
			}
			m_frameHeight = height;
			m_frameWidth = width;
			m_framePitch = pitch;
			m_framePixFmt = pixFmt;

			m_isUpdatedIdentify++;
			m_idtCount = 0;
			m_DrawedCount = 0;
			m_backupIdt = 0;
			m_reqCount = 0;
		}

		~VideoTextureDataSource()
		{
			m_curFrameIndex = -1;
			m_isUpdatedIdentify = 0;
			m_frameHeight = 0;
			m_frameWidth = 0;
			m_framePitch = 0;
			m_framePixFmt = zRender::PIXFMT_UNKNOW;
			for (int i = 0; i < m_frameVector.size(); i++)
			{
				free(m_frameVector[i]);
			}
			m_frameVector.clear();
		}

		bool isUpdated(int identify) const {return m_isUpdatedIdentify>identify;}

		int getTextureProfile(const zRender::RECT_f& textureReg, int& dataLen, int& yPitch, int& uPitch, int& vPitch, int& width, int& height, zRender::PIXFormat& pixelFmt)
		{
			if(textureReg.left<0 || textureReg.left>=1 || textureReg.top<0 || textureReg.top>=1
				|| textureReg.width() <= 0 || textureReg.height()<=0 || textureReg.right>1 ||textureReg.bottom>1)
			{
		#ifdef _DEBUG
				printf("Error in DisplayElement::setTextureDataSource : param [textureReg] is invalid.(L=%f, R=%f, T=%f, B=%f)\n",
					textureReg.left, textureReg.right, textureReg.top, textureReg.bottom);
		#endif
				return -1;
			}
			if(m_frameVector.size()<=0 || m_frameWidth==0 || m_frameHeight==0)
				return -2;
			float texReg_width = textureReg.width();
			float texReg_height = textureReg.height();
			float effReg_width = m_effectiveReg.width();
			float effReg_height = m_effectiveReg.height();
			float actual_width = texReg_width * effReg_width * m_frameWidth;
			float actual_height = texReg_height * effReg_height * m_frameHeight;
			width = actual_width;
			height = actual_height;
			yPitch = m_framePitch;
			dataLen = yPitch * height;
			pixelFmt = m_framePixFmt;
			return 0;
		}

		unsigned char* getData(int& dataLen, int& yPitch, int& uPitch, int& vPitch, int& width, int& height, zRender::PIXFormat& pixelFmt, RECT& effectReg, int& identify)
		{
			if(m_frameVector.size()<=0 || m_frameWidth==0 || m_frameHeight==0)
				return NULL;

			if(m_isUpdatedIdentify<=identify)
				return NULL;
			dataLen = m_framePitch * m_frameHeight;
			yPitch = m_framePitch;
			width = m_frameWidth;
			height = m_frameHeight;
			pixelFmt = m_framePixFmt;
			effectReg.left = m_frameWidth * m_effectiveReg.left + 0.5;
			effectReg.right = m_frameWidth * m_effectiveReg.right + 0.5;
			effectReg.top = m_frameHeight * m_effectiveReg.top + 0.5;
			effectReg.bottom = m_frameHeight * m_effectiveReg.bottom + 0.5;
			int frameIndex = m_curFrameIndex % m_frameVector.size();
			identify = m_isUpdatedIdentify;
			int t = m_DrawedCount + 1;
			if(m_backupIdt!=0 && t==m_idtCount)
			{
				InterlockedExchange((long*)&m_isUpdatedIdentify, m_backupIdt);
				InterlockedExchange((long*)&m_DrawedCount, 0);
				InterlockedExchange((long*)&m_backupIdt, 0);
			}
			InterlockedIncrement((long*)&m_DrawedCount);
			return m_frameVector[frameIndex];
		}
	
		zRender::SharedTexture* getSharedTexture(RECT& effectReg, int& identify)
		{
			return NULL;
		}

		zRender::IRawFrameTexture* getTexture() { return NULL; }

		int copyDataToTexture(const zRender::RECT_f& textureReg, unsigned char* dstTextureData, int pitch, int height, int& identify)
		{
			if(m_frameVector.size()<=0 || m_frameWidth==0 || m_frameHeight==0)
				return -1;

			if(m_isUpdatedIdentify<=identify)
				return 1;
			
			//if(m_isUpdatedIdentify==identify+1 || m_isUpdatedIdentify==identify+2)
			//	InterlockedIncrement((long*)&m_reqCount);
			//if(m_reqCount<m_idtCount)
			//	return 2;

			float texReg_width = textureReg.width();
			float texReg_height = textureReg.height();
			float effReg_width = m_effectiveReg.width();
			float effReg_height = m_effectiveReg.height();
			float actual_width = texReg_width * effReg_width * m_frameWidth;
			float actual_height = texReg_height * effReg_height * m_frameHeight;
			if(pitch<actual_width*2 || height<actual_height || NULL==dstTextureData)
			{
#ifdef _DEBUG
				printf("Error copyDataToTexture : param invalid.(pData=%d, pitch=%d, H=%d)\n",
					(int)dstTextureData, pitch, height);
#endif
				return -2;
			}

			//float fStartPosHrz = m_frameWidth * m_effectiveReg.left + actual_width * textureReg.left + 0.5;
			//float fStartPosVtc = m_frameHeight * m_effectiveReg.top + actual_height * textureReg.top + 0.5;
			float fStartPosHrz = m_frameWidth * (m_effectiveReg.left + m_effectiveReg.width()*textureReg.left) + 0.5;
			float fStartPosVtc = m_frameHeight * (m_effectiveReg.top + m_effectiveReg.height() * textureReg.top) + 0.5;
			int iStartPosHrz = fStartPosHrz;
			int iStartPosVtc = fStartPosVtc;
			int iEndPosVtc = iStartPosVtc+actual_height;
			int dataLenCopyed = actual_width*2;
			int frameIndex = m_curFrameIndex % m_frameVector.size();
			unsigned char* frameData = m_frameVector[frameIndex];
			for(int iVtc=iStartPosVtc; iVtc<iEndPosVtc; iVtc++)
			{				
				int dataPos = iVtc * m_framePitch + iStartPosHrz * 2;
				unsigned char* pVtcData = frameData + dataPos;
				memcpy(dstTextureData, pVtcData, dataLenCopyed);
				dstTextureData += pitch;
			}
			identify = m_isUpdatedIdentify;
			int t = m_DrawedCount + 1;
			//if(t==m_idtCount)
			//	m_reqCount = 0;

			if(m_backupIdt!=0 && t==m_idtCount)
			{
				InterlockedExchange((long*)&m_isUpdatedIdentify, m_backupIdt);
				InterlockedExchange((long*)&m_DrawedCount, 0);
				InterlockedExchange((long*)&m_backupIdt, 0);
			}
			InterlockedIncrement((long*)&m_DrawedCount);
			return 0;
		}

		int setEffectiveReg(const zRender::RECT_f& textureEffectiveReg)
		{
			if(textureEffectiveReg.left<0 || textureEffectiveReg.left>=1 || textureEffectiveReg.top<0 || textureEffectiveReg.top>=1
				|| textureEffectiveReg.width() <= 0 || textureEffectiveReg.height()<=0 || textureEffectiveReg.right>1 ||textureEffectiveReg.bottom>1)
			{
		#ifdef _DEBUG
				printf("Error in DisplayElement::setTextureDataSource : param [textureReg] is invalid.(L=%f, R=%f, T=%f, B=%f)\n",
					textureEffectiveReg.left, textureEffectiveReg.right, textureEffectiveReg.top, textureEffectiveReg.bottom);
		#endif
				return -1;
			}
			m_effectiveReg = textureEffectiveReg;
			m_isUpdatedIdentify++;
			return 0;
		}

		int draw()
		{
			//m_curFrameIndex = 0;
			//return 0;
			if(m_frameVector.size()<=0 || m_frameWidth==0 || m_frameHeight==0)
				return -1;
			if(m_DrawedCount>=m_idtCount)
			{
				m_DrawedCount = 0;
				m_backupIdt = 0;
				m_isUpdatedIdentify++;
			}
			else
			{
				int t = m_isUpdatedIdentify+1;
				InterlockedCompareExchange((long*)&t, t, m_isUpdatedIdentify+1);
				m_backupIdt = t;
			}
			m_curFrameIndex++;
			//int index = m_curFrameIndex + 1;
			//index %= m_frameVector.size();

			return 0;
		}

		void increaseAuthorization()
		{
			InterlockedIncrement((long*)&m_idtCount);
		}
		void decreaseAuthorization()
		{
			InterlockedDecrement((long*)&m_idtCount);
		}

	private:
		int m_isUpdatedIdentify;
		int m_idtCount;
		int m_DrawedCount;
		int m_reqCount;
		int m_backupIdt;

		zRender::RECT_f m_effectiveReg;
		int m_frameWidth;
		int m_frameHeight;
		zRender::PIXFormat m_framePixFmt;
		int m_framePitch;
		std::vector<unsigned char*> m_frameVector;
		int m_curFrameIndex;
	};

	class VideoTextureSourceUpdater
	{
	public:
		VideoTextureSourceUpdater(VideoTextureDataSource* src, int frameRate)
			: m_isRunning(false), thread(NULL)
			, m_src(src), m_frameRate(frameRate)
		{
		}

		~VideoTextureSourceUpdater()
		{
			stop();
		}

		int start()
		{
			if(m_src==NULL)
				return -1;
			if(thread)
				return 0;
			m_isRunning = true;
			thread = CreateThread(0, 0, doDrawFrameThreadWork, this, 0, 0);
			if(NULL==thread)
				return -1;
			return 0;
		}

		int stop()
		{
			if(NULL==thread)
				return 0;
			m_isRunning = false;
			WaitForSingleObject(thread, INFINITE);
			CloseHandle(thread);
			thread = NULL;
			return 0;
		}

		static DWORD WINAPI doDrawFrameThreadWork(LPVOID param);

		int drawFrame()
		{
			LARGE_INTEGER freq;
			LARGE_INTEGER liCur;
			LONGLONG lastTimeDrawed = 0;
			LONGLONG curTime = 0;
			QueryPerformanceFrequency(&freq);
			while (m_isRunning)
			{
				QueryPerformanceCounter(&liCur);
				if(lastTimeDrawed==0)
				{
					lastTimeDrawed = liCur.QuadPart * 1000 / freq.QuadPart;
					m_src->draw();
				}
				else
				{
					curTime = liCur.QuadPart * 1000 / freq.QuadPart;
					if(curTime - lastTimeDrawed > 1000 / m_frameRate)
					{
						m_src->draw();
						lastTimeDrawed = curTime;
					}
					else
					{

						Sleep(1);
					}
				}
			}
			return 0;
		}
	private:
		VideoTextureDataSource* m_src;
		HANDLE thread;
		bool m_isRunning;
		int m_frameRate;
	};

	DWORD WINAPI VideoTextureSourceUpdater::doDrawFrameThreadWork(LPVOID param)
	{
		VideoTextureSourceUpdater* updater = (VideoTextureSourceUpdater*)param;
		if(updater)
			updater->drawFrame();
		return 0;
	}

}
}
}

#endif