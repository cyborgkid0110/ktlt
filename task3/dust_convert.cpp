/***********************************************************
* Program: dust_convert.cpp
* Purpose: Create data packet from [dust_aqi_format].file
* Author: Tran Quang Anh 20212398, 1/7/23
************************************************************/

#include "../header.h"
#include "../error.h"
#include "../readfile.h"

#define AQI_FILE_LOCATION "../task2"
#define LOG_FILE "task3.log"

/* Define datatype of each data in packet */
typedef unsigned char packet_id;
typedef unsigned char packet_start_end;
typedef int packet_time;
typedef float packet_value;
typedef short int packet_aqi;
typedef unsigned char packet_checksum;
typedef unsigned char packet_length;

const packet_length PACKET_SIZE = sizeof(packet_aqi) + sizeof(packet_checksum) 
                                + sizeof(packet_id) + sizeof(packet_length)
                                + sizeof(packet_time) + sizeof(packet_value)
                                + 2 * sizeof(packet_start_end);
const packet_start_end START_BYTE = 0x7A;
const packet_start_end END_BYTE = 0x7F;

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

/* Convert date and time to time in seconds
* Input: time string
* Output: time in seconds */
packet_time UnixTimestampConvert(string time_str) 
{
    tm timeStruct;
    stringstream ss(time_str);          
    /* get time from time string */
    ss >> get_time(&timeStruct, "%Y:%m:%d %H:%M:%S");
    time_t time = mktime(&timeStruct); 

    return static_cast<packet_time>(time);
}

/* Check if system is little-endian
* Output: true or false */
bool ifSystemIsLittleEnd() 
{
    union {
        float num;
        uint8_t array[4];
    } temp;

    temp.num = 234.5;
    // 234.5 = 0x436a8000
    if (temp.array[0] == 0x00)
        return true;
    else
        return false;
}

/* Convert data from decimal to hexa 
* Input: Decimal value
* Output: Hexa value */
template <typename data_type>
vector<packet_checksum> decimalToHexa(data_type num, bool little_endian) 
{
    vector<packet_checksum> packet;     /* handle byte array */
    union FloatBytes                    /* structure for conversion */
    {
        data_type num;
        unsigned char byteArray[sizeof(data_type)];
    } converter;
    converter.num = num;

    /* if system is liitle-endian, order the bytes reversely */
    if (little_endian) 
    {
        for (int i = sizeof(data_type) - 1; i >= 0; i--)
            packet.push_back(static_cast<int>(converter.byteArray[i]));
    }
    else 
    {
        for (int i = 0; i < sizeof(data_type); i++)
            packet.push_back(static_cast<int>(converter.byteArray[i]));
    }

    return packet;
}

/* Calculate checksum of packet
* Input: vector<packet_checksum> packet - data packet
* Output: Checksum */
packet_checksum calculateCheckSum(vector<packet_checksum> &packet) 
{
    packet_checksum checksum = 0;
    for (const packet_checksum& byte : packet) 
        checksum += byte;

    return static_cast<packet_checksum>(~checksum + 1);
}

/* Create data packet
* Input: temp_str - data line in input file
* Output: vector<packet_checksum> packet storing byte array */
vector<packet_checksum> loadingPacket(string &temp_str) 
{
    string id_str, time_str, value_str, aqi_str;
    stringstream ss(temp_str);
    vector<packet_checksum> packet;

    /* extract dataline in parts */
    getline(ss, id_str, ',');
    getline(ss, time_str, ',');
    getline(ss, value_str, ',');
    getline(ss, aqi_str, ',');

    /* check if data missing */
    if ((id_str.size() == 0) || (time_str.size() == 0)
        || (value_str.size() == 0)  || (aqi_str.size() == 0) 
        || (checkDateFormat(time_str) == false) && (stoi(id_str) <= 0)) 
    {
        packet.push_back(0);            /* if yes, return zero packet */
        return packet;
    }

    bool little_end = ifSystemIsLittleEnd();

    /* extract data in each line in file and insert byte in packet */
    packet.push_back(PACKET_SIZE);
    vector<packet_checksum> id = decimalToHexa(static_cast<packet_id>(stoi(id_str)), little_end);
    vector<packet_checksum> time = decimalToHexa(static_cast<packet_time>(UnixTimestampConvert(time_str)), little_end);
    vector<packet_checksum> value = decimalToHexa(static_cast<packet_value>(stof(value_str)), little_end);
    vector<packet_checksum> aqi = decimalToHexa(static_cast<packet_aqi>(stoi(aqi_str)), little_end);
    packet.insert(packet.end(), id.begin(), id.end());
    packet.insert(packet.end(), time.begin(), time.end());
    packet.insert(packet.end(), value.begin(), value.end());
    packet.insert(packet.end(), aqi.begin(), aqi.end());
    packet_checksum checksum = calculateCheckSum(packet);
    packet.push_back(checksum);
    packet.insert(packet.begin(), START_BYTE);
    packet.push_back(END_BYTE);    
    
    return packet;
}

/* Write packet of in output file
* Input: vector<packet_checksum> packet from loadingPacket()
* Output: string of hexa array */
string writePacket(vector<packet_checksum> &packet) 
{
    stringstream ss;
    for (const packet_checksum& byte : packet)
        ss << uppercase << hex << setw(2) << setfill('0') << static_cast<int>(byte) << ' ';

    return ss.str();
}

/* Main function
* Input: command-line statement
* Output: 
    ile with name given by user
    task3.log
    nofication (should appear if program run successfully)
* Pre-condition: task3.log is accessible and command-line is valid */
int main(int argc, char *argv[]) 
{
    /* pre-condition */
    if (!ifAccessGranted(LOG_FILE, WRITE_MODE)) 
    {
        cout << "Cannot access " << LOG_FILE << " to record error." << endl;
        return 1;
    }
    /* check command-line */
    if (argc != 3) 
    {
        error(1, LOG_FILE);
        return 1;
    }

    string input_filename(argv[1]), output_filename(argv[2]);       /* input, output file */
    string input_filepath = AQI_FILE_LOCATION;
    input_filepath.push_back('/');
    input_filepath.append(input_filename);          /* input file directory */

    /* check if input and output file are accessible*/
    if (!scanFile(AQI_FILE_LOCATION,input_filename, LOG_FILE, READ_MODE))
        return 1;

    if (!scanFile(output_filename, LOG_FILE, WRITE_MODE))
        return 1;

    /* check if file has correct format */
    if (!if_DUST_AQI_file(input_filepath, LOG_FILE))
        return 1;

    ifstream INPUT_STREAM(input_filepath);          /* stream for input and output file */
    ofstream OUTPUT_STREAM(output_filename);
    string temp_str;
    int linePos = 0;                                /* show line position */

    getline(INPUT_STREAM, temp_str);                /* skip first line */
    /* read data */
    while (getline(INPUT_STREAM, temp_str)) 
    {
        vector<packet_checksum> packet = loadingPacket(temp_str);       /* get packet */
        if (packet[0] != START_BYTE) 
        {
            error(07, LOG_FILE, linePos);;
            return 1;
        }

        OUTPUT_STREAM << writePacket(packet) << endl;               /* write packet */
    }
    INPUT_STREAM.close();
    OUTPUT_STREAM.close();

    cout << "Conversion completed successfully." << endl;           /* notify */
    return 0;
}