#include "movie.h"
#include <exception>
#include <iostream>
std::unordered_map<std::string, std::string> BookingSystem::Movie::getInfo() const {

	try {
		std::unordered_map<std::string, std::string> returnValue = {
			{"title", this->title},
			{"duration", this->duration}
		};
		return returnValue;
	}
	
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		return {};
	}

}	