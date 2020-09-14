#pragma once

#include "imageview.h"
#include "textview.h"
#include "folderfiles.h"
#include "settingsfile.h"

namespace cephimages
{
	class Application
	{
		HWND m_mainWindow;
		HMENU m_rightClickMenu;
		std::wstring m_windowName;
		D2D1_SIZE_F m_windowSize;
		D2D1_POINT_2L m_prevCursor;

		Microsoft::WRL::ComPtr<ID2D1Factory> m_d2dFactory;
		Microsoft::WRL::ComPtr<IDWriteFactory> m_writeFactory;
		Microsoft::WRL::ComPtr<IDWriteTextFormat> m_textFormat;
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_textBrush;
		Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> m_renderTarget;

		std::unique_ptr<ImageView> m_image;
		std::unique_ptr<TextView> m_text;
		std::unique_ptr<FolderFiles> m_folderFiles;
		SettingsFile m_settings;

	private:
		void InitWindow();
		void CreateRightClickMenu();
		void InitDirect2D();
		void InitDirectWrite();

		void Redraw();

		void CloseWindow();
		void Resize(unsigned width, unsigned height);
		void Paint();
		void Command(WPARAM id);
		void ChangeInterpolation();
		void RightButtonUp(int x, int y);
		void LeftButtonDown(int x, int y);
		void MouseMove(int x, int y, bool btnDown);
		void MouseWheel(int delta);
		void KeyDown(WPARAM wparam);
		void DropFile(HDROP drop);
		void FolderChanged();
		void MinimumWindowSize(LPMINMAXINFO minMaxInfo);

		void LoadCmdLineFile();
		void UpdateWindowText();
		void LoadImageFromFile(const wchar_t* filename);
		void LoadNextImage();
		void LoadPreviousImage();
		void ZoomImage(float zoom);
		void RemoveCurrentImage();

		void LoadImageInNewFolder(const wchar_t* filename);
		void CreateText(const std::wstring& text);

	public:
		Application();
		void Init(const std::wstring& name, int width, int height);
		void Run();

		LRESULT MessageHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	};
}