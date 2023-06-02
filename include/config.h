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

namespace srvpro {

    class ConfigVarBase {
    public:
        typedef std::shared_ptr<ConfigVarBase> ptr;
        ConfigVarBase(const std::string& name, const std::string& description)
        :m_name(name), m_description(description) {}
        virtual ~ConfigVarBase() {}
        const std::string& getName() const {return m_name;}
        const std::string& getDescription() const {return m_description;}

        virtual std::string toString() = 0;
        virtual bool parseString(const std::string& str) = 0;
    protected:
        std::string m_name;
        std::string m_description;
    };

    template<class T>
    class ConfigVar : public ConfigVarBase {
    public:
        typedef std::shared_ptr<ConfigVar> ptr;
        ConfigVar(const std::string& name, const T& default_value, const std::string& description)
        :ConfigVarBase(name, description), m_val(default_value) {}

        std::string toString() override {
            try {
                return boost::lexical_cast<std::string>(m_val);
            } catch (std::exception& e) {
                SRVPRO_LOG_ERROR(SRVPRO_LOG_ROOT()) << "ConfigVar::toString exception "
                                                  << e.what() << " convert: " << typeid(m_val).name() << " to string"
                                                  << " name=" << m_name;
            }
            return "";
        }
        bool parseString(const std::string& str) override {
            try {
                m_val = boost::lexical_cast<T>(str);
            } catch (std::exception& e) {
                SRVPRO_LOG_ERROR(SRVPRO_LOG_ROOT()) << "ConfigVar::fromString exception "
                                                  << e.what() << " convert: string to " << typeid(m_val).name();
            }
            return false;
        }

        const T getValue() const {return m_val;}
        void setValue(const T& val) {m_val = val;}
    private:
        T m_val;
    };

    class Config {
    public:
        typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;

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
                /*else {
                    SRVPRO_LOG_ERROR(SRVPRO_LOG_ROOT()) << "Lookup name=" << name << " exists but type not ";
                                                      *//*<< TypeToName<T>() << " real_type=" << it->second->getTypeName()
                                                      << " " << it->second->toString();*//*
                    return nullptr;
                }*/
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

    private:
        static ConfigVarMap& GetDatas() {
            static ConfigVarMap s_datas;
            return s_datas;
        }

    };

}

#endif //SERVERPROJECT_CONFIG_H
