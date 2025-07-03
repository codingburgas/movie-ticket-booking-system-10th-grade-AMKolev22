#pragma once
#include <soci/soci.h>
#include <soci/rowset.h>
#include <iomanip>
#include <limits>
#include <iostream>
#include <../../vendor/nlohmann/json.hpp>
#include <sendEmail.hpp>

#include <fstream>
using json = nlohmann::json;
void payForBooking(soci::session& sql, int bookingId, int userId, double amount, std::string email, std::string chosenType, int showId);
void viewBookings(soci::session& sql) {
    int userId;
    std::cout << "\n=== Your Bookings ===\n";

    std::ifstream file("data/session.json");
    json data;
    file >> data;
    std::string email = data["email"];

    sql << "SELECT id FROM \"User\" WHERE email = :email", soci::use(email), soci::into(userId);

    soci::rowset<soci::row> rs = (
        sql.prepare <<
        "SELECT b.id, b.status::text, b.\"createdAt\", "
        "m.title, s.\"startTime\", seat.row || seat.number, c.name "
        "FROM \"Booking\" b "
        "JOIN \"Show\" s ON b.\"showId\" = s.id "
        "JOIN \"Movie\" m ON s.\"movieId\" = m.id "
        "JOIN \"Seat\" seat ON b.\"seatId\" = seat.id "
        "JOIN \"Hall\" h ON seat.\"hallId\" = h.id "
        "JOIN \"Cinema\" c ON h.\"cinemaId\" = c.id "
        "WHERE b.\"userId\" = :userId "
        "ORDER BY b.\"createdAt\" DESC",
        soci::use(userId)
        );

    std::cout << std::left << std::setw(10) << "Book ID"
        << std::setw(25) << "Movie"
        << std::setw(25) << "Show Time"
        << std::setw(8) << "Seat"
        << std::setw(12) << "Status"
        << std::setw(25) << "Booked On"
        << std::setw(20) << "Cinema" << std::endl;
    std::cout << std::string(125, '-') << std::endl;

    for (const auto& row : rs) {
        int bookingId = row.get<int>(0);
        std::string status = row.get<std::string>(1);
        std::tm createdAt = row.get<std::tm>(2);
        std::string movie = row.get<std::string>(3);
        std::tm showTime = row.get<std::tm>(4);
        std::string seat = row.get<std::string>(5);
        std::string cinema = row.get<std::string>(6);

        std::ostringstream showTimeStr;
        showTimeStr << std::put_time(&showTime, "%Y-%m-%d %H:%M");

        std::ostringstream createdAtStr;
        createdAtStr << std::put_time(&createdAt, "%Y-%m-%d %H:%M");

        std::cout << std::left << std::setw(10) << bookingId
            << std::setw(25) << movie
            << std::setw(25) << showTimeStr.str()
            << std::setw(8) << seat
            << std::setw(12) << status
            << std::setw(25) << createdAtStr.str()
            << std::setw(20) << cinema << std::endl;
    }

    std::cout << "\nPress Enter to return to exit..";
    std::cin.ignore();
    std::cin.get();
    abort();
}


void bookTickets(soci::session& sql) {
    int userId;
    std::ifstream file("data/session.json");
    json data;
    file >> data;
    std::string email = data["email"];
    sql << "SELECT id FROM \"User\" WHERE email = :email", soci::use(email), soci::into(userId);

    std::cout << "\n=== Book Tickets ===\n";
    std::cout << "How would you like to search?\n";
    std::cout << "1. By Movie Title\n";
    std::cout << "2. By Show ID\n";
    std::cout << "Enter your choice (1 or 2): ";

    int choice;
    std::cin >> choice;
    std::cin.ignore(); // Clear the input buffer

    int showId = 0;

    if (choice == 1) {
        std::cout << "Enter Movie Title (or part of it): ";
        std::string movieTitle;
        std::getline(std::cin, movieTitle);

        std::string movieQuery =
            "SELECT id, title, genre, language, \"releaseDate\" "
            "FROM \"Movie\" "
            "WHERE LOWER(title) LIKE LOWER(:title) "
            "ORDER BY title";

        std::string searchPattern = "%" + movieTitle + "%";
        soci::rowset<soci::row> movieResults = (sql.prepare << movieQuery, soci::use(searchPattern));

        std::vector<std::tuple<int, std::string, std::string, std::string, std::string>> movies;
        for (const auto& r : movieResults) {
            std::string releaseDate = "N/A";
            if (r.get_indicator(4) != soci::i_null) {
                try {
                    std::tm dateStruct = r.get<std::tm>(4);
                    std::ostringstream oss;
                    oss << std::put_time(&dateStruct, "%Y-%m-%d");
                    releaseDate = oss.str();
                }
                catch (...) {
                    try {
                        releaseDate = r.get<std::string>(4);
                    }
                    catch (...) {
                        releaseDate = "N/A";
                    }
                }
            }

            movies.emplace_back(
                r.get<int>(0),               // id
                r.get<std::string>(1),       // title
                r.get<std::string>(2),       // genre
                r.get<std::string>(3),       // language
                releaseDate                  // releaseDate
            );
        }

        if (movies.empty()) {
            std::cout << "No movies found matching '" << movieTitle << "'\n";
            return;
        }

        std::cout << "\nFound Movies:\n";
        std::cout << std::left << std::setw(5) << "ID"
            << std::setw(30) << "Title"
            << std::setw(15) << "Genre"
            << std::setw(12) << "Language"
            << std::setw(15) << "Release Date" << std::endl;
        std::cout << std::string(77, '-') << std::endl;

        for (const auto& movie : movies) {
            std::cout << std::left << std::setw(5) << std::get<0>(movie)
                << std::setw(30) << std::get<1>(movie)
                << std::setw(15) << std::get<2>(movie)
                << std::setw(12) << std::get<3>(movie)
                << std::setw(15) << std::get<4>(movie) << std::endl;
        }

        std::cout << "\nEnter Movie ID to confirm: ";
        int selectedMovieId;
        std::cin >> selectedMovieId;

        bool movieFound = false;
        for (const auto& movie : movies) {
            if (std::get<0>(movie) == selectedMovieId) {
                std::cout << "\nConfirmed Movie: " << std::get<1>(movie)
                    << " (Genre: " << std::get<2>(movie) << ")\n";
                movieFound = true;
                break;
            }
        }

        if (!movieFound) {
            std::cout << "Invalid Movie ID selected.\n";
            return;
        }

        std::string showQuery =
            "SELECT s.id, s.\"startTime\", s.\"endTime\", h.name, c.name, city.name "
            "FROM \"Show\" s "
            "JOIN \"Hall\" h ON s.\"hallId\" = h.id "
            "JOIN \"Cinema\" c ON h.\"cinemaId\" = c.id "
            "JOIN \"City\" city ON c.\"cityId\" = city.id "
            "WHERE s.\"movieId\" = :movieId "
            "ORDER BY s.\"startTime\"";

        soci::rowset<soci::row> showResults = (sql.prepare << showQuery, soci::use(selectedMovieId));

        std::vector<std::tuple<int, std::string, std::string, std::string, std::string, std::string>> shows;
        for (const auto& r : showResults) {
            shows.emplace_back(
                r.get<int>(0),               // show id
                r.get<std::string>(1),       // start time
                r.get<std::string>(2),       // end time
                r.get<std::string>(3),       // hall name
                r.get<std::string>(4),       // cinema name
                r.get<std::string>(5)        // city name
            );
        }

        if (shows.empty()) {
            std::cout << "No shows found for this movie.\n";
            return;
        }

        // Display shows
        std::cout << "\nAvailable Shows:\n";
        std::cout << std::left << std::setw(8) << "Show ID"
            << std::setw(20) << "Start Time"
            << std::setw(20) << "End Time"
            << std::setw(15) << "Hall"
            << std::setw(20) << "Cinema"
            << std::setw(15) << "City" << std::endl;
        std::cout << std::string(98, '-') << std::endl;

        for (const auto& show : shows) {
            std::cout << std::left << std::setw(8) << std::get<0>(show)
                << std::setw(20) << std::get<1>(show)
                << std::setw(20) << std::get<2>(show)
                << std::setw(15) << std::get<3>(show)
                << std::setw(20) << std::get<4>(show)
                << std::setw(15) << std::get<5>(show) << std::endl;
        }

        std::cout << "\nEnter Show ID to confirm: ";
        std::cin >> showId;

        bool showFound = false;
        for (const auto& show : shows) {
            if (std::get<0>(show) == showId) {
                std::cout << "\nConfirmed Show: " << std::get<4>(show)
                    << " - " << std::get<3>(show)
                    << " at " << std::get<1>(show) << "\n";
                showFound = true;
                break;
            }
        }

        if (!showFound) {
            std::cout << "Invalid Show ID selected.\n";
            return;
        }

    }
    else if (choice == 2) {
        std::cout << "\n=== Search Options ===\n";
        std::cout << "1. Search by Movie Title\n";
        std::cout << "2. Enter Show ID directly\n";
        std::cout << "Enter your choice (1 or 2): ";

        int subChoice;
        std::cin >> subChoice;
        std::cin.ignore(); 

        if (subChoice == 1) {
            std::cout << "Enter Movie Title (or part of it): ";
            std::string movieTitle;
            std::getline(std::cin, movieTitle);

            std::string movieQuery =
                "SELECT id, title, genre, language, \"releaseDate\" "
                "FROM \"Movie\" "
                "WHERE LOWER(title) LIKE LOWER(:title) "
                "ORDER BY title";

            std::string searchPattern = "%" + movieTitle + "%";
            soci::rowset<soci::row> movieResults = (sql.prepare << movieQuery, soci::use(searchPattern));

            std::vector<std::tuple<int, std::string, std::string, std::string, std::string>> movies;
            for (const auto& r : movieResults) {
                std::string releaseDate = "N/A";
                if (r.get_indicator(4) != soci::i_null) {
                    try {
                        std::tm dateStruct = r.get<std::tm>(4);
                        std::ostringstream oss;
                        oss << std::put_time(&dateStruct, "%Y-%m-%d");
                        releaseDate = oss.str();
                    }
                    catch (...) {
                        try {
                            releaseDate = r.get<std::string>(4);
                        }
                        catch (...) {
                            releaseDate = "N/A";
                        }
                    }
                }

                movies.emplace_back(
                    r.get<int>(0),               // id
                    r.get<std::string>(1),       // title
                    r.get<std::string>(2),       // genre
                    r.get<std::string>(3),       // language
                    releaseDate                  // releaseDate
                );
            }

            if (movies.empty()) {
                std::cout << "No movies found matching '" << movieTitle << "'\n";
                return;
            }

            std::cout << "\nFound Movies:\n";
            std::cout << std::left << std::setw(5) << "ID"
                << std::setw(30) << "Title"
                << std::setw(15) << "Genre"
                << std::setw(12) << "Language"
                << std::setw(15) << "Release Date" << std::endl;
            std::cout << std::string(77, '-') << std::endl;

            for (const auto& movie : movies) {
                std::cout << std::left << std::setw(5) << std::get<0>(movie)
                    << std::setw(30) << std::get<1>(movie)
                    << std::setw(15) << std::get<2>(movie)
                    << std::setw(12) << std::get<3>(movie)
                    << std::setw(15) << std::get<4>(movie) << std::endl;
            }

            std::cout << "\nEnter Movie ID to see shows: ";
            int selectedMovieId;
            std::cin >> selectedMovieId;

            bool movieFound = false;
            for (const auto& movie : movies) {
                if (std::get<0>(movie) == selectedMovieId) {
                    std::cout << "\nShows for Movie: " << std::get<1>(movie)
                        << " (Genre: " << std::get<2>(movie) << ")\n";
                    movieFound = true;
                    break;
                }
            }

            if (!movieFound) {
                std::cout << "Invalid Movie ID selected.\n";
                return;
            }

            std::string showQuery =
                "SELECT s.id, s.\"startTime\", s.\"endTime\", h.name, c.name, city.name "
                "FROM \"Show\" s "
                "JOIN \"Hall\" h ON s.\"hallId\" = h.id "
                "JOIN \"Cinema\" c ON h.\"cinemaId\" = c.id "
                "JOIN \"City\" city ON c.\"cityId\" = city.id "
                "WHERE s.\"movieId\" = :movieId "
                "ORDER BY s.\"startTime\"";

            soci::rowset<soci::row> showResults = (sql.prepare << showQuery, soci::use(selectedMovieId));

            std::vector<std::tuple<int, std::string, std::string, std::string, std::string, std::string>> shows;

            for (const auto& r : showResults) {
                std::tm startTm = r.get<std::tm>(1);  // Get startTime as std::tm
                std::tm endTm = r.get<std::tm>(2);    // Get endTime as std::tm

                std::stringstream ssStart, ssEnd;
                ssStart << std::put_time(&startTm, "%Y-%m-%d %H:%M:%S");
                ssEnd << std::put_time(&endTm, "%Y-%m-%d %H:%M:%S");

                shows.emplace_back(
                    r.get<int>(0),               // show id
                    ssStart.str(),              // formatted start time
                    ssEnd.str(),                // formatted end time
                    r.get<std::string>(3),      // hall name
                    r.get<std::string>(4),      // cinema name
                    r.get<std::string>(5)       // city name
                );
            }

            if (shows.empty()) {
                std::cout << "No shows found for this movie.\n";
            }

            // Display shows
            std::cout << "\nAvailable Shows:\n";
            std::cout << std::left << std::setw(8) << "Show ID"
                << std::setw(20) << "Start Time"
                << std::setw(20) << "End Time"
                << std::setw(15) << "Hall"
                << std::setw(20) << "Cinema"
                << std::setw(15) << "City" << std::endl;
            std::cout << std::string(98, '-') << std::endl;

            for (const auto& show : shows) {
                std::cout << std::left << std::setw(8) << std::get<0>(show)
                    << std::setw(20) << std::get<1>(show)
                    << std::setw(20) << std::get<2>(show)
                    << std::setw(15) << std::get<3>(show)
                    << std::setw(20) << std::get<4>(show)
                    << std::setw(15) << std::get<5>(show) << std::endl;
            }
            std::cout << "\nEnter Show ID to confirm: ";
            std::cin >> showId;

            // confirm show selection
            bool showFound = false;
            for (const auto& show : shows) {
                if (std::get<0>(show) == showId) {
                    std::cout << "\nConfirmed Show: " << std::get<4>(show)
                        << " - " << std::get<3>(show)
                        << " at " << std::get<1>(show) << "\n";
                    showFound = true;
                    break;
                }
            }

            if (!showFound) {
                std::cout << "Invalid Show ID selected.\n";
                return;
            }

        }
        else if (subChoice == 2) {
            std::cout << "Enter Show ID: ";
            std::cin >> showId;

            // verify show exists and display details
            std::string showVerifyQuery =
                "SELECT s.id, m.title, m.genre, s.\"startTime\", h.name, c.name "
                "FROM \"Show\" s "
                "JOIN \"Movie\" m ON s.\"movieId\" = m.id "
                "JOIN \"Hall\" h ON s.\"hallId\" = h.id "
                "JOIN \"Cinema\" c ON h.\"cinemaId\" = c.id "
                "WHERE s.id = :showId";

            soci::rowset<soci::row> showVerify = (sql.prepare << showVerifyQuery, soci::use(showId));

            bool showExists = false;
            for (const auto& r : showVerify) {
                std::cout << "\nShow Details:\n";
                std::cout << "Movie: " << r.get<std::string>(1) << " (Genre: " << r.get<std::string>(2) << ")\n";
                std::cout << "Cinema: " << r.get<std::string>(5) << " - " << r.get<std::string>(4) << "\n";
                std::cout << "Start Time: " << r.get<std::string>(3) << "\n";
                showExists = true;
                break;
            }

            if (!showExists) {
                std::cout << "Show ID " << showId << " not found.\n";
                return;
            }

            std::cout << "\nProceed with this show? (y/n): ";
            char confirm;
            std::cin >> confirm;
            if (confirm != 'y' && confirm != 'Y') {
                std::cout << "Booking cancelled.\n";
                return;
            }

        }
        else {
            std::cout << "Invalid choice.\n";
            return;
        }

    }
    else {
        std::cout << "Invalid choice.\n";
        return;
    }


            std::string seatQuery = R"(
          SELECT
            s.id,
            s.row,
            s.number,
            s.type::text AS seat_type
          FROM "Seat" s
          JOIN "Hall" h  ON s."hallId" = h.id
          JOIN "Show" sh ON sh."hallId" = h.id
          WHERE sh.id = :showId
            AND s.id NOT IN (
              SELECT "seatId"
              FROM "Booking"
              WHERE "showId" = :showId
                AND status = 'CONFIRMED'
            )
          ORDER BY s.row, s.number
        )";

    soci::rowset<soci::row> rs =
        (sql.prepare << seatQuery,
            soci::use(showId, "showId"),
            soci::use(showId, "showId"));

    std::vector<std::tuple<long long, std::string, int, std::string>> seats;
    for (auto const& r : rs) {
        int id = r.get<int>(0);
        std::string row = r.get<std::string>(1);
        int num = r.get<int>(2);
        std::string type = r.get<std::string>(3);
        seats.emplace_back(id, row, num, type);
    }

    if (seats.empty())
    {
        std::cout << "No available seats for this show.\n";
    }

    std::cout << "\nAvailable Seats:\n"
        << std::left
        << std::setw(8) << "Seat ID"
        << std::setw(5) << "Row"
        << std::setw(8) << "Number"
        << std::setw(10) << "Type" << "\n"
        << std::string(31, '-') << "\n";
    for (auto const& s : seats)
    {
        std::cout << std::left
            << std::setw(8) << std::get<0>(s)
            << std::setw(5) << std::get<1>(s)
            << std::setw(8) << std::get<2>(s)
            << std::setw(10) << std::get<3>(s)
            << "\n";
    }

    std::cout << "\nEnter Seat ID to book: ";
    long long chosenSeatId;
    std::cin >> chosenSeatId;

    std::string chosenType;
    bool        found = false;
    for (auto const& s : seats)
    {
        if (std::get<0>(s) == chosenSeatId)
        {
            chosenType = std::get<3>(s);
            found = true;
            std::cout << "\nSelected Seat: Row " << std::get<1>(s)
                << ", Number " << std::get<2>(s)
                << " (" << chosenType << ")\n";
            break;
        }
    }

    if (!found)
    {
        std::cout << "Invalid Seat ID or seat not available.\n";
    }

    double amount = (chosenType == "PLATINUM" ? 20.0
        : chosenType == "GOLD" ? 15.0
        : 10.0);
    std::cout << "Amount to pay: $"
        << std::fixed << std::setprecision(2)
        << amount << "\n"
        << "Confirm booking? (y/n): ";

    char confirm;
    std::cin >> confirm;
    if (confirm != 'y' && confirm != 'Y')
    {
        std::cout << "Booking cancelled.\n";
    }

    try
    {
        int bookingId;
        sql << R"(
        INSERT INTO "Booking" ("userId","showId","seatId",status,"createdAt")
        VALUES (:userId,:showId,:seatId,'CONFIRMED',NOW())
        RETURNING id
      )",
            soci::use(userId),
            soci::use(showId),
            soci::use(chosenSeatId),
            soci::into(bookingId);

        payForBooking(sql, bookingId, userId, amount, email, chosenType, showId);

        std::cout << "\n=== Booking Successful! ===\n"
            << "Booking ID:  " << bookingId << "\n"
            << "Amount paid: $" << amount << "\n"
            << "Seat: Row " << std::get<1>(seats[0])
            << ", Number " << std::get<2>(seats[0]) << "\n"
            << "Type: " << chosenType << "\n";
    }
    catch (std::exception const& e)
    {
        std::cerr << "Error during booking: " << e.what() << "\n";
    }

}


void payForBooking(soci::session& sql, int bookingId, int userId, double amount, std::string email, std::string chosenType, int showId) {
    int methodChoice;
    std::string methodStr;

    std::cout << "\n=== Payment Menu ===\n";
    std::cout << "Booking ID: " << bookingId << "\n";
    std::cout << "Amount due: " << std::fixed << std::setprecision(2) << amount << "\n";
    std::cout << "Choose payment method:\n";
    std::cout << "1. Credit Card\n";
    std::cout << "2. Cash\n";
    std::cout << "Enter choice: ";
    std::cin >> methodChoice;

    if (methodChoice == 1) {
        methodStr = "CREDIT_CARD";
    }
    else if (methodChoice == 2) {
        methodStr = "CASH";
    }
    else {
        std::cout << "Invalid choice. Payment canceled.\n";
        return;
    }

    sql << "INSERT INTO \"Payment\" (\"bookingId\", \"userId\", method, amount, \"paidAt\") "
        "VALUES (:bookingId, :userId, :method, :amount, NOW())",
        soci::use(bookingId), soci::use(userId), soci::use(methodStr), soci::use(amount);

    std::cout << "Payment successful via " << methodStr << "!\n";


    SMTP::sendReceipt(email, bookingId, chosenType);


    sql << "INSERT INTO \"Notification\" (type, message, \"userId\", \"createdAt\") "
        "VALUES ('BOOKING_MADE', 'Payment received for booking ID: " + std::to_string(bookingId) + "', :userId, NOW())",
        soci::use(userId);
}
