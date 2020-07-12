#include "application.h"
#include <windowsx.h>
#include <wincodec.h>
#include <iostream>
#include <filesystem>
#include <sstream>

#pragma comment (lib, "d2d1.lib")
#pragma comment (lib, "dwrite.lib")
#pragma comment (lib, "windowscodecs.lib")

#define FOLDER_CHANGE_MESSAGE WM_USER + 1

namespace filemanager
{
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
			throw std::exception(("Error: " + std::to_string(static_cast<long>(hr))).c_str());
	}

	void Application::InitWindow()
	{
		WNDCLASSEX wc{};
		wc.cbSize = sizeof(wc);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpszClassName = m_windowName.c_str();
		wc.lpfnWndProc = DefWindowProc;
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hInstance = GetModuleHandle(nullptr);
		RegisterClassEx(&wc);

		RECT rect;
		rect.left = 0;
		rect.right = static_cast<LONG>(m_windowSize.width);
		rect.top = 0;
		rect.bottom = static_cast<LONG>(m_windowSize.height);
		DWORD style = WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_THICKFRAME;
		DWORD exStyle = WS_EX_APPWINDOW | WS_EX_ACCEPTFILES;
		AdjustWindowRectEx(&rect, style, false, exStyle);
		m_mainWindow = CreateWindowEx(
			exStyle, wc.lpszClassName, m_windowName.c_str(), style,
			CW_USEDEFAULT, CW_USEDEFAULT,
			rect.right - rect.left, rect.bottom - rect.top,
			nullptr, nullptr, wc.hInstance, this);
	}
	void Application::InitDirect2D()
	{
		ThrowIfFailed(CoInitialize(nullptr));
		ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, m_d2dFactory.GetAddressOf()));

		ThrowIfFailed(m_d2dFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(m_mainWindow),
			m_renderTarget.GetAddressOf()));
	}
	void Application::InitDirectWrite()
	{
		ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&m_writeFactory));
		ThrowIfFailed(m_writeFactory->CreateTextFormat(L"Consolas", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 20, L"en", &m_textFormat));
		ThrowIfFailed(m_renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.9f, 0.9f, 0.95f), &m_textBrush));
	}
	void Application::Redraw()
	{
		InvalidateRect(m_mainWindow, nullptr, false);
	}
	D2D1_SIZE_F Application::WndCoord()
	{
		D2D1_SIZE_F imgSize = m_image->GetSize();
		D2D1_SIZE_F scaledWnd = m_windowSize;
		scaledWnd.width *= imgSize.height / imgSize.width;
		return scaledWnd.width > scaledWnd.height ?
			D2D1::SizeF(scaledWnd.width / scaledWnd.height, 1.0f) :
			D2D1::SizeF(1.0f, scaledWnd.height / scaledWnd.width);
	}
	D2D1_RECT_F Application::ImagePosition()
	{
		D2D1_SIZE_F wndCoord = WndCoord();
		return D2D1::RectF(
			((m_imagePosition.x - 1.0f) * m_zoom + wndCoord.width) * m_windowSize.width / (2.0f * wndCoord.width),
			((m_imagePosition.y - 1.0f) * m_zoom + wndCoord.height) * m_windowSize.height / (2.0f * wndCoord.height),
			((m_imagePosition.x + 1.0f) * m_zoom + wndCoord.width) * m_windowSize.width / (2.0f * wndCoord.width),
			((m_imagePosition.y + 1.0f) * m_zoom + wndCoord.height) * m_windowSize.height / (2.0f * wndCoord.height));
	}
	void Application::Resize(unsigned width, unsigned height)
	{
		m_windowSize.width = static_cast<float>(width);
		m_windowSize.height = static_cast<float>(height);
		m_renderTarget->Resize(D2D1::SizeU(width, height));
	}
	void Application::Paint()
	{
		m_renderTarget->BeginDraw();
		m_renderTarget->Clear(D2D1::ColorF(0.2f, 0.2f, 0.25f));

		if (m_image)
			m_renderTarget->DrawBitmap(m_image.Get(), ImagePosition());
		else if (m_text)
		{
			m_text->SetMaxWidth(m_windowSize.width - 20.0f);
			m_text->SetMaxHeight(m_windowSize.height);
			m_renderTarget->DrawTextLayout(D2D1::Point2F(10.0f, m_windowSize.height * 0.5f - 30.0f), m_text.Get(), m_textBrush.Get());
		}

		m_renderTarget->EndDraw();
		ValidateRect(m_mainWindow, nullptr);
	}
	void Application::MouseMove(int x, int y, bool btnDown)
	{
		if (m_image && btnDown)
		{
			int dx = x - m_prevCursor.x;
			int dy = y - m_prevCursor.y;
			D2D1_SIZE_F wndCoord = WndCoord();
			m_imagePosition.x += static_cast<float>(dx) * 2.0f * wndCoord.width / (m_windowSize.width * m_zoom);
			m_imagePosition.y += static_cast<float>(dy) * 2.0f * wndCoord.height / (m_windowSize.height * m_zoom);
			Redraw();
		}
		m_prevCursor.x = x;
		m_prevCursor.y = y;
	}
	void Application::MouseWheel(int delta)
	{
		if (m_image)
			ZoomImage(delta > 0 ? 1.1f : 1.0f / 1.1f);
	}
	void Application::KeyDown(WPARAM wparam)
	{
		if (!m_folderFiles.empty())
		{
			switch (wparam)
			{
			case VK_LEFT:
				if (m_currentImageIndex-- == 0)
					m_currentImageIndex += m_folderFiles.size();
				LoadImageFromFile(m_folderFiles[m_currentImageIndex].c_str());
				UpdateWindowText();
				Redraw();
				break;
			case VK_RIGHT:
				if (++m_currentImageIndex >= m_folderFiles.size())
					m_currentImageIndex = 0;
				LoadImageFromFile(m_folderFiles[m_currentImageIndex].c_str());
				UpdateWindowText();
				Redraw();
				break;
			case VK_UP:
				ZoomImage(1.1f);
				break;
			case VK_DOWN:
				ZoomImage(1.0f / 1.1f);
				break;
			case VK_DELETE:
				RemoveCurrentImage();
				UpdateWindowText();
				break;
			}
		}
	}
	void Application::DropFile(HDROP drop)
	{
		wchar_t filename[MAX_PATH];
		filename[0] = '\0';
		DragQueryFile(drop, 0, filename, MAX_PATH);

		LoadNewImage(filename);
		Redraw();

		DragFinish(drop);
	}
	void Application::FolderChanged()
	{
		UpdateFolderFiles();
		UpdateWindowText();
	}
	void Application::MinimumWindowSize(LPMINMAXINFO minMaxInfo)
	{
		minMaxInfo->ptMinTrackSize.x = 175;
		minMaxInfo->ptMinTrackSize.y = 150;
	}
	void Application::ZoomImage(float zoom)
	{
		m_zoom *= zoom;

		/*D2D1_SIZE_F wndCoord = WndCoord();
		D2D1_POINT_2F cursor = {
			(static_cast<float>(m_prevCursor.x) / m_wndSize.width * 2.0f - 1.0f) * wndCoord.width / m_zoom,
			(static_cast<float>(m_prevCursor.y) / m_wndSize.height * 2.0f - 1.0f) * wndCoord.height / m_zoom
		};

		m_imagePosition.x -= cursor.x * zoom;
		m_imagePosition.y -= cursor.y * zoom;*/

		Redraw();
	}
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
			std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
		}
		return extension;
	}
	void Application::LoadCmdLineFile()
	{
		int numArgs = 0;
		LPWSTR* args = CommandLineToArgvW(GetCommandLineW(), &numArgs);

		if (numArgs > 1)
			LoadNewImage(args[1]);
		else
			CreateText(L"Drop an image on the window");

		LocalFree(args);
	}
	void Application::UpdateFolderFiles()
	{
		std::wstring folderName = GetFolderName(m_currentFilename.c_str());
		m_currentImageIndex = 0ull;
		m_folderFiles.clear();
		if (std::filesystem::exists(folderName))
		{
			std::filesystem::directory_iterator end;
			for (std::filesystem::directory_iterator iter(folderName); iter != end; iter++)
			{
				if (!iter->is_directory())
				{
					std::wstring dirFile(iter->path().c_str());
					std::wstring extension = GetExtension(dirFile.c_str());
					if (extension == L".jpg" ||
						extension == L".jpeg" ||
						extension == L".png")
					{
						if (dirFile == m_currentFilename)
							m_currentImageIndex = m_folderFiles.size();
						m_folderFiles.emplace_back(std::move(dirFile));
					}
				}
			}
		}
	}
	void Application::UpdateWindowText()
	{
		if (!m_folderFiles.empty())
		{
			std::wstringstream ss;
			ss << L'(' << m_currentImageIndex + 1 << L'/' << m_folderFiles.size() << L") - " << m_folderFiles[m_currentImageIndex];
			SetWindowText(m_mainWindow, ss.str().c_str());
		}
	}
	void Application::LoadImageFromFile(const wchar_t* filename)
	{
		try
		{
			Microsoft::WRL::ComPtr<IWICImagingFactory> factory;
			Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
			Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
			Microsoft::WRL::ComPtr<IWICFormatConverter> converter;

			m_currentFilename = filename;
			m_zoom = 1.0f;
			m_imagePosition.x = 0.0f;
			m_imagePosition.y = 0.0f;
			m_text.Reset();
			m_image.Reset();

			ThrowIfFailed(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, reinterpret_cast<LPVOID*>(factory.GetAddressOf())));
			ThrowIfFailed(factory->CreateDecoderFromFilename(filename, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, decoder.GetAddressOf()));
			ThrowIfFailed(decoder->GetFrame(0, frame.GetAddressOf()));
			ThrowIfFailed(factory->CreateFormatConverter(converter.GetAddressOf()));
			ThrowIfFailed(converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeMedianCut));
			ThrowIfFailed(m_renderTarget->CreateBitmapFromWicBitmap(converter.Get(), nullptr, m_image.ReleaseAndGetAddressOf()));
		}
		catch (const std::exception& e)
		{
			std::string errMsg(e.what());
			CreateText(std::wstring(errMsg.begin(), errMsg.end()));
		}
	}
	void Application::LoadNewImage(const wchar_t* filename)
	{
		LoadImageFromFile(filename);
		UpdateFolderFiles();
		UpdateWindowText();

		if (m_changeNotifyHandle)
		{
			FindCloseChangeNotification(m_changeNotifyHandle);
			m_changeNotifyHandle = nullptr;
		}
		if (m_changeNotifyThread)
			m_changeNotifyThread->join();
		if (m_changeNotifyHandle)
			FindCloseChangeNotification(m_changeNotifyHandle);
		m_changeNotifyHandle = FindFirstChangeNotificationW(GetFolderName(filename).c_str(), false, FILE_NOTIFY_CHANGE_FILE_NAME);
		m_changeNotifyThread = std::make_unique<std::thread>([this]() {
			while (m_changeNotifyHandle)
			{
				if (WAIT_OBJECT_0 == WaitForSingleObject(m_changeNotifyHandle, INFINITE))
				{
					PostMessage(m_mainWindow, FOLDER_CHANGE_MESSAGE, 0, 0);
					FindNextChangeNotification(m_changeNotifyHandle);
				}
			}
			});
	}
	void Application::RemoveCurrentImage()
	{
		m_image.Reset();

		std::wstring deleteFile(m_folderFiles[m_currentImageIndex] + L'\0');
		SHFILEOPSTRUCT fileOp{};
		fileOp.hwnd = m_mainWindow;
		fileOp.wFunc = FO_DELETE;
		fileOp.pFrom = deleteFile.c_str();
		fileOp.fFlags = FOF_ALLOWUNDO | FOF_NO_UI;
		if (SHFileOperationW(&fileOp) == 0)
		{
			m_folderFiles.erase(m_folderFiles.begin() + m_currentImageIndex);
			if (m_folderFiles.empty())
			{
				PostQuitMessage(0);
			}
			else
			{
				if (m_currentImageIndex >= m_folderFiles.size())
					m_currentImageIndex = 0;
				LoadImageFromFile(m_folderFiles[m_currentImageIndex].c_str());
				Redraw();
			}
		}
	}
	void Application::CreateText(const std::wstring& text)
	{
		ThrowIfFailed(m_writeFactory->CreateTextLayout(text.c_str(), text.length(), m_textFormat.Get(), m_windowSize.width, m_windowSize.height, &m_text));
		m_text->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	}
	Application::Application() :
		m_mainWindow(nullptr),
		m_windowSize{ 0.0f, 0.0f },
		m_imagePosition{ 0.0f, 0.0f },
		m_zoom(1.0f),
		m_prevCursor{ 0, 0 },
		m_currentImageIndex(0ull),
		m_changeNotifyHandle(nullptr) {}
	Application::~Application()
	{
		if (m_changeNotifyThread)
		{
			if (m_changeNotifyHandle)
			{
				FindCloseChangeNotification(m_changeNotifyHandle);
				m_changeNotifyHandle = nullptr;
			}
			m_changeNotifyThread->join();
		}
	}
	void Application::Init(const std::wstring& name, unsigned width, unsigned height)
	{
		m_windowName = name;
		m_windowSize.width = static_cast<float>(width);
		m_windowSize.height = static_cast<float>(height);

		InitWindow();
		InitDirect2D();
		InitDirectWrite();

		LoadCmdLineFile();

		LRESULT(*wndProc)(HWND, UINT, WPARAM, LPARAM) = [](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)->LRESULT {
			return reinterpret_cast<Application*>(GetWindowLongPtr(hwnd, GWLP_USERDATA))->MessageHandler(hwnd, msg, wparam, lparam);
		};
		SetWindowLongPtr(m_mainWindow, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
		SetWindowLongPtr(m_mainWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(wndProc));

		if (m_mainWindow)
			ShowWindow(m_mainWindow, SW_SHOWDEFAULT);
	}
	void Application::Run()
	{
		MSG msg{};
		while (GetMessage(&msg, nullptr, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	LRESULT Application::MessageHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		switch (msg)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_SIZE:
			Resize(LOWORD(lparam), HIWORD(lparam));
			return 0;
		case WM_MOUSEMOVE:
			MouseMove(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), wparam & (MK_LBUTTON | MK_RBUTTON));
			return 0;
		case WM_MOUSEWHEEL:
			MouseWheel(GET_WHEEL_DELTA_WPARAM(wparam));
		case WM_KEYDOWN:
			KeyDown(wparam);
			return 0;
		case WM_DROPFILES:
			DropFile(reinterpret_cast<HDROP>(wparam));
			return 0;
		case FOLDER_CHANGE_MESSAGE:
			FolderChanged();
			return 0;
		case WM_PAINT:
			Paint();
			return 0;
		case WM_GETMINMAXINFO:
			MinimumWindowSize(reinterpret_cast<LPMINMAXINFO>(lparam));
			return 0;
		}
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
}
