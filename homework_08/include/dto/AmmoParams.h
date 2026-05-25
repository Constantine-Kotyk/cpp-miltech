#pragma once

#include <string>

struct AmmoParams {
	std::string name;
	float mass; 	// маса (кг)
	float drag; 	// коефіцієнт опору
	float lift; 	// коефіцієнт підйому
};

