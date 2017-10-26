/*
 *	@file		Process.h
 *	@data		2013-3-12 9:31
 *	@author		zhu qing quan
 *	@brief		进程操作的封装类
 *  @Copyright	(C) 2012, by Ragile Corporation
 */

#pragma once
#ifndef _SOA_MIRROR_PROCESS_H_
#define _SOA_MIRROR_PROCESS_H_

namespace SOA
{
namespace Mirror
{
	/**
	 * @name	Process
	 * @brief	进程相关操作的封装
	 */
	class Process
	{
	public:
		Process();
		/**
		 * Method		Process
		 * @brief		构造函数
		 *				如果processExeName名称相对应的进程已经在执行，则获取操作该进程的句柄，这样在start是就不会启动一个新的进程
		 *				如果对应的进程不存在，则与Process()一样
		 * @param[in]	const char * processExeName 进程可执行文件的名称，不可加路径
		 *				如："sample.exe" 
		 */
		Process(const char* processExeName);
		~Process();

		/**
		 * Method		start
		 * @brief		启动进程
		 * @param[in]	const char * name 进程可执行文件的名称
		 *				如："c:\\sample.exe"
		 *					".\\sample.exe"
		 *					"sample.exe"
		 * @return		bool
		 */
		bool start(const char* name);

		/**
		 * Method		close
		 * @brief		通过向进程的主窗口发送关闭消息来关闭拥有用户界面的进程。释放与此进程关联的所有资源。
		 * @return		bool
		 */
		bool close();

		/**
		 * Method		kill
		 * @brief		立即停止进程
		 * @return		void
		 */
		void kill();

		/**
		 * Method		isProcessRunning
		 * @brief		进程是否还在运行
		 * @return		bool true--运行  false--已终止
		 */
		bool isProcessRunning();
	private:
		void* m_startUpInfo;
		void* m_processInfo;

		void InitProcessInfoFromSystemExecutingProcess(const char* processExeName);
	};
}
}

#endif // _SOA_MIRROR_PROCESS_H_