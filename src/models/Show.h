#pragma once
#include "Movie.h"
#include "Seat.h"
class Show {
private:
    Movie movie;
    std::string datetime;
    std::vector<Seat> seats;

public:
    explicit Show(Movie movie, std::string datetime, int rows)
        : movie(movie), datetime(datetime) {
        initializeSeats(rows);
    }

    void initializeSeats(int rows);

    const Movie& getMovie() const { return movie; }
    std::string getDatetime() const { return datetime; }

    bool reserveSeat(int row, int number);
    std::vector<Seat> getAvailableSeats() const;
};