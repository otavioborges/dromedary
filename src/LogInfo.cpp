#include <string.h>
#include "LogInfo.h"
using namespace dromedary;
using namespace std;

vector<LogInfo*> LogInfo::LOG_LIST;

LogInfo::LogInfo(char *sender, char *message, LogLevel level){
    m_sender = (char*) new char[strlen(sender)];
    m_message = (char*) new char[strlen(message)];
    
    strcpy(m_sender, sender);
    strcpy(m_message, message);
    m_level = level;

    gettimeofday(&m_timestamp, NULL);
}

void LogInfo::AddEntry(char *sender, char *message, LogLevel level){
    LOG_LIST.push_back(new LogInfo(sender, message, level));
}
