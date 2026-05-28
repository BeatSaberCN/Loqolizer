#include "EmbbedData.hpp"
#include "main.hpp"
#include "scotland2/shared/loader.hpp"
#include "sslocalization/shared/SSL10n.hpp"

void LangCtrl::InitEmbbedData(){
    auto loaded_mods = modloader::get_loaded();

    for(auto mod : loaded_mods){
        auto it = embbedData.find(mod.info.id);
        if(it == embbedData.end()){
            continue;
        }
        PaperLogger.info("Load Embbed mod translate for {} (hash {})", it->first, it->second.md5);

        //TODO: mod version check

        SSL10n::Database::PolyglotFormat::AddCSVContent(it->second.csv_buffer, it->second.csv_buffer_size);
    }
}