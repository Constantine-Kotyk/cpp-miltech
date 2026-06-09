#pragma once

#include <cmath>

struct Coord {
	float x;
	float y;

	// Додавання координат
	Coord operator+(const Coord& other) const {
    	Coord result;
        result.x = x + other.x;
        result.y = y + other.y;
        return result;
	}

	// Віднімання координат
	Coord operator-(const Coord& other) const {
    	Coord result;
        result.x = x - other.x;
        result.y = y - other.y;
        return result;
	}

	// Множення на скаляр
	Coord operator*(float s) const {
    	Coord result;
        result.x = x * s;
        result.y = y * s;
        return result;
	}

	// Ділення на скаляр
	Coord operator/(float s) const {
    	Coord result;
        result.x = x / s;
        result.y = y / s;
        return result;
	}

	// Порівняння
	bool operator==(const Coord& other) const {
        return x == other.x && y == other.y;
	}

	float length() {
	    return std::hypot(x, y);
	}

	float length(Coord c) {
	    return std::hypot(c.x, c.y);
	}
};
