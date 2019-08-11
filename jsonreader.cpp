
#include <stdio.h>   
#include <stdlib.h>
#include <conio.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string.h>

static char* jsonimport() {
	std::ifstream file("C:\\Users\\Kusuma\\Desktop\\output\\pose9_render_keypoints.json");
	std::ostringstream tmp;
	tmp << file.rdbuf();
	std::string s = tmp.str();
	//std::cout << s << std::endl;
	file.close();

	std::string comp = "{\"0\":";
	std::size_t i = s.find(comp);
	//if (i != std::string::npos)
	//std::cout << i;
	std::string json1 = s.substr(i);
	const char * j = json1.c_str();
	//std::cout << json;
	char jsonstr[5000];
	//const char *back = "\\";
	int p = 0;
	for (int i = 0; json1.substr(i, 2).compare("]}") != 0; i++) {
		if (json1[i] == '"') {
			//std::cout << json[i-1] << '\t';
			jsonstr[p] = '\\';
			p++;
		}
		//strncpy(final, j[i], 1);
		jsonstr[p] = j[i];
		p++;
	}
	jsonstr[p] = ']';
	p++;
	jsonstr[p] = '}';
	p++;
	jsonstr[p] = '\0';
	std::cout << jsonstr << '\n';
	return jsonstr;
}

void main1() {
	jsonimport();
}