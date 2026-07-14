#pragma once

#include <string>

namespace library {

struct Book {
    int id = 0;
    std::string title;
    std::string author;
    int total = 0;
    int borrowed = 0;

    int available() const noexcept { return total - borrowed; }
};

struct Statistics {
    int titles = 0;
    int totalCopies = 0;
    int borrowedCopies = 0;
    int availableCopies = 0;
};

}  // namespace library
