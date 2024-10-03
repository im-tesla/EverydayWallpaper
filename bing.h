#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <iostream>

// HTTP GET
//https://www.bing.com/HPImageArchive.aspx?format=js&idx=0&n=1&mkt=en-US

/* json res
{"images":[{"startdate":"20241003","fullstartdate":"202410030700","enddate":"20241004","url":"/th?id=OHR.TajMahalReflection_EN-US5053333041_1920x1080.jpg&rf=LaDigue_1920x1080.jpg&pid=hp","urlbase":"/th?id=OHR.TajMahalReflection_EN-US5053333041","copyright":"Taj Mahal in Agra, Uttar Pradesh, India (© Tanarch/Getty Images)","copyrightlink":"https://www.bing.com/search?q=Taj+Mahal&form=hpcapt&filters=HpDate%3a%2220241003_0700%22","title":"Grand gesture of love","quiz":"/search?q=Bing+homepage+quiz&filters=WQOskey:%22HPQuiz_20241003_TajMahalReflection%22&FORM=HPQUIZ","wp":true,"hsh":"c217be5d64661c8c2317a29cadab709b","drk":1,"top":1,"bot":1,"hs":[]}],"tooltips":{"loading":"Trwa ładowanie...","previous":"Poprzedni obraz","next":"Następny obraz","walle":"Tego obrazu nie można pobrać jako tapety.","walls":"Pobierz ten obraz. Ten obraz może być używany tylko jako tapeta."}}
*/

//url == bing.com + url

struct wallpaper {
	std::string url;
	std::string name; // JSON copyright field
};

namespace req {
	inline wallpaper getWallpaper() {
		wallpaper wp;

		httplib::Client cli("www.bing.com");
		auto res = cli.Get("/HPImageArchive.aspx?format=js&idx=0&n=1&mkt=en-US");

		if (res) {
			nlohmann::json j = nlohmann::json::parse(res->body);
			wp.url = "https://www.bing.com" + j["images"][0]["url"].get<std::string>();
			wp.url = "\"" + wp.url + "\"";
			std::string name = j["images"][0]["title"].get<std::string>();
			std::replace(name.begin(), name.end(), ' ', '_');
			std::transform(name.begin(), name.end(), name.begin(), ::tolower);
			wp.name = name;

		}
		else {
			std::cout << "Error: " << res.error() << std::endl;
		}

		return wp;
	}
}

class c_Bing {
public:
	wallpaper wp = req::getWallpaper();

	std::string getWallpaperUrl() {
		return wp.url;
	}

	std::string getWallpaperName() {
		return wp.name;
	}

	void downloadWallpaper() {
		std::string cmd = "curl -o %TEMP%\\" + wp.name + ".jpg " + wp.url;
		system(cmd.c_str());
	}

	void setWallpaper() {
		std::string path = "%TEMP%\\" + wp.name + ".jpg";
		int wlen = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, NULL, 0);
		std::wstring wpath(wlen, L'\0');
		MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, &wpath[0], wlen);

		SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, (PVOID)wpath.c_str(), SPIF_UPDATEINIFILE);
	}
};

extern c_Bing bing = c_Bing();
