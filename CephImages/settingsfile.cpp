#include "settingsfile.h"
#include <ShlObj.h>
#include <fstream>
#include <filesystem>

namespace cephimages
{
	void SettingsFile::MakeSettingsFilePath()
	{
		PWSTR path;
		if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_CREATE, nullptr, &path)))
		{
			m_settingsFilePath = path;
			m_settingsFilePath += L"\\CephFiles\\cephimages.settings";
			CoTaskMemFree(path);
		}
	}
	void SettingsFile::WriteToStream(std::wostream& os) const
	{
		os << "wnd_x: " << m_windowRect.left << std::endl;
		os << "wnd_y: " << m_windowRect.top << std::endl;
		os << "wnd_w: " << m_windowRect.right - m_windowRect.left << std::endl;
		os << "wnd_h: " << m_windowRect.bottom - m_windowRect.top << std::endl;
		os << "wnd_flags: " << m_windowFlags << std::endl;
		os << "show_cmd: " << m_showCmd << std::endl;
		os << "bg_r: " << static_cast<int>(m_backgroundColor.r * 255.0f) << std::endl;
		os << "bg_g: " << static_cast<int>(m_backgroundColor.g * 255.0f) << std::endl;
		os << "bg_b: " << static_cast<int>(m_backgroundColor.b * 255.0f) << std::endl;
		os << "fill_mode: " << static_cast<int>(m_fillMode) << std::endl;
		os << "interpolate: " << static_cast<int>(m_interpolate) << std::endl;
	}
	SettingsFile::SettingsFile()
	{
		MakeSettingsFilePath();
		DefaultSettings();
		Load();
	}
	SettingsFile::~SettingsFile()
	{
		Save();
	}
	void SettingsFile::Save() const
	{
		std::wofstream outfile(m_settingsFilePath);
		if (outfile.is_open())
		{
			WriteToStream(outfile);
		}
		else
		{
			std::filesystem::path folder = GetFolderName(m_settingsFilePath.c_str());
			if (!std::filesystem::exists(folder))
			{
				if (std::filesystem::create_directory(folder))
				{
					outfile.open(m_settingsFilePath);
					if (outfile.is_open())
						WriteToStream(outfile);
				}
			}
		}
	}
	void SettingsFile::DefaultSettings()
	{
		m_windowRect.right = 1500;
		m_windowRect.left = 220;
		m_windowRect.top = 220;
		m_windowRect.bottom = 940;
		m_windowFlags = 0;
		m_showCmd = SW_NORMAL;
		m_backgroundColor = D2D1::ColorF(0.2f, 0.2f, 0.25f);
		m_fillMode = ImageView::FillMode::Fit;
		m_interpolate = true;
	}
	void SettingsFile::Load()
	{
		std::wifstream infile(m_settingsFilePath);
		if (infile.is_open())
		{
			std::wstring key;
			int numBuffer = 0;
			LONG width = m_windowRect.right - m_windowRect.left;
			LONG height = m_windowRect.bottom - m_windowRect.top;
			while (infile.good())
			{
				infile >> key;
				if (key == L"wnd_x:")
					infile >> m_windowRect.left;
				else if (key == L"wnd_y:")
					infile >> m_windowRect.top;
				else if (key == L"wnd_w:")
					infile >> width;
				else if (key == L"wnd_h:")
					infile >> height;
				else if (key == L"wnd_flags:")
					infile >> m_windowFlags;
				else if (key == L"show_cmd:")
					infile >> m_showCmd;
				else if (key == L"bg_r:")
				{
					infile >> numBuffer;
					m_backgroundColor.r = static_cast<float>(numBuffer) / 255.0f;
				}
				else if (key == L"bg_g:")
				{
					infile >> numBuffer;
					m_backgroundColor.g = static_cast<float>(numBuffer) / 255.0f;
				}
				else if (key == L"bg_b:")
				{
					infile >> numBuffer;
					m_backgroundColor.b = static_cast<float>(numBuffer) / 255.0f;
				}
				else if (key == L"fill_mode:")
				{
					infile >> numBuffer;
					switch (numBuffer)
					{
					case (static_cast<int>(ImageView::FillMode::Fill)):
						m_fillMode = ImageView::FillMode::Fill;
						break;
					case (static_cast<int>(ImageView::FillMode::OneToOne)):
						m_fillMode = ImageView::FillMode::OneToOne;
						break;
					case (static_cast<int>(ImageView::FillMode::Fit)):
					default:
						m_fillMode = ImageView::FillMode::Fit;
						break;
					}
				}
				else if (key == L"interpolate:")
				{
					infile >> numBuffer;
					m_interpolate = static_cast<bool>(numBuffer);
				}
			}
			m_windowRect.right = m_windowRect.left + width;
			m_windowRect.bottom = m_windowRect.top + height;
		}
	}
	void SettingsFile::StoreWindowPlacement(HWND window)
	{
		WINDOWPLACEMENT wp;
		wp.length = sizeof(wp);
		GetWindowPlacement(window, &wp);
		m_windowRect = wp.rcNormalPosition;
		m_showCmd = wp.showCmd;
		m_windowFlags = wp.flags;
	}
	void SettingsFile::ApplyWindowPlacement(HWND window) const
	{
		WINDOWPLACEMENT wp;
		wp.length = sizeof(wp);
		wp.ptMaxPosition.x = wp.ptMaxPosition.y = wp.ptMinPosition.x = wp.ptMinPosition.x = -1;
		wp.rcNormalPosition = m_windowRect;
		wp.flags = m_windowFlags;
		wp.showCmd = m_showCmd;
		SetWindowPlacement(window, &wp);
	}
}