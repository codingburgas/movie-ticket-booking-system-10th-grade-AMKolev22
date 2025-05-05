#include "Show.h"


void Show::initializeSeats(int rows) {
    for (int row = 1; row <= rows; row++) {
        SeatType type;
        if (row <= 10) type = SeatType::SILVER;
        else if (row <= 20) type = SeatType::GOLD;
        else type = SeatType::PLATINUM;

        for (int num = 1; num <= 10; num++)
            this->seats.emplace_back(row, num, type);
    }
}

bool Show::reserveSeat(int row, int number) {
    for (auto& seat : seats) {
        if (seat.getRow() == row && seat.getNumber() == number) {
            if (!seat.isReserved()) {
                seat.reserve();
                return true;
            }
            return false;
        }
    }
    return false;
}

std::vector<Seat> Show::getAvailableSeats() const {
    std::vector<Seat> availableSeats;
    for (const auto& seat : seats) {
        if (!seat.isReserved())
            availableSeats.push_back(seat);
    }
    return availableSeats;
}

