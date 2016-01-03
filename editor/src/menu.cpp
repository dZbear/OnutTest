#include "ActionManager.h"
#include "menu.h"

extern onut::ActionManager actionManager;

void buildMenu()
{
    auto window = OWindow->getHandle();
    auto menu = CreateMenu();

    {
        auto subMenu = CreatePopupMenu();
        InsertMenu(subMenu, 0, MF_BYPOSITION | MF_STRING, MENU_FILE_NEW, TEXT("&New\tCtrl+N"));
        InsertMenu(subMenu, 1, MF_BYPOSITION | MF_STRING, MENU_FILE_OPEN, TEXT("&Open\tCtrl+O"));
        InsertMenu(subMenu, 2, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
        InsertMenu(subMenu, 3, MF_BYPOSITION | MF_STRING, MENU_FILE_SAVE, TEXT("&Save\tCtrl+S"));
        InsertMenu(subMenu, 4, MF_BYPOSITION | MF_STRING, MENU_FILE_SAVE_AS, TEXT("Save &As\tCtrl+Shift+S"));
        InsertMenu(subMenu, 5, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
        InsertMenu(subMenu, 6, MF_BYPOSITION | MF_STRING, MENU_FILE_EXIT, TEXT("E&xit\tAlt+F4"));
        InsertMenu(menu, 0, MF_BYPOSITION | MF_POPUP, (UINT)subMenu, TEXT("&File"));
    }

    {
        auto subMenu = CreatePopupMenu();
        InsertMenu(subMenu, 0, MF_BYPOSITION | MF_STRING, MENU_EDIT_UNDO, TEXT("&Undo\tCtrl+Z"));
        InsertMenu(subMenu, 1, MF_BYPOSITION | MF_STRING, MENU_EDIT_REDO, TEXT("&Redo\tCtrl+Shift+Z"));
        InsertMenu(subMenu, 2, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
        InsertMenu(subMenu, 3, MF_BYPOSITION | MF_STRING, MENU_EDIT_CUT, TEXT("Cu&t\tCtrl+X"));
        InsertMenu(subMenu, 4, MF_BYPOSITION | MF_STRING, MENU_EDIT_COPY, TEXT("&Copy\tCtrl+C"));
        InsertMenu(subMenu, 5, MF_BYPOSITION | MF_STRING, MENU_EDIT_PASTE, TEXT("&Paste\tCtrl+V"));
        InsertMenu(subMenu, 6, MF_BYPOSITION | MF_STRING, MENU_EDIT_DELETE, TEXT("&Delete\tDel"));
        InsertMenu(subMenu, 7, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
        InsertMenu(subMenu, 8, MF_BYPOSITION | MF_STRING, MENU_EDIT_SELECT_ALL, TEXT("Select &All\tCtrl+A"));
        InsertMenu(subMenu, 9, MF_BYPOSITION | MF_STRING, MENU_EDIT_DESELECT, TEXT("D&eselect\tEsc"));
        InsertMenu(subMenu, 10, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
        InsertMenu(subMenu, 11, MF_BYPOSITION | MF_STRING, MENU_EDIT_FOCUS_SELECTION, TEXT("&Focus on Selection\tF"));
        InsertMenu(menu, 1, MF_BYPOSITION | MF_POPUP, (UINT)subMenu, TEXT("&Edit"));
    }

    SetMenu(window, menu);
    UpdateWindow(window);
}

std::string fileOpen()
{
    auto window = OWindow->getHandle();
    char szFileName[MAX_PATH] = "";

    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = window;

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = window;
    ofn.lpstrFilter = "Seed file (*.xml)\0*.xml\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    ofn.lpstrDefExt = "xml";
    ofn.lpstrTitle = "Open";

    GetOpenFileNameA(&ofn);

    return ofn.lpstrFile;
}

std::string fileSaveAs()
{
    auto window = OWindow->getHandle();
    char szFileName[MAX_PATH] = "";

    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = window;

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = window;
    ofn.lpstrFilter = "Seed file (*.xml)\0*.xml\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
    ofn.lpstrDefExt = "xml";
    ofn.lpstrTitle = "Save As";

    GetOpenFileNameA(&ofn);

    return ofn.lpstrFile;
}

std::string fileNew()
{
    auto window = OWindow->getHandle();
    char szFileName[MAX_PATH] = "";

    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = window;

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = window;
    ofn.lpstrFilter = "Seed file (*.xml)\0*.xml\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_NOCHANGEDIR;
    ofn.lpstrDefExt = "xml";
    ofn.lpstrTitle = "New";

    GetOpenFileNameA(&ofn);

    return ofn.lpstrFile;
}

void onNew(const std::string& filename);
void onOpen(const std::string& filename);
void onSaveAs(const std::string& filename);
void onDelete();
void onCopy();
void onPaste();
void onCut();
void onSave();
void onSelectAll();
void onDeselect();
void onFocusSelection();

extern bool isModified;

bool AskForSave()
{
    auto ret = MessageBoxA(OWindow->getHandle(), "You have unsaved changes. Do you want to save now?", "Attention! Unsaved changes", MB_YESNOCANCEL);
    if (ret == IDYES)
    {
        onSave();
    }
    else if (ret == IDCANCEL)
    {
        return false;
    }
    return true;
}

void onMenu(UINT menuId)
{
    switch (menuId)
    {
        case MENU_FILE_NEW: // New
        {
            if (isModified) if (!AskForSave()) return;
            auto filename = fileNew();
            if (!filename.empty())
            {
                onNew(filename);
            }
            break;
        }
        case MENU_FILE_OPEN: // Open
        {
            if (isModified) if (!AskForSave()) return;
            auto filename = fileOpen();
            if (!filename.empty())
            {
                onOpen(filename);
            }
            break;
        }
        case MENU_FILE_SAVE: // Save
            onSave();
            break;
        case MENU_FILE_SAVE_AS: // Save As
        {
            auto filename = fileSaveAs();
            if (!filename.empty())
            {
                onSaveAs(filename);
            }
            break;
        }
        case MENU_FILE_EXIT: // Exit
            PostQuitMessage(0);
            break;
        case MENU_EDIT_UNDO:
            actionManager.undo();
            break;
        case MENU_EDIT_REDO:
            actionManager.redo();
            break;
        case MENU_EDIT_CUT:
            onCut();
            break;
        case MENU_EDIT_COPY:
            onCopy();
            break;
        case MENU_EDIT_PASTE:
            onPaste();
            break;
        case MENU_EDIT_DELETE:
            onDelete();
            break;
        case MENU_EDIT_SELECT_ALL:
            onSelectAll();
            break;
        case MENU_EDIT_DESELECT:
            onDeselect();
            break;
        case MENU_EDIT_FOCUS_SELECTION:
            onFocusSelection();
            break;
    }
}

void checkShortCut(uintptr_t key)
{
    if (OInput->isStateDown(DIK_LCONTROL))
    {
        if (key == static_cast<uintptr_t>('N'))
        {
            onMenu(MENU_FILE_NEW);
        }
        else if (key == static_cast<uintptr_t>('O'))
        {
            onMenu(MENU_FILE_OPEN);
        }
        else if (key == static_cast<uintptr_t>('S'))
        {
            if (OInput->isStateDown(DIK_LSHIFT))
            {
                onMenu(MENU_FILE_SAVE_AS);
            }
            else
            {
                onMenu(MENU_FILE_SAVE);
            }
        }
        else if (key == static_cast<uintptr_t>('Z'))
        {
            if (OInput->isStateDown(DIK_LSHIFT))
            {
                onMenu(MENU_EDIT_REDO);
            }
            else
            {
                onMenu(MENU_EDIT_UNDO);
            }
        }
        else if (key == static_cast<uintptr_t>('X'))
        {
            onMenu(MENU_EDIT_CUT);
        }
        else if (key == static_cast<uintptr_t>('C'))
        {
            onMenu(MENU_EDIT_COPY);
        }
        else if (key == static_cast<uintptr_t>('V'))
        {
            onMenu(MENU_EDIT_PASTE);
        }
        else if (key == static_cast<uintptr_t>('A'))
        {
            onMenu(MENU_EDIT_SELECT_ALL);
        }
    }
    else if (key == VK_DELETE)
    {
        onMenu(MENU_EDIT_DELETE);
    }
    else if (key == VK_ESCAPE)
    {
        onMenu(MENU_EDIT_DESELECT);
    }
    else if (key == static_cast<uintptr_t>('F'))
    {
        onMenu(MENU_EDIT_FOCUS_SELECTION);
    }
}
