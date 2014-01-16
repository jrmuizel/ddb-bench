#define WINVER 0x0501

#include <stdio.h>
#include <assert.h>
#include <windows.h>
#include <wingdi.h>
#include "dibsection.h"
#include "ddb.h"
#include "timestamp.h"

int width = 2048;
int height = 2048;
HDC tempdib;
void bench(HDC dst, HDC src, const char *dst_name, const char *src_name)
{
	double min = 10000000000000.0;
	double max = 0;
	double average = 0;
	int count = 100;
	for (int i=0; i<count; i++) {
		BOOL result;
		long long start = timestamp();
		result = BitBlt(dst, 0, 0, width, height, src, 0, 0, SRCCOPY);
		// readback the result
		result = BitBlt(tempdib, 0, 0, 1, 1, dst, 0, 0, SRCCOPY);
		GdiFlush();
		double time = to_milliseconds(timestamp() - start);
		average += time;
		if (time > max)
			max = time;
		if (time < min)
			min = time;
		if (!result)
			printf("result: %d\n", result);
	}
	average/=count;
	printf("%f [-%f +%f] %s -> %s\n", average, average-min, max-average, src_name, dst_name);

}

void alpha_bench(HDC dst, HDC src, const char *dst_name, const char *src_name)
{
	BLENDFUNCTION blend;
	blend.BlendOp = AC_SRC_OVER;
	blend.BlendFlags = 0;
	blend.SourceConstantAlpha = 255;
	blend.AlphaFormat = 0;

	double min = 10000000000000.0;
	double max = 0;
	double average = 0;
	int count = 50;
	for (int i=0; i<count; i++) {
		BOOL result;
		long long start = timestamp();
		result = AlphaBlend(dst, 0, 0, width, height, src, 0, 0, width, height, blend);
		double time = to_milliseconds(timestamp() - start);
		average += time;
		if (time > max)
			max = time;
		if (time < min)
			min = time;

		if (!result)
			printf("result: %d\n", result);
	}
	GdiFlush();
	average/=count;
	printf("%f [-%f +%f] %s -> %s\n", average, average-min, max-average, src_name, dst_name);

}

volatile bool ready;
DWORD WINAPI Spin( LPVOID lpParam )
{
	ready = true;
	while (1)
	{
		asm volatile ("");
	}
}

void CreateCPUSpin()
{
	CreateThread(NULL,
		     0,
		     Spin,
		     NULL,
		     0,
		     NULL);
}


DWORD WINAPI HogMem( LPVOID lpParam )
{
	int size = 2048*2048*4*32;
	char *src = (char*)malloc(size);
	char *dest = (char*)malloc(size);
	for (int i=0; i<size; i++) {
		src[i] = dest[i] = i;
	}

	ready = true;
	while (1)
	{
		memcpy(dest, src, size);
	}
}

void CreateMemHog()
{
	CreateThread(NULL,
		     0,
		     HogMem,
		     NULL,
		     0,
		     NULL);

}


int main(int argc, char **argv)
{

	DDB ddb_src(width, height);
	DDB ddb_dst(width, height);
	DIBSection src(width, height);
	DIBSection dest(width, height);
	DIBSection src_bi(width, height, DIBSection::BITFIELDS);
	DIBSection dest_bi(width, height, DIBSection::BITFIELDS);
	DIBSection src_bi_opaque(width, height, DIBSection::BITFIELDS_OPAQUE);
	DIBSection dest_bi_opaque(width, height, DIBSection::BITFIELDS_OPAQUE);
	DIBSection temp(width, height);
	tempdib = temp.dc;
	SYSTEM_INFO sysinfo;
	GetSystemInfo( &sysinfo );

	if (argc > 1) {
		if (argv[1][0] == 'c') {
			printf("spinning %d\n", (int)sysinfo.dwNumberOfProcessors);
			for (DWORD i=0; i<sysinfo.dwNumberOfProcessors; i++) {
				CreateCPUSpin();
			}
			while (!ready);
		}
		if (argv[1][0] == 'm') {
			printf("hogging mem\n");
			CreateMemHog();
			while (!ready);
		}
	}

	// ensure everything is initialized
	for (int y=0; y<height; y++) {
		for (int x=0; x<width; x++) {
			assert(dest.bits[y*dest.width + x] == 0);
			assert(src.bits[y*dest.width + x] == 0);
			dest.bits[y*dest.width + x] = x+4;
			src.bits[y*src.width + x] = x+4;
			dest_bi.bits[y*dest_bi.width + x] = x+4;
			src_bi.bits[y*src_bi.width + x] = x+4;
			dest_bi_opaque.bits[y*dest_bi_opaque.width + x] = x+4;
			src_bi_opaque.bits[y*src_bi_opaque.width + x] = x+4;
		}
	}

	// prime the surfaces
	BitBlt(ddb_dst.dc, 0, 0, width, height, src.dc, 0, 0, SRCCOPY);
	BitBlt(ddb_src.dc, 0, 0, width, height, src.dc, 0, 0, SRCCOPY);
	BitBlt(tempdib, 0, 0, 1, 1, ddb_dst.dc, 0, 0, SRCCOPY);
	BitBlt(tempdib, 0, 0, 1, 1, ddb_src.dc, 0, 0, SRCCOPY);

	printf("RGB\n");
	bench(ddb_dst.dc, ddb_src.dc, "ddb", "ddb");
	bench(ddb_dst.dc, src.dc, "ddb", "dibsection");
	bench(dest.dc, ddb_src.dc, "dibsection", "ddb");
	bench(dest.dc, src.dc, "dibsection", "dibsection");

#if 0
	// On an AMD discrete GPU with XP, Intel Integrated on XP and Win7
	// BITFIELDS and BITFIELDS_OPAQUE give similar results
	printf("BITFIELDS\n");
	bench(ddb_dst.dc, src_bi.dc, "ddb", "dibsection");
	bench(dest_bi.dc, ddb_src.dc, "dibsection", "ddb");
	bench(dest_bi.dc, src_bi.dc, "dibsection", "dibsection");
	printf("BITFIELDS_OPAQUE\n");
	bench(ddb_dst.dc, src_bi_opaque.dc, "ddb", "dibsection");
	bench(dest_bi_opaque.dc, ddb_src.dc, "dibsection", "ddb");
	bench(dest_bi_opaque.dc, src_bi_opaque.dc, "dibsection", "dibsection");
	printf("MIXED\n");
	bench(dest.dc, src_bi.dc, "dibsection", "dibsection");
	bench(dest.dc, src_bi_opaque.dc, "dibsection", "dibsection");
#endif

	printf("alpha\n");
	alpha_bench(ddb_dst.dc, ddb_src.dc, "ddb", "ddb");
	alpha_bench(ddb_dst.dc, src.dc, "ddb", "dibsection");
	alpha_bench(dest.dc, ddb_src.dc, "dibsection", "ddb");
	alpha_bench(dest.dc, src.dc, "dibsection", "dibsection");
	printf("text\n");
	for (int i=0; i<5; i++) {
	long long start = timestamp();
	memcpy(dest.bits, src.bits, width*height*sizeof(src.bits[0]));
	printf("%f memcpy\n", to_milliseconds(timestamp() - start));
	}

}
