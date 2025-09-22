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

    static std::string prettifyName(const std::string& raw) {
        std::string result;
        bool capitalize = true;

        for (char c : raw) {
            if (c == '_') {
                result.push_back(' ');
                capitalize = true;
            }
            else {
                if (capitalize && std::isalpha(static_cast<unsigned char>(c))) {
                    result.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
                    capitalize = false;
                }
                else {
                    result.push_back(c);
                }
            }
        }

        return result;
    }

    std::string getWallpaperName() const {
        std::string raw = Request::fetchWallpaper().name;
        return prettifyName(raw);
    }

    void downloadWallpaper() const {
        Wallpaper wallpaper = Request::fetchWallpaper();
        std::string imagePath = "%TEMP%\\" + wallpaper.name + ".jpg";
        httplib::Client client("www.bing.com");
        auto response = client.Get(wallpaper.url.c_str());
        if (response && response->status == 200) {
            FILE* file = fopen(imagePath.c_str(), "wb");
            if (file) {
                fwrite(response->body.c_str(), 1, response->body.size(), file);
                fclose(file);
            }
            else {
                std::cerr << "Error: Unable to open file for writing: " << imagePath << std::endl;
            }
        }
        else {
            std::cerr << "Error: Failed to download wallpaper. Code: " << (response ? response->status : -1) << std::endl;
		}
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