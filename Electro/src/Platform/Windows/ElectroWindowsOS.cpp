//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "Core/System/ElectroOS.hpp"
#include "Core/ElectroApplication.hpp"
#include <commdlg.h>
#include <shlobj_core.h>
#include <shellapi.h>

namespace Electro
{
    static const char* DialogTypeToString(DialogType type)
    {
        switch (type)
        {
            case DialogType::Ok:              return "ok";
            case DialogType::Ok__Cancel:      return "okcancel";
            case DialogType::Yes__No:         return "yesno";
            case DialogType::Yes__No__Cancel: return "yesnocancel";
        }
        ELECTRO_WARN("Invalid DialogType!");
        return "ok";
    }

    String OS::GetNameWithoutExtension(const String& assetFilepath)
    {
        String name;
        auto lastSlash = assetFilepath.find_last_of("/\\");
        lastSlash = lastSlash == String::npos ? 0 : lastSlash + 1;
        auto lastDot = assetFilepath.rfind('.');
        auto count = lastDot == String::npos ? assetFilepath.size() - lastSlash : lastDot - lastSlash;
        name = assetFilepath.substr(lastSlash, count);
        return name;
    }

    String OS::GetNameWithExtension(const char* assetFilepath)
    {
        return std::filesystem::path(assetFilepath).filename().string();
    }

    String OS::GetExtension(const char* assetFilepath)
    {
        return std::filesystem::path(assetFilepath).extension().string();
    }

    String OS::GetParentPath(const String& fullpath)
    {
        std::filesystem::path p = fullpath;
        return p.parent_path().string();
    }

    Vector<String> OS::GetAllDirsInPath(const char* path)
    {
        Vector<String> paths;
        for (const auto& entry : std::filesystem::directory_iterator(path))
            paths.push_back(entry.path().string());

        return paths;
    }

    Vector<String> OS::GetAllFilePathsFromParentPath(const char* path)
    {
        Vector<String> paths;
        for (const auto& entry : std::filesystem::directory_iterator(path))
            paths.push_back(entry.path().string());

        return paths;
    }

    bool OS::CreateFolder(const char* parentDirectory, const char* name)
    {
        String path = String(parentDirectory) + "/" + String(name);
        if (std::filesystem::create_directory(path) || std::filesystem::exists(path))
            return true;
        return false;
    }

    bool OS::CreateFolder(const char* directory)
    {
        if (std::filesystem::create_directory(directory) || std::filesystem::exists(directory))
            return true;
        return false;
    }

    Uint OS::GetScreenWidth()
    {
        return GetSystemMetrics(SM_CXSCREEN);
    }

    Uint OS::GetScreenHeight()
    {
        return GetSystemMetrics(SM_CYSCREEN);
    }

    String OS::ReadFile(const char* filepath)
    {
        String result;
        std::ifstream in(filepath, std::ios::in | std::ios::binary); // ifstream closes itself due to RAII
        if (in)
        {
            in.seekg(0, std::ios::end);
            result.resize(in.tellg());
            in.seekg(0, std::ios::beg);
            in.read(&result[0], result.size());
        }
        else
            ELECTRO_ERROR("Could not open file path \"%s\"", filepath);

        return result;
    }

    Vector<char> OS::ReadBinaryFile(const char* filepath)
    {
        std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

        if (!stream)
            ELECTRO_ERROR("Cannot open filepath: %s!", filepath);

        auto end = stream.tellg();
        stream.seekg(0, std::ios::beg);
        auto size = std::size_t(end - stream.tellg());
        if (size == 0) return {};

        Vector<char> buffer(size);
        if (!stream.read((char*)buffer.data(), buffer.size()))
            ELECTRO_ERROR("Cannot read file: %s", filepath);

        return buffer;
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

    bool OS::IsDirectory(const String& path)
    {
        return std::filesystem::is_directory(path);
    }

    void OS::RemoveAll(const String& fullpath)
    {
        std::filesystem::remove_all(fullpath);
    }

    void OS::OpenURL(const char* url)
    {
        ShellExecute(0, 0, url, 0, 0, SW_SHOW);
    }
}