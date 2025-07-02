#include "database.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <future>
#include <cli.hpp>
#include <menus.hpp>
#include <fstream>
#include <typeinfo>
#include <filesystem>
#include <models/Movie.h>

#pragma once
void searchMovies(soci::session& sql) {
    std::string rawInput;
    std::cout << "\n=== Search Movies by Title ===\n";
    std::cout << "Enter movie title (or part of it): ";

    // Clear input buffer properly
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::getline(std::cin, rawInput);

    // Trim whitespace
    rawInput.erase(0, rawInput.find_first_not_of(" \t\n\r\f\v"));
    rawInput.erase(rawInput.find_last_not_of(" \t\n\r\f\v") + 1);

    if (rawInput.empty()) {
        std::cout << "Empty search term. Please try again.\n";
        std::cout << "\nPress Enter to return...";
        std::cin.get();
        return;
    }

    std::string searchPattern = "%" + rawInput + "%";

    try {
        std::string query =
            "SELECT id, title, language, genre, "
            "TO_CHAR(\"releaseDate\", 'YYYY-MM-DD') as release_date "
            "FROM \"Movie\" "
            "WHERE LOWER(title) LIKE LOWER(:search) "
            "ORDER BY \"releaseDate\" DESC";

        soci::rowset<soci::row> rs = (sql.prepare << query, soci::use(searchPattern));

        std::cout << "\nSearching for: \"" << rawInput << "\"\n";
        std::cout << "\nSearch Results:\n";
        std::cout << std::left << std::setw(5) << "ID"
            << std::setw(30) << "Title"
            << std::setw(12) << "Language"
            << std::setw(15) << "Genre"
            << std::setw(12) << "Release Date" << "\n";
        std::cout << std::string(74, '-') << "\n";

        int count = 0;
        for (const auto& row : rs) {
            try {
                int id = row.get<int>(0);
                std::string title = row.get<std::string>(1);
                std::string language = row.get<std::string>(2);
                std::string genre = row.get<std::string>(3);
                std::string releaseDate = row.get<std::string>(4);

                std::cout << std::left << std::setw(5) << id
                    << std::setw(30) << title
                    << std::setw(12) << language
                    << std::setw(15) << genre
                    << std::setw(12) << releaseDate << "\n";
                ++count;
            }
            catch (const std::exception& rowError) {
                std::cerr << "Error processing row: " << rowError.what() << "\n";
            }
        }

        if (count == 0) {
            std::cout << "\nNo movies found matching \"" << rawInput << "\".\n";
        }
        else {
            std::cout << "\nFound " << count << " movie(s).\n";
        }

    }
    catch (const std::exception& e) {
        std::cerr << "Error searching movies: " << e.what() << "\n";
    }

    std::cout << "\nPress Enter to return...";
    std::cin.get();
}
