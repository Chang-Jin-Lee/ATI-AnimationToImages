// BatchVideoConverter.cpp - 하위 폴더까지 탐색 및 스크롤 가능 버전

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
#include <ShlObj_core.h>
#include <algorithm>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shell32.lib")

#define ID_BUTTON_FOLDER_PICK 1001
#define ID_BUTTON_BATCH_EXEC 1002
#define ID_LISTBOX_RESULT 2001
#define ID_BUTTON_CLEAR_LIST 1003
#define ID_EDIT_FPS 103
#define MAX_FILES 512

struct BatchEntry {
    std::wstring filePath;
    HWND hEditFolder;
    HWND hEditPrefix;
    HWND hButtonOpen;
    HWND hPathLabel;
};

std::vector<BatchEntry> g_batchEntries;
HWND g_hWndBatch;
HWND g_hListBoxResult;
HWND g_hEditFps;
int g_scrollOffset = 0;

void ShowMsg(const std::wstring& msg) {
    MessageBoxW(g_hWndBatch, msg.c_str(), L"알림", MB_OK);
}

void AddResultLog(const std::wstring& msg) {
    SendMessageW(g_hListBoxResult, LB_ADDSTRING, 0, (LPARAM)msg.c_str());
}

void ClearBatchList() {
    for (const auto& e : g_batchEntries) {
        DestroyWindow(e.hPathLabel);
        DestroyWindow(e.hEditFolder);
        DestroyWindow(e.hEditPrefix);
        DestroyWindow(e.hButtonOpen);
    }
    g_batchEntries.clear();
    InvalidateRect(g_hWndBatch, NULL, TRUE);
}

void BrowseFolderAndPopulate(HWND hParent) {
    BROWSEINFO bi = { 0 };
    bi.lpszTitle = L"비디오가 들어있는 폴더를 선택하세요.";
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (!pidl) return;

    WCHAR selectedPath[MAX_PATH];
    if (!SHGetPathFromIDList(pidl, selectedPath)) return;
    CoTaskMemFree(pidl);

    ClearBatchList();

    CreateWindowW(L"STATIC", L"파일 이름", WS_CHILD | WS_VISIBLE,
        10, 60, 300, 20, hParent, nullptr, nullptr, nullptr);
    CreateWindowW(L"STATIC", L"폴더명", WS_CHILD | WS_VISIBLE,
        320, 60, 80, 20, hParent, nullptr, nullptr, nullptr);
    CreateWindowW(L"STATIC", L"접두사", WS_CHILD | WS_VISIBLE,
        410, 60, 80, 20, hParent, nullptr, nullptr, nullptr);

    int startY = 90;
    int idx = 0;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(selectedPath)) {
        if (!entry.is_regular_file()) continue;
        std::wstring ext = entry.path().extension().wstring();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);
        if (ext != L".mp4" && ext != L".webm" && ext != L".gif") continue;

        if (idx >= MAX_FILES) break;
        std::wstring path = entry.path().wstring();
        std::wstring stem = entry.path().stem().wstring();
        std::wstring filenameOnly = entry.path().filename().wstring();

        HWND hPathText = CreateWindowW(L"STATIC", filenameOnly.c_str(), WS_CHILD | WS_VISIBLE,
            10, startY, 300, 20, hParent, nullptr, nullptr, nullptr);

        HWND hEditFolder = CreateWindowW(L"EDIT", stem.c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER,
            320, startY, 80, 20, hParent, nullptr, nullptr, nullptr);

        HWND hEditPrefix = CreateWindowW(L"EDIT", stem.c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER,
            410, startY, 80, 20, hParent, nullptr, nullptr, nullptr);

        HWND hBtnOpen = CreateWindowW(L"BUTTON", L"폴더 열기", WS_CHILD | WS_VISIBLE,
            500, startY, 80, 20, hParent, (HMENU)(5000 + idx), nullptr, nullptr);

        g_batchEntries.push_back({ path, hEditFolder, hEditPrefix, hBtnOpen, hPathText });

        startY += 28;
        idx++;
    }

    SCROLLINFO si = { sizeof(SCROLLINFO), SIF_RANGE | SIF_PAGE };
    si.nMin = 0;
    si.nMax = startY;
    si.nPage = 400;
    SetScrollInfo(hParent, SB_VERT, &si, TRUE);

    InvalidateRect(hParent, NULL, TRUE);
}

void RunBatchConversion() {
    WCHAR fpsBuf[16];
    GetWindowTextW(g_hEditFps, fpsBuf, 16);
    int fps = _wtoi(fpsBuf);
    if (fps <= 0) fps = 60;

    for (int i = 0; i < g_batchEntries.size(); ++i) {
        const auto& e = g_batchEntries[i];
        WCHAR folderName[100]; GetWindowTextW(e.hEditFolder, folderName, 100);
        WCHAR prefix[100]; GetWindowTextW(e.hEditPrefix, prefix, 100);

        std::wstring folderPath = std::filesystem::path(e.filePath).parent_path().wstring() + L"\\" + folderName;
        std::filesystem::create_directories(folderPath);
        for (const auto& f : std::filesystem::directory_iterator(folderPath))
            std::filesystem::remove_all(f);

        std::wstring cmd = L"ffmpeg -y -i \"" + e.filePath + L"\" -vf fps=" + std::to_wstring(fps) +
            L" \"" + folderPath + L"\\" + prefix + L"_%04d.png\"";

        STARTUPINFOW si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        if (!CreateProcessW(nullptr, &cmd[0], nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
            AddResultLog(L"[오류] " + e.filePath);
            continue;
        }

        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        int fileCount = 0;
        for (const auto& f : std::filesystem::directory_iterator(folderPath))
            if (f.is_regular_file()) fileCount++;

        AddResultLog(std::filesystem::path(e.filePath).filename().wstring() + L" → " + std::to_wstring(fileCount) + L"장 완료");
    }
    AddResultLog(L"✅ 일괄 변환 완료!");
}

LRESULT CALLBACK BatchWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        CreateWindowW(L"BUTTON", L"폴더 선택", WS_CHILD | WS_VISIBLE,
            20, 20, 100, 30, hWnd, (HMENU)ID_BUTTON_FOLDER_PICK, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"FPS", WS_CHILD | WS_VISIBLE,
            140, 25, 30, 20, hWnd, NULL, nullptr, nullptr);
        g_hEditFps = CreateWindowW(L"EDIT", L"60", WS_CHILD | WS_VISIBLE | WS_BORDER,
            170, 22, 50, 22, hWnd, (HMENU)ID_EDIT_FPS, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"일괄 변환 실행", WS_CHILD | WS_VISIBLE,
            240, 20, 120, 30, hWnd, (HMENU)ID_BUTTON_BATCH_EXEC, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"리스트 지우기", WS_CHILD | WS_VISIBLE,
            380, 20, 110, 30, hWnd, (HMENU)ID_BUTTON_CLEAR_LIST, nullptr, nullptr);
        g_hListBoxResult = CreateWindowW(L"LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL,
            10, 450, 580, 100, hWnd, (HMENU)ID_LISTBOX_RESULT, nullptr, nullptr);
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_BUTTON_FOLDER_PICK) BrowseFolderAndPopulate(hWnd);
        else if (LOWORD(wParam) == ID_BUTTON_BATCH_EXEC) std::thread(RunBatchConversion).detach();
        else if (LOWORD(wParam) == ID_BUTTON_CLEAR_LIST) ClearBatchList();
        else if (LOWORD(wParam) >= 5000 && LOWORD(wParam) < 5000 + MAX_FILES) {
            int idx = LOWORD(wParam) - 5000;
            if (idx < g_batchEntries.size()) {
                WCHAR folderName[100]; GetWindowTextW(g_batchEntries[idx].hEditFolder, folderName, 100);
                std::wstring folder = std::filesystem::path(g_batchEntries[idx].filePath).parent_path().wstring() + L"\\" + folderName;
                if (!std::filesystem::exists(folder)) {
                    ShowMsg(L"폴더가 존재하지 않습니다: " + folder);
                }
                else {
                    ShellExecuteW(nullptr, L"open", folder.c_str(), nullptr, nullptr, SW_SHOW);
                }
            }
        }
        break;
    case WM_VSCROLL: {
        SCROLLINFO si = { sizeof(SCROLLINFO), SIF_ALL };
        GetScrollInfo(hWnd, SB_VERT, &si);
        int yPos = si.nPos;
        switch (LOWORD(wParam)) {
        case SB_LINEUP: yPos -= 20; break;
        case SB_LINEDOWN: yPos += 20; break;
        case SB_PAGEUP: yPos -= si.nPage; break;
        case SB_PAGEDOWN: yPos += si.nPage; break;
        case SB_THUMBTRACK: yPos = HIWORD(wParam); break;
        default: break;
        }
        yPos = std::clamp(yPos, si.nMin, si.nMax - (int)si.nPage);
        ScrollWindow(hWnd, 0, g_scrollOffset - yPos, NULL, NULL);
        si.fMask = SIF_POS;
        si.nPos = yPos;
        g_scrollOffset = yPos;
        SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

int RunBatchWindow(HINSTANCE hInstance, int nCmdShow) {
    WNDCLASSW wc = {};
    wc.lpfnWndProc = BatchWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"BatchVideoWindow";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.style = CS_VREDRAW | CS_HREDRAW;
    RegisterClassW(&wc);

    g_hWndBatch = CreateWindowW(L"BatchVideoWindow", L"일괄 비디오 변환기", WS_OVERLAPPEDWINDOW | WS_VSCROLL,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 640, nullptr, nullptr, hInstance, nullptr);

    ShowWindow(g_hWndBatch, nCmdShow);
    UpdateWindow(g_hWndBatch);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
