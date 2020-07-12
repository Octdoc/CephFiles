#include "folderfiles.h"
#include <filesystem>

namespace cephimages
{
	void FolderFiles::ChangeNotifyLoop(HWND window)
	{
		while (m_changeNotifyHandle)
		{
			if (WAIT_OBJECT_0 == WaitForSingleObject(m_changeNotifyHandle, INFINITE))
			{
				PostMessage(window, FOLDER_CHANGE_MESSAGE, 0, 0);
				FindNextChangeNotification(m_changeNotifyHandle);
			}
		}
	}

	void FolderFiles::UpdateCurrentFileName()
	{
		if (m_folderFiles.empty())
			m_currentFilename.clear();
		m_currentFilename = m_folderFiles[m_currentFileIndex];
	}

	FolderFiles::FolderFiles(HWND window, const wchar_t* filename) :
		m_folderName(GetFolderName(filename)),
		m_currentFilename(filename),
		m_currentFileIndex(0ull),
		m_folderFiles(),
		m_changeNotifyHandle(FindFirstChangeNotificationW(m_folderName.c_str(), false, FILE_NOTIFY_CHANGE_FILE_NAME)),
		m_changeNotifyThread([this](HWND window) { ChangeNotifyLoop(window); }, window)
	{
		UpdateFolderFiles();
	}

	FolderFiles::~FolderFiles()
	{
		if (m_changeNotifyHandle)
		{
			FindCloseChangeNotification(m_changeNotifyHandle);
			m_changeNotifyHandle = nullptr;
		}
		m_changeNotifyThread.join();
	}

	void FolderFiles::UpdateFolderFiles()
	{
		m_currentFileIndex = 0ull;
		m_folderFiles.clear();
		if (std::filesystem::exists(m_folderName))
		{
			std::filesystem::directory_iterator end;
			for (std::filesystem::directory_iterator iter(m_folderName); iter != end; iter++)
			{
				if (!iter->is_directory())
				{
					std::wstring dirFile(iter->path().c_str());
					if (FormatSupported(GetExtension(dirFile.c_str())))
					{
						if (dirFile == m_currentFilename)
							m_currentFileIndex = m_folderFiles.size();
						m_folderFiles.emplace_back(std::move(dirFile));
					}
				}
			}
		}
		if (m_folderFiles.empty())
			PostQuitMessage(0);
	}

	void FolderFiles::RemoveCurrentFile()
	{
		std::wstring deleteFile(m_currentFilename + L'\0');
		SHFILEOPSTRUCT fileOp{};
		fileOp.hwnd = nullptr;
		fileOp.wFunc = FO_DELETE;
		fileOp.pFrom = deleteFile.c_str();
		fileOp.fFlags = FOF_ALLOWUNDO | FOF_NO_UI;
		if (0 == SHFileOperationW(&fileOp))
		{
			m_folderFiles.erase(m_folderFiles.begin() + m_currentFileIndex);
			if (m_folderFiles.empty())
			{
				PostQuitMessage(0);
			}
			else
			{
				if (m_currentFileIndex >= m_folderFiles.size())
					m_currentFileIndex = 0;
				m_currentFilename = m_folderFiles[m_currentFileIndex];
			}
		}
	}

	const std::wstring& FolderFiles::NextFile()
	{
		if (++m_currentFileIndex >= m_folderFiles.size())
			m_currentFileIndex = 0;
		UpdateCurrentFileName();
		return m_currentFilename;
	}

	const std::wstring& FolderFiles::PreviousFile()
	{
		if (m_currentFileIndex-- == 0)
			m_currentFileIndex += m_folderFiles.size();
		UpdateCurrentFileName();
		return m_currentFilename;
	}
}