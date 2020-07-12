#include "common.h"

namespace cephimages
{
	std::wstring GetFolderName(const wchar_t* filename)
	{
		std::wstring folderName;
		int lastSlashIndex = -1;
		for (int i = 0; filename[i]; i++)
			if (filename[i] == '\\' || filename[i] == '/')
				lastSlashIndex = i;
		if (lastSlashIndex >= 0)
			folderName = std::wstring(filename, filename + lastSlashIndex);
		return folderName;
	}
	std::wstring GetExtension(const wchar_t* filename)
	{
		std::wstring extension;
		int lastDotIndex = -1;
		for (int i = 0; filename[i]; i++)
			if (filename[i] == '.')
				lastDotIndex = i;
		if (lastDotIndex >= 0)
		{
			extension = std::wstring(filename + lastDotIndex);
			for (wchar_t& ch : extension)
				if (ch >= 'A' && ch <= 'Z')
					ch -= 'A' + 'a';
		}
		return extension;
	}
	bool FormatSupported(const std::wstring& extension)
	{
		return
			extension == L".jfif" ||
			extension == L".jpeg" ||
			extension == L".jpg" ||
			extension == L".jpe" ||
			extension == L".bmp" ||
			extension == L".tif" ||
			extension == L".tiff" ||
			extension == L".dds" ||
			extension == L".ico" ||
			extension == L".gif" ||
			extension == L".png";
	}
}