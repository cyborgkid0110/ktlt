/***********************************************************
* Program: dust_sim.cpp
* Purpose: Simulate sensor to measure dust concentration
* Author: Tran Quang Anh 20212398, 1/7/23
************************************************************/

#include "../header.h"
#include "../error.h"
#include "../readfile.h"

#define PM25_DATAFILE       "dust_sensor.csv"
#define LOG_FILE            "task1.log"

/* Generate dust concentration value
* Output: A double number from 0.0 to 1000.0*/
double generatePM25Value() 
{
    return (double)(rand() % 10001)/10.0;
}

/* Generate date and time from time in seconds
* Inputs: timenow - time in seconds
* Output: a time string with format YYYY:MM:DD hh:mm:ss */
string generateTimestamp(time_t time_now) 
{
    tm* tm_local = localtime(&time_now);            /* get time from time in seconds*/
    
    /* write time */
    stringstream ss;
    ss << setfill('0')
        << setw(4) << tm_local->tm_year + 1900 << ":"
        << setw(2) << tm_local->tm_mon + 1 << ":"
        << setw(2) << tm_local->tm_mday << " "
        << setw(2) << tm_local->tm_hour << ":"
        << setw(2) << tm_local->tm_min << ":"
        << setw(2) << tm_local->tm_sec;
    return ss.str();
}

/* Write simulated data in output file
* Inputs: 
        num_sensors: number of sensors
        sampling: time per simulation
        interval: duration of simulation
* Output: dust_sensor.csv file*/
void simulatingData(int num_sensors, int sampling, int interval) 
{
    ofstream OUTPUTSTREAM(PM25_DATAFILE);           /* stream for write dust_sensor.csv*/
    OUTPUTSTREAM << "id,time,value" << endl;
    
    /* write simulation data */
    srand(time(0));
    time_t start_time = time(NULL) - interval * 3600;
    for (int time_incre = 0; time_incre <= interval * 3600; time_incre += sampling) 
    {

        time_t timenow = start_time + time_incre; /* time */

        for (int id = 1; id <= num_sensors; id++) 
        {
            string Timestamp = generateTimestamp(timenow);
            double dust_concentration = generatePM25Value(); /* value */
            OUTPUTSTREAM << id << "," << Timestamp << "," << fixed << setprecision(1) << dust_concentration << endl; /* write dataline*/
        }
    }
    OUTPUTSTREAM.close();
    cout << "Data has been saved into file dust_sensor.csv."; /* notify when done */
}

/* Main function 
* Input: command-line statement
* Output: 
    dust_sensor.csv
    task1.log
    notification (should appear if program run successfully)
* Pre-condition: task1.log is accessible
*/
int main(int argc, char *argv[]) 
{   
    /* pre-condition */
    if (!ifAccessGranted(LOG_FILE, WRITE_MODE)) 
    {
        cout << "Cannot access " << LOG_FILE << " to record error." << endl;
        return 1;
    }

    /* set defalut value */ 
    int num_sensors = 1;            /* number of sensor */
    int sampling = 30;              /* time per simulation */
    int interval = 24;              /* duration measurement */

    /* check if command-line is correct */
    if (argc <= 2 || argc > 7) 
    {
        error(01, LOG_FILE);
        return 1;
    }
    
    /* check if arguments are valid and get value */
    for (int i = 1; i < argc; i+=2) 
    {
        string str = argv[i];
        if (str == "-n") 
            num_sensors = atoi(argv[i + 1]);                /* new number of sensor */
        else if (str == "-st")
            sampling = atoi(argv[i + 1]);                   /* new time per simulation */
        else if (str == "-si")
            interval = atoi(argv[i + 1]);                   /* new duration measurement */
        else 
        {
            error(02, LOG_FILE);
            return 1;
        }
    }

    if (num_sensors <= 0 || sampling < 1 || interval < 1) 
    {
        error(02, LOG_FILE);
        return 1;
    }

    /* notify input data */
    cout << "Number of sensors: " << num_sensors << endl;
    cout << "Sampling time: " << sampling << endl;
    cout << "Measurement duration: " << interval << endl;
    
    /* check if dust_sensor.csv is accessible */
    if (!scanFile(PM25_DATAFILE, LOG_FILE, WRITE_MODE))
        return 1;
    
    /* write simulation data in dust_sensor.csv */
    simulatingData(num_sensors, sampling, interval); 
    
    return 0;
}