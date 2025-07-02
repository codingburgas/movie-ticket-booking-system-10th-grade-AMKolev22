#pragma once
#include <soci/soci.h>
#include <soci/rowset.h>
#include <iomanip>
#include <limits>
#include <iostream>
#include <../../vendor/nlohmann/json.hpp>
#include <fstream>
using json = nlohmann::json;
void payForBooking(soci::session& sql, int bookingId, int userId, double amount);
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
        "m.title, s.\"startTime\", seat.row || seat.number "
        "FROM \"Booking\" b "
        "JOIN \"Show\" s ON b.\"showId\" = s.id "
        "JOIN \"Movie\" m ON s.\"movieId\" = m.id "
        "JOIN \"Seat\" seat ON b.\"seatId\" = seat.id "
        "WHERE b.\"userId\" = :userId "
        "ORDER BY b.\"createdAt\" DESC",
        soci::use(userId)
        );

    std::cout << std::left << std::setw(10) << "Book ID"
        << std::setw(25) << "Movie"
        << std::setw(20) << "Show Time"
        << std::setw(8) << "Seat"
        << std::setw(12) << "Status"
        << std::setw(20) << "Booked On" << std::endl;
    std::cout << std::string(95, '-') << std::endl;

    for (const auto& row : rs) {
        std::cout << std::left << std::setw(10) << row.get<int>(0)
            << std::setw(25) << row.get<std::string>(3)
            << std::setw(20) << row.get<std::string>(4)
            << std::setw(8) << row.get<std::string>(5)
            << std::setw(12) << row.get<std::string>(1)
            << std::setw(20) << row.get<std::string>(2) << std::endl;
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
    int showId, seatId;

    std::cout << "\n=== Book Tickets ===\n";
    std::cout << "Enter Show ID: ";
    std::cin >> showId;

    std::string seatQuery =
        "SELECT s.id, s.row, s.number, s.type "
        "FROM \"Seat\" s "
        "JOIN \"Hall\" h ON s.\"hallId\" = h.id "
        "JOIN \"Show\" sh ON sh.\"hallId\" = h.id "
        "WHERE sh.id = :showId "
        "AND s.id NOT IN ("
        "    SELECT \"seatId\" FROM \"Booking\" "
        "    WHERE \"showId\" = :showId AND status = 'CONFIRMED'"
        ") "
        "ORDER BY s.row, s.number";

    soci::rowset<soci::row> rs = (sql.prepare << seatQuery, soci::use(showId));

    std::vector<std::tuple<int, std::string, int, std::string>> seats;

    for (const auto& r : rs) {
        seats.emplace_back(
            r.get<int>(0),               // id
            r.get<std::string>(1),       // row
            r.get<int>(2),               // number
            r.get<std::string>(3)        // type
        );
    }

    std::cout << "\nAvailable Seats:\n";
    std::cout << std::left << std::setw(8) << "Seat ID"
        << std::setw(5) << "Row"
        << std::setw(8) << "Number"
        << std::setw(10) << "Type" << std::endl;
    std::cout << std::string(31, '-') << std::endl;

    for (const auto& seat : seats) {
        std::cout << std::left << std::setw(8) << std::get<0>(seat)
            << std::setw(5) << std::get<1>(seat)
            << std::setw(8) << std::get<2>(seat)
            << std::setw(10) << std::get<3>(seat) << std::endl;
    }

    std::cout << "Enter Seat ID to book: ";
    std::cin >> seatId;
    std::string seatType;

    bool found = false;
    for (const auto& seat : seats) {
        if (std::get<0>(seat) == seatId) {
            seatType = std::get<3>(seat);
            found = true;
            break;
        }
    }



    int bookingId = 0;
    sql << "INSERT INTO \"Booking\" (\"userId\", \"showId\", \"seatId\", status, \"createdAt\") "
        "VALUES (:userId, :showId, :seatId, 'CONFIRMED', NOW()) RETURNING id",
        soci::use(userId), soci::use(showId), soci::use(seatId), soci::into(bookingId);

    // Insert Payment
    double amount;
    if (seatType == "PLATINUM")
        amount = 20;

    soci::use(bookingId), soci::use(userId), soci::use(amount);
    payForBooking(sql, bookingId, userId, amount);
    std::cout << "\nBooking successful! Booking ID: " << bookingId << std::endl;
    std::cout << "Amount paid: $" << std::fixed << std::setprecision(2) << amount << std::endl;
}


void payForBooking(soci::session& sql, int bookingId, int userId, double amount) {
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

    sql << "INSERT INTO \"Notification\" (type, message, \"userId\", \"createdAt\") "
        "VALUES ('BOOKING_MADE', 'Payment received for booking ID: " + std::to_string(bookingId) + "', :userId, NOW())",
        soci::use(userId);
}
