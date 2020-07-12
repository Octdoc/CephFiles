#pragma once

#include <Windows.h>
#include <d2d1_3.h>
#include <dwrite.h>
#include <string>
#include <wrl.h>
#include <vector>
#include <memory>
#include <thread>

/*
TODO:
proper zooming
app open settings file
file sorting
*/

namespace filemanager
{
	class Application
	{
		HWND m_mainWindow;
		std::wstring m_windowName;
		D2D1_SIZE_F m_windowSize;
		D2D1_POINT_2F m_imagePosition;
		float m_zoom;
		D2D1_POINT_2L m_prevCursor;
		std::wstring m_currentFilename;
		size_t m_currentImageIndex;
		std::vector<std::wstring> m_folderFiles;
		HANDLE m_changeNotifyHandle;
		std::unique_ptr<std::thread> m_changeNotifyThread;
		Microsoft::WRL::ComPtr<ID2D1Factory> m_d2dFactory;
		Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> m_renderTarget;
		Microsoft::WRL::ComPtr<ID2D1Bitmap> m_image;
		Microsoft::WRL::ComPtr<IDWriteFactory> m_writeFactory;
		Microsoft::WRL::ComPtr<IDWriteTextFormat> m_textFormat;
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_textBrush;
		Microsoft::WRL::ComPtr<IDWriteTextLayout> m_text;

	private:
		void InitWindow();
		void InitDirect2D();
		void InitDirectWrite();

		void Redraw();

		D2D1_SIZE_F WndCoord();
		D2D1_RECT_F ImagePosition();

		void Resize(unsigned width, unsigned height);
		void Paint();
		void MouseMove(int x, int y, bool btnDown);
		void MouseWheel(int delta);
		void KeyDown(WPARAM wparam);
		void DropFile(HDROP drop);
		void FolderChanged();
		void MinimumWindowSize(LPMINMAXINFO minMaxInfo);

		void ZoomImage(float zoom);

		void LoadCmdLineFile();
		void UpdateFolderFiles();
		void UpdateWindowText();
		void LoadImageFromFile(const wchar_t* filename);

		void LoadNewImage(const wchar_t* filename);
		void RemoveCurrentImage();
		void CreateText(const std::wstring& text);

	public:
		Application();
		~Application();
		void Init(const std::wstring& name, unsigned width, unsigned height);
		void Run();

		LRESULT MessageHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	};
}