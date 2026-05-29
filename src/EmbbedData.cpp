#include "EmbbedData.hpp"
#include "main.hpp"
#include "scotland2/shared/loader.hpp"
#include "sslocalization/shared/SSL10n.hpp"
#include "semver.hpp"

void LangCtrl::InitEmbbedData(){
    auto loaded_mods = modloader::get_loaded();

    for(auto mod : loaded_mods){
        auto it = embbedData.find(mod.info.id);
        if(it == embbedData.end()){
            continue;
        }
        PaperLogger.info("Load Embbed mod translate for {}({}) (hash {})", it->first, it->second.version, it->second.md5);

        //TODO: mod version check
        const char * versionRange = it->second.version;

        if(versionRange == NULL
            || (versionRange[0] == '\0')
            || (versionRange[0] == '*' && versionRange[1] == '\0')
        ){
            // load the version
        }else{
            auto mod_version = semver::try_parse(mod.info.version);
            if(mod_version.has_value()){
                semver::range_set rs;
                if(semver::parse(versionRange, rs)){
                    if(!rs.contains(mod_version.value())){
                        PaperLogger.info("Mod version {} doesn't match embbed data version {}, will not load.", mod.info.version, versionRange);
                        continue;
                    }
                }else{
                    PaperLogger.info("Version '{}' broken in embbed data. Anyway we will load the translate.", it->second.version);
                }
            }else{
                PaperLogger.info("Mod version parse failed. Anyway we will load the translate.");
            }
        }

        SSL10n::Database::PolyglotFormat::AddCSVContent(it->second.csv_buffer, it->second.csv_buffer_size);
    }
}