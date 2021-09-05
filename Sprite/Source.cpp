#include <windows.h>
#include <tchar.h>
#include <corecrt_math.h>
#include <gdiplus.h>

using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

#define RECT_ID 0
#define BMP_ID 1

const HBRUSH WIN_BACKGROUND_BRUSH = CreateSolidBrush(RGB(175, 238, 238));
const HBRUSH RECT_BRUSH = CreateSolidBrush(RGB(123, 104, 238));

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void drawBmp(HDC hdc, int x, int y, HBITMAP hBitmap);
HBITMAP PngToBitmap(WCHAR* pngFilePath);


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	HWND hWnd;
	WNDCLASSEX wc{ sizeof(WNDCLASSEX) };

	LPCWSTR const className = L"MyAppClass";
	LPCWSTR windowName = L"SPRITE";
	
	//Fill windowClass structure
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = WIN_BACKGROUND_BRUSH;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = className;
	wc.style = CS_DBLCLKS;

	if (!RegisterClassEx(&wc)) {
		return EXIT_FAILURE;
	}

	hWnd = CreateWindowEx(
		0, className,
		windowName,
		WS_OVERLAPPEDWINDOW, 
		200, 200,  
		600, 500, 
		NULL,    
		NULL,    
		hInstance,
		NULL      
	);
	
	if (!hWnd) {
		const wchar_t* const WND_CREATE_ERROR = L"Cannot create window";
		MessageBoxW(NULL, WND_CREATE_ERROR, NULL, MB_OK);
		return 0;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, NULL, 0, 0))  //message processing cycle
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static int id = RECT_ID;
	PAINTSTRUCT ps;
	HDC hdc, memDC;
	RECT clientRect;
	static HBRUSH hbrush;
	WCHAR picture[] = L"cat.png";
	static HBITMAP myBmp;
	static int x = 200;
	static int y = 200;
	static int size = 70;
	static int vx, vy, alpha;
	static int moveStat = 0;
	int mouseOffset;

	switch (message) {
	case WM_CREATE: {
		hbrush = RECT_BRUSH;
		myBmp = PngToBitmap(picture);
		break;
	}
	case WM_DESTROY: {
		if (moveStat) {
			KillTimer(hWnd, 1);
		}
		DeleteObject(hbrush);
		DeleteObject(myBmp);
		PostQuitMessage(0);
		break;
	}
	case WM_LBUTTONDOWN: {
		if (!moveStat) {
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			alpha = round((rand() % 360 - 180) * 3.14 / 180.0);
			vx = 10.0 * cos(alpha);
			vy = 10.0 * sin(alpha);
			SetTimer(hWnd, 1, 25, NULL);
			moveStat = 1;
		}
		else {
			KillTimer(hWnd, 1);
			moveStat = 0;
		}
		break;
	}
	case WM_TIMER:
	{
		x += vx;
		y += vy;
		GetClientRect(hWnd, &clientRect);
		if ((x + size) >= clientRect.right)
			vx = -abs(vx);
		if (x < clientRect.left)
			vx = abs(vx);
		if ((y + size) > clientRect.bottom)
			vy = -abs(vy);
		if (y < clientRect.top)
			vy = abs(vy);
		x--;
		y--;
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	}
	case WM_PAINT: {
		hdc = BeginPaint(hWnd, &ps);
		if (id == RECT_ID) {
			SelectObject(hdc, hbrush);
			Rectangle(hdc, x, y, x + size, y + size);
		}
		else {			
			drawBmp(hdc, x, y, myBmp);			
		}
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_KEYDOWN:
	{
		if (!moveStat) {
			GetClientRect(hWnd, &clientRect);
			switch (wParam) {
			case VK_LEFT: {
				x -= 10;
				if (x < clientRect.left)
					x += 30;
				break;
			}
			case VK_RIGHT: {
				x += 10;
				if ((x + size) > clientRect.right)
					x -= 30;
				break;
			}
			case VK_UP: {
				y -= 10;
				if (y < clientRect.top)
					y += 30;
				break;
			}
			case VK_DOWN: {
				y += 10;
				if ((y + size) > clientRect.bottom)
					y -= 30;
				break;
			}
			case VK_RETURN: {
				if (id == RECT_ID)
					id = BMP_ID;
				else
					id = RECT_ID;
				break;
			}
			}
			InvalidateRect(hWnd, NULL, TRUE);
		}
		break;
	}
	case WM_MOUSEWHEEL: {
		if (!moveStat) {
			GetClientRect(hWnd, &clientRect);
			mouseOffset = GET_WHEEL_DELTA_WPARAM(wParam);
			if (LOWORD(wParam) == MK_SHIFT) {
				if (mouseOffset > 0) {
					x += 10;
					if ((x + size) > clientRect.right)
						x -= 20;
				}
				else {
					x -= 10;
					if (x < clientRect.left)
						x += 20;
				}
			}
			else {
				if (mouseOffset > 0) {
					y -= 10;
					if (y < clientRect.top)
						y += 20;
				}
				else {
					y += 10;
					if ((y + size) > clientRect.bottom)
						y -= 20;
				}
			}
			InvalidateRect(hWnd, NULL, TRUE);
		}
		break;
	}
	case WM_MOUSEMOVE: {

		if (!moveStat) {
			int oldX = x;
			int oldY = y;
			GetClientRect(hWnd, &clientRect);
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			if (x < clientRect.left)
				x = oldX;
			if ((x + size) > clientRect.right)
				x = oldX;
			if (y < clientRect.top)
				y = oldY;
			if ((y + size) > clientRect.bottom)
				y = oldY;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		break;
	}
	default: return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void drawBmp(HDC hdc, int x, int y, HBITMAP hBitmap) {
	HBITMAP hNewBmp;

	HDC compDc;
	BITMAP bmp;
	int bmpWidth, bmpHeight;

	compDc = CreateCompatibleDC(hdc);

	hNewBmp = (HBITMAP)SelectObject(compDc, hBitmap);

	if (hNewBmp) {
		SetMapMode(compDc, GetMapMode(hdc));
		GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bmp);

		bmpWidth = bmp.bmWidth;
		bmpHeight = bmp.bmHeight;

		POINT bmpSize;
		bmpSize.x = bmpWidth;
		bmpSize.y = bmpHeight;

		DPtoLP(hdc, &bmpSize, 1);

		POINT point;
		point.x = 0;
		point.y = 0;

		DPtoLP(compDc, &point, 1);
		BitBlt(hdc, x, y, bmpWidth, bmpHeight, compDc, point.x, point.y, SRCCOPY);
		SelectObject(compDc, hNewBmp);
	}

	DeleteDC(compDc);
}

HBITMAP PngToBitmap(WCHAR* pngFilePath) {
	GdiplusStartupInput test;
	ULONG_PTR token;
	GdiplusStartup(&token, &test, NULL);
	Color Back = Color(Color::MakeARGB(0, 175, 238, 238));
	HBITMAP convertedBitmap = NULL;
	Bitmap* Bitmap = Bitmap::FromFile(pngFilePath, false);
	if (Bitmap) {
		Bitmap->GetHBITMAP(Back, &convertedBitmap);
		delete Bitmap;
	}
	GdiplusShutdown(token);
	return convertedBitmap;
}

