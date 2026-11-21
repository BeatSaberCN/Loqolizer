#pragma once

#include "EmbbedData.hpp"
#include "bsml/shared/BSML/MainThreadScheduler.hpp"
#include "web-utils/shared/WebUtils.hpp"
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
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


    static NetDataManager instance;

    bool IsBusy() const {
        return status == DownloadingManifest || status == DownloadingMods;
    }

    bool CanDownload() const {
        return status == NewDataAvaliable || (status == Failed && !remoteProtobufDownloadUrl.empty());
    }

    void FetchManifest(){
        if(IsBusy()) return;
        remoteProtobufDownloadUrl.clear();
        // clear data
        download_epoch++;
        downloadingProgress = 0;
        status = DownloadingManifest;
        
        statusString = "Downloading manifest";

        WebUtils::GetAsync<WebUtils::JsonResponse>(WebUtils::URLOptions(LangCtrl::getManifestUrl()), [this, download_epoch = this->download_epoch](auto response){
            auto result = std::make_shared<decltype(response)>(std::move(response));
            BSML::MainThreadScheduler::ScheduleNextFrame([this,download_epoch, result](){
                if(this->download_epoch != download_epoch)
                    return ;
                DownloadModDatas(*result);
            });
        },[this, download_epoch = this->download_epoch,
           lastReport = std::optional<std::chrono::steady_clock::time_point>{}](float prog) mutable {
            const auto now = std::chrono::steady_clock::now();
            if(lastReport && now - *lastReport < std::chrono::milliseconds(200))
                return;
            lastReport = now;
            BSML::MainThreadScheduler::ScheduleNextFrame([this, download_epoch, prog](){
                if(this->download_epoch == download_epoch && IsBusy())
                    downloadingProgress = prog;
            });
        });
    }

    void DownloadData(){
        if(!CanDownload()){
            return;
        }
        ++download_epoch;
        downloadingProgress = 0;
        status = DownloadingMods;
        statusString = "Downloading translations";
        WebUtils::GetAsync<WebUtils::DataResponse>(WebUtils::URLOptions(remoteProtobufDownloadUrl), [this, download_epoch = this->download_epoch](auto response){
            auto result = std::make_shared<decltype(response)>(std::move(response));
            BSML::MainThreadScheduler::ScheduleNextFrame([this,download_epoch, result](){
                if(this->download_epoch != download_epoch)
                    return ;
                OnDownloadDone(*result);
            });
        },[this, download_epoch = this->download_epoch,
           lastReport = std::optional<std::chrono::steady_clock::time_point>{}](float prog) mutable {
            const auto now = std::chrono::steady_clock::now();
            if(lastReport && now - *lastReport < std::chrono::milliseconds(200))
                return;
            lastReport = now;
            BSML::MainThreadScheduler::ScheduleNextFrame([this, download_epoch, prog](){
                if(this->download_epoch == download_epoch && IsBusy())
                    downloadingProgress = prog;
            });
        });

    }
private:
    void Status(const std::string& str){
        statusString = str;
    }
    void DownloadModDatas(const WebUtils::JsonResponse& response){
        if(!response.IsSuccessful()){
            status = Failed;
            Status("Can't download manifest");
            return;
        }

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


        downloadingProgress = 1;
        if(LangCtrl::activated_snapshot && LangCtrl::activated_snapshot->modshashes() == remoteModHashesStr){
            status = AlreadyLatest;
            Status("Already up to date");
        }else{
            status = NewDataAvaliable;
            Status("New translations available");
        }

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

