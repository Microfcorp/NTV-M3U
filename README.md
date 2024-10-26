# NTV-M3U
Парсер телеканала НТВ в формат m3u. Позволяет на основе НТВ API формировать m3u плейлист. Может выбирать нужную передачу по ее имени, может выбирать жанр по названию, может ограничивать количество элементов в плейлисте, может генерировать файлы с описанием передач. Поддерживает популярные теги m3u

## Поддержка автора (donate)
Если данный проект показался вам полезен, то поддержите автора на https://www.donationalerts.com/r/razbiyan

## Актуальная версия (actual version)

```bash
Version: 1.2.1 (Oct 26 2024)
```

## Справка по сборке (make from source)

### Зависимости

Проект имеет седующие зависимости:
```code
libcurl - if you use compile for libcurl - если используется сборка с помощью libcurl
json - https://github.com/nlohmann/json
```

### Сборка: (Build)
```bash
cd ./NTV-M3U
make #default: CompileSocket
#make CompileSocket - NET used sock - Сетевое соединение на основе сокета
#make Compile - NET used libcurl - Сетевое соединение на основе libcurl
#make clear - remove all builded files - Удаляет все построенные файлы
#make tar - packing all source in tar - Пакует все исходники в ТАР
./ntv_x86 -v
```

Переменные в Makefile: (var in Makefile)
```bash
#Compile from c++ source (Компилятор c++)
Compiler = g++
#Name output app (Название выходного приложения)
Binary = ntv_x86
```

Define для компилятора: (Option for g++)
```bash
-DSymPath=/ - Delimiters path - Разделитель строк
-DAllMapperFile="NTV.txt" - Name for mapper file - Название для мапперного файла всех парсенных каналов
-DMaxName=150 - Max length name - Максимальная длина названия
-DMultiThread - Used multithread (not profit) - использовать мультипоток (not profit)
-DUSE_CURL - Used libcurl - использовать libcurl
-DDebug - Debug requests - отладка запросов
-DDebugCurl - Debug curl - отладка работы curl
-DNOMAPPER - Not used mapper files - не использовать мапперные файлы
```

## Справка по использованию (how to use)
```code
NTV - Manager. Copyring 2023 MicrofDev (Alexey Mostovenko)
https://github.com/microfcorp/NTV-M3U
Compile using socket

Usage: ./ntv_x86 [params] [-f file] [-m m3uFile] [-c dictFile] [-p dirOutput] [-w workMode] [-s workParam] [-z seriesCount] [-x prefixID] [-k prefixGroupe]
         -f [file] - optionality, path to parse
         -m [m3uFile] - optionality, path to m3u (txt) file for save
         -p [dirOutput] - optionality, path to dir for save files. Default: ./NTV
         -w [workMode] - optionality, mode working. Supported: pr, prog, genre, desc. (Default: pr)
         -s [workParam] - optionality, param working. Prog id for 'prog', 'desc'; genre for 'genre'
         -z [seriesCount] - optionality, count video for once bundle
         -x [prefixID] - optionality, added prefix for id files
         -k [prefixGroupe] - optionality, added prefix for 'group-title'

Params:
-h - print help information
-v - print version
-a - no generation 'age-level' tag for M3U file
-r - no generation 'raiting' tag for M3U file
-y - no generation 'tvg-logo' tag for M3U file
-c - no generation 'mov-country' tag for M3U file
-d - no generation 'mov-duration' tag for M3U file
-g - no generation 'group-title' tag for M3U file
-t - generate multi 'group-title' (delimiter ;) tag for M3U file
-i - no generation 'series-id' tag for M3U file
-b - no generation 'season-num' tag for M3U file
-u - no generation 'season-name' tag for M3U file
-e - no generation 'episode-id' tag for M3U file
-o - no generation 'asset-type' tag for M3U file
-j - no generation 'mov-year' tag for M3U file
-q - generate videogalerey to m3u
-l - generate only last bundle
-n - using video for SD resolution
```
