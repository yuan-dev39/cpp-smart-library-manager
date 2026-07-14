#include "library/library.hpp"

#include <filesystem>
#include <iostream>
#include <string>

namespace {
int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        ++failures;
        std::cerr << "失敗: " << message << '\n';
    }
}
}

int main() {
    library::Library lib;
    std::string error;

    expect(lib.addBook({2, "The C++ Programming Language", "Bjarne Stroustrup", 2, 0}, error),
           "正しい図書を追加できる");
    expect(lib.addBook({1, "Clean Code, Second Edition", "Robert C. Martin", 1, 0}, error),
           "カンマを含む書名を追加できる");
    expect(!lib.addBook({1, "Duplicate", "Author", 1, 0}, error), "図書番号の重複を拒否する");
    expect(lib.books().front().id == 1, "図書番号順に並べ替えられる");
    expect(lib.search("stroustrup").size() == 1, "著者を大文字小文字を区別せず検索できる");
    expect(lib.search("clean").size() == 1, "書名を部分一致で検索できる");

    expect(lib.borrowBook(1, error), "在庫がある図書を貸し出せる");
    expect(!lib.borrowBook(1, error), "在庫を超えて貸し出せない");
    expect(!lib.removeBook(1, error), "貸出中の図書を削除できない");
    expect(!lib.updateBook(1, "Clean Code", "Robert C. Martin", 0, error),
           "総冊数を貸出中冊数より小さく変更できない");
    expect(lib.returnBook(1, error), "貸出中の図書を返却できる");

    const auto stats = lib.statistics();
    expect(stats.titles == 2 && stats.totalCopies == 3 && stats.availableCopies == 3,
           "統計を正しく計算できる");

    const auto file = std::filesystem::temp_directory_path() / "smart_library_test.csv";
    expect(lib.save(file, error), "データを保存できる");
    library::Library restored;
    expect(restored.load(file, error), "データを読み込める");
    expect(restored.books().size() == 2, "保存・再読込で全図書を維持する");
    expect(restored.findById(1) && restored.findById(1)->title == "Clean Code, Second Edition",
           "保存・再読込で引用されたCSV項目を維持する");
    std::error_code ignored;
    std::filesystem::remove(file, ignored);

    if (failures == 0) std::cout << "すべてのテストに合格しました。\n";
    return failures == 0 ? 0 : 1;
}
