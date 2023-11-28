#ifndef __GDSTRINGEXTRACTOR_H__
#define __GDSTRINGEXTRACTOR_H__
#include <array>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <chrono>
#include <thread>
namespace godot{
    static std::array<char, 32> extractGdString(const String& godotString)
    {
        const char* c_literal = godotString.utf8().get_data();
        const unsigned int C_LITERAL_SIZEOF = strlen(c_literal) + 1;
        if (C_LITERAL_SIZEOF == 1)
        {
            UtilityFunctions::printerr("godotstring empty in gdstringextractor");
        }

        std::array<char, 32> buffer = {'\0'};

        const unsigned int overflow = 
            (C_LITERAL_SIZEOF > sizeof(buffer)) * (C_LITERAL_SIZEOF % sizeof(buffer));
                
        strcat(&buffer[0], &c_literal[overflow]);
        if (overflow > 0){
            buffer[0] = '#';
            UtilityFunctions::printerr("cpp: using modified substring for \"", godotString, "\" -> \"",&buffer[0],"\"");
        }
        return buffer;
    }
}
#endif // __GDSTRINGEXTRACTOR_H__
