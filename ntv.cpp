#include <stdio.h>
#include <string>
#include <iostream>
//#include <regex>
#include <fstream>
#include <unistd.h>
#include <algorithm>
#include <vector>
#include <errno.h>
#include <unistd.h>
#include <resolv.h>
#include <netdb.h>
#include <sstream>
#include <fcntl.h>	//fcntl

#include "json.hpp"

#if defined(USE_CURL)
    #include <curl/curl.h>
	//#pragma comment(lib,"curllib-bcb.lib") // Для C++Builder
	//#pragma comment(lib,"curllib.lib")    // для VC++
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
	#ifndef USE_SOCKET
		#define USE_SOCKET
	#endif
#endif

#include "translete.h"	//fcntl

#ifdef MultiThread
    #include <thread>
    #include <future>
#endif

#define Host "www.ntv.ru"
#ifndef SymPath
	#define SymPath "/"
#endif
#ifndef AllMapperFile
	#define AllMapperFile "NTV.txt"
#endif
#ifndef MaxName
	#define MaxName 250
#endif
#ifndef GTDELIMITER
	#define GTDELIMITER ";"
#endif

#define IsStr(str, fin) (str.find(fin) != std::string::npos)
#define GB() std::string m3uL1 = "#EXTINF:-1 ";\
			m3uL1 += "age-level=\""+AgeLevel(trim(j_prog["r"].value("v", "")))+"\" ";\
			m3uL1 += "tvg-id=\""+PrefixID + std::to_string(it["data"].value("id", 0))+"\" ";\
			std::string logo = j_prog.value("preview", "");\
			std::string mainGenre = "";\
			\
			if(!Settings[3]) m3uL1 += "tvg-logo=\""+logo+"\" ";\
			m3uL1 += "tvg-name=\""+RemoveSpecSym(j_prog.value("title", "")).substr(0, MaxName)+"\" ";\
			if(!Settings[22]) m3uL1 += "asset-type=\""+(std::string)(Settings[15] ? "SD" : "HD")+"\" ";\
			if(!Settings[10]) m3uL1 += "series-id=\""+PrefixID + std::to_string(j_prog.value("id", 0))+"\" ";\
			if(!Settings[11]) m3uL1 += "season-num=\""+std::to_string(seasonNum)+"\" ";\
			if(!Settings[23]) m3uL1 += "mov-year=\""+std::to_string(getYear(it["data"].value("ts", (unsigned long)0)))+"\" ";\
			if(!Settings[20]) m3uL1 += "season-name=\""+RemoveSpecSym(it["data"].value("title", ""))+"\" ";\
			if(!Settings[5] && Settings[19] && Settings[21]){std::string tgr = GetGenre(&j_prog, mainGenre); std::vector<std::string> grs = split(tgr, GTDELIMITER[0]); tgr = ""; for(auto iq : grs){tgr += GroupPrefix + " - " + iq + ";";} tgr = tgr.substr(0, tgr.length() - 1); m3uL1 += "group-title=\""+tgr+"\""; }\
			if(!Settings[5] && !Settings[21]) m3uL1 += "group-title=\""+(Settings[19] ? GroupPrefix + " - " : "")+GetGenre(&j_prog, mainGenre)+"\"";\
			m3uL1 += ","+title;\
			\
			\
			GenerateArchive(fout, &it["data"], title, j_prog.value("shortcat", ""), j_prog.value("id", 0), seasonNum, GetGenre(&j_prog, mainGenre), RemoveSpecSym(j_prog.value("title", "")).substr(0, MaxName), mainGenre);\
			seasonNum++
/*
			(*fout) << m3uL1 << std::endl;\
			(*fout) << j_prog.value("preview", "") << std::endl;\
*/

using json = nlohmann::json;

int CountSeries = 0;
std::string WorkMode = "pr";
std::string WorkParam = "";
std::string FileM3U = "NTV.m3u8";
std::string DirOut = "./NTV";
std::string GenreParse = "";
std::string PrefixID = "";
std::string GroupPrefix = "";
std::vector<std::string> linesD;

#if defined(USE_CURL)
	const std::string LoadMain = "https://www.ntv.ru/m/v10/pr";
	const std::string LoadProgramm = "https://www.ntv.ru/m/v10/prog/{progid}";
	const std::string LoadArchive = "https://www.ntv.ru/m/v10/prog/{progid}/archive/{archiveid}?limit=100&offset={offset}";
	const std::string LoadVideoGalerey = "https://www.ntv.ru/m/v10/prog/{progid}/videogallery/{galereyid}?limit=100&offset={offset}";
	const std::string LoadVideo = "https://www.ntv.ru/m/v10/v/{videoid}";
	const std::string LoadBase = "https://www.ntv.ru/m/v10";
#else
	const std::string LoadMain = "/m/v10/pr";
	const std::string LoadProgramm = "/m/v10/prog/{progid}";
	const std::string LoadArchive = "/m/v10/prog/{progid}/archive/{archiveid}?limit=100&offset={offset}";
	const std::string LoadVideoGalerey = "/m/v10/prog/{progid}/videogallery/{galereyid}?limit=100&offset={offset}";
	const std::string LoadVideo = "/m/v10/v/{videoid}";
	const std::string LoadBase = "/m/v10";
#endif

const char buf[] = "-\\|/";

bool Settings[25];

#if defined(USE_CURL)
std::string escape(CURL *curl_handle, const std::string& text)
{
    std::string result;
    char* esc_text= curl_easy_escape(curl_handle, text.c_str(), text.length());
    if(!esc_text) throw std::runtime_error("Can not convert string to URL");
 
    result = esc_text;
    curl_free(esc_text);
 
    return result;
}
#endif

#ifndef USE_CURL
struct
{
	std::string method;
    std::string query;
    std::string protocol;
	std::string url;
	std::map<std::string, std::string> params;
} typedef HttpRequest;

HttpRequest ParseHeaders(std::string request){
    std::istringstream iss(request);
    HttpRequest ret;
    if(!(iss >> ret.method >> ret.query >> ret.protocol))
    {
        std::cout << "ERROR: parsing request\n";
        return ret;
    }
    iss.clear();
    iss.str(ret.query);
    if(!std::getline(iss, ret.url, '?')) // remove the URL part
    {
        std::cout << "ERROR: parsing request url\n";
        return ret;
    }
    std::string keyval, key, val;
    while(std::getline(iss, keyval, '&')) // split each term
    {
        std::istringstream iss(keyval);
        if(std::getline(std::getline(iss, key, '='), val))
            ret.params[key] = val;
    }  
    return ret;
}
#endif

std::string readFile(const std::string& fileName) {
    std::ifstream f(fileName);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static size_t write_data(char *ptr, size_t size, size_t nmemb, std::string* data)
{
    if (data)
    {
        data->append(ptr, size*nmemb);
        return size*nmemb;
    }
    else 
    return 0;  // будет ошибка
}

#ifndef USE_CURL
int OpenConnection(const char *hostname, const char *port)
{
    struct hostent *host;
    if ((host = gethostbyname(hostname)) == nullptr)
    {
        perror(hostname);
        exit(EXIT_FAILURE);
    }

    struct addrinfo hints = {0}, *addrs;
    //hints.ai_family = AF_UNSPEC;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    const int status = getaddrinfo(hostname, port, &hints, &addrs);
    if (status != 0)
    {
        fprintf(stderr, "%s: %s\n", hostname, gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    int sfd, err;
    for (struct addrinfo *addr = addrs; addr != nullptr; addr = addr->ai_next)
    {
        sfd = socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol);
        if (sfd == -1)
        {
            err = errno;
            continue;
        }

        if (connect(sfd, addr->ai_addr, addr->ai_addrlen) == 0)
        {
            break;
        }

        err = errno;
        sfd = -1;
        close(sfd);
    }

    freeaddrinfo(addrs);

    if (sfd == -1)
    {
        fprintf(stderr, "%s: %s\n", hostname, strerror(err));
        exit(EXIT_FAILURE);
    }
    return sfd;
}
#endif

#if !defined(USE_CURL)
std::string unsafe_getParam(const std::string&request)
{
  return split(request,"\r\n\r\n")[1];
}
#endif

#if defined(USE_CURL)
std::string SendRequest(std::string url){
	CURL* curl_handle = curl_easy_init();
	std::string content;
    if(curl_handle)
    {
        // задаем  url адрес
        #ifdef Debug
            std::cout << url << std::endl;
        #endif
        curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl_handle, CURLOPT_USERAGENT,"User-Agent': 'ru.ntv.client_4.5.1");
		
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &content);
		curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 80L);
		curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, 80L);
		curl_easy_setopt(curl_handle, CURLOPT_DNS_CACHE_TIMEOUT, 80L);
		curl_easy_setopt(curl_handle, CURLOPT_POST, 0); // выключаем POST
		curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);// включаем GET
		curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER , 0L);// включаем GET
		#ifdef DebugCurl
			curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
		#endif
		
        // выполняем запрос
        CURLcode res = curl_easy_perform(curl_handle);
		
        // закрываем дескриптор curl
        curl_easy_cleanup(curl_handle);	
    }
	return content;
}
#else
std::string SendRequest(std::string url){
    const int sfd = OpenConnection(Host, "80");

    std::string req = "GET {url} HTTP/1.0\r\nHost: {host}\r\n\r\n";
    replace(req, "{url}", url);
    replace(req, "{host}", Host);
    
    std::string reads;

    send(sfd, req.c_str(), req.size(), 0);
    
    std::vector<char> result;
    int size;
    char recv_buf[250];

    for(;;)
    {
        if((size = recv(sfd, recv_buf, sizeof(recv_buf), 0)) > 0)
        {
            for(unsigned int i = 0; i < size; ++i)
                result.push_back(recv_buf[i]);
        }
        else if(size == 0)
        {            
            break;
        }
        else
        {
            perror("recv");
            return "";
        }
    }
    reads = std::string(result.begin(), result.end());
    
    //std::cout << reads << std::endl;
    close(sfd);
    
    std::string content = unsafe_getParam(reads);
    return content;
}
#endif

/*void RenameFromDict(std::string &nameC, std::string chID){	
	int size = static_cast<int>(linesD.size());
	for(int i = 0; i < size; i++){
		if(linesD[i][0] == '#' || linesD[i][0] == '\0' || linesD[i][0] == ' ')
			continue;
		
		std::string dName = split(linesD[i], '=')[0];
		std::string dVame = split(linesD[i], '=')[1];
		
		if(IsStr(dName, "ID:") && chID == split(dName, ':')[1]){
			nameC = dVame;
			return;
		}
		
		if(nameC == dName){
			nameC = dVame;
			return;
		}
	}
}*/

int GetHourFromTimeZone(std::string timezone){
	timezone = split(timezone, '+')[1];
	timezone = split(timezone, ':')[0];
	return std::stoi(timezone);
}

std::string GetGenre(json *j_prog, std::string &mainGR){
	if(!Settings[21]){
		for(uint8_t i = 0; i < (*j_prog)["genres"].size(); i++){		
			if((*j_prog)["genres"][i].value("id", 0) != 103){
				mainGR = RemoveSpecSym((*j_prog)["genres"][i].value("title", ""));
				return RemoveSpecSym((*j_prog)["genres"][i].value("title", ""));
			}
		}
	}
	else{
		std::string tmp = "";
		mainGR = "";
		bool isMG = false;
		for(uint8_t i = 0; i < (*j_prog)["genres"].size(); i++){				
			tmp += RemoveSpecSym((*j_prog)["genres"][i].value("title", "")) + GTDELIMITER;
			if(!isMG && (*j_prog)["genres"][i].value("id", 0) != 103){
				isMG = true;
				mainGR = RemoveSpecSym((*j_prog)["genres"][i].value("title", ""));
			}
		}
		if(!isMG)
			mainGR = RemoveSpecSym((*j_prog)["genres"][0].value("title", "Архив программ"));
		tmp = tmp.substr(0, tmp.length() - 1);
		return tmp;
	}
	return "Архив программ";
}

std::string AgeLevel(std::string t){
	return IsStr(t, "Без ограничений") ? "0" : t;
}

std::string GetSerialShortcat(std::string idSerial){
	#ifndef NOMAPPER
	std::string PathMapperFile = DirOut + SymPath + AllMapperFile;
	std::string MapperFile = readFile(PathMapperFile);
	if(MapperFile == ""){
		std::cout << "Mapper file is none found. Load from server" << std::endl;
	#endif
		std::string allplay = SendRequest(LoadMain);
		json j_complete = json::parse(allplay)["data"];
		for (auto it : j_complete["genres"])
		{
			for (auto it1 : it["programs"])
			{				
				if(idSerial == std::to_string(it1.value("id", 0))){			
					return it1.value("link", "");
				}
			}
		}
	#ifndef NOMAPPER
	}
	else{
		for(int i = 0; i < split(MapperFile, '\n').size(); i++){
			std::string mLine = split(MapperFile, '\n')[i];
			std::string serialID = split(mLine, '|')[1];
			std::string serialPath = split(mLine, '|')[2];
			if(serialID == idSerial){
				return serialPath;
			}
		}
	}
	#endif
	std::cout << "Error. Serial ID not found" << std::endl;
	return "";
}

void GenerateDesc(){
	if(WorkParam.size() > 5){
		std::cout << "Is episode" << std::endl;
		std::cout << "None released. Please, donate for developers" << std::endl;
	}
	else if(WorkParam.size() > 4){
		std::cout << "Is bundle" << std::endl;
		std::cout << "None released. Please, donate for developers" << std::endl;
	}
	else{
		std::cout << "Is Serial" << std::endl;
		std::string SC = split(GetSerialShortcat(WorkParam), '/')[2];
		std::string url = LoadProgramm;
		replaceAll(url, "{progid}", SC);
		std::string loadProg = SendRequest(url);			
		json j_prog1 = json::parse(loadProg)["data"];
		std::ofstream fout23(FileM3U);
		for (auto it1 : j_prog1["menus"])
		{
			if(it1.value("type", "") == "about"){ // Текстовик с описанием сериала
				fout23 << RemoveDescSym(it1["data"].value("txt", ""));
				break;
			}
		}		
		fout23.close();
	}
}

void GenerateArchive(std::ofstream *fout, json *j_progit, std::string& title, std::string progid, int serialID, int seasonNum, std::string groupName, std::string serialName, std::string &mainGenre, int offset = 0){
	int CountArchive = (*j_progit).value("issue_count", 0);
	std::string url = LoadArchive;
	replaceAll(url, "{offset}", std::to_string(offset));
	replaceAll(url, "{progid}", progid);
	replaceAll(url, "{archiveid}", std::to_string((*j_progit).value("id", 0)));
	std::string loadProg = SendRequest(url);
	json j_prog = json::parse(loadProg)["data"];
	int episodeNum = 0 + offset;
	
	float dper = 100 / (float)CountArchive;
	
	//Тут парсим
	for (auto it : j_prog["archive"]["issues"])
	{
		/*
			#EXTINF:-1 tvg-id="189" tvg-logo="http://tvcdn.na4u.ru/posters/189.jpg" tvg-name="Три богатыря" series-id="67" season-num="1" group-title="МУЛЬТСЕРИАЛЫ",Три богатыря
			http://85.118.231.98:3001/movies/189.mp4
			#EXTINF:-1 tvg-id="67" mov-year="2004" tvg-logo="http://tvcdn.na4u.ru/icon/7267.jpg" tvg-name="Алеша Попович и Тугарин Змей" series-id="67" season-num="1" episode-num="1" mov-country="РОССИЯ" mov-duration="4560" group-title="МУЛЬТСЕРИАЛЫ",Алеша Попович и Тугарин Змей
			http://85.118.231.98:3001/movies/7267.m2ts
		*/
		episodeNum++;
		
		float percents = dper * episodeNum;
		printf("%c %02d%%", buf[(int)percents % 4], (int)percents);
		fflush(stdout);
		printf("\b\b\b\b\b");
		
		if(true){
			std::string dirN = DirOut + SymPath + Transliterate(mainGenre) + SymPath + "desc";
			std::system(("mkdir -p " + dirN).c_str());
			if(it.value("txt", "") != ""){
				std::ofstream fout3(dirN + SymPath + std::to_string(it.value("id", 0)) + ".txt");
				fout3 << it.value("txt", "");
				fout3.close();
			}
		}
		
		std::string m3uL1 = "#EXTINF:-1 ";
		
		m3uL1 += "age-level=\""+AgeLevel(trim(it["r"].value("v", "")))+"\" ";
		m3uL1 += "tvg-id=\""+PrefixID + std::to_string(it.value("id", 0))+"\" ";
		//m3uL1 += "mov-year=\""+std::to_string(it.value("id", 0))+"\" ";
		if(!Settings[3]){
			std::string logo = it["video_list"][0].value("preview", "");
			//replaceAll(logo, "https", "http");
			m3uL1 += "tvg-logo=\""+logo+"\" ";
		}
		//m3uL1 += "tvg-name=\""+RemoveSpecSym(it.value("title", "").substr(0, MaxName))+"\" ";
		m3uL1 += "tvg-name=\""+serialName+"\" ";
		if(!Settings[22])
			m3uL1 += "asset-type=\""+(std::string)(Settings[15] ? "SD" : "HD")+"\" ";		
		if(!Settings[10])
			m3uL1 += "series-id=\""+PrefixID + std::to_string(serialID)+"\" ";
		if(!Settings[11])
			m3uL1 += "season-num=\""+std::to_string(seasonNum)+"\" ";
		if(!Settings[12])
			m3uL1 += "episode-num=\""+std::to_string(episodeNum)+"\" ";
		if(!Settings[20]) 
			m3uL1 += "season-name=\""+RemoveSpecSym((*j_progit).value("title", ""))+"\" ";
		if(!Settings[23]){
			m3uL1 += "mov-year=\""+std::to_string(getYear(it.value("ts", (unsigned long)0)))+"\" ";
		}
		if(!Settings[4])
			m3uL1 += "mov-country=\"РОССИЯ\" ";
		if(!Settings[17])
			m3uL1 += "mov-duration=\""+std::to_string(it["video_list"][0].value("tt", 0))+"\" ";
		if(!Settings[5] && Settings[19] && Settings[21]){std::string tgr = groupName; std::vector<std::string> grs = split(tgr, GTDELIMITER[0]); tgr = ""; for(auto iq : grs){tgr += GroupPrefix + " - " + iq + ";";} tgr = tgr.substr(0, tgr.length() - 1); m3uL1 += "group-title=\""+tgr+"\""; }\
		else if(!Settings[5] && !Settings[21])
			m3uL1 += "group-title=\""+(Settings[19] ? GroupPrefix + " - " : "")+groupName+"\"";		
		
		m3uL1 += ","+RemoveSpecSym(it.value("title", "").substr(0,MaxName));
		(*fout) << m3uL1 << std::endl;
		std::string playURL = split(Settings[15] ? it["video_list"][0].value("video", "") : (it["video_list"][0]["hi_video"].is_string() ? it["video_list"][0].value("hi_video", "") : it["video_list"][0].value("video", "")), '?')[0];
		replaceAll(playURL, "https", "http");
		(*fout) << playURL << std::endl;		
	
		if(Settings[16] && episodeNum >= CountSeries)
			return;
	}
	//
	if((CountArchive - offset) - 100 > 0){
		GenerateArchive(fout, j_progit, title, progid, serialID, seasonNum, groupName, serialName, mainGenre, offset + 100);
	}
}

void GenerateVideoGalerey(std::ofstream *fout, json *j_progit, std::string& title, std::string progid, int serialID, int seasonNum, std::string groupName, std::string serialName, std::string &mainGenre, int offset = 0){
	//int CountArchive = (*j_progit).value("issue_count", 0);
	std::string url = LoadVideoGalerey;
	replaceAll(url, "{offset}", std::to_string(offset));
	replaceAll(url, "{progid}", progid);
	replaceAll(url, "{galereyid}", std::to_string((*j_progit).value("id", 0)));
	std::string loadProg = SendRequest(url);
	json j_prog = json::parse(loadProg)["data"];
	int episodeNum = 0 + offset;
	
	float dper = 100 / j_prog["videogallery"]["videos"].size();
	
	//Тут парсим
	for (auto it : j_prog["videogallery"]["videos"])
	{
		if(!it.value("allowed", false))
			continue;
		
		episodeNum++;
		
		float percents = dper * episodeNum;
		printf("%c %02d%%", buf[(int)percents % 4], (int)percents);
		fflush(stdout);
		printf("\b\b\b\b\b");
		
		if(true){
			std::string dirN = DirOut + SymPath + Transliterate(mainGenre) + SymPath + "desc";
			std::system(("mkdir -p " + dirN).c_str());
			if(it.value("txt", "") != ""){
				std::ofstream fout3(dirN + SymPath + std::to_string(it.value("id", 0)) + ".txt");
				fout3 << it.value("txt", "");
				fout3.close();
			}
		}
		
		std::string m3uL1 = "#EXTINF:-1 ";
		
		m3uL1 += "age-level=\""+AgeLevel(trim(it["r"].value("v", "")))+"\" ";
		m3uL1 += "tvg-id=\""+PrefixID + std::to_string(it.value("id", 0))+"\" ";
		//m3uL1 += "mov-year=\""+std::to_string(it.value("id", 0))+"\" ";
		if(!Settings[3]){
			std::string logo = it.value("img", "");
			//replaceAll(logo, "https", "http");
			m3uL1 += "tvg-logo=\""+logo+"\" ";
		}
		//m3uL1 += "tvg-name=\""+RemoveSpecSym(it.value("title", "").substr(0, MaxName))+"\" ";			
		m3uL1 += "tvg-name=\""+serialName+"\" ";
		if(!Settings[22])
			m3uL1 += "asset-type=\""+(std::string)(Settings[15] ? "SD" : "HD")+"\" ";		
		if(!Settings[10])
			m3uL1 += "series-id=\""+PrefixID + std::to_string(serialID)+"\" ";
		if(!Settings[11])
			m3uL1 += "season-num=\""+std::to_string(seasonNum)+"\" ";
		if(!Settings[12])
			m3uL1 += "episode-num=\""+std::to_string(episodeNum)+"\" ";
		if(!Settings[20]) 
			m3uL1 += "season-name=\""+RemoveSpecSym((*j_progit).value("title", ""))+"\" ";
		if(!Settings[23]) 
			m3uL1 += "mov-year=\""+std::to_string(getYear(it.value("ts", (unsigned long)0)))+"\" ";\
		if(!Settings[4])
			m3uL1 += "mov-country=\"РОССИЯ\" ";
		if(!Settings[17])
			m3uL1 += "mov-duration=\""+std::to_string(it.value("tt", 0))+"\" ";
		if(!Settings[5] && Settings[19] && Settings[21]){std::string tgr = groupName; std::vector<std::string> grs = split(tgr, GTDELIMITER[0]); tgr = ""; for(auto iq : grs){tgr += GroupPrefix + " - " + iq + ";";} tgr = tgr.substr(0, tgr.length() - 1); m3uL1 += "group-title=\""+tgr+"\""; }\
		else if(!Settings[5])
			m3uL1 += "group-title=\""+(Settings[19] ? GroupPrefix + " - " : "")+groupName+"\"";	
		
		m3uL1 += ","+RemoveSpecSym(it.value("title", "").substr(0,MaxName));
		(*fout) << m3uL1 << std::endl;
		std::string playURL = split(Settings[15] ? it.value("video", "") : (it["hi_video"].is_string() ? it.value("hi_video", "") : it.value("video", "")), '?')[0];
		replaceAll(playURL, "https", "http");
		(*fout) << playURL << std::endl;		
	
		if(Settings[16] && episodeNum >= CountSeries)
			return;
	}
}

void GenerateBandles(std::ofstream *fout, std::string& progid, std::string& genre){
	std::string url = LoadProgramm;
	replaceAll(url, "{progid}", progid);
	std::string loadProg = SendRequest(url);
	/*#ifdef Debug
		std::ofstream fout2("ntv_api_prog_"+split((*j_channel).value("link", ""), '/')[2]+".json");
		fout2 << loadProg;
		fout2.close();
	#endif*/
	
	json j_prog = json::parse(loadProg)["data"];
	std::string mainGenre = "";
	genre = Transliterate(GetGenre(&j_prog, mainGenre));
	genre = Transliterate(mainGenre);
	
	std::cout << "Parse " << j_prog.value("title", "") << "..." << std::endl;
	
	std::string title = RemoveSpecSym(j_prog.value("title", ""));
	uint8_t seasonNum = 1;
	
	if(Settings[2]){
		auto it = j_prog["menus"][j_prog["menus"].size() - 1];
		GB();
	}
	else
		for (auto it : j_prog["menus"])
		{
			if(it.value("type", "") == "about") // Текстовик с описанием сериала
			{
				std::ofstream fout3(DirOut + SymPath + mainGenre + SymPath + "desc" + SymPath + std::to_string(j_prog.value("id", 0)) + ".txt");
				fout3 << it["data"].value("txt", "");
				fout3.close();
			}
			else if(it.value("type", "") == "news") // Текстовик с новостями
			{
				
			}
			else if(it.value("type", "") == "text") // Текстовик с текстом
			{
				
			}
			else if(it.value("type", "") == "faces") // Текстовик с текстом
			{
				
			}
			else if(it.value("type", "") == "videogallery") // Видео нарезки
			{
				if(Settings[6]){								
					GenerateVideoGalerey(fout, &it["data"], title, j_prog.value("shortcat", ""), j_prog.value("id", 0), seasonNum, GetGenre(&j_prog, mainGenre), RemoveSpecSym(j_prog.value("title", "")).substr(0, MaxName), mainGenre);
					seasonNum++;
				}
			}
			else if(it.value("type", "") == "archive"){ //Видосики
				//Это как раз идут бандлы
				GB();
			}
		}
}

void GenerateBandles(std::string progID){
	std::system(("mkdir -p " + DirOut).c_str());
	std::string genre = "";
	std::string filepath = Settings[9] ? (DirOut + SymPath + FileM3U) : (DirOut + SymPath + progID + ".m3u");
	std::ofstream fout2(filepath);	
	fout2 << "#EXTM3U" << std::endl;
	GenerateBandles(&fout2, progID, genre);
	std::string filepath1 = Settings[9] ? DirOut + SymPath + genre + SymPath + FileM3U : (DirOut + SymPath + genre + SymPath + progID + ".m3u");
	std::system(("mv -f " + filepath + " " + filepath1).c_str());
	fout2.close();
}

#if defined(NOMAPPER)
void GenerateSerials(json *j_channel, std::string dirName){
#else
void GenerateSerials(std::ofstream *fout, std::ofstream *fout2, json *j_channel, std::string dirName){	
#endif
	std::string genre = "";
	for (auto it : (*j_channel)["programs"])
	{
		std::string title = RemoveSpecSym(it.value("title", ""));
		replaceAll(title, " ", "_");		
		replaceAll(title, ".", "");	
		#ifndef NOMAPPER
			(*fout) << title << "|" << it.value("id", 0) << "|" << it.value("link", "") << "|" << PrefixID << it.value("id", 0) << std::endl;
			(*fout2) << title << "|" << it.value("id", 0) << "|" << it.value("link", "") << "|" << PrefixID << it.value("id", 0) << std::endl;
		#endif
		std::string fileName = dirName + SymPath + it.value("shortcat", "") + ".m3u";
		std::ofstream fout1(fileName);
		if (!fout1.is_open())
		{
			std::cout << "Parse Error. Error creating file " << fileName << std::endl;
			continue;
		}
		fout1 << "#EXTM3U" << std::endl;	
		#if defined(MultiThread)
			std::thread tA(Generatem3u, &fout1, &it);
			tA.join();
			std::async(std::launch::async, [&] {
				GenerateBandles(&fout1, split(it.value("link", ""), '/')[2], genre);
			});
		#else
			GenerateBandles(&fout1, split(it.value("link", ""), '/')[2], genre);
		#endif
		fout1.close(); // закрываем файл
		//std::cout << "Parse success. File " << fileName << " created" << std::endl;
	}
}

void ParseGenres(json *j_complete){
	std::cout << "Starting Parse genres..." << std::endl;
	std::system(("mkdir -p " + DirOut).c_str());
	#ifndef NOMAPPER
	std::string fileName1 = DirOut + SymPath + AllMapperFile;
	std::ofstream fout1(fileName1);
	if (!fout1.is_open())
	{
		std::cout << "Parse Error. Error creating file " << fileName1 << std::endl;
		return;
	}	
	#endif
	for (auto it : (*j_complete)["genres"])
	{
		std::string itval = it.value("title", "");
		//std::transform(itval.begin(), itval.end(), itval.begin(), ::tolower);
		if(GenreParse != "" && (GenreParse != itval && GenreParse != Transliterate(itval)))
			continue;
				
		std::string dirName = DirOut + SymPath + Transliterate(it.value("title", ""));
		std::system(("mkdir -p " + dirName).c_str());
		#ifndef NOMAPPER
			std::string fileName = DirOut + SymPath + Transliterate(it.value("title", "")) + ".txt";
			std::ofstream fout(fileName);		
			if (!fout.is_open())
			{
				std::cout << "Parse Error. Error creating file " << fileName << std::endl;
				continue;
			}
			fout << "Mappers for Genres:" << std::endl;	
		#endif
		//			
        #if defined(MultiThread)
            std::thread tA(Generatem3u, &fout, &it);
            tA.join();
            std::async(std::launch::async, [&] {
				#if defined(NOMAPPER)
					GenerateSerials(&it, dirName);
				#else
					GenerateSerials(&fout, &fout1, &it, dirName);
				#endif
            });
        #else
            #if defined(NOMAPPER)
				GenerateSerials(&it, dirName);
			#else
				GenerateSerials(&fout, &fout1, &it, dirName);
			#endif
        #endif
		#ifndef NOMAPPER
			fout.close(); // закрываем файл
			fout1.close(); // закрываем файл
			std::cout << "Parse success. File " << fileName << " created" << std::endl;
		#endif		
	}
}

void GenerateAllPR(json *j_complete){
	std::cout << "Loaded ";
	std::cout << (int)(*j_complete)["genres"].size();
	std::cout << " genres" << std::endl;	
	ParseGenres(j_complete);
}

void LoadFromFile(std::string fil){
	std::cout << "Loading from file API. Please wait..." << std::endl;
	std::string allplay = readFile(fil);
	json j_complete = json::parse(allplay);
	if(j_complete["attributes"].value("eid", -1) != -1){ //Это PR
		WorkMode = "pr";
		GenerateAllPR(&j_complete["data"]);
	}
	else if(j_complete["info"].is_object()){ //Это video
		WorkMode = "video";
		//GenerateAllVideo(&j_complete["info"]);
	}
	else{ //Это program
		WorkMode = "prog";
		//GenerateBandles(&j_complete["data"]);
	}
	
}

void GenerateAllPR(){
	std::cout << "Loading from server API. Please wait..." << std::endl;
	
	std::string playpath = LoadMain;	
	std::string allplay = SendRequest(playpath);
	#ifdef Debug
		std::ofstream fout("ntv_api_pr.json");
		fout << allplay;
		fout.close();
	#endif
	if(allplay == ""){
		std::cout << "Error loading from server" << std::endl;
		return;	
	}
	json j_complete = json::parse(allplay)["data"];
	GenerateAllPR(&j_complete);
}

void PrintHelp(char* nm){
    std::cout << "Usage: "<< nm << " [params] [-f file] [-m m3uFile] [-c dictFile] [-p dirOutput] [-w workMode] [-s workParam] [-z seriesCount] [-x prefixID] [-k prefixGroupe]" << std::endl;
    std::cout << "\t -f [file] - optionality, path to parse" << std::endl;
    std::cout << "\t -m [m3uFile] - optionality, path to m3u (txt) file for save" << std::endl;
    std::cout << "\t -p [dirOutput] - optionality, path to dir for save files. Default: " << DirOut << std::endl;
    std::cout << "\t -w [workMode] - optionality, mode working. Supported: pr, prog, genre, desc. (Default: pr)" << std::endl;
    std::cout << "\t -s [workParam] - optionality, param working. Prog id for 'prog', 'desc'; genre for 'genre'" << std::endl;
    std::cout << "\t -z [seriesCount] - optionality, count video for once bundle" << std::endl;
    std::cout << "\t -x [prefixID] - optionality, added prefix for id files" << std::endl;
    std::cout << "\t -k [prefixGroupe] - optionality, added prefix for 'group-title'" << std::endl;
    std::cout << std::endl;
    std::cout << "Params: " << std::endl;
    std::cout << "-h - print help information" << std::endl;
    std::cout << "-v - print version" << std::endl;
    std::cout << "-a - no generation 'age-level' tag for M3U file" << std::endl;
    std::cout << "-r - no generation 'raiting' tag for M3U file" << std::endl;
    std::cout << "-y - no generation 'tvg-logo' tag for M3U file" << std::endl;
    std::cout << "-c - no generation 'mov-country' tag for M3U file" << std::endl;
    std::cout << "-d - no generation 'mov-duration' tag for M3U file" << std::endl;
    std::cout << "-g - no generation 'group-title' tag for M3U file" << std::endl;
    std::cout << "-t - generate multi 'group-title' (delimiter " << GTDELIMITER << ") tag for M3U file" << std::endl;
    std::cout << "-i - no generation 'series-id' tag for M3U file" << std::endl;
	std::cout << "-b - no generation 'season-num' tag for M3U file" << std::endl;
	std::cout << "-u - no generation 'season-name' tag for M3U file" << std::endl;
    std::cout << "-e - no generation 'episode-id' tag for M3U file" << std::endl;
    std::cout << "-o - no generation 'asset-type' tag for M3U file" << std::endl;
    std::cout << "-j - no generation 'mov-year' tag for M3U file" << std::endl;
    std::cout << "-q - generate videogalerey to m3u" << std::endl;
    std::cout << "-l - generate only last bundle" << std::endl;
    std::cout << "-n - using video for SD resolution" << std::endl;
}
void PrintVersion(){
    std::cout << "Version: 1.2.1 ("<< __DATE__ <<")" << std::endl;
}

//ntv_x86 -w pr
//ntv_x86 -w prog -s Muhtar
//ntv_x86 -w v -s 1409622
int main(int argc, char* argv[])
{
    //signal(SIGSEGV, handler);   // install our handler
    std::string FileToParse;
	
	std::cout << "NTV - Manager. Copyring 2023 MicrofDev (Alexey Mostovenko)" << std::endl;
	std::cout << "https://github.com/microfcorp" << std::endl;
    #if defined(USE_CURL)
        std::cout << "Compile using libcurl" << std::endl;
    #else
        std::cout << "Compile using socket" << std::endl;
    #endif
    #if defined(MultiThread)
        std::cout << "MultiThread id enable" << std::endl;
    #endif
	#if defined(Debug)
        std::cout << "Debug mode" << std::endl;
    #endif
	#if defined(DebugCurl)
        std::cout << "Debug curl mode" << std::endl;
    #endif
    std::cout << std::endl;
    
    int rez = 0;

    //	opterr = 0;
	while ( (rez = getopt(argc, argv, "hvarycdibequlntojf:m:p:w:s:z:x:k:")) != -1){
		switch (rez) {
		case 'h': PrintHelp(argv[0]); return 0;
		case 'v': PrintVersion(); return 0;
		case 'a': Settings[0] = true; break;
		case 'r': Settings[1] = true; break;
		case 'y': Settings[3] = true; break;
		case 'c': Settings[4] = true; break;
		case 'g': Settings[5] = true; break;
		case 'q': Settings[6] = true; break;
		case 'i': Settings[10] = true; break;
		case 'b': Settings[11] = true; break;
		case 'e': Settings[12] = true; break;
		case 'd': Settings[17] = true; break;
		case 'u': Settings[20] = true; break;
		case 't': Settings[21] = true; break;
		case 'o': Settings[22] = true; break;
		case 'j': Settings[23] = true; break;
		case 'l': Settings[2] = true; break;
		case 'n': Settings[15] = true; break;
		case 'f':{ Settings[8] = true; FileToParse = optarg; break; }
		case 'm':{ Settings[9] = true; FileM3U = optarg; break; }
		case 'p':{ Settings[7] = true; DirOut = optarg; break; }
		case 'w':{ Settings[14] = true; WorkMode = optarg; break; }
		case 's':{ Settings[13] = true; WorkParam = optarg; break; }
		case 'z':{ Settings[16] = true; CountSeries = atoi(optarg); break; }
		case 'x':{ Settings[18] = true; PrefixID = optarg; break; }
		case 'k':{ Settings[19] = true; GroupPrefix = optarg; break; }
		} // switch
	} // while

	if(Settings[8])
        LoadFromFile(FileToParse);
	else if(WorkMode == "pr")
		GenerateAllPR();
	else if(WorkMode == "prog"){
		GenerateBandles(WorkParam);
	}
	else if(WorkMode == "genre"){
		std::transform(GenreParse.begin(), GenreParse.end(), GenreParse.begin(), tolower);
		GenreParse = WorkParam;
		GenerateAllPR();
	}
	else if(WorkMode == "desc"){	
		replaceAll(WorkParam, PrefixID, "");
		GenerateDesc();
	}
	else{
		std::cout << "WorkMode not found" << std::endl;
		return 1;
	}
    
    std::cout << "Bye" << std::endl;
    return 0;
}
