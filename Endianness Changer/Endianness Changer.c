// Endianness Changer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Endianness Changer.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
const int WIDTH = 450;
const int HEIGHT = 225;
HWND  srcLabel;
HWND  srcTextBox;
HWND  srcBrowseButton;
HWND  desLabel;
HWND  desTextBox;
HWND  desBrowseButton;
HWND  sameCheckBox;
HWND  goButton;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    INITCOMMONCONTROLSEX picce;
    picce.dwICC  =  ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES;
    picce.dwSize = sizeof(picce);
    InitCommonControlsEx(&picce);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_ENDIANNESSCHANGER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ENDIANNESSCHANGER));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ENDIANNESSCHANGER));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_ENDIANNESSCHANGER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable
   const int XPOS = (GetSystemMetrics(SM_CXSCREEN) - WIDTH ) / 2;
   const int YPOS = (GetSystemMetrics(SM_CYSCREEN) - HEIGHT) / 2;

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      XPOS, YPOS, WIDTH, HEIGHT, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        srcLabel = CreateWindowW(WC_STATICW, L"Source file:",
                                 WS_CHILD | WS_VISIBLE,
                                 CW_USEDEFAULT,
                                 CW_USEDEFAULT,
                                 CW_USEDEFAULT,
                                 CW_USEDEFAULT,
                                 hWnd, NULL, NULL, NULL);
        
        srcTextBox = CreateWindowExW(WS_EX_CLIENTEDGE, WC_EDITW, NULL,
                                  WS_CHILD | WS_VISIBLE,
                                  CW_USEDEFAULT,
                                  CW_USEDEFAULT,
                                  CW_USEDEFAULT,
                                  CW_USEDEFAULT,
                                  hWnd, NULL, NULL, NULL);

        srcBrowseButton = CreateWindowW(WC_BUTTONW, L"...",
            WS_CHILD | WS_VISIBLE ,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            hWnd, NULL, NULL, NULL);
        desLabel = CreateWindowW(WC_STATICW, L"Destination file:",
            WS_CHILD | WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            hWnd, NULL, NULL, NULL);

        desTextBox = CreateWindowExW(WS_EX_CLIENTEDGE,WC_EDITW, NULL,
            WS_CHILD | WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            hWnd, NULL, NULL, NULL);

        desBrowseButton = CreateWindowW(WC_BUTTONW, L"...",
            WS_CHILD | WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            hWnd, NULL, NULL, NULL);

        sameCheckBox = CreateWindowW(WC_BUTTONW,L"Write changes to source file",
            WS_CHILD | WS_VISIBLE | BS_CHECKBOX,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            hWnd, NULL, NULL, NULL);

        goButton = CreateWindowW(WC_BUTTONW, L"Run",
            WS_CHILD | WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            hWnd, NULL, NULL, NULL);

        NONCLIENTMETRICS metrics;
        metrics.cbSize = sizeof(NONCLIENTMETRICS);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS),
            &metrics, 0);
        HFONT font = CreateFontIndirect(&metrics.lfMessageFont);
        SendMessageW(srcLabel        , WM_SETFONT, (WPARAM)font, FALSE);
        SendMessageW(srcTextBox      , WM_SETFONT, (WPARAM)font, FALSE);
        SendMessageW(srcBrowseButton , WM_SETFONT, (WPARAM)font, FALSE);
        SendMessageW(desLabel        , WM_SETFONT, (WPARAM)font, FALSE);
        SendMessageW(desTextBox      , WM_SETFONT, (WPARAM)font, FALSE);
        SendMessageW(desBrowseButton , WM_SETFONT, (WPARAM)font, FALSE);
        SendMessageW(sameCheckBox    , WM_SETFONT, (WPARAM)font, FALSE);
        SendMessageW(goButton        , WM_SETFONT, (WPARAM)font, FALSE);
        break;
    }
    case WM_COMMAND:
        break;
    case WM_SIZE:
    {
        const WWIDTH               = LOWORD(lParam);
        const WHEIGHT              = HIWORD(lParam);
        const HMARGIN              = 20;
        const VMARGIN              = 10;
        const VGAP                 = 5;
        const BUTTON_HEIGHT        = 25;
        const BROWSE_BUTTON_WIDTH  = 30;
        const BUTTON_WIDTH         = 75;
        const LABEL_HEIGHT         = 15;
        const TEXTBOX_HEIGHT       = 23;
        const AREA_WIDTH           = WWIDTH - (2 * HMARGIN);

        int currentY = VMARGIN;
        HDWP positions = BeginDeferWindowPos(10);
        DeferWindowPos(positions,
                       srcLabel,
                       NULL,
                       HMARGIN,
                       currentY,
                       AREA_WIDTH, LABEL_HEIGHT, 0);
        currentY += LABEL_HEIGHT + VGAP;
        DeferWindowPos(positions,
            srcTextBox,
            NULL,
            HMARGIN,
            currentY,
            AREA_WIDTH - BROWSE_BUTTON_WIDTH - 5, TEXTBOX_HEIGHT, 0);

        DeferWindowPos(positions,
            srcBrowseButton,
            NULL,
            HMARGIN + AREA_WIDTH - BROWSE_BUTTON_WIDTH,
            currentY-1,
            BROWSE_BUTTON_WIDTH, BUTTON_HEIGHT, 0);
        currentY += TEXTBOX_HEIGHT + VGAP;

        DeferWindowPos(positions,
            desLabel,
            NULL,
            HMARGIN,
            currentY,
            AREA_WIDTH, LABEL_HEIGHT, 0);
        currentY += LABEL_HEIGHT + VGAP;

        DeferWindowPos(positions,
            desTextBox,
            NULL,
            HMARGIN,
            currentY,
            AREA_WIDTH - BROWSE_BUTTON_WIDTH - 5, TEXTBOX_HEIGHT, 0);

        DeferWindowPos(positions,
            desBrowseButton,
            NULL,
            HMARGIN + AREA_WIDTH - BROWSE_BUTTON_WIDTH,
            currentY - 1,
            BROWSE_BUTTON_WIDTH, BUTTON_HEIGHT, 0);
        currentY += TEXTBOX_HEIGHT + VGAP;

        DeferWindowPos(positions,
            sameCheckBox,
            NULL,
            HMARGIN,
            currentY,
            AREA_WIDTH, BUTTON_HEIGHT, 0);

        DeferWindowPos(positions,
            goButton,
            NULL,
            HMARGIN + (AREA_WIDTH - BUTTON_WIDTH)/2,
            WHEIGHT-VMARGIN-BUTTON_HEIGHT,
            BUTTON_WIDTH, BUTTON_HEIGHT, 0);

        EndDeferWindowPos(positions);
        return DefWindowProc(hWnd, message, wParam, MAKELPARAM(WWIDTH, WHEIGHT));
    }
        break;
    case WM_GETMINMAXINFO:
    {
        LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
        lpMMI->ptMinTrackSize.x = WIDTH;
        lpMMI->ptMaxTrackSize.y = HEIGHT;
        lpMMI->ptMinTrackSize.y = HEIGHT;
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

