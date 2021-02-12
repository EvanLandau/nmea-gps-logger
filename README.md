# nmea-gps-logger
Logs NMEA-0183 $GPRMC packets from a serial interface. Designed for Windows.

The correct usage of the program is:
(compiled program).exe serial_port #captures output_file

serial_port - This is the path to the serial port. This usually takes a form like \\.\COM1
#captures - The number of $GPRMC packets to capture and parse
output_file - File to output data to

(A), (B) UTC: (C) N, (D) E, (E) kts heading (F) degrees
A - The date in UTC format (ddmmyy).
B - The UTC time.
C & D - The latitude and longitude in dddmm.mmmm format.
E - Speed in knots.
F - Course in degrees.
