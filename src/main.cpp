
#include <iostream>
#include "cinema.h"

namespace BS = BookingSystem;
int main(){
	auto test = std::make_shared<BS::Movie>("test-title", "test-duration");
	
	auto val = test->getInfo();
	for (const auto& it : val) {
		std::cout << it.second << std::endl;
	};
}	
