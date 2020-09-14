#pragma once

#include "common.h"
#include <d2d1_3.h>
#include <wrl.h>

namespace cephimages
{
	class ImageView
	{
	public:
		enum class FillMode
		{
			Fit,
			Fill,
			OneToOne
		};

	private:
		D2D1_POINT_2F m_position;
		float m_zoom;
		D2D1_SIZE_F(ImageView::* m_WindowCoord)(D2D1_SIZE_F windowSize);
		Microsoft::WRL::ComPtr<ID2D1Bitmap> m_image;
		D2D1_BITMAP_INTERPOLATION_MODE m_interpolation;

	private:
		D2D1_SIZE_F WindowCoord(D2D1_SIZE_F windowSize);
		D2D1_SIZE_F WindowCoord_Fit(D2D1_SIZE_F windowSize);
		D2D1_SIZE_F WindowCoord_Fill(D2D1_SIZE_F windowSize);
		D2D1_SIZE_F WindowCoord_OneToOne(D2D1_SIZE_F windowSize);
		D2D1_RECT_F ImagePosition(D2D1_SIZE_F windowSize);
		void Load(ID2D1RenderTarget* renderTarget, const wchar_t* filename);

	public:
		ImageView(ID2D1RenderTarget* renderTarget, const wchar_t* filename);

		void ResetPosition();
		void SetFillMode(FillMode fillMode);
		void SetInterpolation(bool interpolate);
		void Moved(int dx, int dy, D2D1_SIZE_F windowSize);
		void Zoom(float zoom, D2D1_SIZE_F windowSize, D2D1_POINT_2L cursor);
		void Draw(ID2D1RenderTarget* renderTarget, D2D1_SIZE_F windowSize);
	};
}