#include "textview.h"

namespace cephimages
{
	TextView::TextView(ID2D1RenderTarget* renderTarget, IDWriteFactory* factory, IDWriteTextFormat* format, const std::wstring& text)
	{
		ThrowIfFailed(factory->CreateTextLayout(text.c_str(), text.length(), format, 0, 0, &m_text));
		m_text->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	}
	void TextView::Draw(ID2D1RenderTarget* renderTarget, ID2D1Brush* brush, D2D1_SIZE_F windowSize)
	{
		m_text->SetMaxWidth(windowSize.width - 20.0f);
		m_text->SetMaxHeight(windowSize.height);
		renderTarget->DrawTextLayout(D2D1::Point2F(10.0f, windowSize.height * 0.5f - 30.0f), m_text.Get(), brush);
	}
}
