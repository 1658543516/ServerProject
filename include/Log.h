//
// Created by mrpiao on 23-5-23.
//

#ifndef SERVERPROJECT_LOG_H
#define SERVERPROJECT_LOG_H
#include <memory>
#include <string>
#include <list>
#include <sstream>
#include <fstream>

namespace srvpro{

    //日志事件
    class LogEvent{
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        LogEvent();
    private:
        const char* m_file = nullptr; //文件名
        int32_t m_line = 0; //行号
        uint32_t m_elapse = 0; //程序启动开始到现在的毫秒数
        uint32_t m_threadID = 0; //线程id
        uint32_t m_fiberID = 0; //协程id
        uint64_t m_time = 0; //时间戳
        std::string_view m_content;
    };

    //日志级别
    class LogLevel{
    public:
        enum Level{
            DEBUG = 1,
            INFO = 2,
            WARN = 3,
            ERROR = 4,
            FATAL = 5
        };
    };

    //日志格式器
    class LogFormatter{
    public:
        typedef std::shared_ptr<LogFormatter> ptr;
        std::string_view format(LogEvent::ptr event);
    private:
        class FormatItem{
        public:
            typedef std::shared_ptr<FormatItem> ptr;
            FormatItem() = default;
            virtual ~FormatItem() {}
            virtual std::string_view format(LogEvent::ptr event);
        };
    private:
        std::string_view m_pattern;
        std::list<FormatItem::ptr> m_log_formatter_items;
    };

    //日志输出地
    class LogAppender{
    public:
        typedef std::shared_ptr<LogAppender> ptr;
        virtual ~LogAppender() = default;

        virtual void log(LogLevel::Level level, LogEvent::ptr event) = 0;

        void setLogFormatter(LogFormatter::ptr formatter);
        LogFormatter::ptr getLogFormatter();
    protected:
        LogLevel::Level m_level;
        LogFormatter::ptr m_log_formatter;
    };

    //日志器
    class Logger{
    public:
        typedef std::shared_ptr<Logger> ptr;
        explicit Logger(const std::string_view & name="root");
        void log(LogLevel::Level level, LogEvent::ptr event);
        void debug(LogEvent::ptr event);
        void info(LogEvent::ptr event);
        void warn(LogEvent::ptr event);
        void error(LogEvent::ptr event);
        void fatal(LogEvent::ptr event);

        void addAppender(LogAppender::ptr appender);
        void delAppender(LogAppender::ptr appender);

        void setLevel(LogLevel::Level level);
        LogLevel::Level getLevel() const;

    private:
        std::string_view m_name; //日志名称
        LogLevel::Level m_level; //日志级别
        std::list<LogAppender::ptr> m_appender_list; //Appender集合

    };

    //输出到控制台的Appender
    class StdoutLogAppender : public LogAppender{
    public:
        typedef std::shared_ptr<StdoutLogAppender> ptr;
        void log(LogLevel::Level level, LogEvent::ptr event) override;
    private:

    };

    //定义输出到文件的Appender
    class FileLogAppender : public LogAppender{
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;
        explicit FileLogAppender(std::string  filename);
        void log(LogLevel::Level level, LogEvent::ptr event) override;
        bool reopen(); //如果文件打开，则先关闭再打开
    private:
        std::string m_filename;
        std::ofstream m_filestream;
    };
}


#endif //SERVERPROJECT_LOG_H
