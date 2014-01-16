
#define PELS_72DPI  ((LONG)(72. / 0.0254))
struct DIBSection {
	enum dibstyle {
		RGB,
		BITFIELDS,
		BITFIELDS_OPAQUE,
	};
	DIBSection(int width, int height, enum dibstyle style = RGB) : width(width), height(height)
	{
		BITMAPV4HEADER bmi = {};
		bmi.bV4Size = sizeof(BITMAPV4HEADER);
		bmi.bV4Width = width;
		bmi.bV4Height = -height; // top down
		bmi.bV4SizeImage = 0;
		bmi.bV4XPelsPerMeter = PELS_72DPI;
		bmi.bV4YPelsPerMeter = PELS_72DPI;
		bmi.bV4Planes = 1;
		bmi.bV4BitCount = 32;
		if (style == RGB) {
			bmi.bV4V4Compression = BI_RGB;
		} else {
			// I believe it's possible to use to use
			// the bitfields version of things
			// to achieve a RGB -> ARGB blit that
			// properly sets the alpha values to
			// 0xff
			bmi.bV4V4Compression = BI_BITFIELDS;
			bmi.bV4RedMask    = 0x00ff0000;
			bmi.bV4GreenMask  = 0x0000ff00;
			bmi.bV4BlueMask   = 0x000000ff;
			if (style == BITFIELDS_OPAQUE)
				bmi.bV4AlphaMask = 0x00000000;
			else
				bmi.bV4AlphaMask = 0xff000000;
		}
		bmi.bV4ClrUsed = 0;
		bmi.bV4ClrImportant = 0;

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



