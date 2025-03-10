#pragma once
#include <string>
#include <memory>
#include <unordered_map>

namespace BookingSystem {
	class Movie {
	public:
		Movie(std::string duration, std::string title) : duration(duration), title(title) {};
		std::unordered_map<std::string, std::string> getInfo() const;

	private:
		std::string title;
		std::string duration;
	};
}