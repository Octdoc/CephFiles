#pragma once

#include "common.h"

namespace cephimages
{
	class FolderFiles
	{
		std::wstring m_folderName;
		std::wstring m_currentFilename;
		size_t m_currentFileIndex;
		std::vector<std::wstring> m_folderFiles;
		HANDLE m_changeNotifyHandle;
		std::thread m_changeNotifyThread;

	private:
		void ChangeNotifyLoop(HWND window);
		void UpdateCurrentFileName();
		
	public:
		FolderFiles(HWND window, const wchar_t* filename);
		~FolderFiles();

		void UpdateFolderFiles();
		void RemoveCurrentFile();

		inline size_t FolderFileCount() const { return m_folderFiles.size(); }
		inline size_t CurrentFileIndex() const { return m_currentFileIndex; }
		inline const std::wstring& CurrentFileName() const { return m_currentFilename; }
		const std::wstring& NextFile();
		const std::wstring& PreviousFile();
	};
}