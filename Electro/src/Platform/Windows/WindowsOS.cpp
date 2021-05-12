//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Core/System/OS.hpp"
#include "Core/Application.hpp"
#include "Platform/Windows/WindowsWindow.hpp"
#include <commdlg.h>
#include <shlobj_core.h>
#include <shellapi.h>

namespace Electro
{
    Scope<Window> OS::CreateAppWindow(const WindowProps& props)
    {
        return CreateScope<WindowsWindow>(props);
    }

    void OS::RunInTerminal(const char* cmd)
    {
        system(cmd);
    }

    Uint OS::GetScreenWidth()
    {
        return GetSystemMetrics(SM_CXSCREEN);
    }

    Uint OS::GetScreenHeight()
    {
        return GetSystemMetrics(SM_CYSCREEN);
    }

    int OS::AMessageBox(const String& title, const String& message, DialogType dialogType, IconType iconType, DefaultButton defaultButton)
    {
        int lBoxReturnValue;
        UINT aCode;

        if (iconType == IconType::Warning)
            aCode = MB_ICONWARNING;
        else if (iconType == IconType::Error)
            aCode = MB_ICONERROR;
        else if (iconType == IconType::Question)
            aCode = MB_ICONQUESTION;
        else
            aCode = MB_ICONINFORMATION;

        if (dialogType == DialogType::Ok__Cancel)
        {
            aCode += MB_OKCANCEL;
            if (!(int)defaultButton)
                aCode += MB_DEFBUTTON2;
        }
        else if (dialogType == DialogType::Yes__No)
        {
            aCode += MB_YESNO;
            if (!(int)defaultButton)
                aCode += MB_DEFBUTTON2;
        }
        else if (dialogType == DialogType::Yes__No__Cancel)
        {
            aCode += MB_YESNOCANCEL;
            if (!(int)defaultButton)
                aCode += MB_DEFBUTTON3;
            else if ((int)defaultButton == 2)
                aCode += MB_DEFBUTTON2;
        }
        else
        {
            aCode += MB_OK;
        }

        lBoxReturnValue = MessageBoxA(NULL, message.c_str(), title.c_str(), aCode);

        if (dialogType == DialogType::Yes__No__Cancel && (lBoxReturnValue == IDNO))
            return 2;

        if (((dialogType == DialogType::Yes__No__Cancel && dialogType == DialogType::Ok__Cancel && dialogType == DialogType::Yes__No || (lBoxReturnValue == IDOK) || (lBoxReturnValue == IDYES))))
            return 1;
        else
            return 0;
    }

    std::optional<String> OS::OpenFile(const char* filter)
    {
        OPENFILENAMEA ofn;
        CHAR szFile[260] = { 0 };
        CHAR currentDir[256] = { 0 };
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = (HWND)Application::Get().GetWindow().GetNativeWindow();
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        if (GetCurrentDirectoryA(256, currentDir))
            ofn.lpstrInitialDir = currentDir;
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileNameA(&ofn) == TRUE)
            return ofn.lpstrFile;
        return std::nullopt;
    }

    std::optional<String> OS::SaveFile(const char* filter)
    {
        OPENFILENAMEA ofn;
        CHAR szFile[260] = { 0 };
        CHAR currentDir[256] = { 0 };
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = (HWND)Application::Get().GetWindow().GetNativeWindow();
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        if (GetCurrentDirectoryA(256, currentDir))
            ofn.lpstrInitialDir = currentDir;
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

        // Sets the default extension by extracting it from the filter
        ofn.lpstrDefExt = strchr(filter, '\0') + 1;

        if (GetSaveFileNameA(&ofn) == TRUE)
            return ofn.lpstrFile;
        return std::nullopt;
    }

    static int __stdcall BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
    {
        if (uMsg == BFFM_INITIALIZED)
            SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
        return 0;
    }

    char const* OS::SelectFolder(const String& title)
    {
        BROWSEINFOA bInfo;
        LPITEMIDLIST lpItem;
        HRESULT lHResult;

        lHResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        static char aoBuff[1024] = {};
        char const* const kekw = title.c_str();

        bInfo.hwndOwner = 0;
        bInfo.pidlRoot = NULL;
        bInfo.pszDisplayName = aoBuff;
        bInfo.lpszTitle = kekw && strlen(kekw) ? kekw : NULL;
        if (lHResult == S_OK || lHResult == S_FALSE)
        {
            bInfo.ulFlags = BIF_USENEWUI;
        }
        bInfo.lpfn = BrowseCallbackProc;
        bInfo.lParam = (LPARAM)"";
        bInfo.iImage = -1;

        lpItem = SHBrowseForFolderA(&bInfo);
        if (lpItem != NULL)
        {
            SHGetPathFromIDListA(lpItem, aoBuff);
            if (lHResult == S_OK || lHResult == S_FALSE)
                CoUninitialize();
            return aoBuff;
        }
        else
        {
            if (lHResult == S_OK || lHResult == S_FALSE)
                CoUninitialize();
            return nullptr;
        }
    }

    void OS::OpenURL(const char* url)
    {
        ShellExecute(0, 0, StringToWideString(url).c_str(), 0, 0, SW_SHOW);
    }
}
