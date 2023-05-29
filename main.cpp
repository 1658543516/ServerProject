#include <iostream>
#include <Log.h>

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

    srvpro::LogEvent::ptr event(new srvpro::LogEvent(__FILE__, __LINE__, 0, 1, 2, time(0)));

    logger->log(srvpro::LogLevel::DEBUG, event);

    std::cout << "Hello SrvPro" << std::endl;

    return 0;
}
