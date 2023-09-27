/***********************************************************
* Program: dust_process.cpp
* Purpose: Analyse data from dust sensor
* Author: Tran Quang Anh 20212398, 1/7/23
************************************************************/

#include "../header.h"
#include "../error.h"
#include "../readfile.h"

#define INPUT_FILE_LOCATION     "../task1"
#define OUTLIER_FILE            "dust_outliers.csv"
#define AQI_FILE                "dust_aqi.csv"
#define SENSOR_ANALYSING_FILE   "dust_summary.csv"
#define SENSOR_STATISTICS_FILE  "dust_statistics.csv"
#define LOG_FILE                "task2.log"

/* Check if date and time is in format YYYY:MM:DD hh:mm:ss
* Inputs: time string
* Output: true or false */
bool checkDateFormat(string date) 
{
    /* extract time string in parts */
    stringstream ss(date);
    int year, month, day, hour, min, sec;
    char colon1, colon2, colon3, colon4;
    ss >> year >> colon1 >> month >> colon2 >> day >> hour >> colon3 >> min >> colon4 >> sec;

    /* check if parts are valid */
    if ((month >= 1 && month <= 12)
        && (day >= 1 && day <= 31)
        && (hour >= 0 && hour <= 23)
        && (min >= 0 && min <= 59)
        && (sec >= 0 && sec <= 59)
        && (colon1 == ':') && (colon2 == ':') && (colon3 == ':') && (colon4 == ':'))
        return true;

    return false;
}

/* Structure for storing data from input file */
struct DataField 
{
    int id;
    string time;
    double value;
};

/* Filter outliers and store the rest data in vector
* Inputs:
    file_location: file directory
    num_sensors: number of sensors
* Output: vector<DataField> data */
vector<DataField> sortDatainFile(string& file_location, int* num_sensors) 
{
    vector<DataField> data;                     /* handle valid data */
    ifstream input_file(file_location);         /* stream of input and output file*/
    ofstream outlier_file(OUTLIER_FILE);

    outlier_file << "number of outliers:                 " << endl;             /* first line, blank for write number of outliers */
    outlier_file << "id,time,value" << endl;                                    /* second line */

    string textline;
    int num_outlier = 0;                    /* count outliers */
    int linePos = 1;                        /* show line position */
    int id_max = 0;                         /* to find number of sensors */
    getline(input_file, textline);          /* skip first line */
     
    /* scan dataline */
    while (getline(input_file, textline)) 
    {
        stringstream ss(textline);
        string id_str, time_str, value_str;

        /* get data from line */
        getline(ss, id_str, ',');
        getline(ss, time_str, ',');
        getline(ss, value_str, ',');
    
        int id = stoi(id_str);              /* get id */
        if ((id_str.size() == 0) || (time_str.size() == 0) || (value_str.size() == 0)
            || (id < 1) || (checkDateFormat(time_str) == false)) 
        {
            error(07, LOG_FILE, linePos);
            break;
        }

        if (id_max < id)
            id_max = id;                /* find number of sensors */

        double dust_value = stod(value_str);                    /* get dust concentration */
        if ((dust_value < 0) || (dust_value > 550.5)) 
        {
            num_outlier++;
            outlier_file << textline << endl;
        }
        else
            data.push_back({id,time_str,dust_value});           /* import valid data */

        linePos++;                      /* move to next line*/
    }
    *num_sensors = id_max; // return the maximum sensors

    /* close stream  */
    input_file.close();
    outlier_file.seekp(0, ios::beg);                                /* return to first line */
    outlier_file <<  "number of outliers: " << num_outlier;         /* write number of outliers */
    outlier_file.close();

    cout << "Filter outliers completely. Output file: " << OUTLIER_FILE << endl;            /* notify */
    return data;
}

/* Structure for handle data of dust concentration conversion */
struct AQIRange 
{
    double value_min;
    double value_max;
    int AQI_min;
    int AQI_max;
    string level;
};

/* Dust concentration conversion table */
vector<AQIRange> aqiRanges = 
{
    {0.0, 12.0, 0, 50, "Good"},
    {12.0, 35.5, 50, 100, "Moderate"},
    {35.5, 55.5, 100, 150, "Slightly unhealthy"},
    {55.5, 150.5, 150, 200, "Unhealthy"},
    {150.5, 250.5, 200, 300, "Very unhealthy"},
    {250.5, 350.5, 300, 400, "Hazardous"},
    {350.5, 550.5, 400, 500, "Extremely hazardous"}
};

/* Convert dust concentration to AQI
* Inputs: dust concentration
* Output: AQI */
int convertPM25toAQI(double value) 
{
    double AQI;
    for (const AQIRange& range : aqiRanges) 
    {
        if (value >= range.value_min && value <= range.value_max) 
        {
            AQI = ((range.AQI_max - range.AQI_min) / (range.value_max - range.value_min)) * (value - range.value_min) + range.AQI_min;
            break;
        }
    }
    return (int)AQI;
}

/* Convert AQI to pollution level
* Inputs: AQI
* Output: pollution level string
* Pre-condition: */
string AQItoLevel(int AQI) 
{
    string level;
    for (const AQIRange& range : aqiRanges) {
        if (AQI >= range.AQI_min && AQI <= range.AQI_max) {
            level = range.level;
            break;
        }
    }
    return level;
}

/* Set all elements in array to zero 
* Inputs: integer or double array and array' size
* Output: zero array */
void setValuetoZero(int *array, int size) 
{
    for (int i = 0; i < size; i++)
        array[i] = 0;
}

void setValuetoZero(double *array, int size) 
{
    for (int i = 0; i < size; i++)
        array[i] = 0.0;
}

/* Structure for store data to write dust_aqi.csv and further analysis*/
struct averageValue 
{
    int id;
    string time;
    double value;
    int aqi;
    string level;
};

/* calculate average PM2.5 value, AQI and pollution level
* Inputs:
    vector<DataField> data: from sortDataInFile()
    num_sensors: number of sensors
    interval: duration measurement
* Output: vector<averageValue> list */
vector<averageValue> calculateAverageValue(vector<DataField> &data, int num_sensors, int *interval) 
{
    double sum_value[num_sensors];              /* to find average value per hour*/
    int count[num_sensors];                     /* count id frequency */
    setValuetoZero(sum_value, num_sensors);     /* initial state */
    setValuetoZero(count, num_sensors);

    vector<averageValue> list;          /* handle processed data */
    string time_checkpoint = "";

    /* read each data in each element in vector */
    for (const DataField& entry : data) 
    {
        string timenow = entry.time.substr(0, 13);          /* extract hour part */
        if (time_checkpoint != timenow) {
            for (int id = 0; id < num_sensors; id++) 
            {
                /* process data and put in vector */
                if (count[id] > 0) 
                {
                    double ave_value = sum_value[id]/count[id];         /* value */
                    int ave_aqi = convertPM25toAQI(ave_value);          /* AQI */
                    string level = AQItoLevel(ave_aqi);                 /* level */
                    string time = time_checkpoint;                      /* time */
                    time.append(":00:00");
                    list.push_back({id + 1, time, ave_value, ave_aqi, level});
                }
            }

            (*interval)++;              /* record duration measurement */
            setValuetoZero(sum_value, num_sensors);         /* reset state */
            setValuetoZero(count, num_sensors);
        }

        sum_value[entry.id - 1] += entry.value;         /* sum value increment */
        count[entry.id - 1]++;
        time_checkpoint = timenow;
    }
    /* calculate average value in last hour */
    for (int id = 0; id < num_sensors; id++) 
    {
        if (count[id] > 0) 
        {
            double ave_value = sum_value[id]/count[id];
            int ave_aqi = convertPM25toAQI(ave_value);
            string level = AQItoLevel(ave_aqi);
            string time = time_checkpoint;
            time.append(":00:00");
            list.push_back({id + 1, time, ave_value, ave_aqi, level});
        }
    }

    return list;
}

/* Write data in vector<averageValue> list to dust_aqi.csv
* Inputs: vector<averageValue> list
* Output: dust_aqi.csv */
void writeCalculateAverageValue(vector<averageValue> &list) 
{
    ofstream OUTPUT_STREAM(AQI_FILE);
    OUTPUT_STREAM << "id,time,value,aqi,pollution" << endl;

    /* write data in dust_aqi.csv */
    for (const averageValue& entry : list) 
    {
        OUTPUT_STREAM << fixed << setprecision(1)
        << entry.id << ","
        << entry.time << ","
        << entry.value << ","
        << entry.aqi << ","
        << entry.level << endl;
    }

    OUTPUT_STREAM.close();
}

/* Find max, min, mean value of each sensor
and write in dust_summary.csv
* Inputs: 
    vector<DataField> data: from sortDataInFile()
    num_sensors: number of sensors
    interval: duration measurement
* Output: dust_summary.csv */
void analyseSensorData(vector<DataField> &data, int num_sensors, int interval) 
{
    vector<DataField> maxValue;         /* max */
    vector<DataField> minValue;         /* min */
    double meanValue[num_sensors];      /* mean */
    int count[num_sensors];

    /* initial state */
    setValuetoZero(meanValue, num_sensors);         
    setValuetoZero(count, num_sensors);
    for (int i = 0; i < num_sensors; i++) 
    {
        maxValue.push_back({0,"",0.0});
        minValue.push_back({0,"",560.0});
    }

    /* read data */
    for (const DataField& entry : data) 
    {
        if (maxValue[entry.id - 1].value < entry.value) 
            maxValue[entry.id - 1] = {entry.id, entry.time, entry.value};           /* find max */
        if (minValue[entry.id - 1].value > entry.value)
            minValue[entry.id - 1] = {entry.id, entry.time, entry.value};           /* find min */
        
        meanValue[entry.id - 1] += entry.value;
        count[entry.id - 1]++;
    }

    ofstream OUTPUT_STREAM(SENSOR_ANALYSING_FILE);
    OUTPUT_STREAM << "id,parameter,time,value" << endl;

    /* write analysed data */
    for (int id = 0; id < num_sensors; id++) 
    {
        if (count[id] > 0) 
        {
            meanValue[id] /= count[id]; // calculate mean value
            OUTPUT_STREAM << fixed << setprecision(1)
            << maxValue[id].id << "," << "max"  << "," << maxValue[id].time << "," << maxValue[id].value << endl
            << minValue[id].id << "," << "min"  << "," << minValue[id].time << "," << minValue[id].value << endl
            << id + 1          << "," << "mean" << "," << interval << ":00:00,"    << meanValue[id]      << endl;
        }
    }

    OUTPUT_STREAM.close();
}

/* Structure for handle data of pollution level's frequency*/
struct PollutionLevel 
{
    int Good;
    int Moderate;
    int Slightly_unhealthy;
    int Unhealthy;
    int Very_unhealthy;
    int Hazardous;
    int Extremely_hazardous;
};

/* Count frequency of each pollution level in sensor 
and write in dust_statistics.csv
* Inputs: 
    vector<DataField> data: from sortDataInFile()
    num_sensors: number of sensors
* Output: dust_statistics.csv */
void SensorAverageStat(vector<averageValue> &list, int num_sensors) 
{
    vector<PollutionLevel> status_count;                /* handle data */
    for (int id = 0; id < num_sensors; id++)
        status_count.push_back({0,0,0,0,0,0,0});        /* initial state */
    
    /* read data and count frequency */
    for (const averageValue& entry : list) 
    {
        if (entry.level == "Good") 
            status_count[entry.id - 1].Good++;
        else if (entry.level == "Moderate") 
            status_count[entry.id - 1].Moderate++;
        else if (entry.level == "Slightly unhealthy")
            status_count[entry.id - 1].Slightly_unhealthy++;
        else if (entry.level == "Unhealthy") 
            status_count[entry.id - 1].Unhealthy++;
        else if (entry.level == "Very unhealthy") 
            status_count[entry.id - 1].Very_unhealthy++;
        else if (entry.level == "Hazardous") 
            status_count[entry.id - 1].Hazardous++;
        else if (entry.level == "Extremely hazardous") 
            status_count[entry.id - 1].Extremely_hazardous++;
    }
    
    ofstream OUTPUT_STREAM(SENSOR_STATISTICS_FILE);         /* stream for dust_statistics.csv */
    OUTPUT_STREAM << "id,pollution,duration" << endl;

    /* write data in file */
    for (int id = 0; id < num_sensors; id++) 
    {
        OUTPUT_STREAM
        << id + 1 << "," << "Good" << "," << status_count[id].Good << endl
        << id + 1 << "," << "Moderate" << "," << status_count[id].Moderate << endl
        << id + 1 << "," << "Slightly unhealthy" << "," << status_count[id].Slightly_unhealthy << endl
        << id + 1 << "," << "Unhealthy" << "," << status_count[id].Unhealthy << endl
        << id + 1 << "," << "Very unhealthy" << "," << status_count[id].Very_unhealthy << endl
        << id + 1 << "," << "Hazardous" << "," << status_count[id].Hazardous << endl
        << id + 1 << "," << "Extremely hazardous" << "," << status_count[id].Extremely_hazardous << endl;
    }

    OUTPUT_STREAM.close();
}

/* Find max, min, mean value of each sensor
* Inputs: command-line statement
* Output: 
    dust_outliers.csv
    dust_aqi.csv
    dust_summary.csv
    dust_aqi.csv
    task2.log
    nofication of each sub-task (should appear if process run successfully)
* Pre-condition: task2.log is accessible
*/
int main(int argc, char *argv[]) 
{   
    /* pre-condition */
    if (!ifAccessGranted(LOG_FILE, WRITE_MODE)) 
    {
        cout << "Cannot access " << LOG_FILE << " to record error." << endl;
        return 1;
    }

    string input_filename = "dust_sensor.csv";          /* default input file */
    if (argc == 2) 
    {
        string temp_str = argv[1];
        input_filename.assign(temp_str);                /* new input file */
    }
    /* get file directory */
    string file_location = INPUT_FILE_LOCATION;
    file_location.push_back('/');
    file_location.append(input_filename); 

    /* task 2.1 */
    /* check if input file and dust_outliers.csv are accessible*/
    if (!scanFile(INPUT_FILE_LOCATION, input_filename, LOG_FILE, READ_MODE))
        return 1;
    
    if (!scanFile(OUTLIER_FILE,LOG_FILE,WRITE_MODE)) 
        return 1;

    /* check if file has correct format */
    if (!if_DUST_SENSOR_file(file_location, LOG_FILE)) 
        return 1;

    int num_sensors;            /* number of sensors */
    int interval = -1;          /* duration measurement */
    vector<DataField> data = sortDatainFile(file_location, &num_sensors);       /* handle valid data */

    /* task 2.2 */
    vector<averageValue> list = calculateAverageValue(data, num_sensors, &interval);        /* get average data list */
    /* check if dust_aqi.csv is accessible */
    if (!scanFile(AQI_FILE, LOG_FILE, WRITE_MODE))
        return 1;

    writeCalculateAverageValue(list);           /* write data */
    cout << "Calculate AQI completely. Output file: " << AQI_FILE << endl;      /* notify */

    /* task 2.3 */
    /* check if dust_summary.csv is accessible */
    if (!scanFile(SENSOR_ANALYSING_FILE,LOG_FILE,WRITE_MODE))
        return 1;

    analyseSensorData(data, num_sensors, interval);             
    cout << "Analysed sensor data completely. Output file: " << SENSOR_ANALYSING_FILE << endl;

    /* task 2.4 */
    /* check if dust_statistics.csv is accessible*/
    if (!scanFile(SENSOR_STATISTICS_FILE,LOG_FILE,WRITE_MODE))
        return 1;

    SensorAverageStat(list, num_sensors);
    cout << "Analysed sensor statistics completely. Output file: " << SENSOR_STATISTICS_FILE << endl;

    return 0;    
}