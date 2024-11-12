#include "pattern.h"

bool match_pattern(char *pattern, char *input)
{
    while (*pattern != '\0' && *input != '\0')
    {
        switch (*pattern)
        {
        case '?':
            pattern++;
            input++;
            break;
        case '*':
            if (match_pattern(pattern + 1, input))
                return true;
            input++;
            break;
        default:
            if (*pattern == *input)
            {
                pattern++;
                input++;
            }
            else
            {
                return false;
            }
            break;
        }
    }
    if (*input == '\0')
    {
        while (*pattern == '*')
            pattern += 1;
        return *pattern == '\0';
    }
    return false;
}