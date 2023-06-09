//
// Created by mrpiao on 23-5-23.
//

#ifndef SERVERPROJECT_LOG_H
#define SERVERPROJECT_LOG_H
#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <map>
#include "singleton.h"

#define SRVPRO_LOG_LEVEL(logger, level) \
        if(logger->getLevel() <= level) \
            srvpro::LogEventWrap(srvpro::LogEvent::ptr(new srvpro::LogEvent(logger, level, __FILE__, __LINE__, 0, srvpro::GetThreadID(), srvpro::GetFiberID(), time(0)))).getSS()
#define SRVPRO_LOG_DEBUG(logger) SRVPRO_LOG_LEVEL(logger, srvpro::LogLevel::DEBUG)
#define SRVPRO_LOG_INFO(logger) SRVPRO_LOG_LEVEL(logger, srvpro::LogLevel::INFO)
#define SRVPRO_LOG_WARN(logger) SRVPRO_LOG_LEVEL(logger, srvpro::LogLevel::WARN)
#define SRVPRO_LOG_ERROR(logger) SRVPRO_LOG_LEVEL(logger, srvpro::LogLevel::ERROR)
#define SRVPRO_LOG_FATAL(logger) SRVPRO_LOG_LEVEL(logger, srvpro::LogLevel::FATAL)

#define SRVPRO_LOG_FMT_LEVEL(logger, level, fmt, ...) \
        if (logger->getLevel() <= level)              \
            srvpro::LogEventWrap(srvpro::LogEvent::ptr(new srvpro::LogEvent(logger, level, __FILE__, __LINE__, 0, srvpro::GetThreadID(), srvpro::GetFiberID(), time(0)))).getEvent()->format(fmt, __VA_ARGS__)
#define SRVPRO_LOG_FMT_DEBUG(logger, fmt, ...) SRVPRO_LOG_FMT_LEVEL(logger, srvpro::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define SRVPRO_LOG_FMT_INFO(logger, fmt, ...) SRVPRO_LOG_FMT_LEVEL(logger, srvpro::LogLevel::INFO, fmt, __VA_ARGS__)
#define SRVPRO_LOG_FMT_WARN(logger, fmt, ...) SRVPRO_LOG_FMT_LEVEL(logger, srvpro::LogLevel::WARN, fmt, __VA_ARGS__)
#define SRVPRO_LOG_FMT_ERROR(logger, fmt, ...) SRVPRO_LOG_FMT_LEVEL(logger, srvpro::LogLevel::ERROR, fmt, __VA_ARGS__)
#define SRVPRO_LOG_FMT_FATAL(logger, fmt, ...) SRVPRO_LOG_FMT_LEVEL(logger, srvpro::LogLevel::FATAL, fmt, __VA_ARGS__)

#define SRVPRO_LOG_ROOT() srvpro::LoggerMgr::GetInstance()->getRoot()
#define SRVPRO_LOG_NAME(name) srvpro::LoggerMgr::GetInstance()->getLogger(name)

namespace srvpro{
    class Logger;
    class LoggerManager;

    //日志级别
    class LogLevel{
    public:
        enum Level{
            UNKNOWN = 0,
            DEBUG = 1,
            INFO = 2,
            WARN = 3,
            ERROR = 4,
            FATAL = 5
        };

        static const char* toString(LogLevel::Level level);
        static LogLevel::Level FromString(const std::string& str);
    };

    //日志事件
    class LogEvent{
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, char* file, int32_t line, uint32_t elapse, uint32_t threadID, uint32_t fiberID, uint64_t time)
        :m_logger(logger), m_level(level),m_file(file), m_line(line), m_elapse(elapse), m_threadID(threadID), m_fiberID(fiberID), m_time(time) {}

        const char* getFile() const {return m_file;}
        int32_t getLine() const {return m_line;}
        uint32_t getElapse() const {return m_elapse;}
        uint32_t getThreadID() const {return m_threadID;}
        uint32_t getFiberID() const {return m_fiberID;}
        uint64_t getTime() const {return m_time;}
        const std::string getContent() const {return m_ss.str();}
        std::stringstream& getSS() { return m_ss;}
        std::shared_ptr<Logger> getLogger() const {return m_logger;}
        LogLevel::Level getLevel() const {return m_level;}

        void format(const char* fmt, ...);
        void format(const char* fmt, va_list al);

    private:
        const char* m_file = nullptr; //文件名
        int32_t m_line = 0; //行号
        uint32_t m_elapse = 0; //程序启动开始到现在的毫秒数
        uint32_t m_threadID = 0; //线程id
        uint32_t m_fiberID = 0; //协程id
        uint64_t m_time = 0; //时间戳
        std::string m_content;
        std::stringstream m_ss;

        std::shared_ptr<Logger> m_logger;
        LogLevel::Level m_level;
    };

    class LogEventWrap {
    public:
        LogEventWrap(LogEvent::ptr e);
        ~LogEventWrap();

        LogEvent::ptr getEvent() const {return m_event;}

        std::stringstream& getSS();
    private:
        LogEvent::ptr m_event;
    };

    //日志格式器
    class LogFormatter{
    public:
        typedef std::shared_ptr<LogFormatter> ptr;
        LogFormatter(const std::string& pattern);
        std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);

    public:
        class FormatItem{
        public:
            typedef std::shared_ptr<FormatItem> ptr;
            FormatItem(const std::string& fmt = "") {};
            //FormatItem() = default;
            virtual ~FormatItem() {}
            virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;

        };

        void init();
        
        bool isError() const {return m_error;}

        const std::string getPattern() const {return m_pattern;}
    private:
        std::string m_pattern;
        std::vector<FormatItem::ptr> m_log_formatter_items;
        bool m_error = false;
    };

    //日志输出地
    class LogAppender{
    public:
        typedef std::shared_ptr<LogAppender> ptr;
        virtual ~LogAppender() = default;

        virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
        virtual std::string toYamlString() = 0;

        void setLogFormatter(LogFormatter::ptr formatter);
        LogFormatter::ptr getLogFormatter();

        LogLevel::Level getLevel() const {return m_level;}
        void setLevel(LogLevel::Level level) {m_level = level;}
    protected:
        LogLevel::Level m_level = LogLevel::DEBUG;
        LogFormatter::ptr m_log_formatter;
    };

    //日志器
class Logger : public std::enable_shared_from_this<Logger>{
friend class LoggerManager;
    public:
        typedef std::shared_ptr<Logger> ptr;
        explicit Logger(const std::string & name="root");
        void log(LogLevel::Level level, LogEvent::ptr event);
        void debug(LogEvent::ptr event);
        void info(LogEvent::ptr event);
        void warn(LogEvent::ptr event);
        void error(LogEvent::ptr event);
        void fatal(LogEvent::ptr event);

        void addAppender(LogAppender::ptr appender);
        void delAppender(LogAppender::ptr appender);
        void clearAppenders();

        void setLevel(LogLevel::Level level);
        LogLevel::Level getLevel() const;

        const std::string getName() const {return m_name;}
        LogFormatter::ptr getFormatter() const {return m_formatter;}
        void setFormatter(LogFormatter::ptr val);
        void setFormatter(const std::string& val);
        
        std::string toYamlString();
    private:
        std::string m_name; //日志名称
        LogLevel::Level m_level; //日志级别
        std::vector<LogAppender::ptr> m_appender_list; //Appender集合
        LogFormatter::ptr  m_formatter;
        Logger::ptr m_root;
    };

    //输出到控制台的Appender
    class StdoutLogAppender : public LogAppender{
    public:
        typedef std::shared_ptr<StdoutLogAppender> ptr;
        void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event);
        std::string toYamlString() override;
    private:

    };

    //定义输出到文件的Appender
    class FileLogAppender : public LogAppender{
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;
        explicit FileLogAppender(std::string  filename);
        void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event);
        std::string toYamlString() override;
        bool reopen(); //如果文件打开，则先关闭再打开
    private:
        std::string m_filename;
        std::ofstream m_filestream;
    };

    class LoggerManager{
    public:
        LoggerManager();
        Logger::ptr getLogger(const std::string& name);

        void init();

        Logger::ptr getRoot() const {return m_root;}
        
        std::string toYamlString();
    private:
        std::map<std::string, Logger::ptr> m_loggers;
        Logger::ptr m_root;
    };

    typedef srvpro::Singleton<LoggerManager> LoggerMgr;
}


#endif //SERVERPROJECT_LOG_H
