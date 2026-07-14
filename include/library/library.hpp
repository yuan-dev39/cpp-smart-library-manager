#pragma once

#include "library/book.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace library {

class Library {
public:
    const std::vector<Book>& books() const noexcept { return books_; }

    bool addBook(const Book& book, std::string& error);
    bool updateBook(int id, const std::string& title, const std::string& author,
                    int total, std::string& error);
    bool removeBook(int id, std::string& error);
    bool borrowBook(int id, std::string& error);
    bool returnBook(int id, std::string& error);

    const Book* findById(int id) const noexcept;
    std::vector<Book> search(const std::string& keyword) const;
    Statistics statistics() const noexcept;

    bool load(const std::filesystem::path& file, std::string& error);
    bool save(const std::filesystem::path& file, std::string& error) const;

private:
    Book* findMutable(int id) noexcept;
    static bool validate(const Book& book, std::string& error);
    std::vector<Book> books_;
};

}  // namespace library
