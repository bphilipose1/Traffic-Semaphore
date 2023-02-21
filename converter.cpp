string converter(time_t convert){ // get the current time as a time_t value
    struct tm* timeinfo = localtime(&convert); // convert to local time
    int hours = timeinfo->tm_hour; // extract hours
    int minutes = timeinfo->tm_min; // extract minutes
    int seconds = timeinfo->tm_sec; // extract seconds
    string str1 = to_string(hours);
    string str2 = to_string(minutes);
    string str3 = to_string(seconds);
    
    string time = str1 + ':' + str2 + ':' + str3;
    

    return time; 
}
