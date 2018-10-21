#ifndef SRC_LOGINFO_H_
#define SRC_LOGINFO_H_

#include <sys/time.h>
#include <vector>
#include "LogLevel.h"

namespace dromedary{
    class LogInfo{
        private:
            static std::vector<LogInfo*> LOG_LIST;

            char *m_sender;
            struct timeval m_timestamp;
            char *m_message;
            LogLevel m_level;
            LogInfo(char *sender, char *message, LogLevel level);
        public:
            static void AddEntry(char *sender, char *message, LogLevel level);
    };
}

#endif // SRC_LOGINFO_H_