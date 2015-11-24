/*******************************************************************************
 * Copyright 2015 Dimitri L. <dimdimdimdim at gmx dot fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
// This file is a mess... Well, it's not part of the real calculator anyway...
// Also, most of the acquired resources are not freed. But we acquire
// them once and reuse it all the time, so Windows will take care of
// deallocation once the program closes (at least we hope it does).

#include "stdafx.h"
#include "Geenie.h"
#include <base.h>
#include <thread.h>
#include <debugging.h>
#include <keyboard.h>
#include <util/list.h>
#include <util/string.h>
#include "threading.h"
#include "dbgconsole.h"
#include "display.h"
#include "keyboard.h"
#include "buzzer.h"
#include <system.h>

//#define ENABLE_GLASS

// memory
#define MEMORY_SIZE 128 // in kilobytes

namespace Kernel
{
struct MEMPOOL ramPools[] = {
	{ (uintptr_t)malloc(MEMORY_SIZE * 1024), MEMORY_SIZE * 1024 },
	{ 0, 0 }
};
Kernel::Allocator ram(ramPools);
}

using namespace Kernel;

uint8_t keyCode;
AuxKeyboardData auxKeyData;
Win32::CalcKeyboard *calcKeyboard;
Win32::ComputerKeyboard *computerKeyboard;

void startupCalc()
{
	System *system;

	{ // In this block, we fake the existence of a current thread
	  // This allows us to use gcnew
		uint8_t threadBytes[sizeof(SysThread)] = { 0 };
		Scheduler::_currentThread = (SysThread *)threadBytes; 
		DeviceClass::registerClass(gcnew(Win32::DbgConsoleClass));
		getDebugOut();
		DeviceClass::registerClass(gcnew(Win32::DisplayDeviceClass));
		gc<Win32::KeyboardDeviceClass *> kbdClass = gcnew(Win32::KeyboardDeviceClass);
		calcKeyboard = kbdClass->getDevice(String()).staticCast<Win32::CalcKeyboard *>();
		computerKeyboard = kbdClass->getDevice("aux").staticCast<Win32::ComputerKeyboard *>();
		DeviceClass::registerClass(kbdClass);
		DeviceClass::registerClass(gcnew(Win32::BuzzerDeviceClass));
		system = System::get();
	}
	// back to actual state
	Scheduler::_currentThread = NULL;

	Scheduler::init();
	Win32::dispatcher.start();
	system->start();
}


#define MAX_LOADSTRING	100

#define ID_OPTION		200
#define ID_CONSOLE		201

void				RegisterClasses(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	BtnProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	RamCpuGaugeProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

HWND hConsoleWindow = NULL;
HANDLE hConsoleScreen = NULL;
HANDLE hConsoleInput = NULL;

// Variables globales :
HINSTANCE hInst;
HWND hMainWindow;
HWND hGaugeCtrl;
TCHAR szTitle[MAX_LOADSTRING];
LPCTSTR szWindowClass = _T("Geenie Window Class");
LPCTSTR szButtonClass = _T("Geenie Button Class");
LPCTSTR szToolButtonClass = _T("Geenie Tool Button Class");
LPCTSTR szRamCpuGaugeClass = _T("Geenie RAM/CPU Gauge Class");
HBITMAP hBtnRectBmp[3];
HBITMAP hBtnUpBmp[3];
HBITMAP hBtnDownBmp[3];
HBITMAP hBtnLeftBmp[3];
HBITMAP hBtnRightBmp[3];
HBITMAP hToolBtnHover;
HBITMAP hToolBtnClick;
HBITMAP hRamCpuLeftBmp, hRamCpuRightBmp, hRamCpuEmptyBmp, hRamCpuFullBmp;
HBITMAP hScreenBmp;
HBITMAP hFrameBuffer;
HBRUSH hbrBackground;
void *pFrameBuffer;

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	MSG msg;
	HACCEL hAccelTable;

	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	RegisterClasses(hInstance);
	if (!InitInstance(hInstance, nCmdShow)) {
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GEENIE));

	// Boucle de messages principale :
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			//if (msg.message == WM_KEYDOWN || msg.message == WM_KEYUP || msg.message == WM_CHAR)
			//	SendMessage(hMainWindow, msg.message, msg.wParam, msg.lParam);
			//else
				DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

#ifdef ENABLE_GLASS
#define WINDOW_MARGIN_TOP				2
#define WINDOW_MARGIN_BOTTOM			10
#else
#define WINDOW_MARGIN_TOP				12
#define WINDOW_MARGIN_BOTTOM			14
#endif
#define WINDOW_MARGIN_LEFT				10
#define WINDOW_MARGIN_RIGHT				10
#define KEYBOARD_BUTTON_MARGIN_X		20
#define KEYBOARD_BUTTON_MARGIN_Y		16
#define KEYBOARD_BUTTON_WIDTH			51
#define KEYBOARD_BUTTON_HEIGHT			32
#define KEYBOARD_BUTTON_LRARROW_WIDTH	32
#define KEYBOARD_BUTTON_LRARROW_HEIGHT	46
#define KEYBOARD_BUTTON_UDARROW_WIDTH	60
#define KEYBOARD_BUTTON_UDARROW_HEIGHT	30
#define TOOL_BUTTON_WIDTH				24
#define TOOL_BUTTON_HEIGHT				24
#define COLOR_SHIFT						0x406080

#define BUTTON_ID_BASE					100

typedef void (* Callback)();

class ToolButton
{
private:
	int id;
	HBITMAP hBmp;
	TCHAR szTip[MAX_LOADSTRING];
	Callback action;
	bool hovered;
	bool clicked;
public:
	ToolButton(int id, Callback action)
	{
		this->id = id;
		this->action = action;
		hovered = false;
		clicked = false;
	}

	HWND create(int x, int y, HWND hWndParent, HWND hWndTip)
	{
		hBmp = LoadBitmap(hInst, MAKEINTRESOURCE(id));
		LoadString(hInst, id, szTip, MAX_LOADSTRING);
		HWND hWnd = CreateWindow(szToolButtonClass, NULL, WS_CHILD | WS_VISIBLE, x, y, TOOL_BUTTON_WIDTH, TOOL_BUTTON_HEIGHT, hWndParent, (HMENU)id, hInst, this);
		TTTOOLINFO toolInfo = { TTTOOLINFO_V1_SIZE };
		toolInfo.hwnd = hWndParent;
		toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
		toolInfo.uId = (UINT_PTR)hWnd;
		toolInfo.lpszText = szTip;
		SendMessage(hWndTip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
		return hWnd;
	}

	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
};

void toggleConsole();
void showOptionsMenu();

ToolButton tools[] = {
	ToolButton(IDC_TOOL_CONSOLE, toggleConsole),
	ToolButton(IDC_TOOL_OPTIONS, showOptionsMenu)
};

void updateScreen()
{
	RECT rect;
	rect.left = WINDOW_MARGIN_LEFT;
	rect.top = WINDOW_MARGIN_TOP;
	rect.right = WINDOW_MARGIN_LEFT + SCREEN_WIDTH;
	rect.bottom = WINDOW_MARGIN_TOP + SCREEN_HEIGHT;
	InvalidateRect(hMainWindow, &rect, FALSE);
}

void RegisterClasses(HINSTANCE hInstance)
{
#ifdef ENABLE_GLASS
	hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
#else
	hbrBackground = CreateSolidBrush(RGB(192, 192, 192));
#endif

	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GEENIE));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= hbrBackground;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_GEENIE));
	RegisterClassEx(&wcex);
	wcex.hIcon			= NULL;
	wcex.hIconSm		= NULL;

    wcex.lpfnWndProc	= BtnProc;
    wcex.lpszClassName	= szButtonClass;
    RegisterClassEx(&wcex);

    wcex.lpfnWndProc	= ToolButton::WndProc;
    wcex.lpszClassName	= szToolButtonClass;
    RegisterClassEx(&wcex);

    wcex.lpfnWndProc	= RamCpuGaugeProc;
    wcex.lpszClassName	= szRamCpuGaugeClass;
    RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance;

	hBtnRectBmp[0] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BTN_RECT));
	hBtnRectBmp[1] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BTN_RECT_HOVER));
	hBtnRectBmp[2] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BTN_RECT_CLICK));
	hBtnUpBmp[0] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BTN_UP));
	hBtnUpBmp[1] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BTN_UP_HOVER));
	hBtnUpBmp[2] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BTN_UP_CLICK));
	hBtnDownBmp[0] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BTN_DOWN));
	hBtnDownBmp[1] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BTN_DOWN_HOVER));
	hBtnDownBmp[2] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BTN_DOWN_CLICK));
	hBtnLeftBmp[0] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BTN_LEFT));
	hBtnLeftBmp[1] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BTN_LEFT_HOVER));
	hBtnLeftBmp[2] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BTN_LEFT_CLICK));
	hBtnRightBmp[0] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BTN_RIGHT));
	hBtnRightBmp[1] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BTN_RIGHT_HOVER));
	hBtnRightBmp[2] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BTN_RIGHT_CLICK));
	hToolBtnHover = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_TOOL_HOVER));
	hToolBtnClick = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_TOOL_CLICK));
	hScreenBmp = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_SCREEN_BKGND));
	hRamCpuLeftBmp = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_RAMCPU_LEFT));
	hRamCpuRightBmp = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_RAMCPU_RIGHT));
	hRamCpuEmptyBmp = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_RAMCPU_EMPTY));
	hRamCpuFullBmp = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_RAMCPU_FULL));

	BITMAPINFO *bmpInfo = (BITMAPINFO *)alloca(sizeof(BITMAPINFOHEADER) + 2 * sizeof(RGBQUAD));
	bmpInfo->bmiHeader.biSize = sizeof(BITMAPINFO);
	bmpInfo->bmiHeader.biWidth = SCREEN_WIDTH;
	bmpInfo->bmiHeader.biHeight = -SCREEN_HEIGHT;
	bmpInfo->bmiHeader.biPlanes = 1;
	bmpInfo->bmiHeader.biBitCount = 1;
	bmpInfo->bmiHeader.biCompression = BI_RGB;
	bmpInfo->bmiHeader.biSizeImage = 0;
	bmpInfo->bmiHeader.biXPelsPerMeter = 0;
	bmpInfo->bmiHeader.biYPelsPerMeter = 0;
	bmpInfo->bmiHeader.biXPelsPerMeter = 0;
	bmpInfo->bmiHeader.biClrUsed = 2;
	bmpInfo->bmiHeader.biClrImportant = 0;
	bmpInfo->bmiColors[0].rgbRed = 0;
	bmpInfo->bmiColors[0].rgbBlue = 0;
	bmpInfo->bmiColors[0].rgbGreen = 0;
	bmpInfo->bmiColors[0].rgbReserved = 0;
	bmpInfo->bmiColors[1].rgbRed = 255;
	bmpInfo->bmiColors[1].rgbBlue = 255;
	bmpInfo->bmiColors[1].rgbGreen = 255;
	bmpInfo->bmiColors[1].rgbReserved = 0;
	hFrameBuffer = CreateDIBSection(NULL, bmpInfo, DIB_RGB_COLORS, &pFrameBuffer, NULL, 0);

#ifdef ENABLE_GLASS
	DWORD style = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME& ~WS_MAXIMIZEBOX;
#else
	DWORD style = WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX;
#endif
	RECT wr = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	AdjustWindowRect(&wr, style, false);
	LONG width = wr.right - wr.left + KEYBOARD_BUTTON_WIDTH * 4 + KEYBOARD_BUTTON_MARGIN_X * 4 + WINDOW_MARGIN_LEFT + WINDOW_MARGIN_RIGHT;
	LONG height = wr.bottom - wr.top + KEYBOARD_BUTTON_HEIGHT * 4 + KEYBOARD_BUTTON_MARGIN_Y * 4 + WINDOW_MARGIN_TOP + WINDOW_MARGIN_BOTTOM;
	hWnd = CreateWindow(szWindowClass, szTitle, style, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, hInstance, NULL);
	if (!hWnd)
		return FALSE;
	hMainWindow = hWnd;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	//toggleConsole();
	startupCalc();

	return TRUE;
}

float lastCPUUsage = 0;
size_t lastRAMUsage = 0;
CString ramCpuUsageText;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	switch (message) {
		
	case WM_CREATE:
		{
			HWND hWndTip = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
									  WS_POPUP | TTS_ALWAYSTIP,
									  CW_USEDEFAULT, CW_USEDEFAULT,
									  CW_USEDEFAULT, CW_USEDEFAULT,
									  hWnd, NULL, 
									  hInst, NULL);
			LONG x;
			LONG y = WINDOW_MARGIN_TOP + SCREEN_HEIGHT + KEYBOARD_BUTTON_MARGIN_Y + 3;
			int id = BUTTON_ID_BASE;
			for (int j = 0; j < 4; j++) {
				x = WINDOW_MARGIN_LEFT - 3;
				for (int i = 0; i < 6; i++) {
					CreateWindow(szButtonClass, NULL, WS_CHILD | WS_VISIBLE, x, y, KEYBOARD_BUTTON_WIDTH + KEYBOARD_BUTTON_MARGIN_X, KEYBOARD_BUTTON_HEIGHT + KEYBOARD_BUTTON_MARGIN_Y, hWnd, (HMENU)(id++), hInst, NULL);
					x += KEYBOARD_BUTTON_WIDTH + KEYBOARD_BUTTON_MARGIN_X;
				}
				y += KEYBOARD_BUTTON_HEIGHT + KEYBOARD_BUTTON_MARGIN_Y;
			}
			LONG xx = x;
			y -= (KEYBOARD_BUTTON_HEIGHT + KEYBOARD_BUTTON_MARGIN_Y) * 6;
			for (int j = 0; j < 6; j++) {
				for (int i = 0; i < 4; i++) {
					CreateWindow(szButtonClass, NULL, WS_CHILD | WS_VISIBLE, x, y, KEYBOARD_BUTTON_WIDTH + KEYBOARD_BUTTON_MARGIN_X, KEYBOARD_BUTTON_HEIGHT + KEYBOARD_BUTTON_MARGIN_Y, hWnd, (HMENU)(id++), hInst, NULL);
					x += KEYBOARD_BUTTON_WIDTH + KEYBOARD_BUTTON_MARGIN_X;
				}
				y += KEYBOARD_BUTTON_HEIGHT + KEYBOARD_BUTTON_MARGIN_Y;
				x -= (KEYBOARD_BUTTON_WIDTH + KEYBOARD_BUTTON_MARGIN_X) * 4;
			}
			y -= (KEYBOARD_BUTTON_HEIGHT + KEYBOARD_BUTTON_MARGIN_Y) * 6 + (KEYBOARD_BUTTON_UDARROW_HEIGHT + KEYBOARD_BUTTON_MARGIN_Y) * 2 + 10;
			x += (KEYBOARD_BUTTON_WIDTH + KEYBOARD_BUTTON_MARGIN_X) * 2 - (KEYBOARD_BUTTON_UDARROW_WIDTH + KEYBOARD_BUTTON_MARGIN_X) / 2;
			CreateWindow(szButtonClass, NULL, WS_CHILD | WS_VISIBLE, x, y, KEYBOARD_BUTTON_UDARROW_WIDTH, KEYBOARD_BUTTON_UDARROW_HEIGHT, hWnd, (HMENU)(id++), hInst, NULL);
			y += KEYBOARD_BUTTON_UDARROW_HEIGHT + KEYBOARD_BUTTON_MARGIN_Y;
			CreateWindow(szButtonClass, NULL, WS_CHILD | WS_VISIBLE, x, y, KEYBOARD_BUTTON_UDARROW_WIDTH, KEYBOARD_BUTTON_UDARROW_HEIGHT, hWnd, (HMENU)(id++), hInst, NULL);
			y -= (KEYBOARD_BUTTON_LRARROW_HEIGHT + KEYBOARD_BUTTON_MARGIN_Y) / 2;
			x -= KEYBOARD_BUTTON_LRARROW_WIDTH + KEYBOARD_BUTTON_MARGIN_Y;
			CreateWindow(szButtonClass, NULL, WS_CHILD | WS_VISIBLE, x, y, KEYBOARD_BUTTON_LRARROW_WIDTH, KEYBOARD_BUTTON_LRARROW_HEIGHT, hWnd, (HMENU)(id++), hInst, NULL);
			x += KEYBOARD_BUTTON_LRARROW_WIDTH + KEYBOARD_BUTTON_UDARROW_WIDTH + KEYBOARD_BUTTON_MARGIN_Y * 2;
			CreateWindow(szButtonClass, NULL, WS_CHILD | WS_VISIBLE, x, y, KEYBOARD_BUTTON_LRARROW_WIDTH, KEYBOARD_BUTTON_LRARROW_HEIGHT, hWnd, (HMENU)(id++), hInst, NULL);

			int n = sizeof(tools) / sizeof(ToolButton);
			RECT rect;
			GetClientRect(hWnd, &rect);
			x = rect.right - 3;
			y = 3;
			for (int i = n - 1; i >= 0; i--) {
				x -= TOOL_BUTTON_WIDTH;
				tools[i].create(x, y, hWnd, hWndTip);
			}
			hGaugeCtrl = CreateWindow(szRamCpuGaugeClass, NULL, WS_CHILD | WS_VISIBLE, xx, y + 3, x - 8 - xx, 19, hWnd, NULL, hInst, NULL);
			TTTOOLINFO toolInfo = { TTTOOLINFO_V1_SIZE };
			toolInfo.hwnd = hWnd;
			toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
			toolInfo.uId = (UINT_PTR)hGaugeCtrl;
			toolInfo.lpszText = LPSTR_TEXTCALLBACK;
			SendMessage(hWndTip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
			SendMessage(hWndTip, TTM_SETMAXTIPWIDTH, 0, 200);
		}
		return DefWindowProc(hWnd, message, wParam, lParam);

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		switch (wmId) {
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;

	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hDC = BeginPaint(hWnd, &ps);
			RECT rect;
			rect.left = WINDOW_MARGIN_LEFT;
			rect.top = WINDOW_MARGIN_TOP;
			rect.bottom = WINDOW_MARGIN_TOP + SCREEN_HEIGHT;
			rect.right = WINDOW_MARGIN_LEFT + SCREEN_WIDTH;
#ifndef ENABLE_GLASS
			InflateRect(&rect, 3, 3);
			FrameRect(hDC, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
			InflateRect(&rect, -1, -1);
#endif
			HDC hBmpDC = CreateCompatibleDC(hDC);
			HGDIOBJ hOldBmp;
			hOldBmp = SelectObject(hBmpDC, hScreenBmp);
			StretchBlt(hDC, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, hBmpDC, 0, 0, 1, 240, SRCCOPY);
			SelectObject(hBmpDC, hFrameBuffer);
			BitBlt(hDC, WINDOW_MARGIN_LEFT, WINDOW_MARGIN_TOP, SCREEN_WIDTH, SCREEN_HEIGHT, hBmpDC, 0, 0, SRCAND);
			SelectObject(hBmpDC, hOldBmp);

			DeleteDC(hBmpDC);
			EndPaint(hWnd, &ps);
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_ACTIVATE:
		{
#ifdef ENABLE_GLASS
			// Extend the frame into the client area.
			MARGINS margins;
			margins.cxLeftWidth = WINDOW_MARGIN_LEFT;
			margins.cyTopHeight = WINDOW_MARGIN_TOP;
			margins.cxRightWidth = KEYBOARD_BUTTON_WIDTH * 4 + KEYBOARD_BUTTON_MARGIN_X * 4 + WINDOW_MARGIN_RIGHT;
			margins.cyBottomHeight = KEYBOARD_BUTTON_HEIGHT * 4 + KEYBOARD_BUTTON_MARGIN_Y * 4 + WINDOW_MARGIN_BOTTOM;
			DwmExtendFrameIntoClientArea(hWnd, &margins);
#endif
		}
		return DefWindowProc(hWnd, message, wParam, lParam);

	case WM_CHAR:
		if (wParam == '+') {
			auxKeyData.setCommand("+");
			auxKeyData.setMode(0);
			computerKeyboard->trigger();
		}
		else if (wParam == '-') {
			auxKeyData.setCommand("-");
			auxKeyData.setMode(0);
			computerKeyboard->trigger();
		}
		else if (wParam == '*') {
			auxKeyData.setCommand("*");
			auxKeyData.setMode(0);
			computerKeyboard->trigger();
		}
		else if (wParam == '/') {
			auxKeyData.setCommand("/");
			auxKeyData.setMode(0);
			computerKeyboard->trigger();
		}
		else if (wParam >= 32) {
			if (iswascii(wParam)) {
				if (iswupper(wParam))
					wParam = towlower(wParam);
				else if (iswlower(wParam))
					wParam = towupper(wParam);
			}
			auxKeyData.setInputChar(wParam);
			auxKeyData.setMode(0);
			computerKeyboard->trigger();
		}
		return 0;

	case WM_KEYDOWN:
		auxKeyData.setMode((GetKeyState(VK_SHIFT) & 0x8000) != 0 ? Kbd::SHIFT : 0);
		switch (wParam) {
		case VK_BACK:
			auxKeyData.setSpecial(KeyAction::BS); break;
		case VK_DELETE:
			auxKeyData.setSpecial(KeyAction::DEL); break;
		case VK_RETURN:
			auxKeyData.setSpecial(KeyAction::ENTER); break;
		case VK_UP:
			auxKeyData.setSpecial(KeyAction::UP); break;
		case VK_DOWN:
			auxKeyData.setSpecial(KeyAction::DOWN); break;
		case VK_LEFT:
			auxKeyData.setSpecial(KeyAction::LEFT); break;
		case VK_RIGHT:
			auxKeyData.setSpecial(KeyAction::RIGHT); break;
		case VK_ESCAPE:
			auxKeyData.setSpecial(KeyAction::ABORT); break;
		case VK_F1:
			auxKeyData.setSpecial(KeyAction::F1); break;
		case VK_F2:
			auxKeyData.setSpecial(KeyAction::F2); break;
		case VK_F3:
			auxKeyData.setSpecial(KeyAction::F3); break;
		case VK_F4:
			auxKeyData.setSpecial(KeyAction::F4); break;
		case VK_F5:
			auxKeyData.setSpecial(KeyAction::F5); break;
		case VK_F6:
			auxKeyData.setSpecial(KeyAction::F6); break;
		default:
			auxKeyData.setVoid();
		}
		if (!auxKeyData.isVoid())
			computerKeyboard->trigger();
		return 0;

	case WM_NOTIFY:
		{
			NMHDR *hdr = (NMHDR *)lParam;
			if (hdr->code == TTN_GETDISPINFO) {
				NMTTDISPINFO *ttdi = (NMTTDISPINFO *)lParam;
				if ((HWND)wParam == hGaugeCtrl) {
					ramCpuUsageText.Format(IDS_RAMCPU_USAGE, lastRAMUsage, MEMORY_SIZE * 1024, lastCPUUsage);
					ttdi->lpszText = (LPTSTR)(LPCTSTR)ramCpuUsageText;
				}
			}
			return 0;
		}

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

String getButtonCaption(const KeyAction *action)
{
	if (action->getType() == KeyAction::TYPE_SPECIAL) {
		switch (action->getId()) {
		case KeyAction::ON:					return "ON";
		case KeyAction::OFF:				return "OFF";
		case KeyAction::SHIFT:				return STR_SHIFT;
		case KeyAction::ALPHA:				return STR_ALPHA;
		case KeyAction::UP:					return "";
		case KeyAction::DOWN:				return "";
		case KeyAction::LEFT:				return "";
		case KeyAction::RIGHT:				return "";
		case KeyAction::ENTER:				return "ENTER";
		case KeyAction::BS:					return "\xE2\x86\x90";
		case KeyAction::DEL:				return "DEL";
		case KeyAction::F1:					return "F1";
		case KeyAction::F2:					return "F2";
		case KeyAction::F3:					return "F3";
		case KeyAction::F4:					return "F4";
		case KeyAction::F5:					return "F5";
		case KeyAction::F6:					return "F6";
		case KeyAction::CHANGE_SIGN:		return "+/-";
		}
		return "";
	}
	else {
		if (action->getType() == KeyAction::TYPE_INPUT_BRACES && action->getString() == "''")
			return "'";
		if (action->getType() == KeyAction::TYPE_COMMAND && action->getString() == "SQRT")
			return STR_SQRT "x";
		if (action->getType() == KeyAction::TYPE_COMMAND && action->getString() == "SQ")
			return "x\xC2\xB2";
		if (action->getType() == KeyAction::TYPE_COMMAND && action->getString() == "INV")
			return "1/x";
		if (action->getType() == KeyAction::TYPE_COMMAND && action->getString() == "^")
			return "x^y";
		if (action->getType() == KeyAction::TYPE_COMMAND && action->getString() == "NROOT")
			return "y" STR_SQRT "x";
		if (action->getType() == KeyAction::TYPE_COMMAND && action->getString() == "ALOG")
			return "10^x";
		if (action->getType() == KeyAction::TYPE_COMMAND && action->getString() == "EXP")
			return "e^x";
		if (action->getType() == KeyAction::TYPE_INPUT && action->getString() == " ")
			return "SPC";
		return action->getString();
	}
}

#define FLAG_HOVERED	1
#define FLAG_CLICKED	2

LRESULT CALLBACK BtnProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {

	case WM_PAINT:
		{
			LONG l = GetWindowLong(hWnd, GWL_USERDATA);
			LONG id = GetWindowLong(hWnd, GWL_ID);

			HTHEME hTheme = OpenThemeData(NULL, L"CompositedWindow::Window");
			if (hTheme) {
				RECT rect;
				GetClientRect(hWnd, &rect);
				PAINTSTRUCT ps;
				HDC hDC = BeginPaint(hWnd, &ps);
				HDC hBmpDC = CreateCompatibleDC(hDC);
				HGDIOBJ hOldBmp;
				HBITMAP *hBmpArray;
				int btnWidth, btnHeight;
				if (id == BUTTON_ID_BASE + 48) {
					btnWidth = KEYBOARD_BUTTON_UDARROW_WIDTH;
					btnHeight = KEYBOARD_BUTTON_UDARROW_HEIGHT;
					hBmpArray = hBtnUpBmp;
				}
				else if (id == BUTTON_ID_BASE + 49) {
					btnWidth = KEYBOARD_BUTTON_UDARROW_WIDTH;
					btnHeight = KEYBOARD_BUTTON_UDARROW_HEIGHT;
					hBmpArray = hBtnDownBmp;
				}
				else if (id == BUTTON_ID_BASE + 50) {
					btnWidth = KEYBOARD_BUTTON_LRARROW_WIDTH;
					btnHeight = KEYBOARD_BUTTON_LRARROW_HEIGHT;
					hBmpArray = hBtnLeftBmp;
				}
				else if (id == BUTTON_ID_BASE + 51) {
					btnWidth = KEYBOARD_BUTTON_LRARROW_WIDTH;
					btnHeight = KEYBOARD_BUTTON_LRARROW_HEIGHT;
					hBmpArray = hBtnRightBmp;
				}
				else {
					btnWidth = KEYBOARD_BUTTON_WIDTH;
					btnHeight = KEYBOARD_BUTTON_HEIGHT;
					hBmpArray = hBtnRectBmp;
				}
				if ((l & FLAG_CLICKED) != 0)
					hOldBmp = SelectObject(hBmpDC, hBmpArray[2]);
				else if ((l & FLAG_HOVERED) != 0)
					hOldBmp = SelectObject(hBmpDC, hBmpArray[1]);
				else
					hOldBmp = SelectObject(hBmpDC, hBmpArray[0]);
				HDC hBtnDC = CreateCompatibleDC(hDC);
				BITMAPINFO bmpInfo = { sizeof(BITMAPINFO) };
				bmpInfo.bmiHeader.biWidth = rect.right;
				bmpInfo.bmiHeader.biHeight = -rect.bottom;
				bmpInfo.bmiHeader.biPlanes = 1;
				bmpInfo.bmiHeader.biBitCount = 32;
				bmpInfo.bmiHeader.biCompression = BI_RGB;
				bmpInfo.bmiHeader.biSizeImage = 0;
				bmpInfo.bmiHeader.biXPelsPerMeter = 0;
				bmpInfo.bmiHeader.biYPelsPerMeter = 0;
				bmpInfo.bmiHeader.biXPelsPerMeter = 0;
				bmpInfo.bmiHeader.biClrUsed = 0;
				bmpInfo.bmiHeader.biClrImportant = 0;
				HBITMAP hBtnBmp = CreateDIBSection(hDC, &bmpInfo, DIB_RGB_COLORS, NULL, NULL, 0);

				const KeyAction *normal = getKeyAction(id - BUTTON_ID_BASE, 0);
				const KeyAction *shift = getKeyAction(id - BUTTON_ID_BASE, Kbd::SHIFT);
				const KeyAction *alpha = getKeyAction(id - BUTTON_ID_BASE, Kbd::ALPHA);

				HGDIOBJ hOldBtnBmp = SelectObject(hBtnDC, hBtnBmp);
#ifndef ENABLE_GLASS
				FillRect(hBtnDC, &rect, hbrBackground);
				BLENDFUNCTION bf;
				bf.BlendOp = AC_SRC_OVER;
				bf.BlendFlags = 0;
				bf.SourceConstantAlpha = 255;
				bf.AlphaFormat = AC_SRC_ALPHA;
				AlphaBlend(hBtnDC, 0, 0, btnWidth, btnHeight, hBmpDC, 0, 0, btnWidth, btnHeight, bf);
#else
				BitBlt(hBtnDC, 0, 0, btnWidth, btnHeight, hBmpDC, 0, 0, SRCCOPY);
#endif
				if (id < BUTTON_ID_BASE + 48) {
					LOGFONTW lgFont;
					HFONT hOldFont = NULL;
					if (GetThemeSysFont(hTheme, TMT_CAPTIONFONT, &lgFont) == S_OK) {
						HFONT hFont = CreateFontIndirect(&lgFont);
						hOldFont = (HFONT)SelectObject(hBtnDC, hFont);
					}

					DTTOPTS dttOpts = { sizeof(DTTOPTS) };
					RECT rect2;

					String caption = getButtonCaption(normal);
					String shiftCaption = getButtonCaption(shift);
					rect2.left = 0;
					rect2.right = KEYBOARD_BUTTON_WIDTH;
					if (normal->getType() == KeyAction::TYPE_SPECIAL && shift->getType() == KeyAction::TYPE_SPECIAL && normal->getId() == shift->getId()
						|| normal->getType() == KeyAction::TYPE_INPUT && shift->getType() == KeyAction::TYPE_INPUT && normal->getString() ==  shift->getString())
						rect2.top = 0;
					else
						rect2.top = KEYBOARD_BUTTON_HEIGHT / 2;
					rect2.bottom = KEYBOARD_BUTTON_HEIGHT - 3;
					dttOpts.dwFlags = DTT_COMPOSITED | DTT_GLOWSIZE;
					dttOpts.iGlowSize = 0;
					if (normal->getType() == KeyAction::TYPE_SPECIAL && normal->getId() == KeyAction::SHIFT) {
						dttOpts.dwFlags |= DTT_TEXTCOLOR;
						dttOpts.crText = COLOR_SHIFT;
					}
					DrawThemeTextEx(hTheme, hBtnDC, BP_PUSHBUTTON, PBS_NORMAL, CA2W(caption.getChars(), CP_UTF8), -1, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX, &rect2, &dttOpts);
					if (caption != shiftCaption) {
						dttOpts.dwFlags = DTT_COMPOSITED | DTT_TEXTCOLOR | DTT_GLOWSIZE;
						dttOpts.crText = COLOR_SHIFT;
						dttOpts.iGlowSize = 0;
						rect2.bottom = rect2.top;
						rect2.top = 1;
						DrawThemeTextEx(hTheme, hBtnDC, BP_PUSHBUTTON, PBS_NORMAL, CA2W(shiftCaption.getChars(), CP_UTF8), -1, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX, &rect2, &dttOpts);
					}

					rect2 = rect;
					rect2.left = KEYBOARD_BUTTON_WIDTH + 2;
					rect2.top = KEYBOARD_BUTTON_HEIGHT - 8;
					if (alpha->getType() == KeyAction::TYPE_INPUT && ((normal->getType() != KeyAction::TYPE_INPUT && normal->getType() != KeyAction::TYPE_COMMAND) || alpha->getString() != normal->getString())) {
						dttOpts.dwFlags = DTT_COMPOSITED | DTT_GLOWSIZE;
						dttOpts.iGlowSize = 15;
						DrawThemeTextEx(hTheme, hBtnDC, TEXT_LABEL, 0, CA2W(alpha->getString().getChars(), CP_UTF8), -1, DT_LEFT | DT_NOPREFIX, &rect2, &dttOpts);
					}

					if (hOldFont) {
						SelectObject(hBtnDC, hOldFont);
					}
				}

				BitBlt(hDC, 0, 0, rect.right, rect.bottom, hBtnDC, 0, 0, SRCCOPY);
				SelectObject(hBtnDC, hOldBtnBmp);
				DeleteDC(hBtnDC);
				DeleteObject(hBtnBmp);
				SelectObject(hBmpDC, hOldBmp);
				DeleteDC(hBmpDC);
				EndPaint(hWnd, &ps);
			}
			CloseThemeData(hTheme);
		}
		return 0;

	case WM_MOUSEMOVE:
		{
			LONG l = GetWindowLong(hWnd, GWL_USERDATA);
			WORD x = LOWORD(lParam);
			WORD y = HIWORD(lParam);
			bool b;
			if (GetWindowLong(hWnd, GWL_ID) >= BUTTON_ID_BASE + 48)
				b = true;
			else
				b = x < KEYBOARD_BUTTON_WIDTH && y < KEYBOARD_BUTTON_HEIGHT;
			if (b) {
				if ((l & FLAG_HOVERED) == 0) {
					TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT) };
					tme.dwFlags = TME_LEAVE;
					tme.hwndTrack = hWnd;
					TrackMouseEvent(&tme);
					SetWindowLong(hWnd, GWL_USERDATA, l | FLAG_HOVERED);
					InvalidateRect(hWnd, NULL, TRUE);
				}
			}
			else {
				if ((l & FLAG_HOVERED) != 0) {
					SetWindowLong(hWnd, GWL_USERDATA, l & ~FLAG_HOVERED);
					InvalidateRect(hWnd, NULL, TRUE);
				}
			}
		}
		return 0;

	case WM_MOUSELEAVE:
		{
			LONG l = GetWindowLong(hWnd, GWL_USERDATA);
			if ((l & FLAG_HOVERED) != 0) {
				SetWindowLong(hWnd, GWL_USERDATA, l & ~FLAG_HOVERED);
				InvalidateRect(hWnd, NULL, TRUE);
			}
		}
		return 0;

	case WM_LBUTTONDOWN:
		{
			LONG l = GetWindowLong(hWnd, GWL_USERDATA);
			if ((l & FLAG_HOVERED) != 0) {
				SetCapture(hWnd);
				if ((l & FLAG_CLICKED) == 0) {
					SetWindowLong(hWnd, GWL_USERDATA, l | FLAG_CLICKED);
					InvalidateRect(hWnd, NULL, TRUE);
					keyCode = (uint8_t)GetWindowLong(hWnd, GWL_ID) - BUTTON_ID_BASE;
					calcKeyboard->trigger();
				}
			}
		}
		return 0;

	case WM_LBUTTONUP:
		{
			ReleaseCapture();
			LONG l = GetWindowLong(hWnd, GWL_USERDATA);
			if ((l & FLAG_CLICKED) != 0) {
				SetWindowLong(hWnd, GWL_USERDATA, l &~ FLAG_CLICKED);
				InvalidateRect(hWnd, NULL, TRUE);
				keyCode = ((uint8_t)GetWindowLong(hWnd, GWL_ID) - BUTTON_ID_BASE) | 0x80;
				calcKeyboard->trigger();
			}
		}
		return 0;

	case WM_RBUTTONUP:
		{
			LONG l = GetWindowLong(hWnd, GWL_USERDATA);
			if ((l & FLAG_HOVERED) != 0) {
				SetWindowLong(hWnd, GWL_USERDATA, l ^ FLAG_CLICKED);
				InvalidateRect(hWnd, NULL, TRUE);
				keyCode = ((uint8_t)GetWindowLong(hWnd, GWL_ID) - BUTTON_ID_BASE) | ((l & FLAG_CLICKED) == 0 ? 0x00 : 0x80);
				calcKeyboard->trigger();
			}
		}
		return 0;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

LRESULT CALLBACK ToolButton::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ToolButton *btn;
	if (message == WM_CREATE) {
		CREATESTRUCT *cs = (CREATESTRUCT *)lParam;
		btn = (ToolButton *)cs->lpCreateParams;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)btn);
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	btn = (ToolButton *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	switch (message) {

	case WM_PAINT:
		{
			RECT rect;
			GetClientRect(hWnd, &rect);
			PAINTSTRUCT ps;
			HDC hDC = BeginPaint(hWnd, &ps);
#ifndef ENABLE_GLASS
			FillRect(hDC, &rect, hbrBackground);
#endif
			BLENDFUNCTION bf;
			bf.BlendOp = AC_SRC_OVER;
			bf.BlendFlags = 0;
			bf.SourceConstantAlpha = 255;
			bf.AlphaFormat = AC_SRC_ALPHA;
			HDC hBmpDC = CreateCompatibleDC(hDC);
			HGDIOBJ hOldBmp;
			if (btn->hovered || btn->clicked) {
				if (btn->hovered && btn->clicked)
					hOldBmp = SelectObject(hBmpDC, hToolBtnClick);
				else
					hOldBmp = SelectObject(hBmpDC, hToolBtnHover);
				AlphaBlend(hDC, 0, 0, rect.right, rect.bottom, hBmpDC, 0, 0, rect.right, rect.bottom, bf);
				SelectObject(hBmpDC, btn->hBmp);
			}
			else
				hOldBmp = SelectObject(hBmpDC, btn->hBmp);
			InflateRect(&rect, -4, -4);
			int w = rect.right - rect.left;
			int h = rect.bottom - rect.top;
			AlphaBlend(hDC, rect.left, rect.top, w, h, hBmpDC, 0, 0, w, h, bf);
			SelectObject(hBmpDC, hOldBmp);
			DeleteDC(hBmpDC);
			EndPaint(hWnd, &ps);
		}
		return 0;

	case WM_MOUSEMOVE:
		{
			WORD x = LOWORD(lParam);
			WORD y = HIWORD(lParam);
			RECT rect;
			GetClientRect(hWnd, &rect);
			if (x < rect.right && y < rect.bottom) {
				if (!btn->hovered) {
					TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT) };
					tme.dwFlags = TME_LEAVE;
					tme.hwndTrack = hWnd;
					TrackMouseEvent(&tme);
					btn->hovered = true;
					InvalidateRect(hWnd, NULL, TRUE);
				}
			}
			else {
				if (btn->hovered) {
					btn->hovered = false;
					InvalidateRect(hWnd, NULL, TRUE);
				}
			}
		}
		return 0;

	case WM_MOUSELEAVE:
		btn->hovered = false;
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;

	case WM_LBUTTONDOWN:
		SetCapture(hWnd);
		btn->clicked = true;
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;

	case WM_LBUTTONUP:
		ReleaseCapture();
		btn->clicked = false;
		if (btn->hovered) {
			InvalidateRect(hWnd, NULL, TRUE);
			btn->action();
		}
		return 0;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

namespace Kernel
{
	namespace Win32
	{
		float getCPUUsage();
	}
}

LRESULT CALLBACK RamCpuGaugeProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {

	case WM_CREATE:
		SetTimer(hWnd, 1, 500, NULL);
		lastCPUUsage = Kernel::Win32::getCPUUsage();
		lastRAMUsage = MEMORY_SIZE * 1024 - ram.getFreeMem();
		return DefWindowProc(hWnd, message, wParam, lParam);

	case WM_TIMER:
		lastCPUUsage = Kernel::Win32::getCPUUsage();
		lastRAMUsage = MEMORY_SIZE * 1024 - ram.getFreeMem();
		InvalidateRect(hWnd, NULL, FALSE);
		return 0;

	case WM_PAINT:
		{
			RECT rect;
			GetClientRect(hWnd, &rect);
			PAINTSTRUCT ps;
			HDC hDC = BeginPaint(hWnd, &ps);
#ifndef ENABLE_GLASS
			FillRect(hDC, &rect, hbrBackground);
#endif
			BLENDFUNCTION bf;
			bf.BlendOp = AC_SRC_OVER;
			bf.BlendFlags = 0;
			bf.SourceConstantAlpha = 255;
			bf.AlphaFormat = AC_SRC_ALPHA;
			HDC hBmpDC = CreateCompatibleDC(hDC);
			BITMAP bmp;
			HGDIOBJ hOldBmp = SelectObject(hBmpDC, hRamCpuLeftBmp);
			GetObject(hRamCpuLeftBmp, sizeof(bmp), &bmp);
			AlphaBlend(hDC, 0, 0, bmp.bmWidth, rect.bottom, hBmpDC, 0, 0, bmp.bmWidth, rect.bottom, bf);
			rect.left += bmp.bmWidth;
			SelectObject(hBmpDC, hRamCpuRightBmp);
			GetObject(hRamCpuRightBmp, sizeof(bmp), &bmp);
			AlphaBlend(hDC, rect.right - bmp.bmWidth, 0, bmp.bmWidth, rect.bottom, hBmpDC, 0, 0, bmp.bmWidth, rect.bottom, bf);
			rect.right -= bmp.bmWidth;
			int w1 = (int)(lastRAMUsage * (rect.right - rect.left) / (MEMORY_SIZE * 1024));
			int w2 = (int)(lastCPUUsage * (rect.right - rect.left) / 100);
			SelectObject(hBmpDC, hRamCpuFullBmp);
			AlphaBlend(hDC, rect.left, 0, w1, 9, hBmpDC, 0, 0, 1, 9, bf);
			AlphaBlend(hDC, rect.left, 10, w2, 9, hBmpDC, 0, 0, 1, 9, bf);
			SelectObject(hBmpDC, hRamCpuEmptyBmp);
			AlphaBlend(hDC, rect.left + w1, 0, rect.right - rect.left - w1, 9, hBmpDC, 0, 0, 1, 9, bf);
			AlphaBlend(hDC, rect.left + w2, 10, rect.right - rect.left - w2, 9, hBmpDC, 0, 0, 1, 9, bf);
			SelectObject(hBmpDC, hOldBmp);
			DeleteDC(hBmpDC);
			EndPaint(hWnd, &ps);
		}
		return 0;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
/*
WNDPROC oldConsoleWndProc;

LRESULT CALLBACK consoleWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message != WM_CLOSE)
		return oldConsoleWndProc(hWnd, message, wParam, lParam);
	return 0;
}
*/
void toggleConsole()
{
	static bool consoleVisible = false;
	if (hConsoleWindow == NULL) {
		AllocConsole();
		hConsoleWindow = GetConsoleWindow();
		if (hConsoleWindow != NULL) {
		   HMENU hMenu = ::GetSystemMenu(hConsoleWindow, FALSE);
		   if (hMenu != NULL)
			   DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
			/*oldConsoleWndProc = (WNDPROC)GetWindowLongPtr(hConsoleWindow, GWLP_WNDPROC);
			SetWindowLongPtr(hConsoleWindow, GWLP_WNDPROC, LONG_PTR(consoleWndProc));*/
		}
		SetConsoleTitle(_T("Geenie debug console"));
		AttachConsole(GetCurrentProcessId());
		SetConsoleOutputCP(CP_UTF8);
		hConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
		hConsoleScreen = GetStdHandle(STD_OUTPUT_HANDLE);
		consoleVisible = true;
	}
	else {
		if (consoleVisible) {
			ShowWindow(hConsoleWindow, SW_HIDE);
			consoleVisible = false;
		}
		else {
			ShowWindow(hConsoleWindow, SW_SHOW);
			consoleVisible = true;
		}
	}
}

void showOptionsMenu()
{
}
