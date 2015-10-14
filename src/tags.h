// CppDummy.cpp : Defines the entry point for the console application.
//
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <stack>
#include <sstream>
#include <fstream>
#include <exception>

using std::vector;
using std::map;
using std::stack;
using std::string;
using std::pair;

class HTMLTemplate {
private:
	enum {
		PLAIN = 0, 
		VARIABLE = 1, 
		IF = 2, 
		EQ = 3, 
		FI = 4
	};
	string str;
	vector<pair<unsigned, string>> vec;
	map<unsigned, unsigned> fi_pos;
	stack<pair<unsigned, string>> if_statements;

	map<string, string> i_vars;
	map<string, bool> i_stats;
public:
	HTMLTemplate() = default;
	HTMLTemplate(string s){
		load(s);
	}

	HTMLTemplate(const HTMLTemplate& r){
		str = r.str;
		vec = r.vec;
		fi_pos = r.fi_pos;
	}

	void load_file(string s){
		std::ifstream ifs(s);
		string line, sz;
		while(getline(ifs, line)) sz += trim(line);

		load(sz);
	}

	void load(string s){
		str = s;
		int last_pos = 0;
		int last_var_pos = 0;
		int last_if_pos = 0;

		bool flag = false;

		for (unsigned i = 0; i < str.size(); ++i) {
			if (str[i] == '{' && str[i + 1] == '{') {
				vec.push_back(make_pair(PLAIN, str.substr(last_pos, i - last_pos)));
				last_var_pos = i + 2;
				flag = true;
				i++;
			}

			if (str[i] == '}' && str[i + 1] == '}' && flag) {
				vec.push_back(make_pair(VARIABLE, str.substr(last_var_pos, i - last_var_pos)));
				last_pos = i + 2;
				flag = false;
				i++;
			}

			if (str[i] == '<' && str[i + 1] == '!' && str[i + 2] == '-' && str[i + 3] == '-' && str[i + 4] == '[') {
				vec.push_back(make_pair(PLAIN, str.substr(last_pos, i - last_pos)));
				last_if_pos = i + 5;
				i += 4;
			}

			if (str[i] == ']' && str[i + 1] == '-' && str[i + 2] == '-' && str[i + 3] == '>') {
				string tmp = str.substr(last_if_pos, i - last_if_pos);
				if (tmp.substr(0, 3) == "if ") {
					string if_name = tmp.substr(3);
					if_statements.push(make_pair(vec.size(), if_name));

					vec.push_back(make_pair(IF, if_name));

				}
				else if (tmp.substr(0, 5) == "endif") {
					string if_name = if_statements.top().second;
					unsigned if_pos = if_statements.top().first;

					if_statements.pop();

					vec.push_back(make_pair(FI, if_name));
					fi_pos[if_pos] = vec.size() - 1;
				}

				last_pos = i + 4;
				i += 3;
			}

			if (i == str.size() - 1) vec.push_back(make_pair(PLAIN, str.substr(last_pos)));
		}

		if (!if_statements.empty()) throw std::logic_error("Incorrect if statements");
	}

	string build(map<string, string> &vars, map<string, bool> &stats, bool debug = false) {
		int var_pointer = 0;
		string ret = "", tmp = "";
		
		for (auto i = 0; i < vec.size(); ++i) {
			// std::cout  << vec[i].second << "  =" << vec[i].second.size() << std::endl;

			switch (vec[i].first) {
			case VARIABLE:
				ret.append(vars[vec[i].second]);
				if(debug && vars.find(vec[i].second) == vars.end()) ret.append(vec[i].second);
				break;
			case PLAIN:
				ret.append(vec[i].second);
				break;
			case IF:
				tmp = vec[i].second;
				if (tmp[0] == '!') {
					if (stats[tmp.substr(1)]) i = fi_pos[i];
					if(debug && stats.find(tmp.substr(1)) == stats.end()) ret.append("=" + tmp.substr(1));
				}
				else {
					if (!stats[tmp]) i = fi_pos[i];
					if(debug && stats.find(tmp) == stats.end()) ret.append("=" + tmp);
				}
				break;
			case FI:
			default:
				break;
			}
		}
		return ret;
	}

	HTMLTemplate& var(const string& var_name, const string& var_value){
		i_vars[var_name] = var_value;
		return *this;
	}

	HTMLTemplate& stat(const string& stat_name, const bool stat_value){
		i_stats[stat_name] = stat_value;
		return *this;
	}

	string build2(){
		return build(i_vars, i_stats);
	}

	static string trim(const string& str)
	{
		string::size_type pos = str.find_first_not_of(' ');
		if (pos == string::npos)
		{
			return str;
		}
		string::size_type pos2 = str.find_last_not_of(' ');
		if (pos2 != string::npos)
		{
			return str.substr(pos, pos2 - pos + 1);
		}
		return str.substr(pos);
	}
};

class TemplateManager{
private:
	map<string, HTMLTemplate> m_templates;
public:
	TemplateManager& add_template(const string& name){
		HTMLTemplate tmp;
		tmp.load_file(string("templates/") + name + string(".tpl"));

		m_templates[name] = tmp;

		return *this;
	}

	HTMLTemplate* invoke_template(const string& name){
		return new HTMLTemplate(m_templates[name]);
	}
};