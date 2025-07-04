#pragma once
#include <iostream>
#include <string>
#include <iomanip>  
#include <exception>
#include <../../vendor/soci/include/soci/soci.h>

void addSeatsToHall(soci::session& sql, int hallId, int rows, int seatsPerRow);
void addHallsToCinema(soci::session& sql, bool isAdmin, int cinemaId);
void addShowsForMovie(soci::session& sql, int movieId, const std::string& movieTitle);
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

        std::cout << "\nWould you like to add shows for this movie? (y/n): ";
        char addShows;
        std::cin >> addShows;

        if (addShows == 'y' || addShows == 'Y') {
            std::cin.ignore(); // Clear input buffer
            addShowsForMovie(sql, movieId, title);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error adding movie: " << e.what() << "\n";
    }

    std::cout << "\nPress Enter to continue...";
    std::cin.ignore();
    std::cin.get();
}

void addShowsForMovie(soci::session& sql, int movieId, const std::string& movieTitle) {
    std::cout << "\n=== Add Shows for Movie: " << movieTitle << " ===\n";

    bool addMoreShows = true;
    while (addMoreShows) {
        // Display all cities and cinemas
        std::cout << "\n--- Available Cities and Cinemas ---\n";
        std::cout << std::left << std::setw(8) << "City ID"
            << std::setw(20) << "City Name"
            << std::setw(12) << "Cinema ID"
            << std::setw(25) << "Cinema Name"
            << std::setw(10) << "Hall ID"
            << std::setw(15) << "Hall Name" << std::endl;
        std::cout << std::string(90, '-') << std::endl;

        soci::rowset<soci::row> locationData = (sql.prepare <<
            "SELECT ci.id, ci.name, c.id, c.name, h.id, h.name "
            "FROM \"City\" ci "
            "JOIN \"Cinema\" c ON ci.id = c.\"cityId\" "
            "JOIN \"Hall\" h ON c.id = h.\"cinemaId\" "
            "ORDER BY ci.name, c.name, h.name");

        std::vector<std::tuple<int, std::string, int, std::string, int, std::string>> locations;

        for (const auto& row : locationData) {
            int cityId = row.get<int>(0);
            std::string cityName = row.get<std::string>(1);
            int cinemaId = row.get<int>(2);
            std::string cinemaName = row.get<std::string>(3);
            int hallId = row.get<int>(4);
            std::string hallName = row.get<std::string>(5);

            locations.emplace_back(cityId, cityName, cinemaId, cinemaName, hallId, hallName);

            std::cout << std::left << std::setw(8) << cityId
                << std::setw(20) << cityName
                << std::setw(12) << cinemaId
                << std::setw(25) << cinemaName
                << std::setw(10) << hallId
                << std::setw(15) << hallName << std::endl;
        }

        if (locations.empty()) {
            std::cout << "No halls available. Please add cities, cinemas, and halls first.\n";
            return;
        }

        std::cout << "\nEnter Hall ID for the show: ";
        int selectedHallId;
        std::cin >> selectedHallId;

        bool hallFound = false;
        for (const auto& location : locations) {
            if (std::get<4>(location) == selectedHallId) {
                std::cout << "\nSelected Location:\n";
                std::cout << "City: " << std::get<1>(location) << "\n";
                std::cout << "Cinema: " << std::get<3>(location) << "\n";
                std::cout << "Hall: " << std::get<5>(location) << "\n";
                hallFound = true;
                break;
            }
        }

        if (!hallFound) {
            std::cout << "Invalid Hall ID. Please try again.\n";
            continue;
        }

        std::cout << "\nConfirm this location? (y/n): ";
        char confirmLocation;
        std::cin >> confirmLocation;

        if (confirmLocation != 'y' && confirmLocation != 'Y') {
            std::cout << "Location selection cancelled.\n";
            continue;
        }

        std::cin.ignore(); 
        std::string startTime, endTime;
        std::cout << "\nEnter start time (YYYY-MM-DD HH:MM:SS): ";
        std::getline(std::cin, startTime);
        std::cout << "Enter end time (YYYY-MM-DD HH:MM:SS): ";
        std::getline(std::cin, endTime);

        std::cout << "\n=== Show Details Confirmation ===\n";
        std::cout << "Movie: " << movieTitle << " (ID: " << movieId << ")\n";
        std::cout << "Hall ID: " << selectedHallId << "\n";
        std::cout << "Start Time: " << startTime << "\n";
        std::cout << "End Time: " << endTime << "\n";
        std::cout << "\nConfirm adding this show? (y/n): ";

        char confirmShow;
        std::cin >> confirmShow;

        if (confirmShow == 'y' || confirmShow == 'Y') {
            try {
                int showId;
                sql << "INSERT INTO \"Show\" (\"movieId\", \"hallId\", \"startTime\", \"endTime\") "
                    "VALUES (:mid, :hid, :start, :end) RETURNING id",
                    soci::use(movieId), soci::use(selectedHallId),
                    soci::use(startTime), soci::use(endTime),
                    soci::into(showId);

                std::cout << "\nShow added successfully! Show ID: " << showId << "\n";
            }
            catch (const std::exception& e) {
                std::cerr << "Error adding show: " << e.what() << "\n";
            }
        }
        else {
            std::cout << "Show creation cancelled.\n";
        }

        std::cout << "\nAdd another show for this movie? (y/n): ";
        char addAnother;
        std::cin >> addAnother;
        addMoreShows = (addAnother == 'y' || addAnother == 'Y');
    }
}

void addShow(soci::session& sql, bool isAdmin) {
    if (!isAdmin) {
        std::cout << "Access denied. Admin privileges required.\n";
        return;
    }

    std::cout << "\n=== Add New Show ===\n";

    std::cout << "\n--- Available Movies ---\n";
    std::cout << std::left << std::setw(5) << "ID"
        << std::setw(35) << "Title"
        << std::setw(12) << "Genre"
        << std::setw(12) << "Language"
        << std::setw(15) << "Release Date" << std::endl;
    std::cout << std::string(79, '-') << std::endl;

    soci::rowset<soci::row> movieData = (sql.prepare <<
        "SELECT id, title, genre, language, \"releaseDate\" FROM \"Movie\" ORDER BY title");

    std::vector<std::tuple<int, std::string, std::string, std::string, std::string>> movies;

    for (const auto& row : movieData) {
        int movieId = row.get<int>(0);
        std::string title = row.get<std::string>(1);
        std::string genre = row.get<std::string>(2);
        std::string language = row.get<std::string>(3);
        std::string releaseDate = row.get<std::string>(4);

        movies.emplace_back(movieId, title, genre, language, releaseDate);

        std::string displayTitle = title.length() > 33 ? title.substr(0, 33) + ".." : title;

        std::cout << std::left << std::setw(5) << movieId
            << std::setw(35) << displayTitle
            << std::setw(12) << genre
            << std::setw(12) << language
            << std::setw(15) << releaseDate << std::endl;
    }

    if (movies.empty()) {
        std::cout << "No movies available. Please add movies first.\n";
        return;
    }

    std::cout << "\nEnter Movie ID: ";
    int selectedMovieId;
    std::cin >> selectedMovieId;

    bool movieFound = false;
    std::string selectedMovieTitle;
    for (const auto& movie : movies) {
        if (std::get<0>(movie) == selectedMovieId) {
            selectedMovieTitle = std::get<1>(movie);
            std::cout << "\nSelected Movie:\n";
            std::cout << "Title: " << std::get<1>(movie) << "\n";
            std::cout << "Genre: " << std::get<2>(movie) << "\n";
            std::cout << "Language: " << std::get<3>(movie) << "\n";
            std::cout << "Release Date: " << std::get<4>(movie) << "\n";
            movieFound = true;
            break;
        }
    }

    if (!movieFound) {
        std::cout << "Invalid Movie ID.\n";
        return;
    }

    std::cout << "\nConfirm this movie? (y/n): ";
    char confirmMovie;
    std::cin >> confirmMovie;

    if (confirmMovie != 'y' && confirmMovie != 'Y') {
        std::cout << "Movie selection cancelled.\n";
        return;
    }

    std::cout << "\n--- Available Cities, Cinemas, and Halls ---\n";
    std::cout << std::left << std::setw(8) << "City ID"
        << std::setw(20) << "City Name"
        << std::setw(12) << "Cinema ID"
        << std::setw(25) << "Cinema Name"
        << std::setw(10) << "Hall ID"
        << std::setw(15) << "Hall Name" << std::endl;
    std::cout << std::string(90, '-') << std::endl;

    soci::rowset<soci::row> locationData = (sql.prepare <<
        "SELECT ci.id, ci.name, c.id, c.name, h.id, h.name "
        "FROM \"City\" ci "
        "JOIN \"Cinema\" c ON ci.id = c.\"cityId\" "
        "JOIN \"Hall\" h ON c.id = h.\"cinemaId\" "
        "ORDER BY ci.name, c.name, h.name");

    std::vector<std::tuple<int, std::string, int, std::string, int, std::string>> locations;

    for (const auto& row : locationData) {
        int cityId = row.get<int>(0);
        std::string cityName = row.get<std::string>(1);
        int cinemaId = row.get<int>(2);
        std::string cinemaName = row.get<std::string>(3);
        int hallId = row.get<int>(4);
        std::string hallName = row.get<std::string>(5);

        locations.emplace_back(cityId, cityName, cinemaId, cinemaName, hallId, hallName);

        std::cout << std::left << std::setw(8) << cityId
            << std::setw(20) << cityName
            << std::setw(12) << cinemaId
            << std::setw(25) << cinemaName
            << std::setw(10) << hallId
            << std::setw(15) << hallName << std::endl;
    }

    if (locations.empty()) {
        std::cout << "No halls available. Please add cities, cinemas, and halls first.\n";
        return;
    }

    std::cout << "\nEnter Hall ID for the show: ";
    int selectedHallId;
    std::cin >> selectedHallId;

    bool hallFound = false;
    for (const auto& location : locations) {
        if (std::get<4>(location) == selectedHallId) {
            std::cout << "\nSelected Location:\n";
            std::cout << "City: " << std::get<1>(location) << "\n";
            std::cout << "Cinema: " << std::get<3>(location) << "\n";
            std::cout << "Hall: " << std::get<5>(location) << "\n";
            hallFound = true;
            break;
        }
    }

    if (!hallFound) {
        std::cout << "Invalid Hall ID.\n";
        return;
    }

    std::cout << "\nConfirm this location? (y/n): ";
    char confirmLocation;
    std::cin >> confirmLocation;

    if (confirmLocation != 'y' && confirmLocation != 'Y') {
        std::cout << "Location selection cancelled.\n";
        return;
    }

    std::cin.ignore();
    std::string startTime, endTime;
    std::cout << "\nEnter start time (YYYY-MM-DD HH:MM:SS): ";
    std::getline(std::cin, startTime);
    std::cout << "Enter end time (YYYY-MM-DD HH:MM:SS): ";
    std::getline(std::cin, endTime);

    std::cout << "\n=== Final Show Details Confirmation ===\n";
    std::cout << "Movie: " << selectedMovieTitle << " (ID: " << selectedMovieId << ")\n";
    std::cout << "Hall ID: " << selectedHallId << "\n";
    std::cout << "Start Time: " << startTime << "\n";
    std::cout << "End Time: " << endTime << "\n";
    std::cout << "\nConfirm adding this show? (y/n): ";

    char finalConfirm;
    std::cin >> finalConfirm;

    if (finalConfirm == 'y' || finalConfirm == 'Y') {
        try {
            int showId;
            sql << "INSERT INTO \"Show\" (\"movieId\", \"hallId\", \"startTime\", \"endTime\") "
                "VALUES (:mid, :hid, :start, :end) RETURNING id",
                soci::use(selectedMovieId), soci::use(selectedHallId),
                soci::use(startTime), soci::use(endTime),
                soci::into(showId);

            std::cout << "\nShow added successfully! Show ID: " << showId << "\n";

            std::cout << "\nAdd another show? (y/n): ";
            char addAnother;
            std::cin >> addAnother;

            if (addAnother == 'y' || addAnother == 'Y') {
                addShow(sql, true);
                return;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error adding show: " << e.what() << "\n";
        }
    }
    else {
        std::cout << "Show creation cancelled.\n";
    }

    std::cout << "\nPress Enter to continue...";
    std::cin.ignore();
    std::cin.get();
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
                std::string title = "N/A";
                std::string genre = "N/A";
                long long bookings = 0; 

                if (row.get_indicator(0) != soci::i_null) {
                    movieId = row.get<int>(0);
                }

                if (row.get_indicator(1) != soci::i_null) {
                    title = row.get<std::string>(1);
                    if (title.length() > 28) title = title.substr(0, 28) + "..";
                }

                if (row.get_indicator(2) != soci::i_null) {
                    genre = row.get<std::string>(2);
                    if (genre.length() > 10) genre = genre.substr(0, 10) + "..";
                }

                if (row.get_indicator(3) != soci::i_null) {
                    bookings = row.get<long long>(3);
                }

                std::cout << std::left << std::setw(5) << movieId
                    << std::setw(30) << title
                    << std::setw(12) << genre
                    << std::setw(10) << bookings << "\n";
            }
            catch (const std::exception& rowError) {
                std::cerr << "Error processing movie row: " << rowError.what() << "\n";
                // Try to get raw data for debugging
                std::cerr << "Debug: Row has " << row.size() << " columns\n";
                for (size_t i = 0; i < row.size(); ++i) {
                    std::cerr << "Column " << i << " indicator: " << row.get_indicator(i) << "\n";
                }
            }
        }

        std::cout << "\n--- Seat Types and Their Booking Counts ---\n";
        std::cout << std::left << std::setw(15) << "Seat Type" << std::setw(10) << "Bookings\n";
        std::cout << std::string(25, '-') << "\n";

        soci::rowset<soci::row> seatStats = (sql.prepare <<
            "SELECT s.type::text AS seat_type, "
            "       COALESCE(COUNT(b.id), 0) AS booking_count "
            "FROM \"Seat\" s "
            "LEFT JOIN \"Booking\" b ON s.id = b.\"seatId\" AND b.status = 'CONFIRMED' "
            "GROUP BY s.type "
            "ORDER BY booking_count DESC");

        for (const auto& row : seatStats) {
            try {
                std::string seatType = "N/A";
                long long bookings = 0;

                if (row.get_indicator(0) != soci::i_null) {
                    seatType = row.get<std::string>(0);
                }

                if (row.get_indicator(1) != soci::i_null) {
                    bookings = row.get<long long>(1);
                }

                std::cout << std::left << std::setw(15) << seatType
                    << std::setw(10) << bookings << "\n";
            }
            catch (const std::exception& rowError) {
                std::cerr << "Error processing seat row: " << rowError.what() << "\n";
                std::cerr << "Debug: Row has " << row.size() << " columns\n";
                for (size_t i = 0; i < row.size(); ++i) {
                    std::cerr << "Column " << i << " indicator: " << row.get_indicator(i) << "\n";
                }
            }
        }

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
                int cinemaId = 0;
                std::string cinemaName = "N/A";
                std::string cityName = "N/A";
                long long bookings = 0;

                if (row.get_indicator(0) != soci::i_null) {
                    cinemaId = row.get<int>(0);
                }

                if (row.get_indicator(1) != soci::i_null) {
                    cinemaName = row.get<std::string>(1);
                    if (cinemaName.length() > 18) cinemaName = cinemaName.substr(0, 18) + "..";
                }

                if (row.get_indicator(2) != soci::i_null) {
                    cityName = row.get<std::string>(2);
                    if (cityName.length() > 13) cityName = cityName.substr(0, 13) + "..";
                }

                if (row.get_indicator(3) != soci::i_null) {
                    bookings = row.get<long long>(3);
                }

                std::cout << std::left << std::setw(5) << cinemaId
                    << std::setw(20) << cinemaName
                    << std::setw(15) << cityName
                    << std::setw(10) << bookings << "\n";
            }
            catch (const std::exception& rowError) {
                std::cerr << "Error processing cinema row: " << rowError.what() << "\n";
                std::cerr << "Debug: Row has " << row.size() << " columns\n";
                for (size_t i = 0; i < row.size(); ++i) {
                    std::cerr << "Column " << i << " indicator: " << row.get_indicator(i) << "\n";
                }
            }
        }

    }
    catch (const std::exception& e) {
        std::cerr << "Error fetching statistics: " << e.what() << "\n";
    }

    std::cout << "\nPress Enter to return...";
    std::cin.ignore(99999, '\n');
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


