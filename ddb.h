
#define PELS_72DPI  ((LONG)(72. / 0.0254))
struct DDB {
	DDB(int width, int height, bool opaque = true) : width(width), height(height)
	{
		HDC screen_dc = GetDC (NULL);

		dc = CreateCompatibleDC(screen_dc);
		bitmap = CreateCompatibleBitmap (screen_dc, width, height);

		saved_dc = (HDC)SelectObject(dc, bitmap);
		assert(saved_dc);
		assert(dc);
		assert(bitmap);

	}
	int width;
	int height;
	HDC dc;
	HBITMAP bitmap;
	HDC saved_dc;
	unsigned int *bits;
};



