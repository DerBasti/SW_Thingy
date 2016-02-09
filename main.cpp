#include "D:\Programmieren\QuickInfos\QuickInfo.h"
#include "D:\Programmieren\DLL Injector\Injector.h"
#include <iostream>
#include <cstdio>
#include <vector>
#include "D:\Programmieren\QuickInfos\QuickInfo.h"
#include <Windows.h>

#pragma comment(lib, "Msimg32.lib")

template<class _Ty = DWORD, class = typename std::enable_if<std::_Is_integral<_Ty>::value>::type> class DimensionalArray {
	private:
		_Ty *data;
		DWORD size;
	public:
		DimensionalArray(const DWORD size) {
			this->size = size;
			this->data = new _Ty[this->size];
		}

		_Ty& operator[](const int pos) {
			return this->data[pos];
		}

		const DWORD getSize() const {
			return this->size;
		}
};

class Color {
private:
	Color() {}
public:
	byte toRed(DWORD c) {
		return (c & 0xFF);
	}

	byte toGreen(DWORD c) {
		return (c >> 8) & 0xFF;
	}

	byte toBlue(DWORD c) {
		return (c >> 16) & 0xFF;
	}

	float totalDifference(DWORD color, DWORD comparison) {
		byte colorRGB[] = { Color::toRed(color), Color::toGreen(color), Color::toBlue(color) };
		byte compRGB[] = { Color::toRed(comparison), Color::toGreen(comparison), Color::toBlue(comparison) };

		float diff = 0.0f;
		for (int i = 0; i < 3; i++) {
			diff += (colorRGB[i] - compRGB[i]) / 255.0f;
		}
		return diff;
	}
};

class Raycast {
private:
	Raycast() {}
public:
	template<class _Ty = DWORD> static float castColoredRay(const DimensionalArray<_Ty> *data, DWORD colorToMatch, POINT from, POINT to) {
		float xDistance = abs(from.x - to.x);
		float yDistance = abs(from.y - to.y);
		float totalDistance = sqrt(xDistance * xDistance + yDistance * yDistance);

		float xRatio = xDistance / totalDistance;
		float yRatio = yDistance / totalDistance;

		int pixelOverlap = 0;
		for (float y = from.y;y <= to.y;y += yRatio) {
			for (float x = from.x; x <= to.x; += xRatio) {
				DWORD colorOnPixel = data[(int)y][(int)x];
				float difference = Color::totalDifference(colorOnPixel, colorToMatch);
				if (difference <= 10.0f) {
					pixelOverlap++;
				}
			}
		}
		return pixelOverlap / (xDistance * yDistance);
	}

	template<class _Ty = DWORD> static float castRay(const DimensionalArray<_Ty> *data, const DimensionalArray<_Ty> *comparison, POINT from, POINT to) {
		float xDistance = abs(from.x - to.x);
		float yDistance = abs(from.y - to.y);
		float totalDistance = sqrt(xDistance * xDistance + yDistance * yDistance);

		float xRatio = xDistance / totalDistance;
		float yRatio = yDistance / totalDistance;

		int pixelOverlap = 0;
		for (float y = from.y; y <= to.y; y += yRatio) {
			for (float x = from.x; x <= to.x; += xRatio) {
				DWORD colorOnScreen = data[(int)y][(int)x];
				DWORD colorInBuffer = comparison[(int)y - from.y][(int)x - from.x];
				float difference = Color::totalDifference(colorOnPixel, colorInBuffer);
				if (difference <= 10.0f) {
					pixelOverlap++;
				}
			}
		}
		return pixelOverlap / (xDistance * yDistance);
	}
};

BOOL SaveToFile(HBITMAP hBitmap, LPCTSTR lpszFileName)
{
	HDC hDC;

	int iBits;

	WORD wBitCount;

	DWORD dwPaletteSize = 0, dwBmBitsSize = 0, dwDIBSize = 0, dwWritten = 0;

	BITMAP Bitmap;

	BITMAPFILEHEADER bmfHdr;

	BITMAPINFOHEADER bi;

	LPBITMAPINFOHEADER lpbi;

	HANDLE fh, hDib, hPal, hOldPal = NULL;

	hDC = CreateDC(L"DISPLAY", NULL, NULL, NULL);
	iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
	DeleteDC(hDC);
	if (iBits <= 1)
		wBitCount = 1;
	else if (iBits <= 4)
		wBitCount = 4;
	else if (iBits <= 8)
		wBitCount = 8;
	else
		wBitCount = 24;
	GetObject(hBitmap, sizeof(Bitmap), (LPSTR)&Bitmap);
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = Bitmap.bmWidth;
	bi.biHeight = Bitmap.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = wBitCount;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrImportant = 0;
	bi.biClrUsed = 0;
	dwBmBitsSize = ((Bitmap.bmWidth * wBitCount + 31) / 32) * 4 * Bitmap.bmHeight;

	hDib = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
	*lpbi = bi;

	hPal = GetStockObject(DEFAULT_PALETTE);
	if (hPal)
	{
		hDC = GetDC(NULL);
		hOldPal = SelectPalette(hDC, (HPALETTE)hPal, FALSE);
		RealizePalette(hDC);
	}


	GetDIBits(hDC, hBitmap, 0, (UINT)Bitmap.bmHeight, (LPSTR)lpbi + sizeof(BITMAPINFOHEADER)
		+dwPaletteSize, (BITMAPINFO *)lpbi, DIB_RGB_COLORS);

	if (hOldPal)
	{
		SelectPalette(hDC, (HPALETTE)hOldPal, TRUE);
		RealizePalette(hDC);
		ReleaseDC(NULL, hDC);
	}

	fh = CreateFile(lpszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (fh == INVALID_HANDLE_VALUE)
		return FALSE;

	bmfHdr.bfType = 0x4D42; // "BM"
	dwDIBSize = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+dwPaletteSize + dwBmBitsSize;
	bmfHdr.bfSize = dwDIBSize;
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER)+(DWORD)sizeof(BITMAPINFOHEADER)+dwPaletteSize;

	WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);

	WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL);
	GlobalUnlock(hDib);
	GlobalFree(hDib);
	CloseHandle(fh);
	return TRUE;
}

void test(HWND window, RECT *r) {
	HDC hdc = GetDC(window);
	HDC cpyDC = CreateCompatibleDC(hdc);
	HBITMAP cpyBitmap = CreateCompatibleBitmap(hdc, r->right - r->left, r->bottom - r->top);
	HBITMAP oldBitmap = (HBITMAP)SelectObject(cpyDC, cpyBitmap);

	int width = r->right - r->left;
	int height = r->bottom - r->top;
	BitBlt(cpyDC, 0, 0, width, height, hdc, 0, 0, SRCCOPY);
	SaveToFile(cpyBitmap, L"D:\\test_output.bmp");

	BITMAPINFOHEADER bmi = { 0 };
	bmi.biSize = sizeof(BITMAPINFOHEADER);
	bmi.biPlanes = 1;
	bmi.biBitCount = 32;
	bmi.biWidth = width;
	bmi.biHeight = -height;
	bmi.biCompression = BI_RGB;
	bmi.biSizeImage = 0;// 3 * ScreenX * ScreenY;

	byte *screenData = new byte[width * height * 4];
	GetDIBits(cpyDC, cpyBitmap, 0, height, screenData, (BITMAPINFO*)&bmi, DIB_RGB_COLORS);

	DimensionalArray<DWORD> *data = new DimensionalArray<DWORD>(height);
	for (int i = 0; i < height; i++) {
		data[i] = DimensionalArray<DWORD>(width);
		for (int j = 0; j < width; j++) {
			data[i][j] = (((DWORD*)screenData)[i * width + j]);
		}
	}
	delete[] screenData;
	screenData = nullptr;

	SelectObject(cpyDC, oldBitmap);

	delete[] data;
	data = nullptr;

	::DeleteObject(cpyBitmap);
	::DeleteDC(cpyDC);
	::ReleaseDC(window, hdc);
}

struct EnumAllResult {
	HWND parent;
	HWND targetWindow;
};

BOOL CALLBACK EnumAllWindows(HWND hwnd, LPARAM lParam) {
	EnumAllResult *current = (EnumAllResult*)lParam;
	if (GetWindow(hwnd, GW_OWNER) == current->parent) {
		current->targetWindow = hwnd;
		return FALSE;
	}
	return TRUE;
}

struct EnumChildrenResult {
	HWND windowHandle;
	RECT windowArea;

	EnumChildrenResult() {
		windowHandle = nullptr;
		windowArea = { 0 };
	}
};

BOOL CALLBACK getBiggestChildWindow(HWND hwnd, LPARAM lParam) {
	EnumChildrenResult *ecr = reinterpret_cast<EnumChildrenResult*>(lParam);

	if (ecr->windowHandle == nullptr) {
		ecr->windowHandle = hwnd;
		GetClientRect(hwnd, &ecr->windowArea);
		return TRUE;
	}

	RECT current = { 0 };
	GetClientRect(hwnd, &current);
	if ((ecr->windowArea.right - ecr->windowArea.left) < (current.right - current.left) &&
		(ecr->windowArea.bottom - ecr->windowArea.top) < (current.bottom - current.top)) {

		ecr->windowHandle = hwnd;
		ecr->windowArea = current;
	}

	return TRUE;
}

EnumChildrenResult getRecordingWindow(HWND mobizenWindow) {
	EnumAllResult subWindow;
	subWindow.parent = mobizenWindow;
	EnumWindows(EnumAllWindows, (LPARAM)&subWindow);

	EnumChildrenResult ecr;
	EnumChildWindows(subWindow.targetWindow, getBiggestChildWindow, (LPARAM)&ecr);

	printf("%i, %i, %i, %i\n", ecr.windowArea.left, ecr.windowArea.right, ecr.windowArea.top, ecr.windowArea.bottom);
	return ecr;
}


int main() {
	HWND window;
	QuickInfo::isWindowExistent(L"Mobizen", L"mobizen.exe", &window);
	EnumChildrenResult ecr = getRecordingWindow(window);
	if (ecr.windowHandle) {
		test(ecr.windowHandle, &ecr.windowArea);
	}
 	Sleep(10000);
	return 0;
}