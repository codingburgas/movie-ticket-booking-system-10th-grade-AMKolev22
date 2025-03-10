#include "cinema.h"
#include <exception>
#include <iostream>
void BookingSystem::Cinema::addRecord(BookingSystem::Movie movie) {

	try {
		auto displayCurrent = [&](std::vector<BookingSystem::Movie> moviesList) {
			for (const auto& movie : moviesList) {
				movie.getInfo();
			}
		};
		this->movies.get()->push_back(movie);
		displayCurrent(*(this->movies.get()));
	}
	catch (std::exception e) {
		std::cerr << e.what();
	}

}