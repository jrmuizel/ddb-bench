#include <windows.h>

long long timestamp()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return li.QuadPart;
}

double to_milliseconds(long long k)
{
    LARGE_INTEGER li;
    QueryPerformanceFrequency(&li);

    return 1000.*double(k)/li.QuadPart;
}
