#pragma once

#include <string>
#include <vector>

namespace morseCode
{
    inline std::string getMorseCode(char letter)
    {
        switch (letter)
        {
        case 'A':
        case 'a':
            return ".-";
        case 'B':
        case 'b':
            return "-...";
        case 'C':
        case 'c':
            return "-.-.";
        case 'D':
        case 'd':
            return "-..";
        case 'E':
        case 'e':
            return ".";
        case 'F':
        case 'f':
            return "..-.";
        case 'G':
        case 'g':
            return "--.";
        case 'H':
        case 'h':
            return "....";
        case 'I':
        case 'i':
            return "..";
        case 'J':
        case 'j':
            return ".---";
        case 'K':
        case 'k':
            return "-.-";
        case 'L':
        case 'l':
            return ".-..";
        case 'M':
        case 'm':
            return "--";
        case 'N':
        case 'n':
            return "-.";
        case 'O':
        case 'o':
            return "---";
        case 'P':
        case 'p':
            return ".--.";
        case 'Q':
        case 'q':
            return "--.-";
        case 'R':
        case 'r':
            return ".-.";
        case 'S':
        case 's':
            return "...";
        case 'T':
        case 't':
            return "-";
        case 'U':
        case 'u':
            return "..-";
        case 'V':
        case 'v':
            return "...-";
        case 'W':
        case 'w':
            return ".--";
        case 'X':
        case 'x':
            return "-..-";
        case 'Y':
        case 'y':
            return "-.--";
        case 'Z':
        case 'z':
            return "--..";
        default:
            return "";
        }
    }

    inline std::vector<int> codeToDelays(const char letter, int timeUnit)
    {
        std::string code = getMorseCode(letter);
        std::vector<int> delays;
        for (char c : code)
            delays.push_back(c == '.' ? timeUnit : timeUnit * 3);
        return delays;
    }

}