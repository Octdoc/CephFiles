#include "imageview.h"
#include <wincodec.h>

namespace cephimages
{
	D2D1_SIZE_F ImageView::WindowCoord(D2D1_SIZE_F windowSize)
	{
		D2D1_SIZE_F imgSize = m_image->GetSize();
		D2D1_SIZE_F scaledWnd = windowSize;
		scaledWnd.width *= imgSize.height / imgSize.width;
		return scaledWnd.width > scaledWnd.height ?
			D2D1::SizeF(scaledWnd.width / scaledWnd.height, 1.0f) :
			D2D1::SizeF(1.0f, scaledWnd.height / scaledWnd.width);
	}
	D2D1_RECT_F ImageView::ImagePosition(D2D1_SIZE_F windowSize)
	{
		D2D1_SIZE_F wndCoord = WindowCoord(windowSize);
		return D2D1::RectF(
			((m_position.x - 1.0f) * m_zoom + wndCoord.width) * windowSize.width / (2.0f * wndCoord.width),
			((m_position.y - 1.0f) * m_zoom + wndCoord.height) * windowSize.height / (2.0f * wndCoord.height),
			((m_position.x + 1.0f) * m_zoom + wndCoord.width) * windowSize.width / (2.0f * wndCoord.width),
			((m_position.y + 1.0f) * m_zoom + wndCoord.height) * windowSize.height / (2.0f * wndCoord.height));
	}
	void ImageView::Load(ID2D1RenderTarget* renderTarget, const wchar_t* filename)
	{
		Microsoft::WRL::ComPtr<IWICImagingFactory> factory;
		Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
		Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
		Microsoft::WRL::ComPtr<IWICFormatConverter> converter;

		ResetPosition();
		m_image.Reset();

		ThrowIfFailed(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, reinterpret_cast<LPVOID*>(factory.GetAddressOf())));
		ThrowIfFailed(factory->CreateDecoderFromFilename(filename, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, decoder.GetAddressOf()));
		ThrowIfFailed(decoder->GetFrame(0, frame.GetAddressOf()));
		ThrowIfFailed(factory->CreateFormatConverter(converter.GetAddressOf()));
		ThrowIfFailed(converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeMedianCut));
		ThrowIfFailed(renderTarget->CreateBitmapFromWicBitmap(converter.Get(), nullptr, m_image.ReleaseAndGetAddressOf()));
	}
	ImageView::ImageView(ID2D1RenderTarget* renderTarget, const wchar_t* filename) :
		m_position{ 0.0f, 0.0f },
		m_zoom(1.0f)
	{
		Load(renderTarget, filename);
	}
	void ImageView::ResetPosition()
	{
		m_position.x = 0.0f;
		m_position.y = 0.0f;
		m_zoom = 1.0f;
	}
	void ImageView::Moved(int dx, int dy, D2D1_SIZE_F windowSize)
	{
		D2D1_SIZE_F wndCoord = WindowCoord(windowSize);
		m_position.x += static_cast<float>(dx) * 2.0f * wndCoord.width / (windowSize.width * m_zoom);
		m_position.y += static_cast<float>(dy) * 2.0f * wndCoord.height / (windowSize.height * m_zoom);
	}
	void ImageView::Zoom(float zoom, D2D1_SIZE_F windowSize, D2D1_POINT_2L cursor)
	{
		m_zoom *= zoom;

		/*D2D1_SIZE_F wndCoord = WndCoord();
		D2D1_POINT_2F cursor = {
			(static_cast<float>(m_prevCursor.x) / m_wndSize.width * 2.0f - 1.0f) * wndCoord.width / m_zoom,
			(static_cast<float>(m_prevCursor.y) / m_wndSize.height * 2.0f - 1.0f) * wndCoord.height / m_zoom
		};

		m_imagePosition.x -= cursor.x * zoom;
		m_imagePosition.y -= cursor.y * zoom;*/
	}
	void ImageView::Draw(ID2D1RenderTarget* renderTarget, D2D1_SIZE_F windowSize)
	{
		renderTarget->DrawBitmap(m_image.Get(), ImagePosition(windowSize));
	}
}