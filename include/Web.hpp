#pragma once

#include "bsml/shared/BSML/MainThreadScheduler.hpp"
#include "web-utils/shared/WebUtils.hpp"
#include <cstdint>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace Web{
struct SSL10nManifestResponse : public WebUtils::JsonResponse {

};

struct CsvDownloadResponse : public WebUtils::DataResponse {

};

struct ModItem{
    std::string version;
    std::string csv_url;
    std::string md5;
    
    std::optional<std::vector<uint8_t>> data;
    // std::string csv_name; not used
};

struct ModInfo{
    std::string name;
    std::vector<ModItem> datas;
};

struct FutureDownloadTask{
    ModInfo * modInfo;
    ModItem * modItem;

};

class NetDataManager{
public:
enum DownloadStatus{
    Idle,
    DownloadingManifest,
    DownloadingMods,
    Success,
    Failed
};
    DownloadStatus status = Idle;
    int download_epoch = 0;

    float manifestDownloadingProgress = 0;
    std::map<std::string /* mod id */, ModInfo> mods;
    std::mutex mods_lock;

    std::optional<std::string> statusString;
    std::optional<std::string> remoteMessage;

    void StartNetworkDownload(){
        // clear data
        download_epoch++;
        std::atomic_thread_fence(std::memory_order_release);
        manifestDownloadingProgress = 0;
        status = DownloadingManifest;
        mods.clear();
        statusString = "Downloading";

        WebUtils::GetAsync<SSL10nManifestResponse>(WebUtils::URLOptions("https://frto027.github.io/ssl10n.csv/manifest.json"), [this, download_epoch = this->download_epoch](auto response){
            BSML::MainThreadScheduler::ScheduleNextFrame([this,download_epoch, response = std::move(response)](){
                if(this->download_epoch != download_epoch)
                    return ;
                DownloadModDatas(response);
            });
        },[this, download_epoch = this->download_epoch](float prog){
            if(this->download_epoch != download_epoch)
                return;
            std::atomic_thread_fence(std::memory_order_release);
            manifestDownloadingProgress = prog;
        });
    }

private:
    std::vector<FutureDownloadTask> downloadTask;
    int nextDownloadTask;

    void DownloadModDatas(const SSL10nManifestResponse& response){
        if(!response.IsSuccessful()){
            status = Failed;
            statusString = "Can't download manifest";
            return;
        }
        status = DownloadingMods;
        statusString = "Downloading Mods";
        if(!ParseManifest(response))
            return;

        if(!ScheduleDownloadTasks())
            return;

        nextDownloadTask = 0;
        NextDownloadTask(this->download_epoch);
    }


    bool ParseManifest(const SSL10nManifestResponse& response){
        auto & data = response.responseData;
        if(!data.has_value() || !data->IsObject()){
            status = Failed;
            statusString = "Invalid Manifest";
            return false;
        }

        {
            auto remoteMsg = data->FindMember("message");
            if(remoteMsg != data->MemberEnd()){
                if(remoteMsg->value.IsString()){
                    remoteMessage = remoteMsg->value.GetString();
                }
            }
        }
        auto mods = data->FindMember("mods");
        if(mods == data->MemberEnd()){
            status = Success;
            statusString = "Success, no mods.";
            return false;
        }
        
        if(!mods->value.IsObject()){
            status = Failed;
            statusString = "Failed, invalid manifest(mods).";
            return false;
        }

        for(auto it = mods->value.MemberBegin(), end = mods->value.MemberEnd(); it != end; ++it){
            if(!it->name.IsString())
                continue;
            if(!it->value.IsObject())
                continue;
            std::string mod_id = it->name.GetString();
            auto jsonName = it->value.FindMember("name");
            auto jsonDatas = it->value.FindMember("datas");
            if(jsonName == it->value.MemberEnd() || !jsonName->value.IsString()|| jsonDatas == it->value.MemberEnd() || !jsonDatas->value.IsArray()){
                continue;
            }
            ModInfo info {
                .name = jsonName->value.GetString()
            };
            const auto & jsonDatasArr = jsonDatas->value.GetArray();
            for(auto i=0;i<jsonDatasArr.Size();i++){
                auto & elem = jsonDatasArr[i];
                if(!elem.IsObject())
                    continue;
                auto ver = elem.FindMember("version");
                auto url = elem.FindMember("csv_url");
                //auto name = elem.FindMember("csv_name");
                auto md5 = elem.FindMember("md5");
                if(
                    ver == elem.MemberEnd() ||
                    url == elem.MemberEnd() ||
                    //name == elem.MemberEnd() ||
                    md5 == elem.MemberEnd() ||
                    !ver->value.IsString() ||
                    !url->value.IsString() ||
                    //!name->value.IsString() ||
                    !md5->value.IsString()
                ){
                    continue;
                }
                info.datas.push_back({
                    .version = ver->value.GetString(),
                    .csv_url = url->value.GetString(),
                    .md5 = md5->value.GetString(),
                    .data = std::nullopt
                });
            }

            if(info.datas.size() > 0){
                this->mods[mod_id] = std::move(info);
            }
        }
        return true;
    }

    bool ScheduleDownloadTasks(){
        downloadTask.clear();
        for(auto & mod : this->mods){
            for(auto & item : mod.second.datas){
                downloadTask.push_back({
                    .modInfo = &mod.second,
                    .modItem = &item
                });
            }
        }
        if(downloadTask.size() == 0){
            status = Success;
            statusString = "No mods need download.";
            return false;
        }
        return true;
    }


    void NextDownloadTask(int epoch){
        if(epoch != this->download_epoch)
            return;
        if(this->nextDownloadTask >= this->downloadTask.size()){
            DownloadDone(epoch);
            return;
        }
        statusString = "Downloading...";
        WebUtils::GetAsync<CsvDownloadResponse>(WebUtils::URLOptions(this->downloadTask[this->nextDownloadTask].modItem->csv_url), [this, currentIndex = this->nextDownloadTask, epoch](auto response){
            BSML::MainThreadScheduler::ScheduleNextFrame([this,epoch, currentIndex, response = std::move(response)](){
                if(this->download_epoch != epoch)
                    return ;
                const CsvDownloadResponse & resp = response;
                if(!resp.IsSuccessful()){
                    statusString = "Download Failed.";
                    status = Failed;
                    //TODO retry?
                    return;
                }
                //TODO checksum
                this->downloadTask[currentIndex].modItem->data = resp.GetParsedData();
                if(!checksum(this->downloadTask[currentIndex].modItem)){
                    statusString = "Download Failed. Invalid checksum.";
                    status = Failed;
                    return;
                }
                this->nextDownloadTask++;
                this->NextDownloadTask(epoch);
            });
        },[this, epoch](float prog){
            //do nothing...
        });
    }

    bool checksum(ModItem * item){
        return true;
    }
    void DownloadDone(int epoch){
        //TODO: save disk

        status = Success;
        statusString = "Download success";
    }
};

}

