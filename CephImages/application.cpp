#include "application.h"
#include "resource.h"
#include <windowsx.h>
#include <sstream>
#include <fstream>

namespace cephimages
{
	void Application::InitWindow()
	{
		WNDCLASSEX wc{};
		wc.cbSize = sizeof(wc);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpszClassName = m_windowName.c_str();
		wc.lpfnWndProc = DefWindowProc;
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hInstance = GetModuleHandle(nullptr);
		wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1));
		wc.hIconSm = wc.hIcon;
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
		ThrowIfFailed(m_d2dFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(m_mainWindow), m_renderTarget.GetAddressOf()));
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
	void Application::CloseWindow()
	{
		m_settings.StoreWindowPlacement(m_mainWindow);
		DestroyWindow(m_mainWindow);
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
		m_renderTarget->Clear(m_settings.BackgroundColor());

		if (m_image)
			m_image->Draw(m_renderTarget.Get(), m_windowSize);
		else if (m_text)
			m_text->Draw(m_renderTarget.Get(), m_textBrush.Get(), m_windowSize);

		m_renderTarget->EndDraw();
		ValidateRect(m_mainWindow, nullptr);
	}
	void Application::MouseMove(int x, int y, bool btnDown)
	{
		if (m_image && btnDown)
		{
			m_image->Moved(x - m_prevCursor.x, y - m_prevCursor.y, m_windowSize);
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
		if (m_folderFiles)
		{
			switch (wparam)
			{
			case VK_RIGHT:
				LoadNextImage();
				break;
			case VK_LEFT:
				LoadPreviousImage();
				break;
			case VK_UP:
				ZoomImage(1.1f);
				break;
			case VK_DOWN:
				ZoomImage(1.0f / 1.1f);
				break;
			case VK_DELETE:
				RemoveCurrentImage();
				break;
			}
		}
	}
	void Application::DropFile(HDROP drop)
	{
		wchar_t filename[MAX_PATH];
		filename[0] = '\0';
		DragQueryFile(drop, 0, filename, MAX_PATH);

		LoadImageInNewFolder(filename);
		Redraw();

		DragFinish(drop);
	}
	void Application::FolderChanged()
	{
		m_folderFiles->UpdateFolderFiles();
		UpdateWindowText();
	}
	void Application::MinimumWindowSize(LPMINMAXINFO minMaxInfo)
	{
		minMaxInfo->ptMinTrackSize.x = 175;
		minMaxInfo->ptMinTrackSize.y = 150;
	}
	void Application::LoadCmdLineFile()
	{
		int numArgs = 0;
		LPWSTR* args = CommandLineToArgvW(GetCommandLineW(), &numArgs);

		if (numArgs > 1)
			LoadImageInNewFolder(args[1]);
		else
			CreateText(L"Drop an image on the window");

		LocalFree(args);
	}
	void Application::UpdateWindowText()
	{
		if (m_folderFiles)
		{
			std::wstringstream ss;
			ss << L'(' << m_folderFiles->CurrentFileIndex() + 1 << L'/' << m_folderFiles->FolderFileCount() << L") - " << m_folderFiles->CurrentFileName();
			SetWindowText(m_mainWindow, ss.str().c_str());
		}
	}
	void Application::LoadImageFromFile(const wchar_t* filename)
	{
		try
		{
			m_text.reset();
			m_image.reset();
			m_image = std::make_unique<ImageView>(m_renderTarget.Get(), filename);
		}
		catch (const std::exception& e)
		{
			try
			{
				std::string errMsg(e.what());
				CreateText(std::wstring(errMsg.begin(), errMsg.end()));
			}
			catch (const std::exception&)
			{
				PostQuitMessage(0);
			}
		}
	}
	void Application::LoadNextImage()
	{
		LoadImageFromFile(m_folderFiles->NextFile().c_str());
		UpdateWindowText();
		Redraw();
	}
	void Application::LoadPreviousImage()
	{
		LoadImageFromFile(m_folderFiles->PreviousFile().c_str());
		UpdateWindowText();
		Redraw();
	}
	void Application::ZoomImage(float zoom)
	{
		m_image->Zoom(zoom, m_windowSize, m_prevCursor);
		Redraw();
	}
	void Application::RemoveCurrentImage()
	{
		m_image.reset();
		m_folderFiles->RemoveCurrentFile();
		LoadImageFromFile(m_folderFiles->CurrentFileName().c_str());
		Redraw();
		UpdateWindowText();
	}
	void Application::LoadImageInNewFolder(const wchar_t* filename)
	{
		LoadImageFromFile(filename);
		m_folderFiles = std::make_unique<FolderFiles>(m_mainWindow, filename);
		UpdateWindowText();
	}
	void Application::CreateText(const std::wstring& text)
	{
		m_text = std::make_unique<TextView>(m_renderTarget.Get(), m_writeFactory.Get(), m_textFormat.Get(), text);
	}
	Application::Application() :
		m_mainWindow(nullptr),
		m_windowSize{ 0.0f, 0.0f },
		m_prevCursor{ 0, 0 } {}
	void Application::Init(const std::wstring& name, int width, int height)
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
			m_settings.ApplyWindowPlacement(m_mainWindow);
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
		case WM_CLOSE:
			CloseWindow();
			return 0;
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
