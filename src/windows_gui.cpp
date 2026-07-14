#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include "library/library.hpp"

#include <windows.h>
#include <commctrl.h>

#include <filesystem>
#include <string>
#include <vector>

namespace {

constexpr int ID_EDIT_ID = 1001;
constexpr int ID_EDIT_TITLE = 1002;
constexpr int ID_EDIT_AUTHOR = 1003;
constexpr int ID_EDIT_TOTAL = 1004;
constexpr int ID_LIST = 1005;
constexpr int ID_ADD = 1010;
constexpr int ID_UPDATE = 1011;
constexpr int ID_DELETE = 1012;
constexpr int ID_SEARCH = 1013;
constexpr int ID_CLEAR = 1014;
constexpr int ID_BORROW = 1015;
constexpr int ID_RETURN = 1016;
constexpr int ID_SAVE = 1017;
constexpr int ID_STATUS = 1020;

library::Library g_library;
std::filesystem::path g_dataFile;
HWND g_list = nullptr;
HWND g_id = nullptr;
HWND g_title = nullptr;
HWND g_author = nullptr;
HWND g_total = nullptr;
HWND g_status = nullptr;
std::vector<int> g_visibleIds;

std::wstring toWide(const std::string& value) {
    if (value.empty()) return {};
    const int size = MultiByteToWideChar(CP_UTF8, 0, value.data(),
                                         static_cast<int>(value.size()), nullptr, 0);
    std::wstring result(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()),
                        result.data(), size);
    return result;
}

std::string toUtf8(const std::wstring& value) {
    if (value.empty()) return {};
    const int size = WideCharToMultiByte(CP_UTF8, 0, value.data(),
                                         static_cast<int>(value.size()), nullptr, 0,
                                         nullptr, nullptr);
    std::string result(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()),
                        result.data(), size, nullptr, nullptr);
    return result;
}

std::wstring editText(HWND edit) {
    const int length = GetWindowTextLengthW(edit);
    std::wstring value(static_cast<std::size_t>(length + 1), L'\0');
    if (length > 0) GetWindowTextW(edit, value.data(), length + 1);
    value.resize(static_cast<std::size_t>(length));
    return value;
}

bool readInt(HWND edit, int& value) {
    const std::wstring text = editText(edit);
    if (text.empty()) return false;
    wchar_t* end = nullptr;
    const long parsed = wcstol(text.c_str(), &end, 10);
    if (end == text.c_str() || *end != L'\0') return false;
    value = static_cast<int>(parsed);
    return true;
}

void showError(HWND window, const std::string& message) {
    MessageBoxW(window, toWide(message).c_str(), L"エラー", MB_OK | MB_ICONWARNING);
}

void clearInputs() {
    SetWindowTextW(g_id, L"");
    SetWindowTextW(g_title, L"");
    SetWindowTextW(g_author, L"");
    SetWindowTextW(g_total, L"");
    SetFocus(g_id);
}

void updateStatus(const std::wstring& prefix = L"") {
    const auto stats = g_library.statistics();
    std::wstring text = prefix;
    if (!text.empty()) text += L"  |  ";
    text += L"書目数: " + std::to_wstring(stats.titles) +
            L"    総冊数: " + std::to_wstring(stats.totalCopies) +
            L"    貸出中: " + std::to_wstring(stats.borrowedCopies) +
            L"    在庫: " + std::to_wstring(stats.availableCopies);
    SetWindowTextW(g_status, text.c_str());
}

void setCell(int row, int column, const std::wstring& text) {
    ListView_SetItemText(g_list, row, column, const_cast<wchar_t*>(text.c_str()));
}

void displayBooks(const std::vector<library::Book>& books) {
    ListView_DeleteAllItems(g_list);
    g_visibleIds.clear();
    for (std::size_t index = 0; index < books.size(); ++index) {
        const auto& book = books[index];
        const std::wstring id = std::to_wstring(book.id);
        LVITEMW item{};
        item.mask = LVIF_TEXT;
        item.iItem = static_cast<int>(index);
        item.pszText = const_cast<wchar_t*>(id.c_str());
        ListView_InsertItem(g_list, &item);
        setCell(static_cast<int>(index), 1, toWide(book.title));
        setCell(static_cast<int>(index), 2, toWide(book.author));
        setCell(static_cast<int>(index), 3, std::to_wstring(book.total));
        setCell(static_cast<int>(index), 4, std::to_wstring(book.borrowed));
        setCell(static_cast<int>(index), 5, std::to_wstring(book.available()));
        g_visibleIds.push_back(book.id);
    }
    updateStatus();
}

void displayAll() { displayBooks(g_library.books()); }

bool saveData(HWND window, const std::wstring& successMessage = L"") {
    std::string error;
    if (!g_library.save(g_dataFile, error)) {
        showError(window, error);
        return false;
    }
    updateStatus(successMessage);
    return true;
}

bool selectedId(HWND window, int& id) {
    if (readInt(g_id, id)) return true;
    showError(window, "図書番号を入力するか、一覧から図書を選択してください。");
    return false;
}

void addBook(HWND window) {
    library::Book book;
    if (!readInt(g_id, book.id) || !readInt(g_total, book.total)) {
        showError(window, "図書番号と総冊数は整数で入力してください。");
        return;
    }
    book.title = toUtf8(editText(g_title));
    book.author = toUtf8(editText(g_author));
    std::string error;
    if (!g_library.addBook(book, error)) {
        showError(window, error);
        return;
    }
    if (!saveData(window)) return;
    displayAll();
    updateStatus(L"図書を追加しました");
    clearInputs();
}

void updateBook(HWND window) {
    int id = 0;
    int total = 0;
    if (!selectedId(window, id) || !readInt(g_total, total)) {
        if (id != 0) showError(window, "総冊数は整数で入力してください。");
        return;
    }
    std::string error;
    if (!g_library.updateBook(id, toUtf8(editText(g_title)),
                              toUtf8(editText(g_author)), total, error)) {
        showError(window, error);
        return;
    }
    if (!saveData(window)) return;
    displayAll();
    updateStatus(L"図書情報を変更しました");
}

void deleteBook(HWND window) {
    int id = 0;
    if (!selectedId(window, id)) return;
    if (MessageBoxW(window, L"選択した図書を削除しますか？", L"削除の確認",
                    MB_YESNO | MB_ICONQUESTION) != IDYES) return;
    std::string error;
    if (!g_library.removeBook(id, error)) {
        showError(window, error);
        return;
    }
    if (!saveData(window)) return;
    displayAll();
    updateStatus(L"図書を削除しました");
    clearInputs();
}

void searchBooks(HWND window) {
    std::wstring keyword = editText(g_id);
    if (keyword.empty()) keyword = editText(g_title);
    if (keyword.empty()) keyword = editText(g_author);
    if (keyword.empty()) {
        showError(window, "図書番号、書名、著者のいずれかを入力してください。");
        return;
    }
    const auto results = g_library.search(toUtf8(keyword));
    displayBooks(results);
    updateStatus(L"検索結果: " + std::to_wstring(results.size()) + L"件");
}

void lendingOperation(HWND window, bool borrow) {
    int id = 0;
    if (!selectedId(window, id)) return;
    std::string error;
    const bool success = borrow ? g_library.borrowBook(id, error)
                                : g_library.returnBook(id, error);
    if (!success) {
        showError(window, error);
        return;
    }
    if (!saveData(window)) return;
    displayAll();
    updateStatus(borrow ? L"貸出処理が完了しました" : L"返却処理が完了しました");
}

void onSelectionChanged(int row) {
    if (row < 0 || row >= static_cast<int>(g_visibleIds.size())) return;
    const library::Book* book = g_library.findById(g_visibleIds[static_cast<std::size_t>(row)]);
    if (!book) return;
    SetWindowTextW(g_id, std::to_wstring(book->id).c_str());
    SetWindowTextW(g_title, toWide(book->title).c_str());
    SetWindowTextW(g_author, toWide(book->author).c_str());
    SetWindowTextW(g_total, std::to_wstring(book->total).c_str());
}

HWND createControl(HWND parent, const wchar_t* type, const wchar_t* text, DWORD style,
                   int id) {
    return CreateWindowExW(0, type, text, WS_CHILD | WS_VISIBLE | style, 0, 0, 0, 0,
                           parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
                           GetModuleHandleW(nullptr), nullptr);
}

void layout(HWND window, int width, int height) {
    const int margin = 14;
    const int labelY = 16;
    const int editY = 38;
    MoveWindow(GetDlgItem(window, 2001), margin, labelY, 90, 20, TRUE);
    MoveWindow(g_id, margin, editY, 105, 27, TRUE);
    MoveWindow(GetDlgItem(window, 2002), 130, labelY, 80, 20, TRUE);
    MoveWindow(g_title, 130, editY, 250, 27, TRUE);
    MoveWindow(GetDlgItem(window, 2003), 392, labelY, 80, 20, TRUE);
    MoveWindow(g_author, 392, editY, 220, 27, TRUE);
    MoveWindow(GetDlgItem(window, 2004), 624, labelY, 80, 20, TRUE);
    MoveWindow(g_total, 624, editY, 90, 27, TRUE);

    const int buttonY = 78;
    const int buttonW = 88;
    const int gap = 8;
    const int ids[] = {ID_ADD, ID_UPDATE, ID_DELETE, ID_SEARCH, ID_CLEAR,
                       ID_BORROW, ID_RETURN, ID_SAVE};
    for (int i = 0; i < 8; ++i) {
        MoveWindow(GetDlgItem(window, ids[i]), margin + i * (buttonW + gap), buttonY,
                   buttonW, 30, TRUE);
    }
    MoveWindow(g_list, margin, 122, width - margin * 2, height - 172, TRUE);
    MoveWindow(g_status, margin, height - 40, width - margin * 2, 24, TRUE);
}

void setupColumns() {
    struct Column { const wchar_t* name; int width; } columns[] = {
        {L"図書番号", 90}, {L"書名", 260}, {L"著者", 220},
        {L"総冊数", 80}, {L"貸出中", 80}, {L"在庫", 80}
    };
    for (int index = 0; index < 6; ++index) {
        LVCOLUMNW column{};
        column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        column.pszText = const_cast<wchar_t*>(columns[index].name);
        column.cx = columns[index].width;
        column.iSubItem = index;
        ListView_InsertColumn(g_list, index, &column);
    }
}

LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE: {
            createControl(window, L"STATIC", L"図書番号", 0, 2001);
            createControl(window, L"STATIC", L"書名", 0, 2002);
            createControl(window, L"STATIC", L"著者", 0, 2003);
            createControl(window, L"STATIC", L"総冊数", 0, 2004);
            g_id = createControl(window, L"EDIT", L"", WS_BORDER | ES_AUTOHSCROLL, ID_EDIT_ID);
            g_title = createControl(window, L"EDIT", L"", WS_BORDER | ES_AUTOHSCROLL, ID_EDIT_TITLE);
            g_author = createControl(window, L"EDIT", L"", WS_BORDER | ES_AUTOHSCROLL, ID_EDIT_AUTHOR);
            g_total = createControl(window, L"EDIT", L"", WS_BORDER | ES_AUTOHSCROLL, ID_EDIT_TOTAL);
            createControl(window, L"BUTTON", L"追加", BS_PUSHBUTTON, ID_ADD);
            createControl(window, L"BUTTON", L"変更", BS_PUSHBUTTON, ID_UPDATE);
            createControl(window, L"BUTTON", L"削除", BS_PUSHBUTTON, ID_DELETE);
            createControl(window, L"BUTTON", L"検索", BS_PUSHBUTTON, ID_SEARCH);
            createControl(window, L"BUTTON", L"全件表示", BS_PUSHBUTTON, ID_CLEAR);
            createControl(window, L"BUTTON", L"貸出", BS_PUSHBUTTON, ID_BORROW);
            createControl(window, L"BUTTON", L"返却", BS_PUSHBUTTON, ID_RETURN);
            createControl(window, L"BUTTON", L"保存", BS_PUSHBUTTON, ID_SAVE);
            g_list = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
                                     WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
                                     0, 0, 0, 0, window,
                                     reinterpret_cast<HMENU>(ID_LIST),
                                     GetModuleHandleW(nullptr), nullptr);
            ListView_SetExtendedListViewStyle(g_list,
                                               LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
            g_status = createControl(window, L"STATIC", L"", SS_LEFT, ID_STATUS);
            setupColumns();
            const HFONT font = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
            EnumChildWindows(window, [](HWND child, LPARAM value) -> BOOL {
                SendMessageW(child, WM_SETFONT, static_cast<WPARAM>(value), TRUE);
                return TRUE;
            }, reinterpret_cast<LPARAM>(font));
            std::string error;
            if (!g_library.load(g_dataFile, error)) showError(window, error);
            displayAll();
            return 0;
        }
        case WM_SIZE:
            layout(window, LOWORD(lParam), HIWORD(lParam));
            return 0;
        case WM_GETMINMAXINFO:
            reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize = {820, 500};
            return 0;
        case WM_NOTIFY: {
            const auto* header = reinterpret_cast<NMHDR*>(lParam);
            if (header->idFrom == ID_LIST && header->code == LVN_ITEMCHANGED) {
                const auto* changed = reinterpret_cast<NMLISTVIEW*>(lParam);
                if (changed->uNewState & LVIS_SELECTED) onSelectionChanged(changed->iItem);
            }
            return 0;
        }
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_ADD: addBook(window); break;
                case ID_UPDATE: updateBook(window); break;
                case ID_DELETE: deleteBook(window); break;
                case ID_SEARCH: searchBooks(window); break;
                case ID_CLEAR: clearInputs(); displayAll(); break;
                case ID_BORROW: lendingOperation(window, true); break;
                case ID_RETURN: lendingOperation(window, false); break;
                case ID_SAVE: saveData(window, L"保存しました"); break;
            }
            return 0;
        case WM_CLOSE:
            if (saveData(window)) DestroyWindow(window);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProcW(window, message, wParam, lParam);
    }
}

std::filesystem::path dataPath() {
    wchar_t module[MAX_PATH]{};
    GetModuleFileNameW(nullptr, module, MAX_PATH);
    const std::filesystem::path executable(module);
    const auto projectData = executable.parent_path().parent_path() / L"data" / L"books.csv";
    if (std::filesystem::exists(projectData)) return projectData;
    return executable.parent_path() / L"books.csv";
}

}  // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand) {
    INITCOMMONCONTROLSEX controls{sizeof(controls), ICC_LISTVIEW_CLASSES};
    InitCommonControlsEx(&controls);
    g_dataFile = dataPath();

    const wchar_t* className = L"SmartLibraryManagerWindow";
    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.lpfnWndProc = windowProcedure;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    windowClass.hIconSm = windowClass.hIcon;
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    windowClass.lpszClassName = className;
    if (!RegisterClassExW(&windowClass)) return 1;

    HWND window = CreateWindowExW(0, className, L"スマート図書管理システム",
                                  WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                                  980, 650, nullptr, nullptr, instance, nullptr);
    if (!window) return 1;
    ShowWindow(window, showCommand);
    UpdateWindow(window);

    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return static_cast<int>(message.wParam);
}
