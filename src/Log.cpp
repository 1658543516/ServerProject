//
// Created by mrpiao on 23-5-24.
//
#include <iostream>
#include <string>
#include <utility>
#include "Log.h"

namespace srvpro{
    Logger::Logger(const std::string_view &name):m_name(name) {

    }

    void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
        if (level >= m_level) {
            for (auto& it : m_appender_list) {
                it->log(level, event);
            }
        }
    }

    void Logger::debug(LogEvent::ptr event) {
        //debug(LogLevel::Level, event);
    }

    void Logger::info(LogEvent::ptr event) {

    }

    void Logger::warn(LogEvent::ptr event) {

    }

    void Logger::error(LogEvent::ptr event) {

    }

    void Logger::fatal(LogEvent::ptr event) {

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

    void StdoutLogAppender::log(LogLevel::Level level, LogEvent::ptr event) {
        if (level >= m_level) {
            m_log_formatter->format(event);
        }
    }

    FileLogAppender::FileLogAppender(std::string filename) : m_filename(std::move(filename)) {

    }

    void FileLogAppender::log(LogLevel::Level level, LogEvent::ptr event) {
        if (level >= m_level) {
            m_filestream << m_log_formatter->format(event);
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
}

