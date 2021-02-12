#include "Point.h"
//Reference https://navspark.mybigcommerce.com/content/NMEA_Format_v0.1.pdf

Point::Point(double time, double lat, char latns, double lon, char lonew, float speed, float course, unsigned long int date, char mode, bool checksum_pass, bool ignore_checks)
{
	if ((checksum_pass && !ignore_checks && (mode == 'A' || mode == 'D' || mode == 'E')) || ignore_checks) //Only take valid data that passes checksum, unless checks are ignored
	{
		(latns == 's') ? lat_ = lat * -1 : lat_ = lat; //Convert south to negative, north to positive
		(lonew == 'w') ? lon_ = lon * -1 : lon_ = lon;
		time_ = time;
		speed_ = speed;
		course_ = course;
		date_ = date;
	}
	else {
		throw "checksum failure";
	}
}

std::string Point::out() 
{
	std::string output = std::to_string(date_) + ", " + std::to_string(time_) + " UTC: " + std::to_string(lat_) + " N, " + std::to_string(lon_) + " E, " + std::to_string(speed_) + " kts heading " + std::to_string(course_) + " degrees\n";
	return output;
}