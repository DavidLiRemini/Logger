#include "fileutility.h"
#include <assert.h>
#include <string.h>

namespace Logger_nsp
{
	namespace FileUtility
	{
		FileUtil::FileUtil(std::string fileName)
			:done(false),
			submitBufferPtr(submitBuffer),
			fp(nullptr),
			writtenBytes(0),
			submitSize(0),
			unUsedBytes(0)
		{
			auto result = fileName.find('/');
			assert(result == std::string::npos);
			fp = ::fopen(fileName.c_str(), "a");
			fileHandler.push_back(fp);
			assert(fp);
			memset(buffer, 0, sizeof(buffer));
			memset(submitBuffer, 0, sizeof(submitBuffer));
		}

		FileUtil::~FileUtil()
		{
			printf("FileUtil析构\n");
			std::this_thread::sleep_for(std::chrono::seconds(2));
			done = true;
			if (submitSize == 0)
			{
				this->NotifyMe();
			}
			threadPool.join();
			::fclose(fp);
			
		}

		void FileUtil::Append(const char* src, size_t len)
		{
			//printf("fp: 0x%x %d %s\n", fp, __LINE__, __FILE__);
			size_t bytes = Write(src, len);
			printf("写入 %zd字节\n", bytes);
			size_t remainBytes = len - bytes;

			while (remainBytes > 0)
			{
				size_t num = Write(src + bytes, remainBytes);
				if (num == 0)
				{
					int err = ::ferror(fp);
					if (err)
					{
						fprintf(stderr, "Append file failed %s\n", ::strerror(err));
					}
				}
				bytes += num;
				remainBytes = len - bytes;
			}
			writtenBytes += len;
		}

		size_t FileUtil::WrittenBytes()const
		{
			return writtenBytes;
		}

		void FileUtil::Flush()
		{
			::fflush(fp);
		}

		size_t FileUtil::Write(const char* src, size_t len)
		{
			size_t num = fwrite(src, sizeof(uchar), len, fp);
			return num;
		}

		char* FileUtil::GetSubmitBuffer()const
		{
			return (char*)submitBufferPtr;
		}

		char* FileUtil::GetOriginBuffer()const
		{
			return (char*)submitBuffer;
		}
		FileUtil::FileUtil()
			:done(false),
			writtenBytes(0),
			submitSize(0),
			unUsedBytes(0),
			fp(nullptr)
		{
			memset(buffer, 0, sizeof(buffer));
			memset(submitBuffer, 0, sizeof(submitBuffer));
		}

		void FileUtil::Start()
		{
			try
			{
				//threadPool.push_back(std::thread(&FileUtil::Update, this));
				threadPool = std::thread(&FileUtil::Update, this);
			}
			catch (...)
			{
				done = true;
				throw;
			}
		}

		void FileUtil::SetSubmitSize(size_t len)
		{
			submitSize = len;
		}
		void FileUtil::Update()
		{
			while (!done)
			{
				std::unique_lock<std::mutex>lk(lockMutex);
				data_cond.wait(lk, [&] {return submitSize > 0 || done; });
				if (submitSize > 0)
				{
					//printf("提交缓冲内容为: %s\n", submitBufferPtr);
					this->Append(submitBufferPtr, submitSize);
					submitBufferPtr += submitSize;
					submitSize = 0;
					unUsedBytes = AvailSubmitBuffer();
					Flush();
					if (unUsedBytes <= 64)
					{
						printf("数据超过131072导致有剩余数据未提交\n");
						printf("written bytes %zd\n", writtenBytes);
						memset(submitBuffer, 0, sizeof(submitBuffer));
						submitBufferPtr = submitBuffer;
					}
				}
				lk.unlock();
			}
		}

		void FileUtil::NotifyMe()
		{
			//printf("通知有数据可取\n");
			std::unique_lock<std::mutex>lk(lockMutex);
			data_cond.notify_one();
			lk.unlock();
		}

		std::mutex& FileUtil::GetMutex()const
		{
			return const_cast<std::mutex&>(lockMutex);
		}

		int FileUtil::AvailSubmitBuffer()const
		{
			int used = submitBufferPtr - submitBuffer;
			int avail = 131072 - used;
			return avail;
		}

		void FileUtil::CreateNewLogFile(std::string fileName)
		{
			//std::lock_guard<std::mutex>lk(lockMutex);
			printf("创建新文件中\n");
			//printf("fp: 0x%x %d %s\n", fp, __LINE__, __FILE__);
			// ::fclose(fp);
			// int err = ferror(fp);
			// if (err)
			// {
				// fprintf(stderr, "Close previous file failed %s\n", strerror(err));
			// }
			
			FILE* fd = ::fopen(fileName.c_str(), "a");
			err = ferror(fd);
			if (err)
			{
				fprintf(stderr, "Open file failed %s\n", strerror(err));
			}
			fileHandler.push_back(fd);
		}
		
		void FileUtil::SwitchFileHandler()
		{
			assert(fp != nullptr);
			assert(fileHandler.size() > 1);
			auto index = std::find(fileHandler.begin(), fileHandler.end(), fp);
			if (index != fileHandler.end())
			{
				if(*index != fileHandler.back())
				{
					fp = *(++index);
				}
			}
		}
	}
}
