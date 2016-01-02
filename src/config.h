#ifndef CHANN_CONFIG_HEADER_INCLUDED
#define CHANN_CONFIG_HEADER_INCLUDED

#include <string>
#include <vector>
#include <stack>
#include <fstream>
#include <sstream>
#include <map>
#include <unordered_map>
#include <exception>
#include <typeinfo>


class ConfigManager{
private:
	std::unordered_map<std::string, std::string> configs_str;
	std::unordered_map<std::string, int> configs_int;
	std::unordered_map<std::string, bool> configs_bool;
public:
	ConfigManager& global(){
		static ConfigManager i;
		return i;
	}

	ConfigManager() = default;
	ConfigManager(std::string s){
		load(s);
	}
	
	void load(std::string s){
		std::ifstream ifs(s);
		std::string line;

		std::string cur_space = "";
		std::stack<std::string> stack_space;

		while(getline(ifs, line)){
			line = trim(line);
			
			if(line.empty()) continue;
			if(line[0] == '#') continue;

			std::stringstream ss(line);

			std::vector<std::string> v;
			std::string tmp;

			while(getline(ss, tmp, ' ')){
				tmp = trim(tmp);
				// tmp = cc_replace(tmp, "\t", "");
				if(!tmp.empty()) v.push_back(tmp);
			}

			if(v.size() > 1 && v[1] == "{"){
				stack_space.push(v[0]);
				cur_space += (v[0] + "::");
				continue;
			}

			if(line == "}"){
				cur_space = cur_space.substr(0, cur_space.size() - 2 - stack_space.top().size());
				// std::cout << cur_space << std::endl;
				stack_space.pop();
				continue;
			}

			if(v.size() < 2) continue;
						// std::cout << ";" << line << ";" << std::endl;

			std::string val = v[1];
			std::string key = cur_space + v[0];

			// std::cout << key << ",'" << v[0] << "'" << val << std::endl;

			// std::string alias = "dummy";
			// if(v.size() == 4 && v[2] == "alias") alias = cur_space + v[3];

			if((val[0] == '\'' && val[val.size() - 1] == '\'') || 
				(val[0] == '"' && val[val.size() - 1] == '"'))
			{
				configs_str[key] = val.substr(1, val.size() - 2);
				// configs_str[alias] = configs_str[key];
			}
			else if(val == "on" || val == "off"){
				configs_bool[key] = (val == "on");
				// configs_bool[alias] = configs_bool[key];
			}
			else{
				configs_int[key] = atol(val.c_str());
				// configs_int[alias] = configs_int[key];
			}

		}

		if(!stack_space.empty()) throw std::logic_error("Incorrect namespaces: " + stack_space.top());
	}

	template<typename T>
	T get(const std::string& k){
		if(typeid(T).name() == typeid(int).name())
			return configs_int[k];
		else if(typeid(T).name() == typeid(bool).name())
			return configs_bool[k];
		else
			return 0;
	}

	std::string& get(const std::string& k){
		return configs_str[k];
	}

	ConfigManager& set(const std::string& k, const std::string& v){
		configs_str[k] = v;
		return *this;
	}

	ConfigManager& set(const std::string& k, const bool v){
		configs_bool[k] = v;
		return *this;
	}

	ConfigManager& set(const std::string& k, const int v){
		configs_int[k] = v;
		return *this;
	}

	ConfigManager& try_set(const std::string& k, const std::string& v){
		if(v == "on" || v == "off")
			configs_bool[k] = (v == "on");
		else{
			if(configs_str.find(k) != configs_str.end())
				configs_str[k] = v;
			else if(configs_int.find(k) != configs_int.end())
				configs_int[k] = atol(v.c_str());
		}
		return *this;
	}

	std::string serialize_to_json(){
		std::string ret = "{";
		for(auto i = configs_str.begin(); i != configs_str.end(); ++i)
			ret += ("\"" + i->first + "\":\"" + i->second + "\",");

		for(auto i = configs_int.begin(); i != configs_int.end(); ++i)
			ret += ("\"" + i->first + "\":" + std::to_string(i->second) + ",");

		for(auto i = configs_bool.begin(); i != configs_bool.end(); ++i)
			ret += ("\"" + i->first + "\":" + (i->second ? "true" : "false") + ",");

		if(!ret.empty()) ret = ret.substr(0, ret.size() - 1);

		ret += "}";

		return ret;
	}

	static std::string trim(const std::string& str){
		std::string::size_type pos = str.find_first_not_of(' ');
		if (pos == std::string::npos)
		{
			return str;
		}
		std::string::size_type pos2 = str.find_last_not_of(' ');
		if (pos2 != std::string::npos)
		{
			return str.substr(pos, pos2 - pos + 1);
		}
		return str.substr(pos);
	}
};

// extern ConfigManager configs;

#endif