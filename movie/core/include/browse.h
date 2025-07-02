#pragma once
#include <soci/soci.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>


void viewCinemaDetails(soci::session& sql, int cinemaId) {
    std::cout << "\n=== Cinema Details ===\n";

    std::string cinemaName, cityName;
    soci::indicator ind;
    sql << "SELECT c.name, city.name FROM \"Cinema\" c "
        "JOIN \"City\" city ON c.\"cityId\" = city.id "
        "WHERE c.id = :id",
        soci::use(cinemaId), soci::into(cinemaName), soci::into(cityName);

    std::cout << "Cinema: " << cinemaName << std::endl;
    std::cout << "Location: " << cityName << std::endl;

    std::cout << "\n--- Hall Information ---\n";
    std::cout << std::left << std::setw(15) << "Hall Name"
        << std::setw(8) << "Total"
        << std::setw(8) << "Silver"
        << std::setw(8) << "Gold"
        << std::setw(10) << "Platinum" << std::endl;
    std::cout << std::string(49, '-') << std::endl;

    soci::rowset<soci::row> hallRows = (sql.prepare <<
        "SELECT h.name, COUNT(s.id), "
        "COUNT(CASE WHEN s.type = 'SILVER' THEN 1 END), "
        "COUNT(CASE WHEN s.type = 'GOLD' THEN 1 END), "
        "COUNT(CASE WHEN s.type = 'PLATINUM' THEN 1 END) "
        "FROM \"Hall\" h "
        "LEFT JOIN \"Seat\" s ON h.id = s.\"hallId\" "
        "WHERE h.\"cinemaId\" = :id "
        "GROUP BY h.name ORDER BY h.name",
        soci::use(cinemaId));

    for (const auto& row : hallRows) {
        std::cout << std::left << std::setw(15) << row.get<std::string>(0)
            << std::setw(8) << row.get<int>(1)
            << std::setw(8) << row.get<int>(2)
            << std::setw(8) << row.get<int>(3)
            << std::setw(10) << row.get<int>(4) << std::endl;
    }

    std::cout << "\n--- Upcoming Shows (Next 10) ---\n";
    std::cout << std::left << std::setw(25) << "Movie"
        << std::setw(20) << "Show Time"
        << std::setw(15) << "Hall" << std::endl;
    std::cout << std::string(60, '-') << std::endl;

    soci::rowset<soci::row> showRows = (sql.prepare <<
        "SELECT m.title, s.\"startTime\", h.name FROM \"Show\" s "
        "JOIN \"Movie\" m ON s.\"movieId\" = m.id "
        "JOIN \"Hall\" h ON s.\"hallId\" = h.id "
        "WHERE h.\"cinemaId\" = :id AND s.\"startTime\" >= NOW() "
        "ORDER BY s.\"startTime\" LIMIT 10",
        soci::use(cinemaId));

    for (const auto& row : showRows) {
        std::cout << std::left << std::setw(25) << row.get<std::string>(0)
            << std::setw(20) << row.get<std::string>(1)
            << std::setw(15) << row.get<std::string>(2) << std::endl;
    }
}

void browseCinemas(soci::session& sql) {
    std::cout << "\n=== Browse Cinemas ===\n";

    soci::rowset<soci::row> cinemas = (sql.prepare <<
        "SELECT c.id, c.name, city.name FROM \"Cinema\" c "
        "JOIN \"City\" city ON c.\"cityId\" = city.id "
        "ORDER BY city.name, c.name");

    std::string currentCity = "";
    for (const auto& row : cinemas) {
        int id = row.get<int>(0);
        std::string name = row.get<std::string>(1);
        std::string city = row.get<std::string>(2);

        if (city != currentCity) {
            if (!currentCity.empty()) std::cout << std::endl;
            std::cout << "=== " << city << " ===\n";
            currentCity = city;
        }

        std::cout << "\nCinema ID: " << id << std::endl;
        std::cout << "Name: " << name << std::endl;

        std::cout << "Halls: ";
        soci::rowset<std::string> halls = (sql.prepare <<
            "SELECT name FROM \"Hall\" WHERE \"cinemaId\" = :id ORDER BY name",
            soci::use(id));

        bool first = true;
        for (const auto& hall : halls) {
            if (!first) std::cout << ", ";
            std::cout << hall;
            first = false;
        }
        std::cout << std::endl;

        std::cout << "Current Movies: ";
        soci::rowset<soci::row> movies = (sql.prepare <<
            "SELECT m.title, COUNT(s.id) FROM \"Show\" s "
            "JOIN \"Movie\" m ON s.\"movieId\" = m.id "
            "JOIN \"Hall\" h ON s.\"hallId\" = h.id "
            "WHERE h.\"cinemaId\" = :id AND s.\"startTime\" >= NOW() "
            "GROUP BY m.id, m.title ORDER BY m.title",
            soci::use(id));

        first = true;
        for (const auto& movie : movies) {
            if (!first) std::cout << ", ";
            std::cout << movie.get<std::string>(0) << " (" << movie.get<int>(1) << " shows)";
            first = false;
        }
        if (first) std::cout << "None";
        std::cout << std::endl;
    }

    std::cout << "\nEnter Cinema ID for detailed information (or 0 to return): ";
    int cinemaId;
    std::cin >> cinemaId;
    if (cinemaId > 0) {
        viewCinemaDetails(sql, cinemaId);
    }
}