#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <unordered_map>

class Movie {
private:
    std::string title;
    std::string description;
    int durationMinutes;
    std::string genre;

public:
    Movie(std::string title, std::string description, int durationMinutes, std::string genre)
        : title(title), description(description), durationMinutes(durationMinutes), genre(genre) {
    }

    std::string getTitle() const { return title; }
    std::string getDescription() const { return description; }
    int getDurationMinutes() const { return durationMinutes; }
    std::string getGenre() const { return genre; }
};