#include "myEC2.h"




int main()
{
    StringPairVector* result;
    MyEC2 my;
    my.setParam(2, "regionName", "regionEndpoint");
    if (my.call(1, "Action", "DescribeRegions"))
    {
        result = my.getResult();
        int len = result->size();
        for (int i = 0; i < len; i ++)
        {
            printf("%s: %s\n", (*result)[i].first.c_str(), (*result)[i].second.c_str());
        }
    }
    return 0;
}
