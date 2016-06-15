#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <mutex>
#include <vector>
#include <condition_variable>
#include <atomic>
#include <thread>

namespace Logger_nsp
{
	namespace FileUtility
	{
		class FileUtil
		{
		private:
			typedef unsigned char uchar;
			std::atomic<bool> done;
			std::condition_variable data_cond;
			//std::vector<std::thread>threadPool;
			std::thread threadPool;
			std::mutex lockMutex;
		private:
			size_t writtenBytes;
			size_t submitSize;
			size_t unUsedBytes;
			char buffer[64 * 1024];
			char submitBuffer[64 * 1024];
			char* submitBufferPtr;
			FILE* fp;
			size_t Write(const char* src, size_t len);
		public:
			FileUtil(std::string);
			FileUtil();
			~FileUtil();
			//************************************
			// @Method:    Append
			// @Returns:   void
			// @Parameter: src
			// @Parameter: len
			// @Brief:	文件的写入操作，字节流式写入
			//************************************
			void Append(const char* src, size_t len);
			//************************************
			// @Method:    GetMutex
			// @Returns:   std::mutex&
			// @Brief:	取得互斥量
			//************************************
			std::mutex& GetMutex()const;
			//************************************
			// @Method:    SetSubmitSize
			// @Returns:   void
			// @Parameter: len
			// @Brief:	设置提交的数据量大小
			//************************************
			void SetSubmitSize(size_t len);
			//************************************
			// @Method:    AvailSubmitBuffer
			// @Returns:   int
			// @Brief:	提交缓冲可用大小
			//************************************
			int AvailSubmitBuffer()const;
			//************************************
			// @Method:    GetSubmitBuffer
			// @Returns:   uchar*
			// @Brief:	已提交缓冲区所指地址
			//************************************
			uchar* GetSubmitBuffer()const;
			//************************************
			// @Method:    WrittenBytes
			// @Returns:   size_t
			// @Brief:	已写入数据字节
			//************************************
			size_t WrittenBytes()const;
			void Flush();
			//************************************
			// @Method:    Start
			// @Returns:   void
			// @Brief:	开启后台刷新线程
			//************************************
			void Start();
			//************************************
			// @Method:    Update
			// @Returns:   void
			// @Brief:	后台刷新数据到文件 由于调度问题可能每个文件大小不完全相同
			//************************************
			void Update();
			//************************************
			// @Method:    NotifyMe
			// @Returns:   void
			// @Brief:	条件变量封装，用于通知线程已有数据
			//************************************
			void NotifyMe();
			//************************************
			// @Method:    CreateNewLogFile
			// @Returns:   void
			// @Parameter: fileName
			// @Brief:	文件滚动
			//************************************
			void CreateNewLogFile(std::string fileName);
			//************************************
			// @Method:    GetOriginBuffer
			// @Returns:   char*
			// @Brief:	取得原始缓冲区地址
			//************************************
			char* GetOriginBuffer()const;
		};

	}
}
