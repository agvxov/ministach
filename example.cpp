// @BAKE g++ $@ -o $*.out
#include <stdio.h>
#include <map>
#include <string>
#include "ministach.h"

using namespace std;

const char * ministach_from_dictionary(const char * key, int key_len, void * userdata) {
    map<string, string> &usermap = *(map<string, string>*)userdata;

    auto value = usermap.find(string(key, key_len));

    if (value == usermap.end()) { return NULL; }

    return value->second.c_str();
}

int main() {
    const char * template_str = "{{this}} works with {{whatever}} too.";

    map<string, string> mymap = {
        {"this",     "Ministach"},
        {"whatever", "std::map" },
    };

    ministach_t m;
    ministach_compile(&m, template_str, ministach_from_dictionary, &mymap);

    if (m.status != MINISTACH_OK) { return 1; }

    char * buf = (char*)malloc(m.size + 1);
    ministach_render(&m, buf);
    puts(buf);

    free(buf);
    return 0;
}
