#include "EmbbedData.hpp"
#include "LangCtrl.hpp"
#include "main.hpp"
#include "scotland2/shared/loader.hpp"
#include "SSL10n.hpp"
#include "semver_rustlibrary_include.hpp"
#include <cstddef>
#include <cstring>
void LangCtrl::InitEmbbedData(){
    std::string_view siraLocalizerRes = LangCtrl::siraLocalizerResource();
    if(siraLocalizerRes.size() > 0){
        SSL10n::Database::PolyglotFormat::AddCSVContent(siraLocalizerRes.data(), siraLocalizerRes.size());
    }
    
    auto siraRes = LangCtrl::siraLocalizerModResource();
    SSL10n::Database::PolyglotFormat::AddCSVContent(siraRes.data(), siraRes.size());


    auto loaded_mods = modloader::get_loaded();
    auto metadata = embbedData.find("ModMetadata");
    if(metadata != embbedData.end()){
        if(metadata->second.datas.size() > 0){
            auto & data = metadata->second.datas[0];
            loadedResourceMd5.insert(data.md5);
            SSL10n::Database::PolyglotFormat::AddCSVContent(data.csv_buffer, data.csv_buffer_size);
        }
    }

    for(auto & dat : embbedData){
        PaperLogger.info("Embbed translate for mod id {}", dat.first);
    }
    for(auto mod : loaded_mods){
        auto it = embbedData.find(mod.info.id);
        if(it == embbedData.end()){
            PaperLogger.info("Translate data for {} not found.", mod.info.id);
            continue;
        }
        PaperLogger.info("Load Embbed mod translate for {}", mod.info.id);

        auto & datas = it->second.datas;

        for(auto &versionData : datas){
            PaperLogger.info("Try loading version {}",versionData.version);
            const char * versionRange = versionData.version;

            if(versionRange == NULL
                || (versionRange[0] == '\0')
                || (versionRange[0] == '*' && versionRange[1] == '\0')
            ){
                // load the version
                PaperLogger.info("Translate will load.");
            }else{
                if(mod.info.version == "" || semver_set_mod_version(mod.info.version.c_str(), mod.info.version.size()) == 0){
                    PaperLogger.info("Mod version parse failed. Anyway we will load the translate.");
                }else{
                    int match_result = semver_test_version_match(versionRange, strlen(versionRange));
                    if(match_result == -1){
                        PaperLogger.info("Version '{}' broken in embbed data. Anyway we will load the translate.", versionData.version);
                    }else if(match_result == 0){
                        PaperLogger.info("Mod version {} doesn't match embbed data version {}, skip this.", mod.info.version, versionRange);
                        continue;
                    }else if(match_result == 1){
                        //matched
                    }else{
                        PaperLogger.info("Impossible result, something wrong.");
                    }
                }
            }

            loadedResourceMd5.insert(versionData.md5);
            SSL10n::Database::PolyglotFormat::AddCSVContent(versionData.csv_buffer, versionData.csv_buffer_size);
            break;
        }
    }
}