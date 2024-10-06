#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <iostream>

constexpr const char* bingApiEndpoint = "/HPImageArchive.aspx?format=js&idx=0&n=1&mkt=en-US";
constexpr const char* bingBaseUrl = "https://www.bing.com";

struct Wallpaper {
    std::string url;
    std::string name;
};

namespace Request {
    Wallpaper fetchWallpaper() {
        Wallpaper wallpaper;

        httplib::Client client("www.bing.com");
        auto response = client.Get(bingApiEndpoint);

        if (response) {
            auto jsonResponse = nlohmann::json::parse(response->body);
            const auto& imageInfo = jsonResponse["images"][0];

            wallpaper.url = bingBaseUrl + imageInfo["url"].get<std::string>();

            std::string title = imageInfo["title"].get<std::string>();
            std::replace(title.begin(), title.end(), ' ', '_');
            std::transform(title.begin(), title.end(), title.begin(), ::tolower);
            wallpaper.name = title;

        }
        else {
            std::cerr << "Error: Failed to fetch wallpaper data. Code: " << response.error() << std::endl;
        }

        return wallpaper;
    }
}

class BingWallpaper {
public:
    std::string getWallpaperUrl() const {
        return Request::fetchWallpaper().url;
    }

    std::string getWallpaperName() const {
        return Request::fetchWallpaper().name;
    }

    void downloadWallpaper() const {
        Wallpaper wallpaper = Request::fetchWallpaper();
        std::string downloadCommand = "curl -o %TEMP%\\" + wallpaper.name + ".jpg " + wallpaper.url;
        system(downloadCommand.c_str());
    }

    void setWallpaper() const {
        Wallpaper wallpaper = Request::fetchWallpaper();
        std::string imagePath = "%TEMP%\\" + wallpaper.name + ".jpg";

        int wideLength = MultiByteToWideChar(CP_UTF8, 0, imagePath.c_str(), -1, nullptr, 0);
        std::wstring widePath(wideLength, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, imagePath.c_str(), -1, &widePath[0], wideLength);

        SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, (PVOID)widePath.c_str(), SPIF_UPDATEINIFILE);
    }
};

extern BingWallpaper bing = BingWallpaper();