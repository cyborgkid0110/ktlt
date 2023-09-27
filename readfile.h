/***********************************************************
* Program: readfile.h
* Purpose: Scan file header
* Author: Tran Quang Anh 20212398, 1/7/23
************************************************************/

#ifndef READFILE_H
#define READFILE_H

#include "header.h"
#include "error.h"

/* file access mode */
#define WRITE_MODE 'w'              /* for input file */
#define READ_MODE  'r'              /* for output file*/

/* Check if file is accessible
* Input: file name and access mode
* Output: true or false
* Pre-condition: access_mode argument is correct */
bool ifAccessGranted(string file_name, char access_mode) 
{
    if (access_mode != WRITE_MODE && access_mode != READ_MODE) 
    {
        cout << "Invalid argument in function scanFile()" << endl;
        return false;
    }

    if (access_mode == READ_MODE) 
    {
        ifstream checkFile(file_name);
        if (checkFile)
            return true;
    }
    else if (access_mode == WRITE_MODE) 
    {
        ofstream checkFile(file_name, ios::app);
        if (checkFile)
            return true;
    }
    
    return false;
}

/* Similar to function ifAccessGranted(), but will record error if get false.
Work with file having file path
* Input: file name, access mode, log file, path - file directory
* Output: 
    error text in log file (if occur error)
    true (if no error) or false
* Pre-condition: access_mode argument is correct */
bool scanFile(string path, string file_name, string log_file, char access_mode) 
{
    if (access_mode != WRITE_MODE && access_mode != READ_MODE) 
    {
        cout << "Invalid argument in function scanFile()" << endl;
        return false;
    }

    string file_location = path;
    file_location.push_back('/');
    file_location.append(file_name);

    // check if user has permission to access
    if (!ifAccessGranted(file_location, access_mode)) 
    {
        error(3, log_file, file_name);
        return false;
    }
        
    return true;
}

/* Similar to function ifAccessGranted(), but will record error if get false.
Work with file in same folder with program
* Input: file name, access mode, log file
* Output: 
    error text in log file (if occur error)
    true (if no error) or false
* Pre-condition: access_mode argument is correct */
bool scanFile(string file_name, string log_file, char access_mode) 
{
    if (access_mode != WRITE_MODE && access_mode != READ_MODE) 
    {
        cout << "Invalid argument in function scanFile()" << endl;
        return false;
    }

    // check if user has permission to access
    if (!ifAccessGranted(file_name, access_mode)) 
    {
        error(3, log_file, file_name);
        return false;
    }
        
    return true;
}

/* Check if file have same format as "dust_sensor.csv"
* Input: file name and log file
* Output: true (if correct) or false */
bool if_DUST_SENSOR_file(string file_name, string log_file) 
{
    ifstream INPUT_FILE(file_name);
    string temp_str;
    getline(INPUT_FILE, temp_str);
    if (temp_str != "id,time,value") 
    {
        error(6, log_file);
        return false;
    }
    return true;
}

/* Check if file have same format as "dust_aqi.csv"
* Input: file name and log file
* Output: true (if correct) or false */
bool if_DUST_AQI_file(string file_name, string log_file) 
{
    ifstream INPUT_FILE(file_name);
    string temp_str;
    getline(INPUT_FILE, temp_str);
    
    if (temp_str != "id,time,value,aqi,pollution") 
    {
        error(6, log_file);
        return false;
    }
    return true;
}

#endif