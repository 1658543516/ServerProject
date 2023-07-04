#include <iostream>
#include "srvpro.h"
#include <memory>
#include <thread>
#include <unistd.h>
#include <vector>
#include <yaml-cpp/yaml.h>
#include "iomanager.h"
#include "assert.h"

/*srvpro::ConfigVar<int>::ptr g_int_value_config = srvpro::Config::Lookup("system.port", (int)8080, "system port");

srvpro::ConfigVar<std::vector<int> >::ptr g_vec_value_config = srvpro::Config::Lookup("system.int_vec", std::vector<int>(1, 2), "system int vec");

srvpro::ConfigVar<std::list<int> >::ptr g_list_value_config = srvpro::Config::Lookup("system.int_list", std::list<int>(1, 2), "system int list");

srvpro::ConfigVar<std::set<int> >::ptr g_set_value_config = srvpro::Config::Lookup("system.int_set", std::set<int>{1, 2}, "system int set");

srvpro::ConfigVar<std::unordered_set<int> >::ptr g_uset_value_config = srvpro::Config::Lookup("system.int_uset", std::unordered_set<int>{1, 2}, "system int uset");

srvpro::ConfigVar<std::map<std::string, int> >::ptr g_map_value_config = srvpro::Config::Lookup("system.int_map", std::map<std::string, int>{{"k", 2}}, "system int map");

srvpro::ConfigVar<std::unordered_map<std::string, int> >::ptr g_umap_value_config = srvpro::Config::Lookup("system.int_umap", std::unordered_map<std::string, int>{{"k", 2}}, "system int umap");

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
    YAML::Node root = YAML::LoadFile("../conf/test.yml");
    print_yaml(root, 0);
    //SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << root;
}

void test_config() {
    SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << "before: " << g_int_value_config->getValue();
    //SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << "before: " << g_int_value_config->toString();

#define XX(g_var, name, prefix) \
    {\
    	auto& v = g_var->getValue(); \
	for(auto& i : v) { \
		SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << #prefix " " #name ": " << i; \
	} \
	SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \   
    }

#define XX_M(g_var, name, prefix) \
    {\
        auto& v = g_var->getValue(); \
        for(auto& i : v) { \
                SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << #prefix " " #name ": {" << i.first << " - " << i.second << "} "; \
        } \
        SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \   
    }

    //auto v = g_vec_value_config->getValue();
    //for(auto& i : v) {
    //    SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << "before in_vec: " << i;
    //}
    XX(g_vec_value_config, int_vec, before);
    XX(g_list_value_config, int_list, before);
    XX(g_set_value_config, int_set, before);
    XX(g_uset_value_config, int_uset, before);
    XX_M(g_map_value_config, int_map, before);
    XX_M(g_umap_value_config, int_umap, before);


    YAML::Node root = YAML::LoadFile("../conf/test.yml");
    srvpro::Config::LoadFromYaml(root);

    //SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << "after: " << g_int_value_config->getValue();
    SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << "after: " << g_int_value_config->getValue();
    //v = g_vec_value_config->getValue();
    //for(auto& i : v) {
    //    SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << "after in_vec: " << i;
    //}
    XX(g_vec_value_config, int_vec, after);
    XX(g_list_value_config, list_vec, after);
    XX(g_set_value_config, set_vec, after);
    XX(g_uset_value_config, uset_vec, after);
    XX_M(g_map_value_config, map_vec, after);
    XX_M(g_umap_value_config, umap_vec, after);

}

class Person {
	public:
		std::string m_name = "";
		int m_age = 0;
		bool m_sex = 0;
		std::string toString() const {
			std::stringstream ss;
			ss << "[Person name=" << m_name << " age=" << m_age << " sex=" << m_sex << "]";
			return ss.str();
		}

		bool operator==(const Person& other) const {
			return m_name == other.m_name && m_age == other.m_age && m_sex == other.m_sex;
		}
};

namespace srvpro {
	template<>
	class LexicalCast<std::string, Person> {
		public:
			Person operator()(const std::string& v) {
				YAML::Node node = YAML::Load(v);
				//std::stringstream ss;
				Person person;
				person.m_name = node["name"].as<std::string>();
				person.m_age = node["age"].as<int>();
				person.m_sex = node["sex"].as<bool>();
				return person;
			}
	};

	template<>
	class LexicalCast<Person, std::string> {
		public:
			std::string operator()(const Person& person) {
				YAML::Node node;
				node["name"] = person.m_name;
				node["age"] = person.m_age;
				node["sex"] = person.m_sex;
				std::stringstream ss;
				ss << node;
				return ss.str();
			}
	};
}

srvpro::ConfigVar<Person>::ptr g_person = srvpro::Config::Lookup("class.person", Person(), "system person");

srvpro::ConfigVar<std::map<std::string, Person> >::ptr g_person_map = srvpro::Config::Lookup("class.map", std::map<std::string, Person>(), "system person");

srvpro::ConfigVar<std::map<std::string, std::vector<Person> > >::ptr g_person_vec_map = srvpro::Config::Lookup("class.vec_map", std::map<std::string, std::vector<Person> >(), "system person");

void test_class() {
	SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << "before: " << g_person->getValue().toString() << " - " << g_person->toString();
#define XX_PM(g_var, prefix) \
	{ \
		auto m = g_person_map->getValue(); \
		for(auto& i : m) { \
			SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << prefix << ": " << i.first << " - " << i.second.toString(); \
		} \
		SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << prefix << ": size=" << m.size(); \
	}

	g_person->addListener(1, [](const Person& old_value, const Person& new_value){
				SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << "old_value = " << old_value.toString() << " new_value = " << new_value.toString();
			});

	XX_PM(g_person_map, "class.map, before");
	SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << "before: " << g_person_vec_map->toString();

	YAML::Node root = YAML::LoadFile("../conf/test.yml");
   	srvpro::Config::LoadFromYaml(root);

	SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << "after: " << g_person->getValue().toString() << " - " << g_person->toString();
        XX_PM(g_person_map, "class.map, after");    
	SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << "after: " << g_person_vec_map->toString();
}*/

void test_log() {
	static srvpro::Logger::ptr system_log = SRVPRO_LOG_NAME("system");
	SRVPRO_LOG_INFO(system_log) << "hello system" << std::endl;
	std::cout << srvpro::LoggerMgr::GetInstance()->toYamlString() << std::endl; 
	YAML::Node root = YAML::LoadFile("../conf/log.yml");
   	srvpro::Config::LoadFromYaml(root);
   	std::cout << "============================" << std::endl;
   	std::cout << srvpro::LoggerMgr::GetInstance()->toYamlString() << std::endl; 
   	std::cout << "============================" << std::endl;
   	std::cout << root << std::endl; 
   	SRVPRO_LOG_INFO(system_log) << "hello system" << std::endl;
   	
   	//system_log->setFormatter("%d - %m%n");
   	//SRVPRO_LOG_INFO(system_log) << "hello system" << std::endl;
}

int count = 0;
srvpro::Mutex s_mutex;

srvpro::Logger::ptr g_logger = SRVPRO_LOG_ROOT();

void fun1() {
    SRVPRO_LOG_INFO(g_logger) << "name: " << srvpro::Thread::GetName() 
                              << " this.name: " << srvpro::Thread::GetThis()->GetName()
                              << " id:" << srvpro::GetThreadID()
                              << " this.id:" << srvpro::Thread::GetThis()->getId();
                              
    for(int i = 0; i < 100000; ++i) {
        srvpro::Mutex::Lock lock1(s_mutex);
    	++count;
    }
}

void fun2() {
    while(true) {
    	SRVPRO_LOG_INFO(g_logger) << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    }
}

void fun3() {
    while(true) {
    	SRVPRO_LOG_INFO(g_logger) << "================================";
    }
}

void test_assert() {
    srvpro::Logger::ptr g_logger = SRVPRO_LOG_ROOT();
    SRVPRO_LOG_INFO(g_logger) << srvpro::BacktraceToString(10);
    SRVPRO_ASSERT2(0 == 1, "abcdef xx");
}

void run_in_fiber() {
    SRVPRO_LOG_INFO(g_logger) << "run in fiber begin";
    srvpro::Fiber::YieldToHold();
    SRVPRO_LOG_INFO(g_logger) << "run in fiber end";
    srvpro::Fiber::YieldToHold();
}

void test_fiber() {
    SRVPRO_LOG_INFO(g_logger) << "main begin -1";
    {
    	srvpro::Fiber::GetThis();
    	SRVPRO_LOG_INFO(g_logger) << "main begin";
    	srvpro::Fiber::ptr fiber(new srvpro::Fiber(run_in_fiber));
    	fiber->swapIn();
    	SRVPRO_LOG_INFO(g_logger) << "main after swapIn";
    	fiber->swapIn();
    	SRVPRO_LOG_INFO(g_logger) << "main after end";
    	fiber->swapIn();
    }
    SRVPRO_LOG_INFO(g_logger) << "main after end2";
}

void test_fiber_scheduler() {
    static int s_count = 5;
    SRVPRO_LOG_INFO(g_logger) << "test in fiber s_count=" << s_count;

    sleep(1);
    if(--s_count >= 0) {
        srvpro::Scheduler::GetThis()->schedule(&test_fiber_scheduler, srvpro::GetThreadID());
    }
}

void test_iomanager_fiber() {
    SRVPRO_LOG_INFO(g_logger) << "test_fiber";
}

void test_iomanager1() {
    srvpro::IOManager iom;
    iom.schedule(&test_iomanager_fiber);
}

int main() {
    std::cout << "Hello SrvPro" << std::endl;
    /*srvpro::Logger::ptr logger(new srvpro::Logger);
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
    SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << g_int_value_config->toString();*/

    //test_yaml();
    //test_config();
    //test_class();
    //test_log();
    
    /*SRVPRO_LOG_INFO(g_logger) << "thread test begin";
    YAML::Node root = YAML::LoadFile("../conf/log2.yml");
    srvpro::Config::LoadFromYaml(root);
    
    std::vector<srvpro::Thread::ptr> thrs;
    for(int i = 0; i < 2; ++i) {
        srvpro::Thread::ptr thr(new srvpro::Thread(&fun2, "name_" + std::to_string(i * 2)));
        srvpro::Thread::ptr thr2(new srvpro::Thread(&fun3, "name_" + std::to_string(i * 2 + 1)));
        thrs.push_back(thr);
        thrs.push_back(thr2);
    }
    
    for(int i = 0; i < thrs.size(); ++i) {
        thrs[i]->join();
    }
    
    SRVPRO_LOG_INFO(g_logger) << "thread test end";
    SRVPRO_LOG_INFO(g_logger) << "count = " << count;

    srvpro::Config::Visit([](srvpro::ConfigVarBase::ptr var){
        SRVPRO_LOG_INFO(g_logger) << "name=" << var->getName() << " description=" << var->getDescription() << " type=" << var->getTypeName() << " value=" << var->toString();
    });*/
    
    //test_assert();
    /*srvpro::Thread::SetName("main");
    //test_fiber();
    std::vector<srvpro::Thread::ptr> thrs;
    for(int i = 0; i < 3; ++i) {
        //thr->SetName("name" + std::to_string(i));
        //srvpro::Thread::SetName("main");
    	thrs.push_back(srvpro::Thread::ptr(new srvpro::Thread(&test_fiber, "name_" + std::to_string(i))));
        //std::cout << srvpro::Thread::GetName() << std::endl;
    }
    
    for(auto i : thrs) {
    	i->join();
    }*/

    //test scheduler
    /*SRVPRO_LOG_INFO(g_logger) << "main";
    srvpro::Scheduler sc(3, false, "test");
    sc.start();
    sc.schedule(&test_fiber_scheduler);
    sc.stop();
    SRVPRO_LOG_INFO(g_logger) << "over";*/

    //test iomanager
    test_iomanager1();
    return 0;
}
