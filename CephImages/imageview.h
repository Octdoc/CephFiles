#pragma once

#include "common.h"
#include <d2d1_3.h>
#include <wrl.h>

namespace cephimages
{
	class ImageView
	{
		D2D1_POINT_2F m_position;
		float m_zoom;
		Microsoft::WRL::ComPtr<ID2D1Bitmap> m_image;

	private:
		D2D1_SIZE_F WindowCoord(D2D1_SIZE_F windowSize);
		D2D1_RECT_F ImagePosition(D2D1_SIZE_F windowSize);
		void Load(ID2D1RenderTarget* renderTarget, const wchar_t* filename);

	public:
		ImageView(ID2D1RenderTarget* renderTarget, const wchar_t* filename);

		void ResetPosition();
		void Moved(int dx, int dy, D2D1_SIZE_F windowSize);
		void Zoom(float zoom, D2D1_SIZE_F windowSize, D2D1_POINT_2L cursor);
		void Draw(ID2D1RenderTarget* renderTarget, D2D1_SIZE_F windowSize);
	};
}