#include <iostream>
#include "Log.h"
#include <thread>
#include "util.h"
#include <unistd.h>

int main() {
    std::cout << "Hello SrvPro" << std::endl;
    /*
     * (d) - () - (1)
     * ([ ) - () - (0)
     * (p) - () - (1)
     * (] ) - () - (0)
     * (f) - () - (1)
     * ( ) - () - (0)
     * (l) - () - (1)
     * ( ) - () - (0)
     * (m) - () - (1)
     * ( ) - () - (0)
     * (n) - () - (1)
     * */
    srvpro::Logger::ptr logger(new srvpro::Logger);
    logger->addAppender(srvpro::LogAppender::ptr(new srvpro::StdoutLogAppender));
    char* current_path = get_current_dir_name();
    std::cout << current_path << std::endl;
    srvpro::FileLogAppender::ptr file_appender(new srvpro::FileLogAppender("../log.txt")); ///home/mrpiao/Desktop/ServerProject/log.txt
    srvpro::LogFormatter::ptr fmt(new srvpro::LogFormatter("%d%T%p%T%m%n"));
    file_appender->setLogFormatter(fmt);
    file_appender->setLevel(srvpro::LogLevel::ERROR);

    logger->addAppender(file_appender);
    //srvpro::LogEvent::ptr event(new srvpro::LogEvent(__FILE__, __LINE__, 0, srvpro::GetThreadID(), srvpro::GetFiberID(), time(0)));
    //event->getSS() << "Hello SrvPro";
    //logger->log(srvpro::LogLevel::DEBUG, event);

    //std::cout << "Hello SrvPro" << std::endl;

    SRVPRO_LOG_INFO(logger) << "test";
    SRVPRO_LOG_ERROR(logger) << "test error";

    SRVPRO_LOG_FMT_ERROR(logger, "test fmt %s", "aa");

    auto l = srvpro::LoggerMgr::GetInstance()->getLogger("xxx");
    SRVPRO_LOG_INFO(l) << "xxx";
    return 0;
}
