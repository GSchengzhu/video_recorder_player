#include <cstdio>
#include <map>
#include <string>
int main()
{
    std::map<std::string, int> testMap;
    testMap["0"] = 0;
    testMap["1"] = 0;
    testMap["2"] = 0;
    testMap["3"] = 0;
    
    [&]{
        for(auto iter = testMap.begin(); iter != testMap.end(); iter++)
        {
            std::string key = iter->first;
            iter->second = std::stoi(key);
        }
    }();

    for(auto iter = testMap.begin(); iter != testMap.end(); iter++)
    {
        printf("key: %s, value: %d\n",iter->first.c_str(),iter->second);
    }

    return 0;
}