#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <memory>
#include <thread>

#define FOLDER_CHANGE_MESSAGE WM_USER + 1

namespace cephimages
{
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
			throw std::exception(("Error: " + std::to_string(static_cast<long>(hr))).c_str());
	}

	std::wstring GetFolderName(const wchar_t* filename);
	std::wstring GetExtension(const wchar_t* filename);
	bool FormatSupported(const std::wstring& extension);
}