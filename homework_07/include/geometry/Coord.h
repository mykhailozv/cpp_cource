#pragma once


struct Coord {
	double x;
	double y;

	Coord operator+(const Coord& other) const;
	Coord operator-(const Coord& other) const;
	Coord operator*(double s) const;
	Coord operator/(double s) const;
	bool operator==(const Coord& other) const;
	double length() const;
	Coord getNorm() const;
	double distanceTo(const Coord& other) const;
	double direction() const;
};

