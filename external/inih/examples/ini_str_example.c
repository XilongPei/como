/*
    g++ ini_str_example.c ../ini.c
*/
/* Example: parse a simple configuration file */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ini.h"

const char *iniStr="offset_x=1;offset_y=2;scale=3;enable_target_size=true;target_size_width=4;target_size_height=5;"
"enable_customize_alpha=true;customize_alpha_source_type=\"FILE/DATA\";"
"[file_info];"
"customize_alpha_file_const_path=\"tongji\";customize_alpha_file_path=\"\";"
"customize_alpha_file_path_len=0;";

typedef struct
{
    int version;
    const char* name;
    const char* email;
} configuration;

static int handler(void* user, const char* section, const char* name,
                   const char* value)
{
    configuration* pconfig = (configuration*)user;

    printf("section:%s , name: %s , value: %s\n", section, name, value);

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("protocol", "version")) {
        pconfig->version = atoi(value);
    } else if (MATCH("user", "name")) {
        pconfig->name = strdup(value);
    } else if (MATCH("user", "email")) {
        pconfig->email = strdup(value);
    } else {
        return 0;  /* unknown section/name, error */
    }
    return 1;
}

int main(int argc, char* argv[])
{
    configuration config;
    config.version = 0;  /* set defaults */
    config.name = NULL;
    config.email = NULL;

    if (ini_parse_string(iniStr, handler, &config) < 0) {
        printf("Can't load 'test.ini'\n");
        return 1;
    }

    /*
    printf("Config loaded from 'test.ini': version=%d, name=%s, email=%s\n",
        config.version, config.name, config.email);
    */

    if (config.name)
        free((void*)config.name);
    if (config.email)
        free((void*)config.email);

    return 0;
}
