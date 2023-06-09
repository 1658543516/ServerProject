//
// Created by mrpiao on 23-5-24.
//
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <functional>
#include <stdarg.h>
#include "Log.h"
#include "config.h"

namespace srvpro{


    class MessageFormatItem : public LogFormatter::FormatItem {
    public:
        MessageFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getContent();
        }
    };

    class LevelFormatItem : public LogFormatter::FormatItem {
    public:
        LevelFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << LogLevel::toString(level);
        }
    };

    class ElapseFormatItem : public LogFormatter::FormatItem {
    public:
        ElapseFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getElapse();
        }
    };

    class NameFormatItem : public LogFormatter::FormatItem {
    public:
        NameFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getLogger()->getName();
        }
    };

    class ThreadIDFormatItem : public LogFormatter::FormatItem {
    public:
        ThreadIDFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getThreadID();
        }
    };

    class FiberIDFormatItem : public LogFormatter::FormatItem {
    public:
        FiberIDFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getFiberID();
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
        FilenameFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getFile();
        }
    };

    class LineFormatItem : public LogFormatter::FormatItem {
    public:
        LineFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getLine();
        }
    };

    class NewLineFormatItem : public LogFormatter::FormatItem {
    public:
        NewLineFormatItem(const std::string& str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << std::endl;
        }
    };

    class StringFormatItem : public LogFormatter::FormatItem {
    public:
        explicit StringFormatItem(const std::string& str):m_string(str) {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << m_string;
        }
    private:
        std::string m_string;
    };

    class TabFormatItem : public LogFormatter::FormatItem {
    public:
        explicit TabFormatItem(const std::string& str = ""):m_string(str) {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << "\t";
        }
    private:
        std::string m_string;
    };



    Logger::Logger(const std::string &name):m_name(name), m_level(LogLevel::DEBUG) {
        //%d [%p] %f %l %m %n
        m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
    }
    
    void Logger::setFormatter(LogFormatter::ptr val) {
    	m_formatter = val;
    }
    
    void Logger::setFormatter(const std::string& val) {
    	srvpro::LogFormatter::ptr new_val(new srvpro::LogFormatter(val));
    	if (new_val->isError()) {
    		std::cout << "Logger setFormatter name=" << m_name << " value=" << val << " invalid formatter" << std::endl;
    		return;
    	}
    	m_formatter = new_val;
    }
    
    std::string Logger::toYamlString() {
        YAML::Node node;
        node["name"] = m_name;
        if(m_level != LogLevel::UNKNOWN) {
        	node["level"] = LogLevel::toString(m_level);
        }
        
        if(m_formatter) {
        	node["formatter"] = m_formatter->getPattern();
        }
        
        for(auto& i : m_appender_list) {
        	node["appenders"].push_back(YAML::Load(i->toYamlString()));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
        if (level >= m_level) {
            auto self = shared_from_this();
            if(!m_appender_list.empty()) {
            	for (auto& it : m_appender_list) {
		        it->log(self, level, event);
		    }
            } 
            else if(m_root) {
            	m_root->log(level, event);
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
    
    void Logger::clearAppenders() {
    	m_appender_list.clear();
    }

    void Logger::setLevel(LogLevel::Level level) {
        m_level = level;
    }

    LogLevel::Level Logger::getLevel() const{
        return m_level;
    }

    const char* LogLevel::toString(LogLevel::Level level) {
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
    
    LogLevel::Level LogLevel::FromString(const std::string& str) {
    #define XX(level, v) \
    	if(str == #v) { \
    		return LogLevel::level; \
    	}
    	XX(DEBUG, debug);
        XX(INFO, info);
        XX(WARN, warn);
        XX(ERROR, error);
        XX(FATAL, fatal);
        
        XX(DEBUG, DEBUG);
        XX(INFO, INFO);
        XX(WARN, WARN);
        XX(ERROR, ERROR);
        XX(FATAL, FATAL);
        return LogLevel::UNKNOWN;
    #undef XX
    
    }

    LogEventWrap::LogEventWrap(LogEvent::ptr e):m_event(e) {

    }

    LogEventWrap::~LogEventWrap() {
        m_event->getLogger()->log(m_event->getLevel(), m_event);
    }

    void LogEvent::format(const char *fmt, ...) {
        va_list al;
        va_start(al, fmt);
        format(fmt, al);
        va_end(al);
    }

    void LogEvent::format(const char *fmt, va_list al) {
        char* buf = nullptr;
        int len = vasprintf(&buf, fmt, al);
        if (len != -1) {
            m_ss << std::string(buf, len);
            free(buf);
        }
    }

    std::stringstream &LogEventWrap::getSS() {
        return m_event->getSS();
    }

    void StdoutLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) {
        if (level >= m_level) {
            std::cout << m_log_formatter->format(logger, level, event);
        }
    }
    
    std::string StdoutLogAppender::toYamlString() {
        YAML::Node node;
        node["type"] = "StdoutLogAppender";
        if(m_level != LogLevel::UNKNOWN) {
        	node["level"] = LogLevel::toString(m_level);
        }
        //node["level"] = LogLevel::toString(m_level);
        if(m_log_formatter) {
        	node["formatter"] = m_log_formatter->getPattern();
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    FileLogAppender::FileLogAppender(std::string filename) : m_filename(std::move(filename)) {

    }

    void FileLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) {
        if (level >= m_level) {
            std::fstream file(m_filename);
            reopen();
                //std::cout << true << std::endl;
            std::cout << m_filename << std::endl;
                m_filestream << m_log_formatter->format(logger, level, event);
                //m_filestream.close();



        }
    }
    
    std::string FileLogAppender::toYamlString() {
        YAML::Node node;
        node["type"] = "FileLogAppender";
        node["file"] = m_filename;
        if(m_level != LogLevel::UNKNOWN) {
        	node["level"] = LogLevel::toString(m_level);
        }
        //node["level"] = LogLevel::toString(m_level);
        if(m_log_formatter) {
        	node["formatter"] = m_log_formatter->getPattern();
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    bool FileLogAppender::reopen() {
        if (m_filestream) {
            m_filestream.close();
        }

        m_filestream.open(m_filename, std::ios::out|std::ios::app);
        /*if (m_filestream.is_open()) {
            std::cout << "ooo" << std::endl;
            m_filestream << 123;
            m_filestream.close();
        }*/
        //m_filestream << 123;
        //if (m_filestream.is_open()) std::cout<<"open"<<std::endl;

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

    std::string LogFormatter::format(Logger::ptr logger, LogLevel::Level level,LogEvent::ptr event) {
        std::stringstream ss;

        //std::cout << m_log_formatter_items[0] << std::endl;

        for (auto& i : logger->getFormatter()->m_log_formatter_items) {
            i->format(ss, logger, level, event);
        }
        return ss.str();
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
        // int index = 0;
        /*while(index < m_pattern.size()) {
            if(m_pattern[index] == '%' && m_pattern[index + 1] != '%') {
                m_pattern.insert(index+2, " ");
                index += 2;
            }
            ++index;
        }*/

        for (size_t i = 0; i < m_pattern.size(); ++i) {
            if(m_pattern[i] != '%') {
                nstr.append(1, m_pattern[i]);
                continue;
            }

            if((i + 1) < m_pattern.size()) {
                if(m_pattern[i + 1] == '%') {
                    nstr.append(1, '%');
                    continue;
                }
            }

            size_t n = i + 1;
            int fmt_status = 0;
            size_t fmt_begin = 0;

            std::string str;
            std::string fmt;
            while(n < m_pattern.size()) {
                if(!fmt_status && (!isalpha(m_pattern[n]) && m_pattern[n] != '{'
                                   && m_pattern[n] != '}')) {
                    str = m_pattern.substr(i + 1, n - i - 1);
                    break;
                }
                if(fmt_status == 0) {
                    if(m_pattern[n] == '{') {
                        str = m_pattern.substr(i + 1, n - i - 1);
                        //std::cout << "*" << str << std::endl;
                        fmt_status = 1; //解析格式
                        fmt_begin = n;
                        ++n;
                        continue;
                    }
                } else if(fmt_status == 1) {
                    if(m_pattern[n] == '}') {
                        fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                        //std::cout << "#" << fmt << std::endl;
                        fmt_status = 0;
                        ++n;
                        break;
                    }
                }
                ++n;
                if(n == m_pattern.size()) {
                    if(str.empty()) {
                        str = m_pattern.substr(i + 1);
                    }
                }
            }

            if(fmt_status == 0) {
                if(!nstr.empty()) {
                    format_vec.emplace_back(std::make_tuple(nstr, std::string(), 0));
                    nstr.clear();
                }
                format_vec.emplace_back(std::make_tuple(str, fmt, 1));
                i = n - 1;
            } else if(fmt_status == 1) {
                std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
                //m_error = true;
                format_vec.emplace_back(std::make_tuple("<<pattern_error>>", fmt, 0));
            }
        }
        if (!nstr.empty()) {
            format_vec.emplace_back(nstr, "", 0);
        }

        static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)>> s_format_items = {
#define XX(str, C) \
                {#str, [](const std::string& fmt){return FormatItem::ptr(new C(fmt));}}

                XX(m, MessageFormatItem),
                XX(p, LevelFormatItem),
                XX(r, ElapseFormatItem),
                XX(c, NameFormatItem),
                XX(t, ThreadIDFormatItem),
                XX(n, NewLineFormatItem),
                XX(d, DateTimeFormatItem),
                XX(f, FilenameFormatItem),
                XX(l, LineFormatItem),
                XX(T, TabFormatItem),
                XX(F, FiberIDFormatItem)
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
                    m_error = true;
                }
                else {
                    m_log_formatter_items.emplace_back(it->second(std::get<1>(i)));
                }
            }
            //std::cout << "(" << std::get<0>(i) << ") - (" << std::get<1>(i) << ") - (" << std::get<2>(i) << ")" << std::endl;
        }

    }

    LoggerManager::LoggerManager() {
        m_root.reset(new Logger);
        m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));
        
        m_loggers[m_root->m_name] = m_root;
        
        init();
    }

    Logger::ptr LoggerManager::getLogger(const std::string &name) {
        auto it = m_loggers.find(name);
        if (it != m_loggers.end()) {
        	return it->second;
        }
        Logger::ptr logger(new Logger(name));
        logger->m_root = m_root;
        m_loggers[name] = logger;
        return logger;
        //return it == m_loggers.end() ? m_root : it->second;
    }
    
    struct LogAppenderDefine {
    	int type = 0; // 1 is File, 2 is StdOut
    	LogLevel::Level level = LogLevel::UNKNOWN;
    	std::string formatter;
    	std::string file;
    	
    	bool operator==(const LogAppenderDefine& other) const {
    		return type == other.type && level == other.level && formatter == other.formatter && file == other.file;
    	}
    	
    };
    
    struct LogDefine {
    	std::string name;
    	LogLevel::Level level = LogLevel::UNKNOWN;
    	std::string formatter;
    	std::vector<LogAppenderDefine> appenders;
    	
    	bool operator==(const LogDefine& other) const {
    		return name == other.name && level == other.level && formatter == other.formatter && appenders == appenders;
    	}
    	
    	bool operator<(const LogDefine& other) const {
    		return name < other.name;
    	}
    };
    
    template<>
    class LexicalCast<std::string, std::set<LogDefine> > {
    public:
        std::set<LogDefine> operator()(const std::string& v) {
            YAML::Node node = YAML::Load(v);
            std::stringstream  ss;
            std::set<LogDefine> vec;
            for(size_t i = 0; i < node.size(); ++i) {
                auto n = node[i];
                if(!n["name"].IsDefined()) {
                	std::cout << "log config error: name is null, " << n << std::endl;
                	continue;
                }
                
                LogDefine ld;
                ld.name = n["name"].as<std::string>();
                ld.level = LogLevel::FromString(n["level"].IsDefined() ? n["level"].as<std::string>() : "");
                if(n["formatter"].IsDefined()) {
                	ld.formatter = n["formatter"].as<std::string>();
                }
                
                if(n["appenders"].IsDefined()) {
                	for(size_t x = 0; x < n["appenders"].size(); ++x) {
                		auto a = n["appenders"][x];
                		if(!a["type"].IsDefined()) {
                			std::cout << "log config error: appender type is null, " << n << std::endl;
                			continue;
                		}
                		std::string type = a["type"].as<std::string>();
                		LogAppenderDefine lad;
                		if(type == "FileLogAppender") {
                			lad.type = 1;
                			if(!a["file"].IsDefined()) {
                				std::cout << "log config error: fileappender file is null, " << n << std::endl;
                				continue;
                			}
                			lad.file = a["file"].as<std::string>();
                			if(a["formatter"].IsDefined()) {
                				lad.formatter = a["formatter"].as<std::string>();
                			}
                		}
                		else if(type == "StdoutLogAppender") {
                			lad.type = 2; 
                		}
                		else {
                			std::cout << "log config error: appender type is invalid, " << n << std::endl;
                			continue;
                		}
                		
                		ld.appenders.push_back(lad);
                	}
                }
                vec.insert(ld);
            }
            return vec;
        }
    };

    template<>
    class LexicalCast<std::set<LogDefine>, std::string> {
    public:
        std::string operator()(const std::set<LogDefine>& v) {
            YAML::Node node;
            for(auto& i : v) {
                YAML::Node n;
                n["name"] = i.name;
                if(i.level != LogLevel::UNKNOWN) {
                	n["level"] = LogLevel::toString(i.level);
                }
                
                if(i.formatter.empty()) {
		    	n["formatter"] = i.formatter;
		}
		    
		for(auto& a : i.appenders) {
		    YAML::Node na;
		    if(a.type == 1) {
		   	na["type"] = "FileLogAppender";
		    	na["file"] = a.file;
		    }
		    else if(a.type == 2) {
		    	na["type"] = "StdoutLogAppender";
		    }
		    
		    if(a.level != LogLevel::UNKNOWN) {
		    	na["level"] = LogLevel::toString(a.level);
		    }
		    	
		    if(!a.formatter.empty()) {
		    	na["formatter"] = a.formatter;
		    }
		    	
		    n["appenders"].push_back(na);
		}
		
		node.push_back(n);
            }
            
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };
    
    srvpro::ConfigVar<std::set<LogDefine> >::ptr g_log_defines = srvpro::Config::Lookup("logs", std::set<LogDefine>(), "logs config");
    
    struct LogIniter {
    	LogIniter() {
    		g_log_defines->addListener(0xF1E231, [](const std::set<LogDefine>& old_value, const std::set<LogDefine>& new_value){
    			SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << "on_logger_conf_changed";
    			//insert
	    		for(auto& i : new_value) {
	    			auto it = old_value.find(i);
	    			srvpro::Logger::ptr logger;
	    			if(it == old_value.end()) {
	    				//insert new logger
	    				/*logger.reset(new srvpro::Logger(i.name));
	    				logger->setLevel(i.level);
	    				if(!i.formatter.empty()) {
	    					logger->setFormatter(i.formatter);
	    				}
	    				logger->clearAppenders();
	    				for(auto& a : i.appenders) {
	    					srvpro::LogAppender::ptr ap;
	    					if(a.type == 1) {
	    						ap.reset(new FileLogAppender(a.file));
	    					}
	    					else if(a.type == 2) {
	    						ap.reset(new StdoutLogAppender);
	    					}
	    					ap->setLevel(a.level);
	    					logger->addAppender(ap);
	    				} */
	    				logger = SRVPRO_LOG_NAME(i.name);
	    			}
	    			else {
	    				if(!(i == *it)) {
	    				//update old logger
	    					logger = SRVPRO_LOG_NAME(i.name);
	    				}
	    			}
	    			
	    			logger->setLevel(i.level);
	    			if(!i.formatter.empty()) {
	    				logger->setFormatter(i.formatter);
	    			}
	    			logger->clearAppenders();
	    			for(auto& a : i.appenders) {
	    				srvpro::LogAppender::ptr ap;
	    				if(a.type == 1) {
	    					ap.reset(new FileLogAppender(a.file));
	    				}
	    				else if(a.type == 2) {
	    					ap.reset(new StdoutLogAppender);
	    				}
	    				ap->setLevel(a.level);
	    				logger->addAppender(ap);
	    			} 
	    		}
	    		
	    		for(auto& i : old_value) {
	    			auto it = new_value.find(i);
	    			if(it == new_value.end()) {
	    				//delete old logger
	    				auto logger = SRVPRO_LOG_NAME(i.name);
	    				logger->setLevel((LogLevel::Level)100);
	    				logger->clearAppenders();
	    			}
	    			/*else {
	    				if(!(i == *it) {
	    				//update old logger
	    				}
	    			}*/
	    		}
    		});
    	}
    };
    
    static LogIniter __log_init;
    
    std::string LoggerManager::toYamlString() {
    	YAML::Node node;
    	for(auto& i : m_loggers) {
    		node.push_back(YAML::Load(i.second->toYamlString()));
    	}
    	std::stringstream ss;
    	ss << node;
    	return ss.str();
    }
    
    void LoggerManager::init() {
    	
    }
}

