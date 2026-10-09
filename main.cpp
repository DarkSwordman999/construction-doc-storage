#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <codecvt>
#include <locale>
#include <algorithm>
#include <cctype>
#include <set>
#include <iomanip>
#include <map>
#include <unordered_set>

#ifndef IDC_STATIC
#define IDC_STATIC -1
#endif

#define IDC_LISTVIEW 1001
#define IDC_ADDBUTTON 1002
#define IDC_EDITBUTTON 1003
#define IDC_DELETEBUTTON 1004
#define IDC_LOADBUTTON 1005
#define IDC_SAVEBUTTON 1006
#define IDC_SEARCHEDIT 1007
#define IDC_SEARCHBUTTON 1008
#define IDC_CLEARBUTTON 1009          // Кнопка "Очистить все"
#define IDC_CLEARSEARCHBUTTON 1010    // Кнопка "Очистить поиск"

#define IDC_NAME_EDIT 101
#define IDC_AUTHOR_EDIT 102
#define IDC_DATE_EDIT 103
#define IDC_PATH_EDIT 104
#define IDC_BROWSE_BUTTON 105
#define IDC_OK_BUTTON 106
#define IDC_CANCEL_BUTTON 107

// Вспомогательные функции для конвертации
std::string WStringToUTF8(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(),
                                          NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(),
                       &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

std::wstring UTF8ToWString(const std::string& str) {
    if (str.empty()) return L"";
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(),
                                         NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(),
                       &wstrTo[0], size_needed);
    return wstrTo;
}

std::string WStringToString(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int size_needed = WideCharToMultiByte(CP_ACP, 0, &wstr[0], (int)wstr.size(),
                                          NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_ACP, 0, &wstr[0], (int)wstr.size(),
                       &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

std::wstring StringToWString(const std::string& str) {
    if (str.empty()) return L"";
    int size_needed = MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(),
                                         NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(),
                       &wstrTo[0], size_needed);
    return wstrTo;
}

std::wstring TrimWString(const std::wstring& str) {
    size_t first = str.find_first_not_of(L" \t\n\r");
    if (first == std::wstring::npos) return L"";
    size_t last = str.find_last_not_of(L" \t\n\r");
    return str.substr(first, last - first + 1);
}

std::string TrimString(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

bool FileExists(const std::wstring& filename) {
    DWORD dwAttrib = GetFileAttributesW(filename.c_str());
    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
           !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

// Функция для генерации триграмм из строки
std::unordered_set<std::wstring> GenerateTrigrams(const std::wstring& str) {
    std::unordered_set<std::wstring> trigrams;
    if (str.length() < 3) {
        // Для коротких строк добавляем саму строку как триграмму
        if (!str.empty()) {
            trigrams.insert(str);
        }
        return trigrams;
    }

    for (size_t i = 0; i <= str.length() - 3; ++i) {
        trigrams.insert(str.substr(i, 3));
    }
    return trigrams;
}

// Функция для вычисления коэффициента сходства по триграммам (коэффициент Жаккара)
double CalculateSimilarity(const std::unordered_set<std::wstring>& trigrams1,
                          const std::unordered_set<std::wstring>& trigrams2) {
    if (trigrams1.empty() && trigrams2.empty()) return 1.0;
    if (trigrams1.empty() || trigrams2.empty()) return 0.0;

    int intersection = 0;
    for (const auto& trigram : trigrams1) {
        if (trigrams2.find(trigram) != trigrams2.end()) {
            intersection++;
        }
    }

    int union_size = trigrams1.size() + trigrams2.size() - intersection;
    return union_size > 0 ? static_cast<double>(intersection) / union_size : 0.0;
}

// Класс документа
class Document {
private:
    std::wstring name;
    std::wstring author;
    std::wstring date;
    std::wstring filePath;
    std::unordered_set<std::wstring> nameTrigrams;
    std::unordered_set<std::wstring> authorTrigrams;

public:
    Document(const std::wstring& n = L"", const std::wstring& a = L"",
             const std::wstring& d = L"", const std::wstring& f = L"")
        : name(TrimWString(n)), author(TrimWString(a)),
          date(TrimWString(d)), filePath(TrimWString(f)) {
        updateTrigrams();
    }

    void updateTrigrams() {
        nameTrigrams = GenerateTrigrams(name);
        authorTrigrams = GenerateTrigrams(author);
    }

    bool isValid() const {
        return !name.empty() && !filePath.empty();
    }

    std::wstring getName() const { return name; }
    std::wstring getAuthor() const { return author; }
    std::wstring getDate() const { return date; }
    std::wstring getFilePath() const { return filePath; }

    const std::unordered_set<std::wstring>& getNameTrigrams() const { return nameTrigrams; }
    const std::unordered_set<std::wstring>& getAuthorTrigrams() const { return authorTrigrams; }

    void setName(const std::wstring& n) {
        name = TrimWString(n);
        updateTrigrams();
    }
    void setAuthor(const std::wstring& a) {
        author = TrimWString(a);
        updateTrigrams();
    }
    void setDate(const std::wstring& d) { date = TrimWString(d); }
    void setFilePath(const std::wstring& f) { filePath = TrimWString(f); }

    // Для сравнения документов (для предотвращения дубликатов)
    bool operator==(const Document& other) const {
        return name == other.name &&
               author == other.author &&
               date == other.date &&
               filePath == other.filePath;
    }

    // Для хранения в set
    bool operator<(const Document& other) const {
        if (name != other.name) return name < other.name;
        if (author != other.author) return author < other.author;
        if (date != other.date) return date < other.date;
        return filePath < other.filePath;
    }

    std::string toString() const {
        std::string name_utf8 = WStringToUTF8(name);
        std::string author_utf8 = WStringToUTF8(author);
        std::string date_utf8 = WStringToUTF8(date);
        std::string filePath_utf8 = WStringToUTF8(filePath);

        return name_utf8 + ";" + author_utf8 + ";" + date_utf8 + ";" + filePath_utf8;
    }

    static Document fromString(const std::string& str) {
        std::string trimmed = TrimString(str);
        if (trimmed.empty()) return Document();

        size_t pos1 = trimmed.find(';');
        if (pos1 == std::string::npos) return Document(UTF8ToWString(trimmed));

        size_t pos2 = trimmed.find(';', pos1 + 1);
        if (pos2 == std::string::npos) return Document();

        size_t pos3 = trimmed.find(';', pos2 + 1);
        if (pos3 == std::string::npos) return Document();

        std::string name_utf8 = trimmed.substr(0, pos1);
        std::string author_utf8 = trimmed.substr(pos1 + 1, pos2 - pos1 - 1);
        std::string date_utf8 = trimmed.substr(pos2 + 1, pos3 - pos2 - 1);
        std::string filePath_utf8 = trimmed.substr(pos3 + 1);

        return Document(UTF8ToWString(name_utf8), UTF8ToWString(author_utf8),
                       UTF8ToWString(date_utf8), UTF8ToWString(filePath_utf8));
    }
};

// Класс хранилища документов
class DocumentStorage {
private:
    std::vector<Document> documents;
    bool isSearchMode;
    std::vector<Document> searchResults;
    std::wstring lastSearchQuery;

public:
    DocumentStorage() : isSearchMode(false), lastSearchQuery(L"") {}

    void addDocument(const Document& doc) {
        if (doc.isValid()) {
            // Проверяем дубликаты
            if (std::find(documents.begin(), documents.end(), doc) == documents.end()) {
                documents.push_back(doc);
            }
        }
    }

    bool editDocument(size_t index, const Document& doc) {
        if (index < documents.size() && doc.isValid()) {
            documents[index] = doc;
            return true;
        }
        return false;
    }

    bool deleteDocument(size_t index) {
        if (isSearchMode) {
            // Удаляем из результатов поиска и из основного хранилища
            if (index < searchResults.size()) {
                Document docToDelete = searchResults[index];
                // Находим и удаляем из основного списка
                auto it = std::find(documents.begin(), documents.end(), docToDelete);
                if (it != documents.end()) {
                    documents.erase(it);
                }
                // Обновляем результаты поиска
                searchDocuments(lastSearchQuery);
                return true;
            }
        } else {
            if (index < documents.size()) {
                documents.erase(documents.begin() + index);
                return true;
            }
        }
        return false;
    }

    void searchDocuments(const std::wstring& keyword) {
        lastSearchQuery = keyword;
        searchResults.clear();

        if (keyword.empty()) {
            isSearchMode = false;
            return;
        }

        isSearchMode = true;

        // Генерируем триграммы для поискового запроса
        std::wstring lowerKeyword = keyword;
        std::transform(lowerKeyword.begin(), lowerKeyword.end(), lowerKeyword.begin(), ::towlower);
        std::unordered_set<std::wstring> keywordTrigrams = GenerateTrigrams(lowerKeyword);

        const double SIMILARITY_THRESHOLD = 0.3; // 30% сходства

        for (const auto& doc : documents) {
            // Проверяем по названию
            std::wstring docName = doc.getName();
            std::transform(docName.begin(), docName.end(), docName.begin(), ::towlower);

            std::unordered_set<std::wstring> docNameTrigrams = GenerateTrigrams(docName);
            double nameSimilarity = CalculateSimilarity(keywordTrigrams, docNameTrigrams);

            // Проверяем по автору
            std::wstring docAuthor = doc.getAuthor();
            std::transform(docAuthor.begin(), docAuthor.end(), docAuthor.begin(), ::towlower);

            std::unordered_set<std::wstring> docAuthorTrigrams = GenerateTrigrams(docAuthor);
            double authorSimilarity = CalculateSimilarity(keywordTrigrams, docAuthorTrigrams);

            // Также проверяем обычное вхождение для коротких запросов
            bool exactMatch = (keyword.length() <= 3 &&
                              (docName.find(lowerKeyword) != std::wstring::npos ||
                               docAuthor.find(lowerKeyword) != std::wstring::npos));

            // Также проверяем по дате и пути
            std::wstring docDate = doc.getDate();
            std::transform(docDate.begin(), docDate.end(), docDate.begin(), ::towlower);

            std::wstring docPath = doc.getFilePath();
            std::transform(docPath.begin(), docPath.end(), docPath.begin(), ::towlower);

            // Добавляем в результаты, если превышен порог или есть точное совпадение
            if (nameSimilarity >= SIMILARITY_THRESHOLD ||
                authorSimilarity >= SIMILARITY_THRESHOLD ||
                exactMatch ||
                docName.find(lowerKeyword) != std::wstring::npos ||
                docAuthor.find(lowerKeyword) != std::wstring::npos ||
                docDate.find(lowerKeyword) != std::wstring::npos ||
                docPath.find(lowerKeyword) != std::wstring::npos) {
                searchResults.push_back(doc);
            }
        }
    }

    void clearSearch() {
        isSearchMode = false;
        searchResults.clear();
        lastSearchQuery = L"";
    }

    void clearAllDocuments() {
        documents.clear();
        clearSearch();
    }

    const std::vector<Document>& getCurrentDocuments() const {
        return isSearchMode ? searchResults : documents;
    }

    Document getDocument(size_t index) const {
        const std::vector<Document>& currentDocs = isSearchMode ? searchResults : documents;
        if (index < currentDocs.size()) {
            return currentDocs[index];
        }
        return Document();
    }

    Document getDocumentFromStorage(size_t index) const {
        if (index < documents.size()) {
            return documents[index];
        }
        return Document();
    }

    size_t getDocumentCount() const {
        return isSearchMode ? searchResults.size() : documents.size();
    }

    size_t getTotalDocumentCount() const {
        return documents.size();
    }

    bool isInSearchMode() const {
        return isSearchMode;
    }

    std::wstring getLastSearchQuery() const {
        return lastSearchQuery;
    }

    bool saveToFile(const std::wstring& filename) const {
        if (documents.empty()) {
            return false;
        }

        std::string filename_str = WStringToString(filename);
        std::ofstream file(filename_str, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }

        // Пишем UTF-8 BOM
        unsigned char bom[] = {0xEF, 0xBB, 0xBF};
        file.write(reinterpret_cast<const char*>(bom), sizeof(bom));

        // Используем set для устранения дубликатов при сохранении
        std::set<Document> uniqueDocs(documents.begin(), documents.end());

        for (const auto& doc : uniqueDocs) {
            std::string line = doc.toString();
            file << line << "\r\n";
        }

        file.close();
        return true;
    }

    bool loadFromFile(const std::wstring& filename, bool clearExisting = true) {
        std::string filename_str = WStringToString(filename);
        std::ifstream file(filename_str, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }

        std::vector<Document> loadedDocuments;
        if (!clearExisting) {
            loadedDocuments = documents;
        }

        file.seekg(0, std::ios::end);
        std::streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        if (fileSize == 0) {
            file.close();
            return true;
        }

        std::string line;
        int loadedCount = 0;
        int errorCount = 0;

        if (fileSize >= 3) {
            unsigned char bom[3];
            file.read(reinterpret_cast<char*>(bom), 3);
            if (!(bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF)) {
                file.seekg(0, std::ios::beg);
            }
        }

        while (std::getline(file, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            if (line.size() >= 3 &&
                static_cast<unsigned char>(line[0]) == 0xEF &&
                static_cast<unsigned char>(line[1]) == 0xBB &&
                static_cast<unsigned char>(line[2]) == 0xBF) {
                line = line.substr(3);
            }

            std::string trimmed = TrimString(line);
            if (trimmed.empty()) {
                continue;
            }

            try {
                Document doc = Document::fromString(trimmed);
                if (doc.isValid()) {
                    // Проверяем дубликаты
                    if (std::find(loadedDocuments.begin(), loadedDocuments.end(), doc) == loadedDocuments.end()) {
                        loadedDocuments.push_back(doc);
                        loadedCount++;
                    }
                } else {
                    errorCount++;
                }
            } catch (...) {
                errorCount++;
                continue;
            }
        }

        file.close();

        if (loadedCount > 0) {
            documents = loadedDocuments;
            clearSearch(); // Сбрасываем поиск после загрузки

            if (errorCount > 0) {
                std::wstring message = L"Загружено " + std::to_wstring(loadedCount) +
                                     L" документов.\n" +
                                     L"Пропущено " + std::to_wstring(errorCount) +
                                     L" строк с ошибками.";
                MessageBoxW(NULL, message.c_str(), L"Загрузка с предупреждениями",
                           MB_OK | MB_ICONWARNING);
            }
            return true;
        }

        return false;
    }
};

// Глобальные объекты
DocumentStorage storage;
HWND hListView;
HWND hSearchEdit;
HWND hMainWindow = NULL;
HWND hDialog = NULL;
bool isEditMode = false;
int editIndex = -1;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);
void InitListView(HWND hwndListView);
void RefreshListView(HWND hwndListView);
void ShowDocumentDialog(HWND hParent, bool edit, int index);
bool ValidateDocumentFields(const std::wstring& name, const std::wstring& path);
std::wstring GetFileSizeString(const std::wstring& filename);

// Инициализация ListView
void InitListView(HWND hwndListView) {
    ListView_SetExtendedListViewStyle(hwndListView,
        LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);

    LVCOLUMNW lvc;
    ZeroMemory(&lvc, sizeof(LVCOLUMNW));
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
    lvc.fmt = LVCFMT_LEFT;

    const wchar_t* headers[] = {L"Название", L"Автор", L"Дата", L"Путь к файлу"};
    int widths[] = {200, 150, 120, 250};

    for (int i = 0; i < 4; i++) {
        lvc.pszText = const_cast<wchar_t*>(headers[i]);
        lvc.cx = widths[i];
        SendMessageW(hwndListView, LVM_INSERTCOLUMNW, i, (LPARAM)&lvc);
    }
}

// Обновление ListView
void RefreshListView(HWND hwndListView) {
    SendMessageW(hwndListView, LVM_DELETEALLITEMS, 0, 0);

    const std::vector<Document>& docs = storage.getCurrentDocuments();

    for (size_t i = 0; i < docs.size(); ++i) {
        LVITEMW lvi;
        ZeroMemory(&lvi, sizeof(LVITEMW));
        lvi.mask = LVIF_TEXT;
        lvi.iItem = (int)i;

        lvi.iSubItem = 0;
        std::wstring name = docs[i].getName();
        lvi.pszText = const_cast<LPWSTR>(name.c_str());
        SendMessageW(hwndListView, LVM_INSERTITEMW, 0, (LPARAM)&lvi);

        lvi.iSubItem = 1;
        std::wstring author = docs[i].getAuthor();
        lvi.pszText = const_cast<LPWSTR>(author.c_str());
        SendMessageW(hwndListView, LVM_SETITEMW, 0, (LPARAM)&lvi);

        lvi.iSubItem = 2;
        std::wstring date = docs[i].getDate();
        lvi.pszText = const_cast<LPWSTR>(date.c_str());
        SendMessageW(hwndListView, LVM_SETITEMW, 0, (LPARAM)&lvi);

        lvi.iSubItem = 3;
        std::wstring filePath = docs[i].getFilePath();
        lvi.pszText = const_cast<LPWSTR>(filePath.c_str());
        SendMessageW(hwndListView, LVM_SETITEMW, 0, (LPARAM)&lvi);
    }

    // Обновляем заголовок окна
    std::wstring title = L"Хранилище проектной документации";
    if (storage.isInSearchMode()) {
        title += L" - Найдено: " + std::to_wstring(storage.getDocumentCount()) +
                L" (Всего: " + std::to_wstring(storage.getTotalDocumentCount()) + L")";
    } else {
        title += L" - Всего документов: " + std::to_wstring(storage.getTotalDocumentCount());
    }
    SetWindowTextW(hMainWindow, title.c_str());
}

bool ValidateDocumentFields(const std::wstring& name, const std::wstring& path) {
    if (name.empty()) {
        MessageBoxW(NULL, L"Название документа не может быть пустым",
                   L"Ошибка валидации", MB_OK | MB_ICONERROR);
        return false;
    }

    if (path.empty()) {
        MessageBoxW(NULL, L"Путь к файлу не может быть пустым",
                   L"Ошибка валидации", MB_OK | MB_ICONERROR);
        return false;
    }

    if (!FileExists(path)) {
        if (MessageBoxW(NULL, L"Указанный файл не существует.\nПродолжить сохранение?",
                       L"Предупреждение", MB_YESNO | MB_ICONWARNING) != IDYES) {
            return false;
        }
    }

    return true;
}

std::wstring GetFileSizeString(const std::wstring& filename) {
    if (!FileExists(filename)) {
        return L"Файл не найден";
    }

    HANDLE hFile = CreateFileW(filename.c_str(), GENERIC_READ,
                               FILE_SHARE_READ, NULL, OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        return L"Ошибка доступа";
    }

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) {
        CloseHandle(hFile);
        return L"Ошибка определения размера";
    }

    CloseHandle(hFile);

    if (fileSize.QuadPart < 1024) {
        return std::to_wstring(fileSize.QuadPart) + L" байт";
    } else if (fileSize.QuadPart < 1024 * 1024) {
        double kb = fileSize.QuadPart / 1024.0;
        std::wstringstream ws;
        ws << std::fixed << std::setprecision(2) << kb << L" КБ";
        return ws.str();
    } else if (fileSize.QuadPart < 1024 * 1024 * 1024) {
        double mb = fileSize.QuadPart / (1024.0 * 1024.0);
        std::wstringstream ws;
        ws << std::fixed << std::setprecision(2) << mb << L" МБ";
        return ws.str();
    } else {
        double gb = fileSize.QuadPart / (1024.0 * 1024.0 * 1024.0);
        std::wstringstream ws;
        ws << std::fixed << std::setprecision(2) << gb << L" ГБ";
        return ws.str();
    }
}

LRESULT CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        HFONT hFont = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));

        CreateWindowW(L"STATIC", L"Название документа:", WS_CHILD | WS_VISIBLE,
                      10, 10, 150, 25, hDlg, NULL, GetModuleHandle(NULL), NULL);
        HWND hNameEdit = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP,
                      170, 10, 250, 25, hDlg, (HMENU)IDC_NAME_EDIT, GetModuleHandle(NULL), NULL);
        if (hFont) SendMessage(hNameEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

        CreateWindowW(L"STATIC", L"Автор:", WS_CHILD | WS_VISIBLE,
                      10, 45, 150, 25, hDlg, NULL, GetModuleHandle(NULL), NULL);
        HWND hAuthorEdit = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP,
                      170, 45, 250, 25, hDlg, (HMENU)IDC_AUTHOR_EDIT, GetModuleHandle(NULL), NULL);
        if (hFont) SendMessage(hAuthorEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

        CreateWindowW(L"STATIC", L"Дата (ДД.ММ.ГГГГ):", WS_CHILD | WS_VISIBLE,
                      10, 80, 150, 25, hDlg, NULL, GetModuleHandle(NULL), NULL);
        HWND hDateEdit = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP,
                      170, 80, 250, 25, hDlg, (HMENU)IDC_DATE_EDIT, GetModuleHandle(NULL), NULL);
        if (hFont) SendMessage(hDateEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

        CreateWindowW(L"STATIC", L"Путь к файлу:", WS_CHILD | WS_VISIBLE,
                      10, 115, 150, 25, hDlg, NULL, GetModuleHandle(NULL), NULL);
        HWND hPathEdit = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP,
                      170, 115, 200, 25, hDlg, (HMENU)IDC_PATH_EDIT, GetModuleHandle(NULL), NULL);
        if (hFont) SendMessage(hPathEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

        CreateWindowW(L"BUTTON", L"Обзор...", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                      380, 115, 80, 25, hDlg, (HMENU)IDC_BROWSE_BUTTON, GetModuleHandle(NULL), NULL);

        CreateWindowW(L"BUTTON", L"ОК", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
                      100, 160, 80, 30, hDlg, (HMENU)IDC_OK_BUTTON, GetModuleHandle(NULL), NULL);
        CreateWindowW(L"BUTTON", L"Отмена", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                      220, 160, 80, 30, hDlg, (HMENU)IDC_CANCEL_BUTTON, GetModuleHandle(NULL), NULL);

        if (isEditMode && editIndex >= 0) {
            Document doc;
            if (storage.isInSearchMode()) {
                doc = storage.getDocument(editIndex);
            } else {
                doc = storage.getDocumentFromStorage(editIndex);
            }
            SetDlgItemTextW(hDlg, IDC_NAME_EDIT, doc.getName().c_str());
            SetDlgItemTextW(hDlg, IDC_AUTHOR_EDIT, doc.getAuthor().c_str());
            SetDlgItemTextW(hDlg, IDC_DATE_EDIT, doc.getDate().c_str());
            SetDlgItemTextW(hDlg, IDC_PATH_EDIT, doc.getFilePath().c_str());
            SetWindowTextW(hDlg, L"Редактировать документ");
        } else {
            SetWindowTextW(hDlg, L"Добавить документ");
        }

        return 0;
    }
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        if (wmId == IDC_BROWSE_BUTTON) {
            OPENFILENAMEW ofn;
            wchar_t szFile[MAX_PATH] = {0};

            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hDlg;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile)/sizeof(szFile[0]);
            ofn.lpstrFilter = L"Все файлы\0*.*\0\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

            if (GetOpenFileNameW(&ofn)) {
                SetDlgItemTextW(hDlg, IDC_PATH_EDIT, szFile);
            }
        }
        else if (wmId == IDC_OK_BUTTON) {
            wchar_t buffer[512];

            GetDlgItemTextW(hDlg, IDC_NAME_EDIT, buffer, 512);
            std::wstring name(buffer);
            GetDlgItemTextW(hDlg, IDC_AUTHOR_EDIT, buffer, 512);
            std::wstring author(buffer);
            GetDlgItemTextW(hDlg, IDC_DATE_EDIT, buffer, 512);
            std::wstring date(buffer);
            GetDlgItemTextW(hDlg, IDC_PATH_EDIT, buffer, 512);
            std::wstring path(buffer);

            if (ValidateDocumentFields(name, path)) {
                Document newDoc(name, author, date, path);

                if (isEditMode && editIndex >= 0) {
                    // Определяем, нужно ли редактировать по индексу из поиска или основного списка
                    if (storage.isInSearchMode()) {
                        // Получаем документ из результатов поиска
                        Document oldDoc = storage.getDocument(editIndex);
                        // Находим его позицию в основном списке
                        const std::vector<Document>& allDocs = storage.getCurrentDocuments();
                        auto it = std::find(allDocs.begin(), allDocs.end(), oldDoc);
                        if (it != allDocs.end()) {
                            size_t realIndex = std::distance(allDocs.begin(), it);
                            storage.editDocument(realIndex, newDoc);
                        }
                    } else {
                        storage.editDocument(editIndex, newDoc);
                    }
                } else {
                    storage.addDocument(newDoc);
                }

                // Если был активен поиск, обновляем результаты
                if (storage.isInSearchMode()) {
                    storage.searchDocuments(storage.getLastSearchQuery());
                }

                RefreshListView(hListView);
            } else {
                return 0;
            }

            DestroyWindow(hDlg);
            hDialog = NULL;
        } else if (wmId == IDC_CANCEL_BUTTON) {
            DestroyWindow(hDlg);
            hDialog = NULL;
        }
        break;
    }
    case WM_CLOSE:
        DestroyWindow(hDlg);
        hDialog = NULL;
        break;
    case WM_DESTROY:
        hDialog = NULL;
        break;
    }
    return DefWindowProcW(hDlg, message, wParam, lParam);
}

void ShowDocumentDialog(HWND hParent, bool edit, int index) {
    if (hDialog != NULL) {
        SetForegroundWindow(hDialog);
        return;
    }

    isEditMode = edit;
    editIndex = index;

    WNDCLASSEXW wc;
    ZeroMemory(&wc, sizeof(WNDCLASSEXW));
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = DialogProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"DocumentDialogClass";

    static bool classRegistered = false;
    if (!classRegistered) {
        if (!RegisterClassExW(&wc)) {
            MessageBoxW(NULL, L"Не удалось зарегистрировать класс диалога",
                       L"Ошибка", MB_OK | MB_ICONERROR);
            return;
        }
        classRegistered = true;
    }

    hDialog = CreateWindowExW(0, L"DocumentDialogClass", L"",
                           WS_POPUP | WS_CAPTION | WS_SYSMENU,
                           0, 0, 480, 230, hParent, NULL, GetModuleHandle(NULL), NULL);

    if (!hDialog) {
        MessageBoxW(NULL, L"Не удалось создать диалоговое окно",
                   L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }

    RECT rcParent, rcDlg;
    GetWindowRect(hParent, &rcParent);
    GetWindowRect(hDialog, &rcDlg);

    int x = rcParent.left + (rcParent.right - rcParent.left - (rcDlg.right - rcDlg.left)) / 2;
    int y = rcParent.top + (rcParent.bottom - rcParent.top - (rcDlg.bottom - rcDlg.top)) / 2;
    SetWindowPos(hDialog, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

    ShowWindow(hDialog, SW_SHOW);
    UpdateWindow(hDialog);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icex);

    WNDCLASSEXW wc;
    ZeroMemory(&wc, sizeof(WNDCLASSEXW));
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = L"DocStorageClass";
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassExW(&wc)) {
        MessageBoxW(NULL, L"Не удалось зарегистрировать класс окна",
                   L"Ошибка", MB_OK | MB_ICONERROR);
        return 0;
    }

    hMainWindow = CreateWindowW(L"DocStorageClass", L"Хранилище проектной документации",
                              WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                              CW_USEDEFAULT, CW_USEDEFAULT,
                              900, 600, NULL, NULL, hInstance, NULL);

    if (!hMainWindow) {
        MessageBoxW(NULL, L"Не удалось создать главное окно",
                   L"Ошибка", MB_OK | MB_ICONERROR);
        return 0;
    }

    ShowWindow(hMainWindow, nCmdShow);
    UpdateWindow(hMainWindow);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hSearchLabel; // Добавляем переменную для хранения метки поиска

    switch (msg) {
    case WM_CREATE: {
        HFONT hFont = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));

        hListView = CreateWindowW(L"SysListView32", L"",
                                 WS_CHILD | WS_VISIBLE | WS_BORDER |
                                 LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
                                 10, 50, 860, 400, hwnd,
                                 (HMENU)IDC_LISTVIEW, GetModuleHandle(NULL), NULL);
        if (hFont) SendMessage(hListView, WM_SETFONT, (WPARAM)hFont, TRUE);
        InitListView(hListView);

        // Первый ряд кнопок
        HWND hAddBtn = CreateWindowW(L"BUTTON", L"Добавить",
                                    WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                                    10, 10, 100, 30, hwnd,
                                    (HMENU)IDC_ADDBUTTON, NULL, NULL);
        if (hFont) SendMessage(hAddBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND hEditBtn = CreateWindowW(L"BUTTON", L"Редактировать",
                                     WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                                     120, 10, 120, 30, hwnd,
                                     (HMENU)IDC_EDITBUTTON, NULL, NULL);
        if (hFont) SendMessage(hEditBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND hDeleteBtn = CreateWindowW(L"BUTTON", L"Удалить",
                                       WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                                       250, 10, 100, 30, hwnd,
                                       (HMENU)IDC_DELETEBUTTON, NULL, NULL);
        if (hFont) SendMessage(hDeleteBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND hLoadBtn = CreateWindowW(L"BUTTON", L"Загрузить",
                                     WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                                     360, 10, 100, 30, hwnd,
                                     (HMENU)IDC_LOADBUTTON, NULL, NULL);
        if (hFont) SendMessage(hLoadBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND hSaveBtn = CreateWindowW(L"BUTTON", L"Сохранить",
                                     WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                                     470, 10, 100, 30, hwnd,
                                     (HMENU)IDC_SAVEBUTTON, NULL, NULL);
        if (hFont) SendMessage(hSaveBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND hClearBtn = CreateWindowW(L"BUTTON", L"Очистить все",
                                     WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                                     580, 10, 110, 30, hwnd,
                                     (HMENU)IDC_CLEARBUTTON, NULL, NULL);
        if (hFont) SendMessage(hClearBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Второй ряд: поиск
        hSearchLabel = CreateWindowW(L"STATIC", L"Поиск:", WS_CHILD | WS_VISIBLE,
                      10, 460, 50, 25, hwnd, (HMENU)IDC_STATIC, GetModuleHandle(NULL), NULL);
        if (hFont) SendMessage(hSearchLabel, WM_SETFONT, (WPARAM)hFont, TRUE);

        hSearchEdit = CreateWindowW(L"EDIT", L"",
                                   WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,
                                   65, 460, 200, 25, hwnd,
                                   (HMENU)IDC_SEARCHEDIT, NULL, NULL);
        if (hFont) SendMessage(hSearchEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND hSearchBtn = CreateWindowW(L"BUTTON", L"Найти",
                                       WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                                       275, 460, 80, 25, hwnd,
                                       (HMENU)IDC_SEARCHBUTTON, NULL, NULL);
        if (hFont) SendMessage(hSearchBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND hClearSearchBtn = CreateWindowW(L"BUTTON", L"Очистить поиск",
                                           WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                                           365, 460, 120, 25, hwnd,
                                           (HMENU)IDC_CLEARSEARCHBUTTON, NULL, NULL);
        if (hFont) SendMessage(hClearSearchBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

        RefreshListView(hListView);
        break;
    }
    case WM_SIZE: {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);

        // Изменяем размер ListView
        MoveWindow(hListView, 10, 50, width - 30, height - 140, TRUE);

        // Перемещаем элементы поиска
        MoveWindow(hSearchLabel, 10, height - 80, 50, 25, TRUE);
        MoveWindow(hSearchEdit, 65, height - 80, 200, 25, TRUE);
        MoveWindow(GetDlgItem(hwnd, IDC_SEARCHBUTTON), 275, height - 80, 80, 25, TRUE);
        MoveWindow(GetDlgItem(hwnd, IDC_CLEARSEARCHBUTTON), 365, height - 80, 120, 25, TRUE);
        break;
    }
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);

        if (wmId == IDC_ADDBUTTON) {
            ShowDocumentDialog(hwnd, false, -1);
        }
        else if (wmId == IDC_EDITBUTTON) {
            int index = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
            if (index >= 0) {
                ShowDocumentDialog(hwnd, true, index);
            } else {
                MessageBoxW(hwnd, L"Выберите документ для редактирования",
                           L"Информация", MB_OK | MB_ICONINFORMATION);
            }
        }
        else if (wmId == IDC_DELETEBUTTON) {
            int index = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
            if (index >= 0) {
                std::wstring docName = storage.getDocument(index).getName();
                std::wstring message = L"Удалить документ \"" + docName + L"\"?";

                if (MessageBoxW(hwnd, message.c_str(),
                               L"Подтверждение", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    if (storage.deleteDocument(index)) {
                        RefreshListView(hListView);
                        MessageBoxW(hwnd, L"Документ успешно удален",
                                   L"Успех", MB_OK | MB_ICONINFORMATION);
                    }
                }
            } else {
                MessageBoxW(hwnd, L"Выберите документ для удаления",
                           L"Информация", MB_OK | MB_ICONINFORMATION);
            }
        }
        else if (wmId == IDC_LOADBUTTON) {
            size_t beforeCount = storage.getTotalDocumentCount();
            bool mergeData = false;

            if (beforeCount > 0) {
                int result = MessageBoxW(hwnd,
                    L"Выберите вариант загрузки:\n\n"
                    L"Да - добавить к существующим документам\n"
                    L"Нет - заменить все документы\n"
                    L"Отмена - отменить загрузку",
                    L"Загрузка документов",
                    MB_YESNOCANCEL | MB_ICONQUESTION);

                if (result == IDCANCEL) break;
                mergeData = (result == IDYES);
            }

            if (!FileExists(L"documents.txt")) {
                if (MessageBoxW(hwnd, L"Файл documents.txt не найден.\n"
                                 L"Создать пустой файл?",
                               L"Файл не найден",
                               MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    std::ofstream file("documents.txt", std::ios::binary);
                    if (file.is_open()) {
                        unsigned char bom[] = {0xEF, 0xBB, 0xBF};
                        file.write(reinterpret_cast<const char*>(bom), sizeof(bom));
                        file.close();
                        MessageBoxW(hwnd, L"Создан пустой файл documents.txt",
                                   L"Информация", MB_OK | MB_ICONINFORMATION);
                    }
                }
                break;
            }

            if (storage.loadFromFile(L"documents.txt", !mergeData)) {
                size_t afterCount = storage.getTotalDocumentCount();
                RefreshListView(hListView);

                std::wstring message;
                if (mergeData) {
                    size_t added = afterCount - beforeCount;
                    message = L"Добавлено " + std::to_wstring(added) +
                             L" новых документов\n" +
                             L"Всего документов: " + std::to_wstring(afterCount);
                } else {
                    message = L"Загружено " + std::to_wstring(afterCount) + L" документов";
                }
                MessageBoxW(hwnd, message.c_str(),
                           L"Загрузка завершена", MB_OK | MB_ICONINFORMATION);
            } else {
                MessageBoxW(hwnd, L"Ошибка при чтении файла documents.txt\n"
                             L"Файл может быть поврежден или иметь неверный формат.",
                           L"Ошибка загрузки", MB_OK | MB_ICONERROR);
            }
        }
        else if (wmId == IDC_SAVEBUTTON) {
            size_t count = storage.getTotalDocumentCount();
            if (count == 0) {
                if (MessageBoxW(hwnd, L"Нет документов для сохранения.\n"
                                 L"Создать пустой файл?",
                               L"Предупреждение", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    std::ofstream file("documents.txt", std::ios::binary);
                    if (file.is_open()) {
                        unsigned char bom[] = {0xEF, 0xBB, 0xBF};
                        file.write(reinterpret_cast<const char*>(bom), sizeof(bom));
                        file.close();
                        MessageBoxW(hwnd, L"Создан пустой файл documents.txt",
                                   L"Информация", MB_OK | MB_ICONINFORMATION);
                    }
                }
                break;
            }

            if (FileExists(L"documents.txt")) {
                std::wstring fileInfo = GetFileSizeString(L"documents.txt");
                if (MessageBoxW(hwnd, (L"Файл documents.txt уже существует.\n" +
                                      fileInfo + L"\n" +
                                      L"Перезаписать файл?").c_str(),
                               L"Подтверждение",
                               MB_YESNO | MB_ICONQUESTION) != IDYES) {
                    break;
                }
            }

            if (storage.saveToFile(L"documents.txt")) {
                std::wstring message = L"Успешно сохранено " + std::to_wstring(count) +
                                     L" документов в documents.txt";
                MessageBoxW(hwnd, message.c_str(),
                           L"Сохранение завершена", MB_OK | MB_ICONINFORMATION);
            } else {
                MessageBoxW(hwnd, L"Ошибка при сохранении в documents.txt\n"
                             L"Проверьте права доступа к файлу.",
                           L"Ошибка сохранения", MB_OK | MB_ICONERROR);
            }
        }
        else if (wmId == IDC_CLEARBUTTON) {
            size_t count = storage.getTotalDocumentCount();
            if (count == 0) {
                MessageBoxW(hwnd, L"Хранилище уже пустое",
                           L"Информация", MB_OK | MB_ICONINFORMATION);
                break;
            }

            std::wstring message = L"Вы действительно хотите удалить ВСЕ документы?\n"
                                 L"Всего документов: " + std::to_wstring(count) +
                                 L"\n\nЭта операция необратима!";

            if (MessageBoxW(hwnd, message.c_str(),
                           L"Подтверждение удаления",
                           MB_YESNO | MB_ICONWARNING) == IDYES) {
                storage.clearAllDocuments();
                RefreshListView(hListView);
                MessageBoxW(hwnd, L"Все документы успешно удалены",
                           L"Очистка завершена", MB_OK | MB_ICONINFORMATION);
            }
        }
        else if (wmId == IDC_SEARCHBUTTON) {
            wchar_t buffer[256];
            GetWindowTextW(hSearchEdit, buffer, 256);
            std::wstring keyword = TrimWString(std::wstring(buffer));

            if (keyword.empty()) {
                // Если поле поиска пустое, сбрасываем поиск
                storage.clearSearch();
                RefreshListView(hListView);
                MessageBoxW(hwnd, L"Поле поиска пустое. Показаны все документы.",
                           L"Поиск", MB_OK | MB_ICONINFORMATION);
            } else {
                storage.searchDocuments(keyword);
                RefreshListView(hListView);

                if (storage.getDocumentCount() == 0) {
                    MessageBoxW(hwnd, L"Документы не найдены",
                               L"Поиск", MB_OK | MB_ICONINFORMATION);
                }
            }
        }
        else if (wmId == IDC_CLEARSEARCHBUTTON) {
            SetWindowTextW(hSearchEdit, L"");
            storage.clearSearch();
            RefreshListView(hListView);
        }
        break;
    }
    case WM_NOTIFY: {
        LPNMHDR nmhdr = (LPNMHDR)lParam;
        if (nmhdr->idFrom == IDC_LISTVIEW && nmhdr->code == NM_DBLCLK) {
            int index = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
            if (index >= 0) {
                ShowDocumentDialog(hwnd, true, index);
            }
        }
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}