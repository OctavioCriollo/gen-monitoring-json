#include <RTClib.h>
#include <NTPClient.h>
//#include <SPI.h>

#define RTC_ERROR "RTC module Failure"

String dateToStringOld(DateTime& date){
    char now[20];
    snprintf(now, sizeof(now),"%04d-%02d-%02dT%02d:%02d:%02d",date.year(),date.month(),date.day(),date.hour(),date.minute(),date.second());
    return String(now);
}

String dateToString(DateTime& dateTime){
    char format[] = "YYYY-MM-DDThh:mm:ssZ";
    if(dateTime.year()==2000)
        return RTC_ERROR;
    return dateTime.toString(format);
}

DateTime parseISO8601(String& dateString) {
  int year = dateString.substring(0, 4).toInt();
  int month = dateString.substring(5, 7).toInt();
  int day = dateString.substring(8, 10).toInt();
  int hour = dateString.substring(11, 13).toInt();
  int minute = dateString.substring(14, 16).toInt();
  int second = dateString.substring(17, 19).toInt();
  return DateTime(year, month, day, hour, minute, second);
}

String updateRTCFromNTP(RTC_DS3231& rtc, NTPClient& timeClient){
    String date_NTP_Str =""; 
    if(rtc.begin()){
        timeClient.begin();
        timeClient.setTimeOffset(-5*3600);  /*Ajusta a hora de Ecuador*/
        timeClient.update();
        time_t epochTime = timeClient.getEpochTime();
        struct tm *timeinfo;
        timeinfo = localtime(&epochTime);
        char formattedDateTime[20];
        strftime(formattedDateTime, sizeof(formattedDateTime), "%Y-%m-%dT%H:%M:%S", timeinfo);
        date_NTP_Str = String(formattedDateTime);
        //rtc.adjust(parseISO8601(date_NTP_Str));
        rtc.adjust(DateTime(timeClient.getEpochTime())); 
    }
    else
        Serial.printf("%s\n\n",RTC_ERROR);
    return date_NTP_Str;
}

DateTime getRTC(RTC_DS3231& rtc){
    DateTime now;
    if(!rtc.begin()){
        return DateTime(2000,1,1,0,0,0);
    }
    now = rtc.now();
    return now;
}