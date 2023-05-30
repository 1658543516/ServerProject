//
// Created by mrpiao on 23-5-24.
//
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <functional>
#include "Log.h"

namespace srvpro{
    std::string_view LogFormatter::format(Logger::ptr logger, LogLevel::Level level,LogEvent::ptr event) {
        std::stringstream ss;
        for (auto& i : m_log_formatter_items) {
            i->format(ss, logger, level, event);
        }
        return ss.str();
    }

    class MessageFormatItem : public LogFormatter::FormatItem {
    public:
        MessageFormatItem(const std::string_view& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getContent();
        }
    };

    class LevelFormatItem : public LogFormatter::FormatItem {
    public:
        LevelFormatItem(const std::string_view& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << LogLevel::toString(level);
        }
    };

    class ElapseFormatItem : public LogFormatter::FormatItem {
    public:
        ElapseFormatItem(const std::string_view& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getElapse();
        }
    };

    class NameFormatItem : public LogFormatter::FormatItem {
    public:
        NameFormatItem(const std::string_view& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << logger->getName();
        }
    };

    class ThreadIDFormatItem : public LogFormatter::FormatItem {
    public:
        ThreadIDFormatItem(const std::string_view& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getThreadID();
        }
    };

    class DateTimeFormatItem : public LogFormatter::FormatItem {
    public:
        explicit DateTimeFormatItem(const std::string& format="%Y:%m:%d %H:%M:%S"):m_format(format) {
            if(m_format.empty()) {
                m_format = "%Y-%m-%d %H:%M:%S";
            }
        }
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            struct tm tm;
            time_t time = event->getTime();
            localtime_r(&time, &tm);
            char buf[64];
            strftime(buf, sizeof(buf), m_format.c_str(), &tm);
            os << buf;
        }
    private:
        std::string m_format;
    };

    class FilenameFormatItem : public LogFormatter::FormatItem {
    public:
        FilenameFormatItem(const std::string_view& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getFile();
        }
    };

    class LineFormatItem : public LogFormatter::FormatItem {
    public:
        LineFormatItem(const std::string_view& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getLine();
        }
    };

    class NewLineFormatItem : public LogFormatter::FormatItem {
    public:
        NewLineFormatItem(const std::string_view& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << std::endl;
        }
    };

    class StringFormatItem : public LogFormatter::FormatItem {
    public:
        explicit StringFormatItem(const std::string_view& str):m_string(str) {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << m_string;
        }
    private:
        std::string_view m_string;
    };

    Logger::Logger(const std::string_view &name):m_name(name), m_level(LogLevel::DEBUG) {
        //%d [%p] %f %l %m %n
        m_formatter.reset(new LogFormatter("%d  [%p] <%f:%l>   %m %n"));
    }

    void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
        if (level >= m_level) {
            auto self = shared_from_this();
            for (auto& it : m_appender_list) {
                it->log(self, level, event);
            }
        }
    }

    void Logger::debug(LogEvent::ptr event) {
        log(LogLevel::DEBUG, event);
    }

    void Logger::info(LogEvent::ptr event) {
        log(LogLevel::INFO, event);
    }

    void Logger::warn(LogEvent::ptr event) {
        log(LogLevel::WARN, event);
    }

    void Logger::error(LogEvent::ptr event) {
        log(LogLevel::ERROR, event);
    }

    void Logger::fatal(LogEvent::ptr event) {
        log(LogLevel::FATAL, event);
    }

    void Logger::addAppender(LogAppender::ptr appender) {
        m_appender_list.emplace_back(appender);
    }

    void Logger::delAppender(LogAppender::ptr appender) {
        for (auto it = m_appender_list.begin(); it != m_appender_list.end(); ++it) {
            if (*it == appender) {
                m_appender_list.erase(it);
                break;
            }
        }
    }

    void Logger::setLevel(LogLevel::Level level) {
        m_level = level;
    }

    LogLevel::Level Logger::getLevel() const{
        return m_level;
    }

    const char *LogLevel::toString(LogLevel::Level level) {
        switch (level) {
#define XX(name) \
        case LogLevel::name: \
        return #name;        \
        break;
            XX(DEBUG);
            XX(INFO);
            XX(WARN);
            XX(ERROR);
            XX(FATAL);
#undef XX
            default:
                return "UNKNOWN";

       }
       return "UNKNOWN";
    }

    void StdoutLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) {
        if (level >= m_level) {
            m_log_formatter->format(logger, level, event);
        }
    }

    FileLogAppender::FileLogAppender(std::string filename) : m_filename(std::move(filename)) {

    }

    void FileLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) {
        if (level >= m_level) {
            m_filestream << m_log_formatter->format(logger, level, event);
        }
    }

    bool FileLogAppender::reopen() {
        if (m_filestream) {
            m_filestream.close();
        }
        m_filestream.open(m_filename);

        return !!m_filestream;
    }

    void LogAppender::setLogFormatter(LogFormatter::ptr formatter) {
        m_log_formatter = formatter;
    }

    LogFormatter::ptr LogAppender::getLogFormatter() {
        return m_log_formatter;
    }

    LogFormatter::LogFormatter(const std::string& pattern):m_pattern(pattern) {
        init();
    }

    //日志输出格式:
    /*例如，%d - %m%n或%d{yyyy-MM-dd HH:mm:ss} %p [%c] %m%n
    %c 输出日志信息所属的类的全名
    %d 输出日志时间点的日期或时间，默认格式为ISO8601，也可以在其后指定格式，比如：%d{yyy-M-dd HH:mm:ss }，输出类似：2002-10-18- 22：10：28
    %f 输出日志信息所属的类的类名
    %l 输出日志事件的发生位置，即输出日志信息的语句处于它所在的类的第几行
    %m 输出代码中指定的信息，如log(message)中的message
    %n 输出一个回车换行符，Windows平台为“rn”，Unix平台为“n”
    %p 输出优先级，即DEBUG，INFO，WARN，ERROR，FATAL。如果是调用debug()输出的，则为DEBUG，依此类推
    %r 输出自应用启动到输出该日志信息所耗费的毫秒数
    %t 输出产生该日志事件的线程名*/
    void LogFormatter::init() {
        // str, format, type
        std::vector<std::tuple<std::string, std::string, int>> format_vec;
        std::string nstr;
        // "%d [%p] %f %l %m %n""%d  [ %p]  %f %l %m %n"
        int index = 0;
        while(index < m_pattern.size()) {
            if(m_pattern[index] == '%' && m_pattern[index + 1] != '%') {
                m_pattern.insert(index+2, " ");
                index += 2;
            }
            ++index;
        }

        for (size_t i = 0; i < m_pattern.size(); ++i) {
            if (m_pattern[i] != '%') {
                nstr.append(1, m_pattern[i]);
                continue;
            }

            if (i + 1 < m_pattern.size()) {
                if (m_pattern[i + 1] == '%') {
                    nstr.append(1, '%');
                    continue;
                }
            }

            size_t n = i + 1;
            int fmt_status = 0;
            size_t fmt_begin = 0;

            std::string fmt, str;

            while (n < m_pattern.size()) {
                if (isspace(m_pattern[n])) {
                    break;
                }

                if (fmt_status == 0) {
                    if (m_pattern[n] == '(') {
                        str = m_pattern.substr(i + 1, n - i - 1);
                        fmt_status = 1;
                        fmt_begin = n;
                        ++n;
                        continue;
                    }
                }

                if (fmt_status == 1) {
                    if (m_pattern[n] == '{') {
                        fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                        fmt_status = 2;
                        break;
                    }
                }
                ++n;
            }

            if (fmt_status == 0) {
                if (!nstr.empty()) {
                    format_vec.emplace_back(nstr, "", 0);
                }
                str = m_pattern.substr(i + 1, n - i - 1);
                format_vec.emplace_back(str, fmt, 1);
                i = n;
                nstr.clear();
            }
            else if (fmt_status == 1) {
                std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
                format_vec.emplace_back("<<pattern error>>", fmt, 0);
            }
            else if (fmt_status == 2) {
                if (!nstr.empty()) {
                    format_vec.emplace_back(nstr, std::string(), 0);
                }
                format_vec.emplace_back(str, fmt, 1);
                i = n;
                nstr.clear();
            }

        }
        if (!nstr.empty()) {
            format_vec.emplace_back(nstr, "", 0);
        }

        static std::map<std::string_view, std::function<FormatItem::ptr(const std::string_view& str)>> s_format_items = {
#define XX(str, C) \
                {#str, [](const std::string_view& fmt){return FormatItem::ptr(new C(fmt));}}

                XX(m, MessageFormatItem),
                XX(p, LevelFormatItem),
                XX(r, ElapseFormatItem),
                XX(c, NameFormatItem),
                XX(t, ThreadIDFormatItem),
                XX(n, NewLineFormatItem),
                XX(d, DateTimeFormatItem),
                XX(f, FilenameFormatItem),
                XX(l, LineFormatItem),

#undef XX
        };

        for (auto& i : format_vec) {
            if (std::get<2>(i) == 0) {
                m_log_formatter_items.emplace_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
            }
            else {
                auto it = s_format_items.find(std::get<0>(i));
                if (it == s_format_items.end()) {
                    m_log_formatter_items.emplace_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
                }
                else {
                    m_log_formatter_items.emplace_back(it->second(std::get<1>(i)));
                }
            }
            std::cout << "(" << std::get<0>(i) << ") - (" << std::get<1>(i) << ") - (" << std::get<2>(i) << ")" << std::endl;
        }
    }


}

