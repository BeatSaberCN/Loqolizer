#pragma once

#include <cstddef>
#include <string>
#include <map>
#include <vector>

namespace LangCtrl{

void InitEmbbedData();

struct DataVersionSource{
    const char * csv_buffer;
    size_t csv_buffer_size;
    const char * md5;
    const char * version;
};

struct DataModInfo{
    const char * modId;
    const char * name;

    std::vector<DataVersionSource> datas;
};

extern std::map<std::string, DataModInfo> embbedData;
extern const char * embbedDataTimestamp;
}

