#include "stdafx.h"
#include "Endianness Changer.h"
#include "convert.h"

#define MAX_LOADSTRING 100

/* Global variables */

/* Current instance */
HINSTANCE hInst;                                

/* Title bar text */
WCHAR szTitle[] = L"Endianness Changer";

/* Main window class name */
WCHAR szWindowClass[] = L"MainWindowClass";

/* Main window size global constants */
const MIN_WIDTH = 350;
const MIN_HEIGHT = 240;
const WIDTH = 450;
const HEIGHT = 240;
const MAX_WIDTH = 1000;
const MAX_HEIGHT = 240;

/* Global handle to "Source" label */
HWND  srcLabel;

/* Global handle to source file input text box */
HWND  srcTextBox;

/* Global handle to source browse button */
HWND  srcBrowseButton;

/* Global handle to "Destination" label */
HWND  desLabel;

/* Global handle to destination file input textbox */
HWND  desTextBox;

/* Global handle to destination browse button */
HWND  desBrowseButton;

/* Global handle to same file checkbox */
HWND  sameCheckBox;

/* Global handle to global handle to word size dropdown list */
HWND  wordSelect;

/* Global handle to word size label */
HWND  wordLabel;

/* Global handle to main command button */
HWND  goButton;

/* Source file path */
WCHAR srcPath[MAX_PATH];

/* Destination file path */
WCHAR desPath[MAX_PATH];

/* Amount of word sizes available */
const UCHAR WORD_SIZES = 4;

/* Text for each word size */
const WCHAR * WORD_SIZES_STRINGS[4] = { L"2", L"4", L"8", L"16" };

/* Handle to endianness changer thread */
HANDLE workerThread;

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
    registerWindowClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
        return FALSE;

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
    wcex.hIcon = NULL;
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_ENDIANNESSCHANGER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = NULL;

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

void updateUI()
{
    BOOL isSrcSet = GetWindowTextLengthW(srcTextBox) > 0;
    BOOL isDesSet = GetWindowTextLengthW(desTextBox) > 0;
    BOOL isWordSizeSet = SendMessage(wordSelect, CB_GETCURSEL, (WPARAM)NULL, (LPARAM)NULL) != CB_ERR;
    BOOL isDestSrc = !Button_GetCheck(sameCheckBox);
    EnableWindow(goButton, isSrcSet && isWordSizeSet && (!isDestSrc || isDesSet));
}

void printErrorMessage(HWND hWnd, DWORD errorCode)
{
    LPWSTR message;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
        errorCode,
        0,
        (LPTSTR)&message,
        512,
        NULL);
    MessageBoxW(hWnd,
        message, L"Error", MB_OK | MB_ICONERROR);
    HeapFree(GetProcessHeap(), 0, message);
}

/* Intermediate function needed because it needs to take an LPVOID argument */
DWORD CALLBACK doConvert(LPVOID arg)
{
    return convertFiles((CONVERTARGS*)arg);
}

BOOL fileExists(LPWSTR szPath)
{
    DWORD dwAttrib = GetFileAttributesW(szPath);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
        !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}


/* Task dialog window procedure */
HRESULT CALLBACK taskDialogCallbackProc(HWND hWnd,
    UINT uNotification, 
    WPARAM wParam, 
    LPARAM lParam, 
    LONG_PTR convertArgs)
{
    switch (uNotification)
    {
    case TDN_CREATED:
    {
        SendMessageW(hWnd, TDM_ENABLE_BUTTON, TDCBF_CANCEL_BUTTON, FALSE);
        /* Start conversion thread */
        workerThread = CreateThread(NULL, 0, doConvert, (LPVOID) convertArgs, 0, NULL);
        break;
    }
    case TDN_DESTROYED:
    {
        /* Wait for worker thread termination, just in case */
        WaitForSingleObject(workerThread, INFINITE);
        break;
    }
    case TDN_TIMER:
    {
        /* Poll conversion progress and update progress bar*/
        int progress = ((CONVERTARGS *)convertArgs)->progress;
        SendMessage(hWnd, TDM_SET_PROGRESS_BAR_POS, progress, (LPARAM)NULL);

        if (WaitForSingleObject(workerThread, 0) == WAIT_OBJECT_0)
        {
            DWORD exitCode;

            /* Workaround to skip progress bar animation */
            SendMessage(hWnd, TDM_SET_PROGRESS_BAR_POS, 99, (LPARAM)NULL);
            SendMessage(hWnd, TDM_SET_PROGRESS_BAR_POS, 100, (LPARAM)NULL);

            GetExitCodeThread(workerThread, &exitCode);
            workerThread = NULL;


            if (!((CONVERTARGS *)convertArgs)->cancel)
            {
                /* If not cancelled, check exit code and show error message if necessary */
                if (!exitCode)
                    printErrorMessage(hWnd, GetLastError());
                else
                    MessageBoxW(hWnd, L"Conversion successful", L"Finished", MB_OK | MB_ICONASTERISK);
            }
            PostMessage(hWnd, WM_CLOSE, 0, 0);
        }
        break;
    }
    case TDN_BUTTON_CLICKED:
    {
        /*
         * Signal cancelation. This variable is meant to be written only by 
         * this thread and read by the worker thread.
         */
        ((CONVERTARGS *)convertArgs)->cancel = TRUE;
        break;
    }
    default:
        break;
    }
    return 0;
}

void startConversion(HWND hWnd, CONVERTARGS * convertArgs)
{
    TASKDIALOGCONFIG dialogConfig;
    memset(&dialogConfig, 0, sizeof(dialogConfig));
    dialogConfig.cbSize = sizeof(dialogConfig);
    dialogConfig.hwndParent = hWnd;
    dialogConfig.hInstance = hInst;
    dialogConfig.dwFlags = 
        TDF_SHOW_PROGRESS_BAR |
        TDF_CALLBACK_TIMER |
        TDF_POSITION_RELATIVE_TO_WINDOW | 
        TDF_ALLOW_DIALOG_CANCELLATION;
    dialogConfig.dwCommonButtons = TDCBF_CANCEL_BUTTON;
    dialogConfig.pszContent = L"Converting.";
    dialogConfig.pfCallback = taskDialogCallbackProc;
    dialogConfig.lpCallbackData = (LONG_PTR)convertArgs;
    /*
     * Create a task dialog with a progress bar, no custom button,
     * no checkbox and no radio button.
     */
    TaskDialogIndirect(&dialogConfig, NULL, NULL, NULL);
}

/* Main window procedure*/
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
        /* Handle all commands */
        if (HIWORD(wParam) == BN_CLICKED)
        {
            if ((HWND)lParam == srcBrowseButton)
            {
                /* Get source file name from open dialog */
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
                /* Get destination file name from open dialog */
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
                /* We assume everything was input by now */

                /* Get combobox selection index */
                LRESULT selectResult = SendMessage(wordSelect, CB_GETCURSEL, (WPARAM)NULL, (LPARAM)NULL);

                /* Get checkbox status */
                BOOL destIsSrc = Button_GetCheck(sameCheckBox);

                /* Update source and destination file paths from widgets */
                GetWindowTextW(srcTextBox, srcPath, MAX_PATH);
                GetWindowTextW(desTextBox, desPath, MAX_PATH);
                
                /* Zero out our conversion structure */
                CONVERTARGS convertArgs;
                memset(&convertArgs, 0, sizeof(convertArgs));

                /* We fill out the fields of our conversion structure */
                convertArgs.wordSize = 1 << (selectResult + 1);

                if (destIsSrc)
                    convertArgs.src = CreateFileW(srcPath,
                        GENERIC_READ | GENERIC_WRITE,
                        0,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );
                else
                    convertArgs.src = CreateFileW(srcPath,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                    );

                if (convertArgs.src == INVALID_HANDLE_VALUE)
                {
                    printErrorMessage(hWnd,GetLastError());
                    return 0;
                }

                /* Check convertibility of file */
                LARGE_INTEGER size;
                GetFileSizeEx(convertArgs.src, &size);
                if (size.QuadPart % convertArgs.wordSize != 0)
                {
                    MessageBoxW(hWnd,
                        L"The source file size is not a multiple of the word size selected. Please choose another file or another word size.",
                        L"Wrong file size", MB_OK | MB_ICONEXCLAMATION);
                    CloseHandle(convertArgs.src);
                    return 0;
                }
           
                /* Open destination file, according to mode of operation */
                if (!destIsSrc)
                {
                    if (fileExists(desPath))
                    {
                        int mbResult = MessageBoxW(hWnd,
                            L"Do you want to overwrite the destination file?",
                            L"Destination file already exists",
                            MB_OKCANCEL | MB_ICONQUESTION);
                        if (mbResult == IDCANCEL)
                        {
                            CloseHandle(convertArgs.src);
                            return 0;
                        }
                    }

                    convertArgs.des = CreateFileW(desPath,
                        GENERIC_WRITE,
                        0,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                    );

                    if (convertArgs.des == INVALID_HANDLE_VALUE)
                    {
                        CloseHandle(convertArgs.des);
                        printErrorMessage(hWnd, GetLastError());
                    }
                }
                startConversion(hWnd, &convertArgs);
                 
                /* Close both files */
                CloseHandle(convertArgs.src);
                CloseHandle(convertArgs.des);
            }
            
        }
        /* Disable or enable controls according to state */
        updateUI();
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
