#include <iostream>
#include "Log.h"
#include <thread>
#include "util.h"
#include "config.h"
#include <unistd.h>
#include <vector>
#include <yaml-cpp/yaml.h>

srvpro::ConfigVar<int>::ptr g_int_value_config = srvpro::Config::Lookup("system.port", (int)8080, "system port");

srvpro::ConfigVar<std::vector<int> >::ptr g_vec_value_config = srvpro::Config::Lookup("system.int_vec", std::vector<int>(1, 2), "system int vec");

void print_yaml(const YAML::Node& node, int level) {
    if (node.IsScalar()) {
        SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << std::string(level * 4, ' ') << node.Scalar() << " - " << node.Type() << " - " << level;
    }
    else if(node.IsNull()) {
        SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << std::string(level * 4, ' ') << "NULL - " << node.Type() << " - " << level;
    }
    else if(node.IsMap()) {
        for (auto it = node.begin(); it != node.end() ; ++it) {
            SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << std::string(level * 4, ' ') << it->first << " - " << it->second.Type() << " - " << level;
            print_yaml(it->second, level + 1);
        }
    }
    else if(node.IsSequence()) {
        for (size_t i = 0; i < node.size() ; ++i) {
            SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << std::string(level * 4, ' ') << i << " - " << node[i].Type() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }
}

void test_yaml() {
    YAML::Node root = YAML::LoadFile("../conf/log.yml");
    print_yaml(root, 0);
    //SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << root;
}

void test_config() {
    SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << "before: " << g_int_value_config->getValue();
    //SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << "before: " << g_int_value_config->toString();
    auto v = g_vec_value_config->getValue();
    for(auto& i : v) {
        SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << "before in_vec: " << i;
    }

    YAML::Node root = YAML::LoadFile("../conf/log.yml");
    srvpro::Config::LoadFromYaml(root);

    //SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << "after: " << g_int_value_config->getValue();
    SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << "after: " << g_int_value_config->getValue();
    v = g_vec_value_config->getValue();
    for(auto& i : v) {
        SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << "after in_vec: " << i;
    }

}

int main() {
    std::cout << "Hello SrvPro" << std::endl;
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

    SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << g_int_value_config->getValue();
    SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << g_int_value_config->toString();

    //test_yaml();
    test_config();
    return 0;
}
