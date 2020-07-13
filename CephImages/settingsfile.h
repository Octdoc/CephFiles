#pragma once

#include "common.h"
#include <d2d1_3.h>

namespace cephimages
{
	class SettingsFile
	{
		std::wstring m_settingsFilePath;
		RECT m_windowRect;
		UINT m_windowFlags;
		UINT m_showCmd;
		D2D1_COLOR_F m_backgroundColor;

	private:
		void MakeSettingsFilePath();
		void WriteToStream(std::wostream& os) const;

	public:
		SettingsFile();
		~SettingsFile();

		void Save() const;
		void DefaultSettings();
		void Load();

		void StoreWindowPlacement(HWND window);
		void ApplyWindowPlacement(HWND window) const;
		inline void BackgroundColor(D2D1_COLOR_F color) { m_backgroundColor = color; }
		inline D2D1_COLOR_F BackgroundColor() const { return m_backgroundColor; }
	};
}