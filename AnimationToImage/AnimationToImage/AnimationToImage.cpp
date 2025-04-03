// BatchVideoConverter.cpp (일괄 변환 전체 구현)
// 단일 모드와 -batch 모드를 모두 지원하는 메인 파일

#include <windows.h>
#include <commdlg.h>
#include <shlwapi.h>
#include <string>
#include <thread>
#include <vector>
#include <filesystem>
#include <commctrl.h>
#include <gdiplus.h>
#include <shellapi.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shell32.lib")

#define ID_BUTTON_SELECT_FILE 101
#define ID_BUTTON_START_CONVERT 102
#define ID_EDIT_FPS 103
#define ID_PROGRESS_BAR 104
#define ID_LISTBOX_ERRORS 105
#define ID_STATIC_THUMBNAIL 106
#define ID_BUTTON_OPEN_FOLDER 107
#define ID_EDIT_FOLDERNAME 108
#define ID_EDIT_PREFIX 109
#define ID_BUTTON_BATCH_CONVERT 110

HINSTANCE g_hInst;
HWND g_hWnd;
HWND hEditFps, hProgressBar, hListBoxErrors, hStaticThumbnail, hButtonOpenFolder;
HWND hEditFolderName, hEditPrefix;
WCHAR g_SelectedFile[MAX_PATH] = {};
std::wstring g_OutputDir;
ULONG_PTR g_GdiPlusToken;
std::vector<std::wstring> g_thumbPaths;

void ShowError(const std::wstring& msg)
{
    MessageBoxW(g_hWnd, msg.c_str(), L"오류", MB_ICONERROR);
}

void AddErrorToList(const std::wstring& msg)
{
    SendMessageW(hListBoxErrors, LB_ADDSTRING, 0, (LPARAM)msg.c_str());
}

void ClearOutputDirectory(const std::wstring& directory)
{
    try {
        if (std::filesystem::exists(directory)) {
            for (const auto& entry : std::filesystem::directory_iterator(directory)) {
                std::filesystem::remove_all(entry);
            }
        }
    }
    catch (const std::exception& e) {
        AddErrorToList(L"폴더 정리 중 오류 발생: " + std::wstring(e.what(), e.what() + strlen(e.what())));
    }
}

void RunFFmpegConvert()
{
    WCHAR fpsBuffer[10];
    GetWindowTextW(hEditFps, fpsBuffer, 10);
    int fps = _wtoi(fpsBuffer);
    if (fps <= 0) fps = 60;

    WCHAR folderNameBuffer[100] = L"frames";
    WCHAR prefixBuffer[100] = L"frame";
    GetWindowTextW(hEditFolderName, folderNameBuffer, 100);
    GetWindowTextW(hEditPrefix, prefixBuffer, 100);

    std::wstring inputPath = g_SelectedFile;
    std::wstring folderPath = inputPath.substr(0, inputPath.find_last_of(L"\\"));
    g_OutputDir = folderPath + L"\\" + folderNameBuffer;

    std::filesystem::create_directories(g_OutputDir);
    ClearOutputDirectory(g_OutputDir);

    std::wstring command = L"ffmpeg -y -i \"" + inputPath + L"\" -vf fps=" + std::to_wstring(fps) +
        L" \"" + g_OutputDir + L"\\" + prefixBuffer + L"_%04d.png\"";

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    if (!CreateProcessW(nullptr, &command[0], nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        ShowError(L"ffmpeg 실행 실패\n\n경로에 ffmpeg.exe가 있는지 확인하세요.");
        return;
    }

    SendMessageW(hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    SendMessageW(hProgressBar, PBM_SETPOS, 0, 0);

    while (WaitForSingleObject(pi.hProcess, 100) == WAIT_TIMEOUT) {
        static int percent = 0;
        percent = (percent + 5) % 100;
        SendMessageW(hProgressBar, PBM_SETPOS, percent, 0);
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    SendMessageW(hProgressBar, PBM_SETPOS, 100, 0);

    int fileCount = 0;
    for (const auto& entry : std::filesystem::directory_iterator(g_OutputDir)) {
        if (entry.is_regular_file()) {
            fileCount++;
        }
    }

    std::wstring msg = L"총 생성된 이미지 수: " + std::to_wstring(fileCount) + L"장";
    AddErrorToList(msg);
    MessageBoxW(g_hWnd, L"프레임 추출이 완료되었습니다!", L"완료", MB_OK);
}

void ShowThumbnail()
{
    std::wstring inputPath = g_SelectedFile;
    std::wstring folderPath = inputPath.substr(0, inputPath.find_last_of(L"\\"));
    std::wstring thumbPath = folderPath + L"\\thumbnail_temp.png";

    std::wstring command = L"ffmpeg -y -i \"" + inputPath + L"\" -frames:v 1 -q:v 2 \"" + thumbPath + L"\"";
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    if (CreateProcessW(nullptr, &command[0], nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        Gdiplus::Image tempImage(thumbPath.c_str());
        Gdiplus::Bitmap* pCopy = new Gdiplus::Bitmap(tempImage.GetWidth(), tempImage.GetHeight(), PixelFormat32bppARGB);
        {
            Gdiplus::Graphics g(pCopy);
            g.DrawImage(&tempImage, 0, 0);
        }

        HDC hdc = GetDC(hStaticThumbnail);
        Gdiplus::Graphics g2(hdc);
        g2.DrawImage(pCopy, 0, 0, 160, 90);
        ReleaseDC(hStaticThumbnail, hdc);

        delete pCopy;
        g_thumbPaths.push_back(thumbPath);
    }
}

void SelectFile()
{
    OPENFILENAMEW ofn = {};
    WCHAR szFile[MAX_PATH] = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = g_hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Video Files (*.mp4;*.gif;*.webm)\0*.mp4;*.gif;*.webm\0All Files\0*.*\0";
    ofn.lpstrTitle = L"비디오 파일 선택";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileNameW(&ofn)) {
        wcscpy_s(g_SelectedFile, szFile);
        ShowThumbnail();
    }
}

void OpenOutputFolder()
{
    if (!g_OutputDir.empty()) {
        ShellExecuteW(nullptr, L"open", g_OutputDir.c_str(), nullptr, nullptr, SW_SHOW);
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        CreateWindowW(L"BUTTON", L"비디오 선택", WS_CHILD | WS_VISIBLE,
            20, 20, 120, 30, hWnd, (HMENU)ID_BUTTON_SELECT_FILE, g_hInst, NULL);

        CreateWindowW(L"BUTTON", L"변환 시작", WS_CHILD | WS_VISIBLE,
            160, 20, 120, 30, hWnd, (HMENU)ID_BUTTON_START_CONVERT, g_hInst, NULL);

        hEditFps = CreateWindowW(L"EDIT", L"60", WS_CHILD | WS_VISIBLE | WS_BORDER,
            300, 20, 50, 30, hWnd, (HMENU)ID_EDIT_FPS, g_hInst, NULL);

        CreateWindowW(L"STATIC", L"폴더명", WS_CHILD | WS_VISIBLE,
            20, 60, 60, 20, hWnd, NULL, g_hInst, NULL);
        hEditFolderName = CreateWindowW(L"EDIT", L"frames", WS_CHILD | WS_VISIBLE | WS_BORDER,
            90, 60, 100, 20, hWnd, (HMENU)ID_EDIT_FOLDERNAME, g_hInst, NULL);

        CreateWindowW(L"STATIC", L"접두사", WS_CHILD | WS_VISIBLE,
            200, 60, 60, 20, hWnd, NULL, g_hInst, NULL);
        hEditPrefix = CreateWindowW(L"EDIT", L"frame", WS_CHILD | WS_VISIBLE | WS_BORDER,
            270, 60, 100, 20, hWnd, (HMENU)ID_EDIT_PREFIX, g_hInst, NULL);

        hProgressBar = CreateWindowW(PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
            20, 90, 350, 25, hWnd, (HMENU)ID_PROGRESS_BAR, g_hInst, NULL);

        hStaticThumbnail = CreateWindowW(L"STATIC", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
            120, 120, 160, 90, hWnd, (HMENU)ID_STATIC_THUMBNAIL, g_hInst, NULL);

        hListBoxErrors = CreateWindowW(L"LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY,
            20, 220, 350, 100, hWnd, (HMENU)ID_LISTBOX_ERRORS, g_hInst, NULL);

        //hButtonOpenFolder = CreateWindowW(L"BUTTON", L"폴더 열기", WS_CHILD | WS_VISIBLE,
        //    20, 330, 100, 30, hWnd, (HMENU)ID_BUTTON_OPEN_FOLDER, g_hInst, NULL);

        CreateWindowW(L"BUTTON", L"일괄 변환 창 열기", WS_CHILD | WS_VISIBLE,
            240, 330, 150, 30, hWnd, (HMENU)ID_BUTTON_BATCH_CONVERT, g_hInst, NULL);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_BUTTON_SELECT_FILE:
            SelectFile();
            break;
        case ID_BUTTON_START_CONVERT:
            if (wcslen(g_SelectedFile) == 0) {
                ShowError(L"먼저 비디오 파일을 선택하세요.");
                return 0;
            }
            std::thread(RunFFmpegConvert).detach();
            break;
        case ID_BUTTON_OPEN_FOLDER:
            OpenOutputFolder();
            break;
        case ID_BUTTON_BATCH_CONVERT:
        {
            STARTUPINFOW si = { sizeof(si) };
            PROCESS_INFORMATION pi;
            WCHAR exePath[MAX_PATH];
            GetModuleFileNameW(NULL, exePath, MAX_PATH);
            std::wstring batchCmd = L"\"" + std::wstring(exePath) + L"\" -batch";
            CreateProcessW(NULL, &batchCmd[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            break;
        }
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Batch 모드 진입 함수
int RunBatchWindow(HINSTANCE hInstance, int nCmdShow);

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, int nCmdShow)
{
    if (lpCmdLine && wcsstr(lpCmdLine, L"-batch"))
        return RunBatchWindow(hInstance, nCmdShow);

    INITCOMMONCONTROLSEX icex = { sizeof(icex), ICC_PROGRESS_CLASS };
    InitCommonControlsEx(&icex);

    Gdiplus::GdiplusStartupInput gsi;
    Gdiplus::GdiplusStartup(&g_GdiPlusToken, &gsi, nullptr);

    g_hInst = hInstance;
    WNDCLASSW wc = { sizeof(wc) };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"AnimationToImageApp";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassW(&wc);

    g_hWnd = CreateWindowW(L"AnimationToImageApp", L"애니메이션 → 이미지 추출기", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 420, 420, nullptr, nullptr, hInstance, nullptr);

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Gdiplus::GdiplusShutdown(g_GdiPlusToken);
    for (auto thumbPath : g_thumbPaths) {
        std::filesystem::remove(thumbPath);
    }
    return (int)msg.wParam;
}