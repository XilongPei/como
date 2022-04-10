/*
    g++ ini_std_string_example.cpp ../ini.c
*/
/* Example: parse a simple configuration file */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ini.h"
#include <string>

const char *iniStr="offset_x=1;offset_y=2;scale=3;enable_target_size=true;target_size_width=4;target_size_height=5;"
"enable_customize_alpha=true;customize_alpha_source_type=\"FILE/DATA\";"
"[file_info];"
"customize_alpha_file_const_path=\"tongji\";customize_alpha_file_path=\"\";"
"customize_alpha_file_path_len=0;";

static int handler(void* user, const char* section, const char* name, const char* value)
{
    if (strcmp(section, "file_info") == 0 && strcmp(name, "customize_alpha_file_const_path") == 0) {
        ((std::string *)user)->assign(value);
    }
    return 1;
}

int main(int argc, char* argv[])
{
    std::string sfunc;

    if (ini_parse_string(iniStr, handler, &sfunc) < 0) {
        printf("Can't load 'test.ini'\n");
        return 1;
    }

    printf("[file_info] customize_alpha_file_const_path=%s\n", sfunc.c_str());
    return 0;
}
