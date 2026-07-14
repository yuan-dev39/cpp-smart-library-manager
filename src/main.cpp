#include "library/library.hpp"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

using library::Book;
using library::Library;

std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string value;
    std::getline(std::cin, value);
    return value;
}

bool readInt(const std::string& prompt, int& value) {
    const std::string input = readLine(prompt);
    try {
        std::size_t used = 0;
        value = std::stoi(input, &used);
        return used == input.size();
    } catch (...) {
        return false;
    }
}

void printBooks(const std::vector<Book>& books) {
    if (books.empty()) {
        std::cout << "該当する図書はありません。\n";
        return;
    }
    std::cout << '\n' << std::left << std::setw(7) << "ID" << std::setw(30) << "書名"
              << std::setw(24) << "著者" << std::right << std::setw(8) << "総冊数"
              << std::setw(10) << "貸出中" << std::setw(10) << "在庫" << '\n'
              << std::string(89, '-') << '\n';
    for (const Book& book : books) {
        std::cout << std::left << std::setw(7) << book.id
                  << std::setw(30) << book.title.substr(0, 28)
                  << std::setw(24) << book.author.substr(0, 22)
                  << std::right << std::setw(8) << book.total
                  << std::setw(10) << book.borrowed << std::setw(10) << book.available() << '\n';
    }
}

void addBook(Library& library) {
    Book book;
    if (!readInt("図書番号: ", book.id)) {
        std::cout << "図書番号の形式が不正です。\n";
        return;
    }
    book.title = readLine("書名: ");
    book.author = readLine("著者: ");
    if (!readInt("総冊数: ", book.total)) {
        std::cout << "冊数の形式が不正です。\n";
        return;
    }
    std::string error;
    std::cout << (library.addBook(book, error) ? "図書を追加しました。\n" : "エラー: " + error + "\n");
}

void updateBook(Library& library) {
    int id = 0;
    if (!readInt("変更する図書番号: ", id)) {
        std::cout << "図書番号の形式が不正です。\n";
        return;
    }
    const Book* current = library.findById(id);
    if (!current) {
        std::cout << "該当する図書が見つかりません。\n";
        return;
    }
    const std::string title = readLine("新しい書名: ");
    const std::string author = readLine("新しい著者名: ");
    int total = 0;
    if (!readInt("新しい総冊数: ", total)) {
        std::cout << "冊数の形式が不正です。\n";
        return;
    }
    std::string error;
    std::cout << (library.updateBook(id, title, author, total, error)
                      ? "図書情報を変更しました。\n"
                      : "エラー: " + error + "\n");
}

void idOperation(Library& library, int action) {
    int id = 0;
    if (!readInt("図書番号: ", id)) {
        std::cout << "図書番号の形式が不正です。\n";
        return;
    }
    std::string error;
    bool success = false;
    const char* message = "";
    if (action == 1) {
        success = library.removeBook(id, error);
        message = "図書を削除しました。";
    } else if (action == 2) {
        success = library.borrowBook(id, error);
        message = "貸出処理が完了しました。";
    } else {
        success = library.returnBook(id, error);
        message = "返却処理が完了しました。";
    }
    std::cout << (success ? std::string(message) + "\n" : "エラー: " + error + "\n");
}

void showStatistics(const Library& library) {
    const auto stats = library.statistics();
    std::cout << "書目数: " << stats.titles << " | 総冊数: " << stats.totalCopies
              << " | 貸出中: " << stats.borrowedCopies
              << " | 在庫: " << stats.availableCopies << '\n';
}

}  // namespace

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    const std::filesystem::path dataFile = argc > 1 ? argv[1] : "data/books.csv";
    Library library;
    std::string error;
    if (!library.load(dataFile, error)) {
        std::cerr << "図書データを読み込めません: " << error << '\n';
        return 1;
    }

    bool dirty = false;
    while (true) {
        std::cout << "\n========== 図書管理システム ==========\n"
                  << "1. 一覧表示             2. 検索\n"
                  << "3. 図書を追加           4. 図書情報を変更\n"
                  << "5. 図書を削除           6. 貸出\n"
                  << "7. 返却                 8. 統計\n"
                  << "9. 保存                 0. 保存して終了\n";
        int choice = -1;
        if (!readInt("選択してください: ", choice)) {
            std::cout << "0から9までの数字を入力してください。\n";
            continue;
        }
        switch (choice) {
            case 1: printBooks(library.books()); break;
            case 2: printBooks(library.search(readLine("図書番号・書名・著者: "))); break;
            case 3: addBook(library); dirty = true; break;
            case 4: updateBook(library); dirty = true; break;
            case 5: idOperation(library, 1); dirty = true; break;
            case 6: idOperation(library, 2); dirty = true; break;
            case 7: idOperation(library, 3); dirty = true; break;
            case 8: showStatistics(library); break;
            case 9:
                if (library.save(dataFile, error)) {
                    dirty = false;
                    std::cout << dataFile.string() << " に保存しました。\n";
                } else std::cout << "エラー: " << error << '\n';
                break;
            case 0:
                if (!dirty || library.save(dataFile, error)) return 0;
                std::cout << "エラー: " << error << '\n';
                break;
            default: std::cout << "0から9までの数字を入力してください。\n";
        }
    }
}
