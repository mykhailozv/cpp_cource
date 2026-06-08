#include <cmath>

#include "geometry/Coord.h"
#include "utils/MathUtils.h"

// Додавання координат
Coord Coord::operator+(const Coord& other) const {
    Coord result;
    result.x = x + other.x;
    result.y = y + other.y;
    return result;
}

// Віднімання координат
Coord Coord::operator-(const Coord& other) const {
    Coord result;
    result.x = x - other.x;
    result.y = y - other.y;
    return result;
}

// Множення на скаляр
Coord Coord::operator*(double s) const {
    Coord result;
    result.x = x * s;
    result.y = y * s;
    return result;
}

// Ділення на скаляр
Coord Coord::operator/(double s) const {
    Coord result;
    result.x = x / s;
    result.y = y / s;
    return result;
}

// Порівняння
bool Coord::operator==(const Coord& other) const {
    return (std::fabs(x - other.x) < MathUtils::EPS) && (std::fabs(y - other.y) < MathUtils::EPS);
}

double Coord::length() const {
    return std::sqrt(x*x + y*y);
}

Coord Coord::getNorm() const {
    double len = length();

    if (len < MathUtils::EPS)
    {
        return {0,0};
    }
    
    return (*this) / len;        
}

double Coord::distanceTo(const Coord& other) const{
    return std::sqrt((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y));
}

double Coord::direction() const{
    return std::atan2(y, x);
}
