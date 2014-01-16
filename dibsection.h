
#define PELS_72DPI  ((LONG)(72. / 0.0254))
struct DIBSection {
	DIBSection(int width, int height, bool opaque = false) : width(width), height(height)
	{
		struct {
			BITMAPINFOHEADER bmiHeader;
			RGBQUAD          bmiColors[3];
		} bmi = {};
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = width;
		bmi.bmiHeader.biHeight = -height; // top down
		bmi.bmiHeader.biSizeImage = 0;
		bmi.bmiHeader.biXPelsPerMeter = PELS_72DPI;
		bmi.bmiHeader.biYPelsPerMeter = PELS_72DPI;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		if (opaque) {
			bmi.bmiHeader.biCompression = BI_BITFIELDS;
			bmi.bmiColors[0].rgbRed = 0xff;
			bmi.bmiColors[1].rgbGreen = 0xff;
			bmi.bmiColors[2].rgbBlue = 0xff;
		} else {
			bmi.bmiHeader.biCompression = BI_RGB;
		}
		bmi.bmiHeader.biClrUsed = 0;
		bmi.bmiHeader.biClrImportant = 0;

		union {
			void *v;
			unsigned int *i;
		} output_bits;

		BITMAPINFO *bitmap_info = reinterpret_cast<BITMAPINFO*>(&bmi);
		dc = CreateCompatibleDC(NULL);
		HBITMAP bitmap = CreateDIBSection(dc, bitmap_info, DIB_RGB_COLORS, &output_bits.v, NULL, 0);
		saved_dc = (HDC)SelectObject(dc, bitmap);
		assert(saved_dc);
		assert(dc);
		assert(bitmap);
		bits = output_bits.i;

	}
	int width;
	int height;
	HDC dc;
	HDC saved_dc;
	unsigned int *bits;
};



