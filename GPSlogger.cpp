// GPSlogger.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <ctime>
#include <Windows.h>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <codecvt>

#include "Point.h"

std::vector<Point> log(unsigned long long int n_events, HANDLE hSerial);
char* slice(char* array, int start, int end);
bool ReadLine(const HANDLE* port, char output[]);

int main(int argc, char* argv[])
{
    //Command line arguments
    if (argc < 4) //Make sure serial port is specified
    {
        std::cerr << "Usage: " << argv[0] << " COM# #captures output" << std::endl;
        return 1;
    }

    std::string port_name = argv[argc - 3];
    std::cout << port_name << '\n';
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring port_wide = converter.from_bytes(port_name);
    LPCWSTR port = port_wide.c_str();
    //LPCWSTR port = L"\\\\.\\COM10";

    unsigned long long int log_n = std::stoull(argv[argc - 2]);

    //Create handler (https://web.archive.org/web/20180127160838/http://bd.eduweb.hhs.nl/micprg/pdf/serial-win.pdf https://www.codeproject.com/Articles/8860/Non-Overlapped-Serial-Port-Communication-using-Win)
    HANDLE hSerial;
    hSerial = CreateFile(port, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hSerial == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND || GetLastError() == ERROR_PATH_NOT_FOUND) { std::cerr << "Serial port not found, program quitting.\n" << GetLastError() << "\n"; return 1; }
        else { std::cerr << "Unknown error creating handler, program quitting.\n" << GetLastError() << "\n"; return 1; }
    }
    std::cout << "Serial port opened...";

    //Set parameters for serial port
    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {std::cerr << "Error getting port state, program quitting.\n"; return 1;}
    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT; //Will need to be updated to fit format
    dcbSerialParams.Parity = NOPARITY; //Will need to be updated to fit format
    if (!GetCommState(hSerial, &dcbSerialParams)) { std::cerr << "Error setting port state, program quitting.\n"; return 1; }

    //Set timeouts 
    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    if (!SetCommTimeouts(hSerial, &timeouts)) { std::cerr << "Error setting timeouts, program quitting.\n"; return 1; }

    std::cout << "\b\b\b, parameters set. Launching logger...\n";
    //Log points
    std::vector<Point> point_list = log(log_n, hSerial);

    //Closing handle
    CloseHandle(hSerial);

    //Output file
     std::ofstream outfile;
    outfile.open(argv[argc - 1]);
    for (Point i : point_list) 
    {
        outfile << i.out();
    }
    outfile.close();
    return 0;
}

std::vector<Point> log(unsigned long long int n_events, HANDLE hSerial) //Logs events for a specified amount of time
{
    std::vector<Point> point_list;

    std::cout << "Events logged: 0";

    SetCommMask(hSerial, EV_RXCHAR); //Create mask
    unsigned long long int i = 0;
    while (i < n_events) //Loop until the maximum number of events is reached
    {
        char* text = new char[256];
        ReadLine(&hSerial, text);
        //Check first 6 letters (data type)
        char * first_six = slice(text, 0, 6);
        if (0 == strcmp(first_six, "$GPRMC"))
        {
            i++;
            //Find last text
            unsigned char textend = 6;
            for (int i = 0; i < 256; i++) {if (text[i] == '\0') { textend = i; break; }}
            //Parse
            unsigned int remaining_length = textend - 6;
            char* last_data = slice(text, 6, textend);
            //Find all commas
            std::vector<int> comma_indices;
            for (unsigned int i = 0; i < remaining_length; i++)
            {
                if (last_data[i] == ',') { comma_indices.push_back(i); }
            }
            //std::cout << '\n' << comma_indices.size(); Used to check if vector is of the proper size
            //Get data from areas defined by commas
            char* timea = slice(last_data, comma_indices[0] + 1, comma_indices[1]);
            char status = slice(last_data, comma_indices[1] + 1, comma_indices[2])[0];
            char* lata = slice(last_data, comma_indices[2] + 1, comma_indices[3]);
            char ns = slice(last_data, comma_indices[3] + 1, comma_indices[4])[0];
            char* lona = slice(last_data, comma_indices[4] + 1, comma_indices[5]);
            char ew = slice(last_data, comma_indices[5] + 1, comma_indices[6])[0];
            char* speeda = slice(last_data, comma_indices[6] + 1, comma_indices[7]);
            char* coursea = slice(last_data, comma_indices[7] + 1, comma_indices[8]);
            char* datea = slice(last_data, comma_indices[8] + 1, comma_indices[9]);
            char mode = slice(last_data, comma_indices[9] + 1, comma_indices[10])[0];
            char* checksuma = slice(last_data, remaining_length - 2, remaining_length);
            //Convert to proper types
            double time, lat, lon = 0; float speed, course = 0; unsigned long int date = 0; int checksum = 0;
            sscanf_s(timea, "%lf", &time);
            sscanf_s(lata, "%lf", &lat);
            sscanf_s(lona, "%lf", &lon);
            sscanf_s(speeda, "%f", &speed);
            sscanf_s(coursea, "%f", &course);
            sscanf_s(datea, "%lui", &date);
            sscanf_s(checksuma, "x");
            //Checksum - bitwise xor of all characters 
            char existing_char = 'K'; //Represents xoring of 'GPRMC'
            for (unsigned int i = 0; i < remaining_length - 3; i++)
            {
                existing_char ^= last_data[i];
            }
            bool checksum_pass = (existing_char == checksum) ? true : false;
            //Create the event file
            Point point = *new Point(time, lat, ns, lon, ew, speed, course, date, mode, checksum_pass, true);
            point_list.push_back(point);

            //Cleanup
            delete[] timea, lata, lona, speeda, coursea, datea, checksuma;

            (i == 1) ? std::cout << '\b' << i : std::cout << std::string((int)floor(log10(i - 1) + 1.0), '\b') << i;
        }
        delete[] text;
    }
    std::cout << "\n";

    return point_list;
}

char* slice(char* array, int start, int end)
{
    char *out = new char[end - start + 1];
    for (int i = 0; i < end - start; i++) 
    {
        out[i] = array[i + start];
    }
    out[end - start] = '\0';
    return out;
}

bool ReadLine(const HANDLE* port, char output[])  //Based on https://stackoverflow.com/questions/23371504/windows-serial-port-read-line
{
    DWORD byte_read_amount;
    char buffer[2];
    unsigned count = 0;

    while (true) 
    {
        if (ReadFile(*port, &buffer, 1, &byte_read_amount, NULL))
        {
            if ((byte_read_amount > 0) && (buffer[0] != '\n') && (buffer[0] != '\r'))
            {
                output[count] = buffer[0];
                count++;
            }
            else if (buffer[0] == '\r') 
            {
                output[count] = '\0';
                return true;
            }
        }
        else //Timeout
        {
            return false;
        }
    }
}