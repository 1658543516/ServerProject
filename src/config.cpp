//
// Created by mrpiao on 23-6-5.
//
#include "config.h"
#include <yaml-cpp/yaml.h>

namespace srvpro {

    ConfigVarBase::ptr Config::LookupBase(const std::string &name) {
        auto it = Config::GetDatas().find(name);
        return it == Config::GetDatas().end() ? nullptr : it->second;
    }

    static void ListAllMember(const std::string& prefix, const YAML::Node& node, std::vector<std::pair<std::string, const YAML::Node> >& output ) {
        if(prefix.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678") != std::string::npos) {
            SRVPRO_LOG_ERROR(SRVPRO_LOG_ROOT()) << "Config invalid name: " << prefix << " : " << node;
            return;
        }
        output.emplace_back(std::make_pair(prefix, node));
        if (node.IsMap()) {
            for (auto it = node.begin(); it != node.end() ; ++it) {
                ListAllMember(prefix.empty()? it->first.Scalar() : prefix + "." + it->first.Scalar(), it->second, output);
            }
        }
    }

    void Config::LoadFromYaml(const YAML::Node &root) {
        std::vector<std::pair<std::string, const YAML::Node> > all_nodes;
        ListAllMember("", root, all_nodes);

        for (auto& i: all_nodes) {
            std::string key = i.first;
            if (key.empty()) {
                continue;
            }
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            ConfigVarBase::ptr var = LookupBase(key);

            if (var) {
                if (i.second.IsScalar()) {
                    var->parseString(i.second.Scalar());
                }
                else {
                    std::stringstream ss;
                    ss << i.second;
                    var->parseString(ss.str());
                }
            }
        }
    }

}