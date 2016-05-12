#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "shlwapi")

#include <windows.h>
#include <shlwapi.h>

TCHAR szClassName[] = TEXT("Window");

VOID WriteFile(LPCTSTR lpszFileName, LPBYTE lpszText, DWORD dwSize)
{
	const HANDLE hFile = CreateFile(lpszFileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwWritten;
		WriteFile(hFile, lpszText, dwSize, &dwWritten, 0);
		CloseHandle(hFile);
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hStatic1;
	static HWND hEdit1;
	static HWND hStatic2;
	static HWND hEdit2;
	static HWND hButton;
	switch (msg)
	{
	case WM_CREATE:
		hStatic1 = CreateWindow(TEXT("STATIC"), TEXT("ファイルパス:"), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hEdit1 = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT("C:\\text.txt"), WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hStatic2 = CreateWindow(TEXT("STATIC"), TEXT("分割行数:"), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hEdit2 = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT("10"), WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL | ES_NUMBER, 0, 0, 0, 0, hWnd, (HMENU)100, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hButton = CreateWindow(TEXT("Button"), TEXT("分割"), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hWnd, (HMENU)IDOK, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		DragAcceptFiles(hWnd, TRUE);
		break;
	case WM_SIZE:
		MoveWindow(hStatic1, 10, 10, 128, 32, TRUE);
		MoveWindow(hEdit1, 10+128, 10, 512, 32, TRUE);
		MoveWindow(hStatic2, 10, 50, 128, 32, TRUE);
		MoveWindow(hEdit2, 10 + 128, 50, 64, 32, TRUE);
		MoveWindow(hButton, 10, 90, 128, 32, TRUE);
		break;
	case WM_DROPFILES:
		{
			const HDROP hDrop = (HDROP)wParam;
			const UINT nFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
			if (nFiles == 1)
			{
				TCHAR szFileName[MAX_PATH];
				DragQueryFile(hDrop, 0, szFileName, sizeof(szFileName));
				SetWindowText(hEdit1, szFileName);
			}
			DragFinish(hDrop);
		}
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			EnableWindow(hEdit1, FALSE);
			EnableWindow(hEdit2, FALSE);
			EnableWindow(hButton, FALSE);
			const DWORD dwSize = GetWindowTextLength(hEdit1);
			const int nLine = GetDlgItemInt(hWnd, 100, 0, 0);
			if (dwSize && nLine > 0)
			{
				LPTSTR lpszFilePath = (LPTSTR)GlobalAlloc(0, sizeof(TCHAR)*(dwSize + 1));
				GetWindowText(hEdit1, lpszFilePath, dwSize + 1);
				PathUnquoteSpaces(lpszFilePath);
				const HANDLE hFile1 = CreateFile(lpszFilePath, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
				if (hFile1 != INVALID_HANDLE_VALUE)
				{
					const DWORD dwFileSize = GetFileSize(hFile1, 0);
					DWORD dwReadSize;
					LPSTR lpszText = (LPSTR)GlobalAlloc(0, dwFileSize + 1);
					ReadFile(hFile1, lpszText, dwFileSize, &dwReadSize, 0);
					lpszText[dwReadSize] = 0;
					TCHAR szFileName[MAX_PATH];
					LPSTR pStart = lpszText;
					LPSTR pEnd = lpszText;
					int nLineCount = 0;
					int nFileCount = 0;
					TCHAR szOutputFileName[MAX_PATH];
					lstrcpy(szOutputFileName, lpszFilePath);
					PathRemoveExtension(szOutputFileName);
					while (true)
					{
						if (*pEnd == '\n' || *pEnd == '\0')
						{
							++nLineCount;
							if (nLineCount == nLine || *pEnd == '\0')
							{
								wsprintf(szFileName, TEXT("%s_%05d%s"), szOutputFileName, ++nFileCount, PathFindExtension(lpszFilePath));
								if (*pStart != '\0' )
									WriteFile(szFileName, (LPBYTE)pStart, (int)(pEnd - pStart + ((*pEnd == '\n') ? 1 : 0)));
								if (*pEnd == '\0')
									break;
								nLineCount = 0;
								++pEnd;
								pStart = pEnd;
							}
						}
						++pEnd;
					}
					GlobalFree(lpszText);
					CloseHandle(hFile1);
				}
				GlobalFree(lpszFilePath);
			}
			EnableWindow(hEdit1, TRUE);
			EnableWindow(hEdit2, TRUE);
			EnableWindow(hButton, TRUE);
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefDlgProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	MSG msg = { 0 };
	const WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		DLGWINDOWEXTRA,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		0,
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("Window"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		if (!IsDialogMessage(hWnd, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}
