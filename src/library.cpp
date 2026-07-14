#include "library/library.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <system_error>

namespace library {
namespace {

std::string trim(const std::string& value) {
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) return {};
    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}

std::string lowerAscii(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::string escapeCsv(const std::string& value) {
    if (value.find_first_of(",\"\r\n") == std::string::npos) return value;
    std::string result = "\"";
    for (char ch : value) {
        if (ch == '"') result += '"';
        result += ch;
    }
    result += '"';
    return result;
}

bool parseCsvLine(const std::string& line, std::vector<std::string>& fields) {
    fields.clear();
    std::string field;
    bool quoted = false;
    for (std::size_t i = 0; i < line.size(); ++i) {
        const char ch = line[i];
        if (quoted) {
            if (ch == '"' && i + 1 < line.size() && line[i + 1] == '"') {
                field += '"';
                ++i;
            } else if (ch == '"') {
                quoted = false;
            } else {
                field += ch;
            }
        } else if (ch == ',') {
            fields.push_back(field);
            field.clear();
        } else if (ch == '"' && field.empty()) {
            quoted = true;
        } else {
            field += ch;
        }
    }
    if (quoted) return false;
    fields.push_back(field);
    return true;
}

bool parseInt(const std::string& value, int& result) {
    try {
        std::size_t used = 0;
        const int parsed = std::stoi(trim(value), &used);
        if (used != trim(value).size()) return false;
        result = parsed;
        return true;
    } catch (...) {
        return false;
    }
}

}  // namespace

bool Library::validate(const Book& book, std::string& error) {
    if (book.id <= 0) error = "図書番号は正の整数で入力してください。";
    else if (trim(book.title).empty()) error = "書名を入力してください。";
    else if (trim(book.author).empty()) error = "著者名を入力してください。";
    else if (book.total < 0) error = "総冊数に負の値は指定できません。";
    else if (book.borrowed < 0 || book.borrowed > book.total)
        error = "貸出中の冊数は0以上かつ総冊数以下である必要があります。";
    else return true;
    return false;
}

Book* Library::findMutable(int id) noexcept {
    const auto it = std::find_if(books_.begin(), books_.end(),
                                 [id](const Book& b) { return b.id == id; });
    return it == books_.end() ? nullptr : &*it;
}

const Book* Library::findById(int id) const noexcept {
    const auto it = std::find_if(books_.begin(), books_.end(),
                                 [id](const Book& b) { return b.id == id; });
    return it == books_.end() ? nullptr : &*it;
}

bool Library::addBook(const Book& book, std::string& error) {
    if (!validate(book, error)) return false;
    if (findById(book.id)) {
        error = "同じ図書番号が既に登録されています。";
        return false;
    }
    Book copy = book;
    copy.title = trim(copy.title);
    copy.author = trim(copy.author);
    books_.push_back(std::move(copy));
    std::sort(books_.begin(), books_.end(), [](const Book& a, const Book& b) {
        return a.id < b.id;
    });
    return true;
}

bool Library::updateBook(int id, const std::string& title, const std::string& author,
                         int total, std::string& error) {
    Book* current = findMutable(id);
    if (!current) {
        error = "該当する図書が見つかりません。";
        return false;
    }
    Book candidate{id, trim(title), trim(author), total, current->borrowed};
    if (!validate(candidate, error)) return false;
    *current = std::move(candidate);
    return true;
}

bool Library::removeBook(int id, std::string& error) {
    const Book* book = findById(id);
    if (!book) {
        error = "該当する図書が見つかりません。";
        return false;
    }
    if (book->borrowed > 0) {
        error = "貸出中の図書は削除できません。すべて返却してから削除してください。";
        return false;
    }
    books_.erase(std::remove_if(books_.begin(), books_.end(),
                                [id](const Book& b) { return b.id == id; }),
                 books_.end());
    return true;
}

bool Library::borrowBook(int id, std::string& error) {
    Book* book = findMutable(id);
    if (!book) error = "該当する図書が見つかりません。";
    else if (book->available() <= 0) error = "貸出可能な在庫がありません。";
    else {
        ++book->borrowed;
        return true;
    }
    return false;
}

bool Library::returnBook(int id, std::string& error) {
    Book* book = findMutable(id);
    if (!book) error = "該当する図書が見つかりません。";
    else if (book->borrowed <= 0) error = "この図書には貸出記録がありません。";
    else {
        --book->borrowed;
        return true;
    }
    return false;
}

std::vector<Book> Library::search(const std::string& keyword) const {
    const std::string needle = lowerAscii(trim(keyword));
    if (needle.empty()) return books_;
    std::vector<Book> matches;
    for (const Book& book : books_) {
        if (std::to_string(book.id) == needle ||
            lowerAscii(book.title).find(needle) != std::string::npos ||
            lowerAscii(book.author).find(needle) != std::string::npos) {
            matches.push_back(book);
        }
    }
    return matches;
}

Statistics Library::statistics() const noexcept {
    Statistics result;
    result.titles = static_cast<int>(books_.size());
    for (const Book& book : books_) {
        result.totalCopies += book.total;
        result.borrowedCopies += book.borrowed;
    }
    result.availableCopies = result.totalCopies - result.borrowedCopies;
    return result;
}

bool Library::load(const std::filesystem::path& file, std::string& error) {
    std::ifstream input(file, std::ios::binary);
    if (!input) {
        if (!std::filesystem::exists(file)) return true;
        error = "データファイルを開けません: " + file.string();
        return false;
    }

    std::vector<Book> loaded;
    std::string line;
    std::size_t lineNumber = 0;
    while (std::getline(input, line)) {
        ++lineNumber;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (lineNumber == 1 && line.rfind("id,title,author,total,borrowed", 0) == 0) continue;
        if (trim(line).empty()) continue;

        std::vector<std::string> fields;
        Book book;
        if (!parseCsvLine(line, fields) || fields.size() != 5 ||
            !parseInt(fields[0], book.id) || !parseInt(fields[3], book.total) ||
            !parseInt(fields[4], book.borrowed)) {
            error = "CSVの" + std::to_string(lineNumber) + "行目の形式が不正です。";
            return false;
        }
        book.title = fields[1];
        book.author = fields[2];
        std::string validationError;
        if (!validate(book, validationError)) {
            error = "CSVの" + std::to_string(lineNumber) + "行目が不正です: " + validationError;
            return false;
        }
        if (std::any_of(loaded.begin(), loaded.end(), [&](const Book& b) { return b.id == book.id; })) {
            error = "CSVの" + std::to_string(lineNumber) + "行目に重複した図書番号があります。";
            return false;
        }
        loaded.push_back(std::move(book));
    }
    std::sort(loaded.begin(), loaded.end(), [](const Book& a, const Book& b) { return a.id < b.id; });
    books_ = std::move(loaded);
    return true;
}

bool Library::save(const std::filesystem::path& file, std::string& error) const {
    std::error_code ec;
    if (file.has_parent_path()) std::filesystem::create_directories(file.parent_path(), ec);
    if (ec) {
        error = "データ保存用フォルダーを作成できません: " + ec.message();
        return false;
    }

    const auto temporary = file.string() + ".tmp";
    std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
    if (!output) {
        error = "データファイルへ書き込めません: " + file.string();
        return false;
    }
    output << "id,title,author,total,borrowed\n";
    for (const Book& book : books_) {
        output << book.id << ',' << escapeCsv(book.title) << ',' << escapeCsv(book.author)
               << ',' << book.total << ',' << book.borrowed << '\n';
    }
    output.close();
    if (!output) {
        error = "データファイルの書き込み中にエラーが発生しました。";
        return false;
    }
    std::filesystem::remove(file, ec);
    ec.clear();
    std::filesystem::rename(temporary, file, ec);
    if (ec) {
        error = "データファイルを更新できません: " + ec.message();
        return false;
    }
    return true;
}

}  // namespace library
