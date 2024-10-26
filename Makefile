CPP = ntv.cpp
Binary = ntv_x86
TarName = ntv.tar
DocsFile =  #IF you not docs file, then replace this string to DocsFile = 

Compiler = g++

#Option for gcc:
#-DSymPath=/ - Разделитель строк
#-DAllMapperFile="NTV.txt" - Название для мапперного файла всех парсенных каналов
#-DMaxName=150 - Максимальная длина названия
#-DMultiThread - использовать мультипоток
#-DUSE_CURL - использовать libcurl
#-DDebug - отладка запросов
#-DDebugCurl - отладка работы curl
#-DNOMAPPER - не использовать мапперные файлы

all: CompileSocket

Compile:
	$(Compiler) -o $(Binary) $(CPP) -lcurl -DUSE_CURL
	
CompileSocket:
	$(Compiler) -o $(Binary) $(CPP) -DUSE_SOCKET
	
CompileThreader:
	$(Compiler) -o $(Binary)_threader $(CPP) -lpthread -DMultiThread -lcurl -DUSE_CURL
	
help: Compile
	./$(Binary) -h
	
tar: Compile
	file $(Binary) >> $(Binary)_info.txt
	./$(Binary) -h >> $(Binary)_help.txt
	./$(Binary) -v >> $(Binary)_version.txt
	tar -cf $(TarName) $(Binary) $(DocsFile) $(CPP) translete.h json.hpp Makefile $(Binary)_info.txt $(Binary)_help.txt $(Binary)_version.txt
	rm -f $(Binary) $(Binary)_threader $(Binary)_info.txt $(Binary)_help.txt $(Binary)_version.txt
	
rm:
	rm -f $(TarName) $(Binary) $(Binary)_threader *.m3u *.m3u8 ntv_api.json $(Binary)_help.txt $(Binary)_version.txt $(Binary)_info.txt
	
clear: rm
