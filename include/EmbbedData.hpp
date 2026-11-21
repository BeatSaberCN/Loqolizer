#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <map>
#include <vector>

class Snapshot;

namespace LangCtrl{

void InitEmbbedData();

std::string_view embbedDataSnapshot();

extern Snapshot *activated_snapshot;

}

