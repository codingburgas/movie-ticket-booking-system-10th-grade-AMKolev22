#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <unordered_map>
#include "../config.h"

class Seat {
private:
    int row;
    int number;
    SeatType type;
    bool reserved;

public:
    Seat(int row, int number, SeatType type)
        : row(row), number(number), type(type), reserved(false) {
    }

    int getRow() const { return row; }
    int getNumber() const { return number; }
    SeatType getType() const { return type; }
    bool isReserved() const { return reserved; }
    void reserve() { reserved = true; }
    void release() { reserved = false; }

    std::string getSeatId() const {
        return std::to_string(row) + "-" + std::to_string(number);
    }
};