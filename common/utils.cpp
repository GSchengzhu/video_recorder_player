#include "utils.h"

void logging(const char* fmt,...)
{
    char msg[1024] = {0};
    
    va_list params;
    va_start(params, fmt);
    vsprintf(msg,fmt,params);
    va_end(params);

    printf("%s\n",msg);
}