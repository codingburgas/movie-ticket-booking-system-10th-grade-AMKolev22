#pragma once
#include <soci/soci.h>
#include <soci/rowset.h>
#include <iomanip>
#include <limits>
#include <iostream>
void viewShows(soci::session& sql) {
    int movieId, hallId;
    std::cout << "\n=== View Shows ===\n";
    std::cout << "Enter Movie ID: ";
    std::cin >> movieId;
    std::cout << "Enter Hall ID: ";
    std::cin >> hallId;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    try {
        std::string query =
            "SELECT s.id, "
            "TO_CHAR(s.\"startTime\", 'YYYY-MM-DD HH24:MI') as start_time, "
            "TO_CHAR(s.\"endTime\", 'YYYY-MM-DD HH24:MI') as end_time, "
            "m.title, c.name as cinema_name, h.name as hall_name "
            "FROM \"Show\" s "
            "JOIN \"Movie\" m ON s.\"movieId\" = m.id "
            "JOIN \"Hall\" h ON s.\"hallId\" = h.id "
            "JOIN \"Cinema\" c ON h.\"cinemaId\" = c.id "
            "WHERE s.\"movieId\" = :movieId "
            "AND s.\"hallId\" = :hallId "
            "AND s.\"startTime\" >= NOW() "
            "ORDER BY s.\"startTime\"";

        soci::rowset<soci::row> rs = (sql.prepare << query,
            soci::use(movieId),
            soci::use(hallId));

        std::cout << "\nShows for Movie ID " << movieId << " in Hall ID " << hallId << ":\n";
        std::cout << "\nAvailable Shows:\n";
        std::cout << std::left << std::setw(8) << "Show ID"
            << std::setw(20) << "Start Time"
            << std::setw(20) << "End Time"
            << std::setw(25) << "Movie Title"
            << std::setw(15) << "Cinema"
            << std::setw(10) << "Hall" << "\n";
        std::cout << std::string(98, '-') << "\n";

        int count = 0;
        for (const auto& row : rs) {
            try {
                int showId = row.get<int>(0);
                std::string startTime = row.get<std::string>(1);
                std::string endTime = row.get<std::string>(2);
                std::string title = row.get<std::string>(3);
                std::string cinema = row.get<std::string>(4);
                std::string hall = row.get<std::string>(5);

                std::cout << std::left << std::setw(8) << showId
                    << std::setw(20) << startTime
                    << std::setw(20) << endTime
                    << std::setw(25) << title
                    << std::setw(15) << cinema
                    << std::setw(10) << hall << "\n";
                ++count;
            }
            catch (const std::exception& rowError) {
                std::cerr << "Error processing row: " << rowError.what() << "\n";
                // Continue with next row
            }
        }

        if (count == 0) {
            std::cout << "\nNo upcoming shows found for Movie ID " << movieId
                << " in Hall ID " << hallId << ".\n";
        }
        else {
            std::cout << "\nFound " << count << " upcoming show(s).\n";
        }

    }
    catch (const std::exception& e) {
        std::cerr << "Error retrieving shows: " << e.what() << "\n";
    }

    std::cout << "\nPress Enter to return...";
    std::cin.get();
}
