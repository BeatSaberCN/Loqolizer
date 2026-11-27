#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <map>
#include <vector>
#include <set>
class Snapshot;

#define MOD_STORAGE_FOLDER "/sdcard/ModData/com.beatgames.beatsaber/Mods/Loqolizer"
#define MOD_REMOTE_CACHE MOD_STORAGE_FOLDER "/remote_cache.protobuf"

namespace LangCtrl{

enum SnapshotStatus{
    Snapshot_INVALID,
    Snapshot_EMBBED,
    Snapshot_REMOTE,
};

void InitEmbbedData();
void LoadSnapshot(Snapshot&& data);

const char * getManifestUrl();

std::string_view embbedDataSnapshot();

extern Snapshot *activated_snapshot;
extern SnapshotStatus activated_snapshot_status;
extern std::map<std::string/*mod id*/, std::string/*mod version*/> loadedMods;
extern std::vector<std::string> loadedPcMods;

}

