/***********************************************************
* Program: error.h
* Purpose: Record run-time error in log files
* Author: Tran Quang Anh 20212398, 1/7/23
************************************************************/

#ifndef ERROR_H
#define ERROR_H

#include "header.h"

/* error table */
vector<string> ERROR_LIST = 
{
    "Error 01: invalid command",
    "Error 02: invalid argument",
    "Error 03: file access denied",
    "Error 04: invalid csv file format",
    "Error 05: data missing at line X"
};

/* Generate time string occuring error
* Output: a local time with format YYYY:MM:DD hh:mm:ss */
string recordTimeOccurError() 
{
    time_t now;
    time(&now);
    tm* tm_local = localtime(&now);         /* get time */
    
    /* write time */
    stringstream ss;
    ss << "[" << setfill('0')
        << setw(4) << tm_local->tm_year + 1900 << ":"
        << setw(2) << tm_local->tm_mon + 1 << ":"
        << setw(2) << tm_local->tm_mday << " "
        << setw(2) << tm_local->tm_hour << ":"
        << setw(2) << tm_local->tm_min << ":"
        << setw(2) << tm_local->tm_sec << "]";
    return ss.str();
}

/* Write error in log file
* Input: log file and error index
* Output: error text in log file
Error list:
Error 01: invalid command
Error 02: invalid argument
Error 04: invalid csv file format
*/
void error(int i, string log_file) 
{
    ofstream ERROR_STREAM;
    ERROR_STREAM.open(log_file, ios::app);
    string error_text = ERROR_LIST[i-1];
    
    switch (i) {
        case 01:
        case 02:
        case 04:
            ERROR_STREAM << recordTimeOccurError() << " " << error_text << endl;
            break;
    }
    
    ERROR_STREAM.close();
}

/* Write error in log file
* Input: log file, error index
    linePos: line position in input file occuring error
* Output: error text in log file
Error list:
Error 05: data missing at line X
*/
void error(int i, string log_file, int linePos) 
{
    ofstream ERROR_STREAM;
    ERROR_STREAM.open(log_file, ios_base::app);
    string error_text = ERROR_LIST[i-1];

    error_text.replace(31, 4, to_string(linePos));

    switch (i) {
        case 05:
            ERROR_STREAM << recordTimeOccurError() << " " << error_text << endl;
            break;
    }
    ERROR_STREAM.close();
}

/* Write error in log file
* Input: log file, error index and file name
* Output: error text in log file
Error list:
Error 03: file access denied
*/
void error(int i, string log_file, string file_name) 
{
    ofstream ERROR_STREAM;
    ERROR_STREAM.open(log_file, ios_base::app);
    string error_text = ERROR_LIST[i-1];

    error_text.replace(10, 4, file_name);

    switch (i) {
        case 03:
            ERROR_STREAM << recordTimeOccurError() << " " << error_text << endl;
            break;
    }
    ERROR_STREAM.close();
}

#endif