#pragma once
#include <iostream>
#include <string>
#include <iomanip>  
#include <exception>
#include <../../vendor/soci/include/soci/soci.h>
void addSeatsToHall(soci::session& sql, int hallId, int rows, int seatsPerRow);
void addHallsToCinema(soci::session& sql, bool isAdmin, int cinemaId);

void addMovie(soci::session& sql, bool isAdmin) {
    if (!isAdmin) {
        std::cout << "Access denied. Admin privileges required.\n";
        return;
    }

    std::string title, language, genre, releaseDate;

    std::cout << "\n=== Add New Movie ===\n";
    std::cin.ignore();
    std::cout << "Enter movie title: ";
    std::getline(std::cin, title);
    std::cout << "Enter language: ";
    std::getline(std::cin, language);
    std::cout << "Enter genre: ";
    std::getline(std::cin, genre);
    std::cout << "Enter release date (YYYY-MM-DD): ";
    std::getline(std::cin, releaseDate);

    try {
        int movieId;
        sql << "INSERT INTO \"Movie\" (title, language, genre, \"releaseDate\") "
            "VALUES (:title, :lang, :genre, :reldate) RETURNING id",
            soci::use(title), soci::use(language), soci::use(genre),
            soci::use(releaseDate), soci::into(movieId);

        std::string message = "New movie added: " + title;
        sql << "INSERT INTO \"Notification\" (type, message, \"createdAt\") "
            "VALUES ('NEW_MOVIE', :msg, NOW())",
            soci::use(message);

        std::cout << "\nMovie added successfully! Movie ID: " << movieId << "\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Error adding movie: " << e.what() << "\n";
    }
}


void addShow(soci::session& sql, bool isAdmin) {
    if (!isAdmin) {
        std::cout << "Access denied. Admin privileges required.\n";
        return;
    }

    int movieId, hallId;
    std::string startTime, endTime;

    std::cout << "\n=== Add New Show ===\n";
    std::cout << "Enter Movie ID: ";
    std::cin >> movieId;
    std::cout << "Enter Hall ID: ";
    std::cin >> hallId;
    std::cin.ignore();
    std::cout << "Enter start time (YYYY-MM-DD HH:MM:SS): ";
    std::getline(std::cin, startTime);
    std::cout << "Enter end time (YYYY-MM-DD HH:MM:SS): ";
    std::getline(std::cin, endTime);

    try {
        int showId;
        sql << "INSERT INTO \"Show\" (\"movieId\", \"hallId\", \"startTime\", \"endTime\") "
            "VALUES (:mid, :hid, :start, :end) RETURNING id",
            soci::use(movieId), soci::use(hallId),
            soci::use(startTime), soci::use(endTime),
            soci::into(showId);

        std::cout << "\nShow added successfully! Show ID: " << showId << "\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Error adding show: " << e.what() << "\n";
    }
}


void viewStatistics(soci::session& sql, bool isAdmin) {
    if (!isAdmin) {
        std::cout << "Access denied. Admin privileges required.\n";
        return;
    }

    try {
        std::cout << "\n=== Booking Statistics ===\n";

        // Total bookings and revenue
        int totalBookings = 0;
        sql << "SELECT COUNT(*) FROM \"Booking\" WHERE status = 'CONFIRMED'", soci::into(totalBookings);

        double totalRevenue = 0.0;
        sql << "SELECT COALESCE(SUM(amount), 0) FROM \"Payment\"", soci::into(totalRevenue);

        std::cout << "\n--- Summary Statistics ---\n";
        std::cout << "Total Confirmed Bookings: " << totalBookings << "\n";
        std::cout << "Total Revenue: $" << std::fixed << std::setprecision(2) << totalRevenue << "\n";

        double avgBookingValue = (totalBookings > 0) ? (totalRevenue / totalBookings) : 0.0;
        std::cout << "Average Booking Value: $" << std::fixed << std::setprecision(2) << avgBookingValue << "\n";

        // Movies with booking counts
        std::cout << "\n--- Movies and Their Booking Counts ---\n";
        std::cout << std::left << std::setw(5) << "ID"
            << std::setw(30) << "Movie Title"
            << std::setw(12) << "Genre"
            << std::setw(10) << "Bookings\n";
        std::cout << std::string(57, '-') << "\n";

        soci::rowset<soci::row> movieStats = (sql.prepare <<
            "SELECT m.id, m.title, m.genre, COALESCE(booking_counts.booking_count, 0) "
            "FROM \"Movie\" m "
            "LEFT JOIN ("
            "    SELECT s.\"movieId\", COUNT(b.id) AS booking_count "
            "    FROM \"Show\" s "
            "    LEFT JOIN \"Booking\" b ON s.id = b.\"showId\" AND b.status = 'CONFIRMED' "
            "    GROUP BY s.\"movieId\""
            ") booking_counts ON m.id = booking_counts.\"movieId\" "
            "ORDER BY booking_count DESC, m.title");

        for (const auto& row : movieStats) {
            try {
                int movieId = 0;
                if (row.get_indicator(0) != soci::i_null) {
                    movieId = row.get<int>(0);
                }

                std::string title = "N/A";
                if (row.get_indicator(1) != soci::i_null) {
                    title = row.get<std::string>(1);
                }

                std::string genre = "N/A";
                if (row.get_indicator(2) != soci::i_null) {
                    genre = row.get<std::string>(2);
                }

                int bookings = 0;
                if (row.get_indicator(3) != soci::i_null) {
                    bookings = row.get<int>(3);
                }

                std::cout << std::left << std::setw(5) << movieId
                    << std::setw(30) << title
                    << std::setw(12) << genre
                    << std::setw(10) << bookings << "\n";
            }
            catch (const std::exception& rowError) {
                std::cerr << "Error processing movie row: " << rowError.what() << "\n";
            }
        }

        // Seat types with booking counts
        std::cout << "\n--- Seat Types and Their Booking Counts ---\n";
        std::cout << std::left << std::setw(15) << "Seat Type" << std::setw(10) << "Bookings\n";
        std::cout << std::string(25, '-') << "\n";

        soci::rowset<soci::row> seatStats = (sql.prepare <<
            "SELECT seat_types.type::text, COALESCE(booking_counts.booking_count, 0) "
            "FROM ("
            "    SELECT DISTINCT type::text AS type FROM \"Seat\""
            ") seat_types "
            "LEFT JOIN ("
            "    SELECT s.type::text AS type, COUNT(b.id) AS booking_count "
            "    FROM \"Seat\" s "
            "    JOIN \"Booking\" b ON s.id = b.\"seatId\" "
            "    WHERE b.status = 'CONFIRMED' "
            "    GROUP BY s.type"
            ") booking_counts ON seat_types.type = booking_counts.type "
            "ORDER BY booking_count DESC");

        for (const auto& row : seatStats) {
            try {
                std::string seatType = (row.get_indicator(0) != soci::i_null) ? row.get<std::string>(0) : "N/A";
                int bookings = row.get<int>(1);

                std::cout << std::left << std::setw(15) << seatType
                    << std::setw(10) << bookings << "\n";
            }
            catch (const std::exception& rowError) {
                std::cerr << "Error processing seat row: " << rowError.what() << "\n";
            }
        }

        // Cinema performance
        std::cout << "\n--- Cinema Performance ---\n";
        std::cout << std::left << std::setw(5) << "ID"
            << std::setw(20) << "Cinema Name"
            << std::setw(15) << "City"
            << std::setw(10) << "Bookings\n";
        std::cout << std::string(50, '-') << "\n";

        soci::rowset<soci::row> cinemaStats = (sql.prepare <<
            "SELECT c.id, c.name, ci.name, COALESCE(booking_counts.booking_count, 0) "
            "FROM \"Cinema\" c "
            "JOIN \"City\" ci ON c.\"cityId\" = ci.id "
            "LEFT JOIN ("
            "    SELECT h.\"cinemaId\", COUNT(b.id) AS booking_count "
            "    FROM \"Hall\" h "
            "    JOIN \"Show\" s ON h.id = s.\"hallId\" "
            "    LEFT JOIN \"Booking\" b ON s.id = b.\"showId\" AND b.status = 'CONFIRMED' "
            "    GROUP BY h.\"cinemaId\""
            ") booking_counts ON c.id = booking_counts.\"cinemaId\" "
            "ORDER BY booking_count DESC, c.name");

        for (const auto& row : cinemaStats) {
            try {
                int cinemaId = row.get<int>(0);
                std::string cinemaName = (row.get_indicator(1) != soci::i_null) ? row.get<std::string>(1) : "N/A";
                std::string cityName = (row.get_indicator(2) != soci::i_null) ? row.get<std::string>(2) : "N/A";
                int bookings = row.get<int>(3);

                std::cout << std::left << std::setw(5) << cinemaId
                    << std::setw(20) << cinemaName
                    << std::setw(15) << cityName
                    << std::setw(10) << bookings << "\n";
            }
            catch (const std::exception& rowError) {
                std::cerr << "Error processing cinema row: " << rowError.what() << "\n";
            }
        }

    }
    catch (const std::exception& e) {
        std::cerr << "Error fetching statistics: " << e.what() << "\n";
    }

    std::cout << "\nPress Enter to return...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}



void addCinema(soci::session& sql, bool isAdmin) {
    if (!isAdmin) {
        std::cout << "Access denied. Admin privileges required.\n";
        return;
    }

    std::string cinemaName;
    int cityId;

    std::cout << "\n=== Add New Cinema ===\n";

    soci::rowset<soci::row> cities = (sql.prepare << "SELECT id, name FROM \"City\" ORDER BY name");
    std::cout << "\nAvailable Cities:\n";
    std::cout << std::left << std::setw(5) << "ID" << std::setw(20) << "City Name" << std::endl;
    std::cout << std::string(25, '-') << std::endl;

    for (auto& row : cities) {
        std::cout << std::left << std::setw(5) << row.get<int>(0) << std::setw(20) << row.get<std::string>(1) << std::endl;
    }

    std::cout << "\nEnter City ID: ";
    std::cin >> cityId;
    std::cin.ignore();
    std::cout << "Enter cinema name: ";
    std::getline(std::cin, cinemaName);

    int cinemaId;
    sql << "INSERT INTO \"Cinema\" (name, \"cityId\") VALUES (:name, :cityId) RETURNING id",
        soci::use(cinemaName), soci::use(cityId), soci::into(cinemaId);

    std::cout << "\nCinema added successfully! Cinema ID: " << cinemaId << std::endl;

    char addHalls;
    std::cout << "Do you want to add halls to this cinema? (y/n): ";
    std::cin >> addHalls;

    if (addHalls == 'y' || addHalls == 'Y') {
        addHallsToCinema(sql, isAdmin, cinemaId);
    }
}

void addHallsToCinema(soci::session& sql, bool isAdmin, int cinemaId) {
    if (!isAdmin) {
        std::cout << "Access denied. Admin privileges required.\n";
        return;
    }

    int hallCount;
    std::cout << "\nHow many halls do you want to add? ";
    std::cin >> hallCount;

    for (int i = 0; i < hallCount; i++) {
        std::string hallName;
        int seatRows, seatsPerRow;

        std::cout << "\n--- Hall " << (i + 1) << " ---\n";
        std::cout << "Enter hall name: ";
        std::cin.ignore();
        std::getline(std::cin, hallName);

        int hallId;
        sql << "INSERT INTO \"Hall\" (name, \"cinemaId\") VALUES (:name, :cinemaId) RETURNING id",
            soci::use(hallName), soci::use(cinemaId), soci::into(hallId);

        std::cout << "Hall added! Hall ID: " << hallId << std::endl;

        std::cout << "Enter number of seat rows: ";
        std::cin >> seatRows;
        std::cout << "Enter seats per row: ";
        std::cin >> seatsPerRow;

        addSeatsToHall(sql, hallId, seatRows, seatsPerRow);
    }
}

void addSeatsToHall(soci::session& sql, int hallId, int rows, int seatsPerRow) {
    std::cout << "\nAdding seats to hall...\n";

    for (int row = 0; row < rows; row++) {
        char rowLetter = 'A' + row;

        for (int seatNum = 1; seatNum <= seatsPerRow; seatNum++) {
            std::string seatType = "SILVER";
            if (row >= rows - 3) seatType = "PLATINUM";
            else if (row >= rows / 2) seatType = "GOLD";

            sql << "INSERT INTO \"Seat\" (row, number, type, \"hallId\") VALUES (:row, :number, :type, :hallId)",
                soci::use(std::string(1, rowLetter)), soci::use(seatNum), soci::use(seatType), soci::use(hallId);
        }
    }

    std::cout << "Added " << (rows * seatsPerRow) << " seats to the hall.\n";
}

void addCity(soci::session& sql, bool isAdmin) {
    if (!isAdmin) {
        std::cout << "Access denied. Admin privileges required.\n";
        return;
    }

    std::string cityName;
    std::cout << "\n=== Add New City ===\n";
    std::cout << "Enter city name: ";
    std::cin.ignore();
    std::getline(std::cin, cityName);

    int cityId;
    sql << "INSERT INTO \"City\" (name) VALUES (:name) RETURNING id",
        soci::use(cityName), soci::into(cityId);

    std::cout << "City added successfully! City ID: " << cityId << std::endl;
}


