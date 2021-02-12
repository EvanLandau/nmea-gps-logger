#include <string>
#pragma once
class Point
{
private:
	double time_; //UTC time, hhmmss.sss
	double lat_; //Lat and long are in dddmm.mmmm format, with positive numbers being N and E (and negative being S and W) respectively
	double lon_;
	float speed_; //In knots
	float course_; //Direction in degrees
	unsigned long int date_; //UTC date, in format ddmmyy
public:
	Point(double time, double lat, char latns, double lon, char lonew, float speed, float course, unsigned long int date, char mode, bool checksum_pass, bool ignore_checks);
	std::string out();
};

