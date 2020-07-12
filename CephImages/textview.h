#pragma once

#include "common.h"
#include <d2d1_3.h>
#include <dwrite.h>
#include <wrl.h>

namespace cephimages
{
	class TextView
	{
		Microsoft::WRL::ComPtr<IDWriteTextLayout> m_text;

	public:
		TextView(ID2D1RenderTarget* renderTarget, IDWriteFactory* factory, IDWriteTextFormat* format, const std::wstring& text);

		void Draw(ID2D1RenderTarget* renderTarget, ID2D1Brush* brush, D2D1_SIZE_F windowSize);
	};
}