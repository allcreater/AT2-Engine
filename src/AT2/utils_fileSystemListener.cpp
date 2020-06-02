#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)

#include "utils.hpp"
#include <Windows.h>

#include <vector>
#include <map>
#include <thread>
#include <stdexcept>

using namespace AT2::Utils;

class Win32FileSystemListenerImpl : public IFileSystemListener
{
public:
	Win32FileSystemListenerImpl() : 
		m_workingThread(&Win32FileSystemListenerImpl::workerThreadFunction, this),
		m_directoryInfoBuffer(1000000)
	{
		m_workingThread.detach();
	}

	~Win32FileSystemListenerImpl()
	{

	}

private:
	void workerThreadFunction ()
	{
		while (true)
		{
			std::vector<HANDLE> handlesCopy; handlesCopy.reserve(m_handlesMap.size());
			std::vector<namestring> dirNamesCopy; dirNamesCopy.reserve(m_handlesMap.size());
			for (auto& callback : m_handlesMap)
			{
				dirNamesCopy.push_back(callback.first);
				handlesCopy.push_back(callback.second);
			}
			auto callbacksCopy = m_callbacksMap;

			auto waitStatus = WaitForMultipleObjects(handlesCopy.size(), handlesCopy.data(), FALSE, INFINITE);
			if (waitStatus != WAIT_TIMEOUT)
			{
				const int callbackNumber = WAIT_OBJECT_0 - waitStatus;
				if (callbackNumber >= 0 && callbackNumber < handlesCopy.size())
				{
					auto handle = handlesCopy[callbackNumber];

					ProcessDirectoryChanges(dirNamesCopy[callbackNumber], callbacksCopy[handle]);

					FindNextChangeNotification(handle);
				}
			}
		}
	}

	void ProcessDirectoryChanges(namestring dirName, const callbackFunc& callback)
	{
		HANDLE dirHandle = CreateFile(dirName.c_str(),
			FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL, //security attributes
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | //required privileges: SE_BACKUP_NAME and SE_RESTORE_NAME.
			FILE_FLAG_OVERLAPPED,
			NULL);

		DWORD bytesReturned = 0;
		OVERLAPPED notUsed;
		ReadDirectoryChangesW(
			dirHandle,
			m_directoryInfoBuffer.data(),
			m_directoryInfoBuffer.size(),
			FALSE,
			FILE_NOTIFY_CHANGE_LAST_WRITE,
			&bytesReturned,
			&notUsed,
			NULL
			);

		CloseHandle(dirHandle);

		auto notifyInformationNode = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(m_directoryInfoBuffer.data());
		do
		{
			namestring filename(notifyInformationNode->FileName, notifyInformationNode->FileName + notifyInformationNode->FileNameLength);
			callback(filename);

			notifyInformationNode = (notifyInformationNode + notifyInformationNode->NextEntryOffset);
			//notifyInformationNode = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(reinterpret_cast<DWORD>(notifyInformationNode) + notifyInformationNode->NextEntryOffset);
		} while (notifyInformationNode->NextEntryOffset > 0);
	}

public:
	virtual void SetFolderCallback(const namestring& folderName, callbackFunc callback)
	{
		auto iter = m_handlesMap.find(folderName);
		if (iter != m_handlesMap.end())
			throw std::invalid_argument("folderName already exists!");

		HANDLE handle = FindFirstChangeNotification(folderName.c_str(), FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);
		m_handlesMap[folderName] = handle;
		m_callbacksMap[handle] = callback;
	}

	virtual void ResetFolderCallback(const namestring& folderName)
	{
		auto iter = m_handlesMap.find(folderName);
		if (iter != m_handlesMap.end())
		{
			FindCloseChangeNotification(iter->second);

			m_callbacksMap.erase(iter->second);
			m_handlesMap.erase(iter);
		}
	}

private:
	std::vector<char> m_directoryInfoBuffer;

	std::map<namestring, HANDLE> m_handlesMap;
	std::map<HANDLE, callbackFunc> m_callbacksMap;

	std::thread m_workingThread;
};

IFileSystemListener* IFileSystemListener::s_instance = nullptr;

IFileSystemListener* IFileSystemListener::GetInstance()
{
	if (!s_instance)
		s_instance = new Win32FileSystemListenerImpl();

	return s_instance;
}

#endif