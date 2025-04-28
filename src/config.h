#pragma once
#include "pch.h"
#include "states.h"

std::unordered_map<SeatType, double> seatPrices = {
    {SeatType::SILVER, 5.0},
    {SeatType::GOLD, 8.0},
    {SeatType::PLATINUM, 12.0}
};