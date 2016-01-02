
#ifndef CHANN_HTML_TEMPLATE_HEADER_INCLUDED
#define CHANN_HTML_TEMPLATE_HEADER_INCLUDED

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <stack>
#include <sstream>
#include <fstream>
#include <exception>
#include <queue>

#include <dirent.h>

extern "C" {
#include "../lib/mongoose/mongoose.h"
}

using std::vector;
using std::map;
using std::stack;
using std::string;
using std::pair;
using std::stringstream;
using std::queue;

class HTMLTemplate {
private:
	enum {
		PLAIN = 0,
		VARIABLE = 1,
		IF = 2,
		IFTEST = 3,
		ENDIF = 4,
		LOOP = 5,
		ENDLOOP = 6
	};
	string str;
	vector<pair<unsigned, string>> vec;

	stack<pair<unsigned, string>> if_statements;
	stack<pair<unsigned, string>> loop_statements;

	map<unsigned, unsigned> fi_pos;
	map<unsigned, unsigned> loop_pos;
	map<unsigned, unsigned> loop_pos_rev;
	map<unsigned, pair<string, string>> if_test;

	map<string, string> i_vars;
	map<string, bool> i_stats;
	map<string, queue<string>> i_loops;
public:
	HTMLTemplate() = default;
	HTMLTemplate(string s) {
		load(s);
	}

	HTMLTemplate(const HTMLTemplate& r) {
		str = r.str;
		vec = r.vec;
		fi_pos = r.fi_pos;
		loop_pos = r.loop_pos;
		loop_pos_rev = r.loop_pos_rev;
		if_test = r.if_test;

		i_vars = r.i_vars;
		i_stats = r.i_stats;
		i_loops = r.i_loops;
	}

	void load_file(string s) {
		std::ifstream ifs(s);
		string line, sz;
		while (getline(ifs, line)) sz += trim(line);

		load(sz);
	}

	void load(string s) {
		str = s;
		int last_pos = 0;
		int last_var_pos = 0;
		int last_if_pos = 0;

		bool flag = false;

		vec.clear();
		fi_pos.clear();
		if_test.clear();

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
					if(tmp.find("=") == std::string::npos){
						string if_name = tmp.substr(3);
						if_statements.push(make_pair(vec.size(), if_name));

						vec.push_back(make_pair(IF, if_name));

					}
					else {
						stringstream ifss(tmp.substr(3));
						string if_name, if_condition;

						getline(ifss, if_name, '=');
						getline(ifss, if_condition);

						if_statements.push(make_pair(vec.size(), if_name));
						vec.push_back(make_pair(IFTEST, if_name));

						if_test[vec.size() - 1] = make_pair(if_name, if_condition);
					}
				}
				else if (tmp == "endif") {
					string if_name = if_statements.top().second;
					unsigned if_pos = if_statements.top().first;

					if_statements.pop();

					vec.push_back(make_pair(ENDIF, if_name));
					fi_pos[if_pos] = vec.size() - 1;
				}
				else if (tmp.substr(0, 5) == "loop ") {
					string loop_name = tmp.substr(5);

					loop_statements.push(make_pair(vec.size(), loop_name));
					vec.push_back(make_pair(LOOP, loop_name));
				}
				else if (tmp == "endloop") {
					string loop_name = loop_statements.top().second;
					unsigned pos = loop_statements.top().first;

					loop_statements.pop();

					vec.push_back(make_pair(ENDLOOP, loop_name));

					loop_pos[pos] = vec.size() - 1;
					loop_pos_rev[vec.size() - 1] = pos;
				}

				last_pos = i + 4;
				i += 3;
			}

			if (i == str.size() - 1) vec.push_back(make_pair(PLAIN, str.substr(last_pos)));
		}

		if (!if_statements.empty()) throw std::logic_error("Incorrect if statements");
		if (!loop_statements.empty()) throw std::logic_error("Incorrect loop statements");
	}

	string build(map<string, string> &vars, map<string, bool> &stats, map<string, queue<string>> &loops) {
		int var_pointer = 0;
		string ret = "", tmp = "";

		for (auto i = 0; i < vec.size(); ++i) {
			// std::cout  << vec[i].second << "  =" << vec[i].second.size() << std::endl;

			switch (vec[i].first) {
			case VARIABLE:
				ret.append(vars[vec[i].second]);
				break;
			case PLAIN:
				ret.append(vec[i].second);
				break;
			case IF:
				tmp = vec[i].second;

				if (tmp[0] == '!') {
					if (stats[tmp.substr(1)]) i = fi_pos[i];
				}
				else
					if (!stats[tmp]) i = fi_pos[i];
				break;
			case IFTEST:
				tmp = if_test[i].first;

				if (tmp[0] == '!') {
					if (vars[tmp.substr(1)] == if_test[i].second) i = fi_pos[i];
				}
				else
					if (vars[tmp] != if_test[i].second) i = fi_pos[i];
				break;
			case LOOP:
				if (loops[vec[i].second].empty())
					i = loop_pos[i];
				else {
					vars[vec[i].second] = loops[vec[i].second].front();
					loops[vec[i].second].pop();
				}
				break;
			case ENDLOOP:
				if (!loops[vec[i].second].empty())
					i = loop_pos_rev[i] - 1;
				break;
			case ENDIF:
			default:
				break;
			}
		}
		return ret;
	}

	HTMLTemplate& var(const string& var_name, const string& var_value) {
		i_vars[var_name] = var_value;
		return *this;
	}

	HTMLTemplate& var(const string& var_name, const int var_value) {
		i_vars[var_name] = std::to_string(var_value);
		return *this;
	}

	HTMLTemplate& toggle(const string& stat_name, const bool stat_value = true) {
		i_stats[stat_name] = stat_value;
		return *this;
	}

	HTMLTemplate& loop(const string& loop_name, const queue<string> loop_stack) {
		i_loops[loop_name] = loop_stack;
		return *this;
	}

	HTMLTemplate& pipe_to(mg_connection* conn) {
		mg_printf_data(conn, "%s", build(i_vars, i_stats, i_loops).c_str());
		return *this;
	}

	void destory() { delete this; }

	string build2() {
		return build(i_vars, i_stats, i_loops);
	}

	string build_destory() {
		string ret = build(i_vars, i_stats, i_loops);
		delete this;
		return ret;
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
	std::string lang;
public:
	void use_lang(std::string l){
		lang = l;
	}

	int load_templates(){
		DIR *pDIR;
        struct dirent *entry;
        int c = 0;
        std::string root_dir = "./templates/" + lang + "/";

        if(pDIR = opendir(root_dir.c_str()) ){
            while(entry = readdir(pDIR)){
                if( strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 ){
	                string filename = entry->d_name;

	        		HTMLTemplate tmp;
					tmp.load_file(root_dir + filename);

	            	filename = filename.substr(0, filename.size() - 4);
					m_templates[filename] = tmp;

					c++;
				}

            }
            closedir(pDIR);
        }

        return c;
	}

	HTMLTemplate& add_template(const string& name){
		HTMLTemplate tmp;
		std::string root_dir = "./templates/" + lang + "/";

		tmp.load_file(root_dir + name + string(".tpl"));

		m_templates[name] = tmp;

		return m_templates[name]; //*this;
	}

	HTMLTemplate& invoke(const string& name){
		return *(new HTMLTemplate(m_templates[name]));
	}

	HTMLTemplate* invoke_pointer(const string& name){
		return new HTMLTemplate(m_templates[name]);
	}
};

#endif