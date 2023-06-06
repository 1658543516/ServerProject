//
// Created by mrpiao on 23-6-2.
//

#ifndef SERVERPROJECT_CONFIG_H
#define SERVERPROJECT_CONFIG_H
#include <memory>
#include <sstream>
#include <string>
#include "Log.h"
#include "util.h"
#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>


namespace srvpro {

    class ConfigVarBase {
    public:
        typedef std::shared_ptr<ConfigVarBase> ptr;
        ConfigVarBase(const std::string& name, const std::string& description)
        :m_name(name), m_description(description) {
            std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
        }
        virtual ~ConfigVarBase() {}
        const std::string& getName() const {return m_name;}
        const std::string& getDescription() const {return m_description;}

        virtual std::string toString() = 0;
        virtual bool parseString(const std::string& str) = 0;
	virtual std::string getTypeName() const = 0;
    protected:
        std::string m_name;
        std::string m_description;
    };

    //F from_type, T to_type
    template<class F, class T>
    class LexicalCast {
    public:
        T operator()(const F& v) {
            return boost::lexical_cast<T>(v);
        }
    };

    template<class T>
    class LexicalCast<std::string, std::vector<T> > {
    public:
        std::vector<T> operator()(const std::string& v) {
            YAML::Node node = YAML::Load(v);
            std::stringstream  ss;
            typename std::vector<T> vec;
            for(size_t i = 0; i < node.size(); ++i) {
                ss.str("");
                ss << node[i];
                vec.emplace_back(LexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };

    template<class T>
    class LexicalCast<std::vector<T>, std::string> {
    public:
        std::string operator()(const std::vector<T>& v) {
            YAML::Node node;
            for(auto& i : v) {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    template<class T>
    class LexicalCast<std::string, std::list<T> > {
    public:
        std::list<T> operator()(const std::string& v) {
            YAML::Node node = YAML::Load(v);
            std::stringstream  ss;
            typename std::list<T> lst;
            for(size_t i = 0; i < node.size(); ++i) {
                ss.str("");
                ss << node[i];
                lst.emplace_back(LexicalCast<std::string, T>()(ss.str()));
            }
            return lst;
        }
    };

    template<class T>
    class LexicalCast<std::list<T>, std::string> {
    public:
        std::string operator()(const std::list<T>& v) {
            YAML::Node node;
            for(auto& i : v) {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    template<class T>
    class LexicalCast<std::string, std::set<T> > {
    public:
        std::set<T> operator()(const std::string& v) {
            YAML::Node node = YAML::Load(v);
            std::stringstream  ss;
            typename std::set<T> st;
            for(size_t i = 0; i < node.size(); ++i) {
                ss.str("");
                ss << node[i];
                st.insert(LexicalCast<std::string, T>()(ss.str()));
            }
            return st;
        }
    };

    template<class T>
    class LexicalCast<std::set<T>, std::string> {
    public:
        std::string operator()(const std::set<T>& v) {
            YAML::Node node;
            for(auto& i : v) {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
   };

    template<class T>
    class LexicalCast<std::string, std::unordered_set<T> > {
    public:
        std::unordered_set<T> operator()(const std::string& v) {
            YAML::Node node = YAML::Load(v);
            std::stringstream  ss;
            typename std::unordered_set<T> st;
            for(size_t i = 0; i < node.size(); ++i) {
                ss.str("");
                ss << node[i];
                st.insert(LexicalCast<std::string, T>()(ss.str()));
            }
            return st;
        }
    };

    template<class T>
    class LexicalCast<std::unordered_set<T>, std::string> {
    public:
        std::string operator()(const std::unordered_set<T>& v) {
            YAML::Node node;
            for(auto& i : v) {
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
   };

    template<class T>
    class LexicalCast<std::string, std::map<std::string, T> > {
    public:
        std::map<std::string, T> operator()(const std::string& v) {
            YAML::Node node = YAML::Load(v);
            std::stringstream  ss;
            typename std::map<std::string, T> mp;
            for(auto it = node.begin(); it != node.end(); ++it) {
                ss.str("");
                ss << it->second;
                mp.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
            }
            return mp;
        }
    };

    template<class T>
    class LexicalCast<std::map<std::string, T>, std::string> {
    public:
        std::string operator()(const std::map<std::string, T>& v) {
            YAML::Node node;
            for(auto& i : v) {
                node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
   };

    template<class T>
    class LexicalCast<std::string, std::unordered_map<std::string, T> > {
    public:
        std::unordered_map<std::string, T> operator()(const std::string& v) {
            YAML::Node node = YAML::Load(v);
            std::stringstream  ss;
            typename std::unordered_map<std::string, T> mp;
            for(auto it = node.begin(); it != node.end(); ++it) {
                ss.str("");
                ss << it->second;
                mp.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
            }
            return mp;
        }
    };

    template<class T>
    class LexicalCast<std::unordered_map<std::string, T>, std::string> {
    public:
        std::string operator()(const std::unordered_map<std::string, T>& v) {
            YAML::Node node;
            for(auto& i : v) {
                node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
   };



    //FromStr T operator()(const std::string&)
    //ToStr std::string operator()(const T&)
    template<class T, class FromStr = LexicalCast<std::string, T>, class ToStr = LexicalCast<T, std::string> >
    class ConfigVar : public ConfigVarBase {
    public:
        typedef std::shared_ptr<ConfigVar> ptr;
        ConfigVar(const std::string& name, const T& default_value, const std::string& description)
        :ConfigVarBase(name, description), m_val(default_value) {}

        std::string toString() override {
            try {
                //return boost::lexical_cast<std::string>(m_val);
                return ToStr()(m_val);
            } catch (std::exception& e) {
                SRVPRO_LOG_ERROR(SRVPRO_LOG_ROOT()) << "ConfigVar::toString exception "
                                                  << e.what() << " convert: " << typeid(m_val).name() << " to string"
                                                  << " name=" << m_name;
            }
            return "";
        }
        bool parseString(const std::string& str) override {
            try {
                //m_val = boost::lexical_cast<T>(str);
                setValue(FromStr()(str));
            } catch (std::exception& e) {
                SRVPRO_LOG_ERROR(SRVPRO_LOG_ROOT()) << "ConfigVar::fromString exception "
                                                  << e.what() << " convert: string to " << typeid(m_val).name();
            }
            return false;
        }

        const T getValue() const {return m_val;}
        void setValue(const T& val) {m_val = val;}
	std::string getTypeName() const override {return typeid(T).name();}
    private:
        T m_val;
    };

    class Config {
    public:
        typedef std::unordered_map<std::string, ConfigVarBase::ptr> ConfigVarMap;

        template<class T>
        static typename ConfigVar<T>::ptr Lookup(const std::string& name,
                                                 const T& default_value, const std::string& description = "") {
            auto it = GetDatas().find(name);
            if(it != GetDatas().end()) {
                auto tmp = std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
                if(tmp) {
                    SRVPRO_LOG_INFO(SRVPRO_LOG_ROOT()) << "Lookup name=" << name << " exists";
                    return tmp;
                }
                else {
                    SRVPRO_LOG_ERROR(SRVPRO_LOG_ROOT()) << "Lookup name=" << name << " exists but type not " << typeid(T).name() << " real_type=" << it->second->getTypeName() << " " << it->second->toString();
                    return nullptr; 
                }
            }

            if(name.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678")
               != std::string::npos) {
                SRVPRO_LOG_ERROR(SRVPRO_LOG_ROOT()) << "Lookup name invalid " << name;
                throw std::invalid_argument(name);
            }

            typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
            GetDatas()[name] = v;
            return v;
        }

        template<class T>
        static typename ConfigVar<T>::ptr Lookup(const std::string& name) {
            auto it = GetDatas().find(name);
            if(it == GetDatas().end()) {
                return nullptr;
            }
            return std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
        }

        static void LoadFromYaml(const YAML::Node& root);

        static ConfigVarBase::ptr LookupBase(const std::string& name);

    private:
        static ConfigVarMap& GetDatas() {
            static ConfigVarMap s_datas;
            return s_datas;
        }

    };

}

#endif //SERVERPROJECT_CONFIG_H
