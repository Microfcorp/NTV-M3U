#pragma once

#define LEAP_YEAR(Y)     ( (Y>0) && !(Y%4) && ( (Y%100) || !(Y%400) ) )

bool replace(std::string& str, std::string from, std::string to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

void replaceAll(std::string& str, std::string from, std::string to) {
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

const std::string WHITESPACE = " \n\r\t\f\v+";
std::string ltrim(const std::string &s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}
 
std::string rtrim(const std::string &s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}
 
std::string trim(const std::string &s) {
    return rtrim(ltrim(s));
}

std::vector<std::string> split(std::string s, char delim) {
	std::stringstream ss(s);
	std::string item;
	std::vector<std::string> elems;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	 //elems.push_back(std::move(item)); // if C++11
	}
	return elems;
}

std::vector<std::string> split(const std::string&s,const std::string&needle)
{
	std::vector<std::string> arr;
	if(s.empty())return arr;
	size_t p=0;
	for(;;){
	auto pos=s.find(needle,p);
	if(pos==std::string::npos){arr.push_back(s.substr(p));return arr;}
		arr.push_back(s.substr(p,pos-p));
		p=pos+needle.size();
	}
	return arr;
}

std::string RemoveSpecSym(std::string str){
	replaceAll(str, "«", "");
	replaceAll(str, "»", "");
	replaceAll(str, "=", "");
	replaceAll(str, ",", "");
	replaceAll(str, "—", " ");	
	replaceAll(str, "#", "");	
	replaceAll(str, "@", "");	
	replaceAll(str, "+", "");	
	replaceAll(str, "~", "");
	replaceAll(str, "-", " ");
	replaceAll(str, "\"", "");
	replaceAll(str, "!", "");
	return str;
}

std::string RemoveDescSym(std::string str){
	replaceAll(str, "«", "");
	replaceAll(str, "»", "");
	replaceAll(str, ",", "");
	replaceAll(str, "—", "-");	
	replaceAll(str, "#", "");	
	replaceAll(str, "@", "");	
	replaceAll(str, "+", "");	
	replaceAll(str, "~", "");
	replaceAll(str, "\"", "");
	return str;
}

std::string Transliterate(std::string str)
{
	//if(str[0] == ' ')
	//	str[0] = '\0';
	
	replaceAll(str, "а", "a");
	replaceAll(str, "б", "b");
	replaceAll(str, "в", "v");
	replaceAll(str, "г", "g");
	replaceAll(str, "д", "d");
	replaceAll(str, "е", "e");
	replaceAll(str, "ё", "ye");
	replaceAll(str, "ж", "zh");
	replaceAll(str, "з", "z");
	replaceAll(str, "и", "i");
	replaceAll(str, "й", "y");
	replaceAll(str, "к", "k");
	replaceAll(str, "л", "l");
	replaceAll(str, "м", "m");
	replaceAll(str, "н", "n");
	replaceAll(str, "о", "o");
	replaceAll(str, "п", "p");
	replaceAll(str, "р", "r");
	replaceAll(str, "с", "s");
	replaceAll(str, "т", "t");
	replaceAll(str, "у", "u");
	replaceAll(str, "ф", "f");
	replaceAll(str, "х", "h");
	replaceAll(str, "ц", "z");
	replaceAll(str, "ч", "ch");
	replaceAll(str, "ш", "sh");
	replaceAll(str, "щ", "ch");
	replaceAll(str, "ъ", "");
	replaceAll(str, "ы", "y");
	replaceAll(str, "ь", "");
	replaceAll(str, "э", "ye");
	replaceAll(str, "ю", "yu");
	replaceAll(str, "я", "ya");
	
	replaceAll(str, "А", "A");
	replaceAll(str, "Б", "B");
	replaceAll(str, "В", "V");
	replaceAll(str, "Г", "G");
	replaceAll(str, "Д", "D");
	replaceAll(str, "Е", "E");
	replaceAll(str, "Ё", "YE");
	replaceAll(str, "Ж", "ZH");
	replaceAll(str, "З", "Z");
	replaceAll(str, "И", "I");
	replaceAll(str, "Й", "Y");
	replaceAll(str, "К", "K");
	replaceAll(str, "Л", "L");
	replaceAll(str, "М", "M");
	replaceAll(str, "Н", "N");
	replaceAll(str, "О", "O");
	replaceAll(str, "П", "P");
	replaceAll(str, "Р", "R");
	replaceAll(str, "С", "S");
	replaceAll(str, "Т", "T");
	replaceAll(str, "У", "U");
	replaceAll(str, "Ф", "F");
	replaceAll(str, "Х", "H");
	replaceAll(str, "Ц", "Z");
	replaceAll(str, "Ч", "CH");
	replaceAll(str, "Ш", "SH");
	replaceAll(str, "Щ", "CH");
	replaceAll(str, "Ъ", "");
	replaceAll(str, "Ы", "Y");
	replaceAll(str, "Ь", "");
	replaceAll(str, "Э", "YE");
	replaceAll(str, "Ю", "YU");
	replaceAll(str, "Я", "YA");
	replaceAll(str, " ", "");
	replaceAll(str, "°", "");
	replaceAll(str, "!", "");
	replaceAll(str, "’", "");
	replaceAll(str, ".", "");
	return str;
}

int getYear(unsigned long epochtime) {
	epochtime /= 1000;
    unsigned long rawTime = epochtime / 86400L;  // in days
    unsigned long days = 0, year = 1970;
    while ((days += (LEAP_YEAR(year) ? 366 : 365)) <= rawTime)
        year++;
    return year;
}
