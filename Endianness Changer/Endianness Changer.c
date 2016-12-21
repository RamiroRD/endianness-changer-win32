// Endianness Changer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Endianness Changer.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
const MIN_WIDTH = 350;
const MIN_HEIGHT = 240;
const WIDTH = 450;
const HEIGHT = 240;
const MAX_WIDTH = 1000;
const MAX_HEIGHT = 240;
HWND  srcLabel;
HWND  srcTextBox;
HWND  srcBrowseButton;
HWND  desLabel;
HWND  desTextBox;
HWND  desBrowseButton;
HWND  sameCheckBox;
HWND  wordSelect;
HWND  wordLabel;
HWND  goButton;
WCHAR  srcPath[MAX_PATH];
WCHAR  desPath[MAX_PATH];
const UCHAR WORD_SIZES = 4;
const WCHAR * WORD_SIZES_STRINGS[4] = { L"2", L"4", L"8", L"16" };
const WCHAR * fuckyou = L"Ugh";
// Forward declarations of functions included in this code module:
ATOM                registerWindowClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                InitCommCtl(void);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    InitCommCtl();
    
    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_ENDIANNESSCHANGER, szWindowClass, MAX_LOADSTRING);
    registerWindowClass(hInstance);

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


ATOM registerWindowClass(HINSTANCE hInstance)
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

void createWidgets(HWND hWnd)
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
        WS_CHILD | WS_VISIBLE,
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

    desTextBox = CreateWindowExW(WS_EX_CLIENTEDGE, WC_EDITW, NULL,
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

    wordLabel = CreateWindowW(WC_STATICW, L"Word size (bytes):",
        WS_CHILD | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        hWnd, NULL, NULL, NULL);

    wordSelect = CreateWindowW(WC_COMBOBOXW, NULL,
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        hWnd, NULL, NULL, NULL);

    sameCheckBox = CreateWindowW(WC_BUTTONW, L"Write changes to source file",
        WS_CHILD | WS_VISIBLE | BS_CHECKBOX,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        hWnd, NULL, NULL, NULL);

    goButton = CreateWindowW(WC_BUTTONW, L"Convert",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        hWnd, NULL, NULL, NULL);

    // Add word sizes to the word size selector
    for(UCHAR i = 0; i<WORD_SIZES; i++)
        SendMessage(wordSelect, CB_ADDSTRING, (WPARAM)NULL, (LPARAM)WORD_SIZES_STRINGS[i]);

    // Set default GUI fonts to all windows. 
    // "System default font" is that 3.1-like ugly ass font.
    NONCLIENTMETRICS metrics;
    metrics.cbSize = sizeof(NONCLIENTMETRICS);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS),
        &metrics, 0);
    HFONT font = CreateFontIndirect(&metrics.lfMessageFont);
    SendMessageW(srcLabel, WM_SETFONT, (WPARAM)font, FALSE);
    SendMessageW(srcTextBox, WM_SETFONT, (WPARAM)font, FALSE);
    SendMessageW(srcBrowseButton, WM_SETFONT, (WPARAM)font, FALSE);
    SendMessageW(desLabel, WM_SETFONT, (WPARAM)font, FALSE);
    SendMessageW(desTextBox, WM_SETFONT, (WPARAM)font, FALSE);
    SendMessageW(desBrowseButton, WM_SETFONT, (WPARAM)font, FALSE);
    SendMessageW(wordLabel, WM_SETFONT, (WPARAM)font, FALSE);
    SendMessageW(wordSelect, WM_SETFONT, (WPARAM)font, FALSE);
    SendMessageW(sameCheckBox, WM_SETFONT, (WPARAM)font, FALSE);
    SendMessageW(goButton, WM_SETFONT, (WPARAM)font, FALSE);
    EnableWindow(goButton, FALSE);
}

void resizeWidgets(const int width, const int height)
{
    const WWIDTH = width;
    const WHEIGHT = height;
    const HMARGIN = 20;
    const VMARGIN = 10;
    const VGAP = 5;
    const BUTTON_HEIGHT = 25;
    const BROWSE_BUTTON_WIDTH = 30;
    const BUTTON_WIDTH = 75;
    const LABEL_HEIGHT = 15;
    const TEXTBOX_HEIGHT = 23;
    const COMBOBOX_HEIGHT = 23;
    const AREA_WIDTH = WWIDTH - (2 * HMARGIN);

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
        currentY - 1,
        BROWSE_BUTTON_WIDTH, BUTTON_HEIGHT, 0);
    currentY += TEXTBOX_HEIGHT + VGAP;

    DeferWindowPos(positions,
        sameCheckBox,
        NULL,
        HMARGIN,
        currentY,
        AREA_WIDTH, BUTTON_HEIGHT, 0);
    currentY += BUTTON_HEIGHT + VGAP;

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
        wordLabel,
        NULL,
        HMARGIN ,
        currentY,
        AREA_WIDTH, LABEL_HEIGHT, 0);
    currentY += LABEL_HEIGHT + VGAP;

    DeferWindowPos(positions,
        wordSelect,
        NULL,
        HMARGIN,
        currentY,
        BUTTON_WIDTH, COMBOBOX_HEIGHT, 0);

    DeferWindowPos(positions,
        goButton,
        NULL,
        HMARGIN + (AREA_WIDTH - BUTTON_WIDTH),
        currentY,
        BUTTON_WIDTH, BUTTON_HEIGHT, 0);

    EndDeferWindowPos(positions);
}

void setGoButtonState()
{
    BOOL isSrcSet = GetWindowTextLengthW(srcTextBox) > 0;
    BOOL isDesSet = GetWindowTextLengthW(desTextBox) > 0;
    BOOL isWordSizeSet = SendMessage(wordSelect, CB_GETCURSEL, (WPARAM)NULL, (LPARAM)NULL) != CB_ERR;
    BOOL isDestSrc = !Button_GetCheck(sameCheckBox);
    EnableWindow(goButton, isSrcSet && isWordSizeSet && (!isDestSrc || isDesSet));
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        createWidgets(hWnd);
        break;
    }
    case WM_COMMAND:
        if (HIWORD(wParam) == BN_CLICKED)
        {
            if ((HWND)lParam == srcBrowseButton)
            {
                OPENFILENAME lpofn;
                memset(&lpofn, 0, sizeof(lpofn));
                lpofn.lStructSize = sizeof(lpofn);
                lpofn.hwndOwner = hWnd;
                lpofn.hInstance = hInst;
                lpofn.lpstrFile = srcPath;
                lpofn.nMaxFile  = MAX_PATH;
                if(GetOpenFileName(&lpofn))
                    SetWindowTextW(srcTextBox, srcPath);
            }
            if ((HWND)lParam == desBrowseButton)
            {
                OPENFILENAME lpofn;
                memset(&lpofn, 0, sizeof(lpofn));
                lpofn.lStructSize = sizeof(lpofn);
                lpofn.hwndOwner = hWnd;
                lpofn.hInstance = hInst;
                lpofn.lpstrFile = desPath;
                lpofn.nMaxFile = MAX_PATH;
                if (GetOpenFileName(&lpofn))
                    SetWindowTextW(desTextBox, desPath);
            }
            if ((HWND)lParam == sameCheckBox)
            {
                if (Button_GetCheck(sameCheckBox))
                {
                    EnableWindow(desLabel       , TRUE);
                    EnableWindow(desBrowseButton, TRUE);
                    EnableWindow(desTextBox     , TRUE);
                    Button_SetCheck(sameCheckBox, BST_UNCHECKED);
                }
                else {
                    EnableWindow(desLabel       , FALSE);
                    EnableWindow(desBrowseButton, FALSE);
                    EnableWindow(desTextBox     , FALSE);
                    Button_SetCheck(sameCheckBox, BST_CHECKED);
                }
            }
            if ((HWND)lParam == goButton)
            {
                // We assume everything was input by now
                LRESULT selectResult = SendMessage(wordSelect, CB_GETCURSEL, (WPARAM)NULL, (LPARAM)NULL);
                BOOL destIsSrc = !Button_GetCheck(sameCheckBox);
                int wordSize = 1 << (selectResult + 1);
                // Copy src and dest from text boxes
                // I dont know if its safe to just their pointers
                //wmemcpy_s()
            }
            
        }
        setGoButtonState();
        break;
    case WM_SIZE:
    {
        resizeWidgets(LOWORD(lParam), HIWORD(lParam));
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    case WM_GETMINMAXINFO:
    {
        LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
        lpMMI->ptMinTrackSize.x = MIN_WIDTH;
        lpMMI->ptMaxTrackSize.x = MAX_WIDTH;
        lpMMI->ptMaxTrackSize.y = MAX_HEIGHT;
        lpMMI->ptMinTrackSize.y = MIN_HEIGHT;
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

void InitCommCtl()
{
    INITCOMMONCONTROLSEX picce;
    picce.dwICC = ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES;
    picce.dwSize = sizeof(picce);
    InitCommonControlsEx(&picce);
}
