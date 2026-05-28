#pragma once

#include <cstddef>
#include <string>
#include <map>

namespace LangCtrl{

void InitEmbbedData();

struct EmbbedData{
    const char * modId;
    const char * name;

    const char * csv_buffer;
    size_t csv_buffer_size;
    const char * md5;
    const char * version;
};

extern std::map<std::string, EmbbedData> embbedData;
extern const char * embbedDataTimestamp;
}

