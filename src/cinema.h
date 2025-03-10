#pragma once
#include "movie.h"
#include <vector>
#include <functional>
#include <memory>
namespace BookingSystem {
	class Cinema {

		public:
			Cinema(std::string cinemaName) : m_cinemaName(cinemaName) {};
			void addRecord(BookingSystem::Movie movie);

		private:
			std::shared_ptr<std::vector<BookingSystem::Movie>> movies;
			std::string m_cinemaName;
	};
}