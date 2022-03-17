// Example that shows simple usage of the INIReader class

#include <iostream>
#include <string.h>
#include "../cpp/INIReader.h"

const char *iniStr="offset_x=1;offset_y=2;scale=3;enable_target_size=true;target_size_width=4;target_size_height=5;"
"enable_customize_alpha=true;customize_alpha_source_type=\"FILE/DATA\";"
"[file_info];"
"customize_alpha_file_const_path=\"tongji\";customize_alpha_file_path=\"\";"
"customize_alpha_file_path_len=0;";

int main()
{
    INIReader reader(iniStr, strlen(iniStr));

    if (reader.ParseError() < 0) {
        std::cout << "Can't load 'test.ini'\n";
        return 1;
    }
    std::cout << "file_info.customize_alpha_file_const_path: "
              << reader.Get("file_info", "customize_alpha_file_const_path", "UNKNOWN") << "\n";;
    return 0;
}
