#pragma once

#include "EmbbedData.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"
#include "web-utils/shared/WebUtils.hpp"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <string>

#include "../src/protobuf/snapshot.pb.h"

namespace Web{

class NetDataManager{
public:
enum DownloadStatus{
    Idle,
    DownloadingManifest,
    AlreadyLatest,

    NewDataAvaliable,
    DownloadingMods,
    Success,
    Failed
};
    DownloadStatus status = Idle;
    int download_epoch = 0;

    float downloadingProgress = 0;
    std::string statusString = "";

    std::string remoteProtobufDownloadUrl = "";


    void FetchManifest(){
        // clear data
        download_epoch++;
        std::atomic_thread_fence(std::memory_order_release);
        downloadingProgress = 0;
        status = DownloadingManifest;
        
        statusString = "Downloading";

        WebUtils::GetAsync<WebUtils::JsonResponse>(WebUtils::URLOptions(LangCtrl::getManifestUrl()), [this, download_epoch = this->download_epoch](auto response){
            BSML::MainThreadScheduler::ScheduleNextFrame([this,download_epoch, response = std::move(response)](){
                if(this->download_epoch != download_epoch)
                    return ;
                DownloadModDatas(response);
            });
        },[this, download_epoch = this->download_epoch](float prog){
            if(this->download_epoch != download_epoch)
                return;
            std::atomic_thread_fence(std::memory_order_release);
            downloadingProgress = prog;
        });
    }

    void DownloadData(){
        if(status != NewDataAvaliable){
            return;
        }
        status = DownloadingMods;
        WebUtils::GetAsync<WebUtils::DataResponse>(WebUtils::URLOptions(remoteProtobufDownloadUrl), [this, download_epoch = this->download_epoch](auto response){
            BSML::MainThreadScheduler::ScheduleNextFrame([this,download_epoch, response = std::move(response)](){
                if(this->download_epoch != download_epoch)
                    return ;
                OnDownloadDone(response);
            });
        },[this, download_epoch = this->download_epoch](float prog){
            if(this->download_epoch != download_epoch)
                return;
            std::atomic_thread_fence(std::memory_order_release);
            downloadingProgress = prog;
        });

    }
private:
    void Status(const std::string& str){
        BSML::MainThreadScheduler::Schedule([this, str = str](){
            statusString = str;
        });
    }
    void DownloadModDatas(const WebUtils::JsonResponse& response){
        if(!response.IsSuccessful()){
            status = Failed;
            Status("Can't download manifest");
            return;
        }

        status = DownloadingMods;
        Status("Downloading Mods");
        if(!ParseManifest(response))
            return;
    }


    bool ParseManifest(const WebUtils::JsonResponse& response){
        auto & data = response.responseData;
        if(!data.has_value() || !data->IsObject()){
            status = Failed;
            Status("Invalid Manifest");
            return false;
        }

        std::string remoteModHashesStr;
        {
            auto remoteModHashes = data->FindMember("modsHashesInProtobuf");
            if(remoteModHashes == data->MemberEnd() || !remoteModHashes->value.IsString()){
                status = Failed;
                Status("Invalid Manifest(mod hashes not found)");
                return false;
            }
            remoteModHashesStr = remoteModHashes->value.GetString();
        }
        {
            auto binUrl = data->FindMember("protobufSnapshotBinaryUrl");
            if(binUrl == data->MemberEnd() || !binUrl->value.IsString()){
                status = Failed;
                Status("Invalid Manifest(url not found)");
                return false;
            }
            remoteProtobufDownloadUrl = binUrl->value.GetString();
        }


        BSML::MainThreadScheduler::Schedule([this, remoteModHashesStr](){
            if(LangCtrl::activated_snapshot && LangCtrl::activated_snapshot->modshashes() == remoteModHashesStr){
                status = AlreadyLatest;
                return ;
            }
            status = NewDataAvaliable;
        });

        return true;
    }

    bool OnDownloadDone(const WebUtils::DataResponse& response){
        if(!response.IsSuccessful() || !response.responseData.has_value()){
            status = Failed;
            Status("Download Failed.");
            return false;
        }

        // try parse
        Snapshot snapshot;
        if(snapshot.ParseFromArray(response.responseData->data(), response.responseData->size())){
            BSML::MainThreadScheduler::Schedule([this, download_epoch = this->download_epoch, snapshot = std::move(snapshot)]() mutable {
                if(this->download_epoch != download_epoch)
                    return;

                std::error_code error;
                std::filesystem::create_directories(MOD_STORAGE_FOLDER, error);
                if(error){
                    status = Failed;
                    statusString = "Can't create snapshot cache directory";
                    return;
                }

                std::ofstream cache(MOD_REMOTE_CACHE, std::ios::binary | std::ios::trunc);
                if(!cache || !snapshot.SerializeToOstream(&cache)){
                    status = Failed;
                    statusString = "Can't save snapshot";
                    return;
                }
                cache.close();
                if(!cache){
                    status = Failed;
                    statusString = "Can't save snapshot";
                    return;
                }

                LangCtrl::LoadSnapshot(std::move(snapshot));
                LangCtrl::activated_snapshot_status = LangCtrl::Snapshot_REMOTE;
                downloadingProgress = 1;
                statusString = "Download success";
                status = Success;
            });
        }else{
            status = Failed;
            Status("Invalid binary");
            return false;
        }

        return true;
    }

};

}

