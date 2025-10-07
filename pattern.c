#include "pattern.h"
#include <ctype.h>

bool match_pattern(char *pattern, char *input, bool ignore_case)
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
            if (match_pattern(pattern + 1, input, ignore_case))
                return true;
            input++;
            break;
        default:
            if (ignore_case)
            {
                if (tolower((unsigned char)*pattern) == tolower((unsigned char)*input))
                {
                    pattern++;
                    input++;
                    break;
                }
                else
                {
                    return false;
                }
            }
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