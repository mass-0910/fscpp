#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#elif defined(__unix__)
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "parsearg.hpp"

#include "version.h"

constexpr int FILETYPE_MAX = 21;
constexpr int EXTENSION_MAXNUM = 16;
constexpr int ENTRY_MODE_DIR = 1;
constexpr int ENTRY_MODE_FILE = 2;

constexpr int COLOR_BLUE = 0x0001;
constexpr int COLOR_GREEN = 0x0002;
constexpr int COLOR_RED = 0x0004;

const std::map<std::string, std::string> media_file{
    {".jpg", "JPEG"},
    {".png",  "PNG"},
    {".bmp",  "BMP"},
    {".wav", "WAVE"},
    {".mp3",  "MP3"},
    {".ico", "icon"}
};

const std::map<std::string, std::string> code_file{
    {   ".c",               "C"},
    {".java",            "Java"},
    {  ".py",          "Python"},
    { ".cpp",             "C++"},
    { ".hsp",             "HSP"},
    {   ".h", "C/C++/C# header"},
    { ".txt",            "Text"},
    {".html",            "HTML"},
    { ".css",             "CSS"},
    {  ".js",      "JavaScript"},
    { ".xml",             "XML"},
    { ".ini",      "Initialize"}
};

const std::map<std::string, std::string> binery_file{
    {".exe", "Application"},
    {".bin",      "Binery"},
    {".pdf",         "PDF"},
};

struct Size {
    unsigned int w, h;
};

// fsrc
int FNAME_MAX = 24;
int COLUMN_SIZE = 4;
char USE_COLOR = 1;
unsigned short DIR_COLOR = COLOR_GREEN;
unsigned short LINK_COLOR = COLOR_BLUE;

std::vector<std::filesystem::directory_entry> list_dirent(std::filesystem::directory_iterator dir);
int ls_normal(std::string filepath);
int ls_info(std::string filepath, parsearg::parser &parser);
void print_color(std::string str, int foreground_color, int background_color = 0, bool intensity = false, bool add_endl = false);
void set_fsrc();
void out_entry_name(std::filesystem::directory_entry dp, bool full_path = false);
void out_filetype(std::filesystem::directory_entry dp);
void out_picsize(std::filesystem::directory_entry dp);
void out_filesize(std::filesystem::directory_entry dp);
int ext_exist(std::string ext_array[], std::string extension);
Size get_jpeg_size(const std::string jpg);
Size get_png_size(const std::string png);
Size get_bmp_size(const std::string bmp);

std::string strong_ext[EXTENSION_MAXNUM];
bool strong = false;
std::string only_ext[EXTENSION_MAXNUM];
bool only = false;

int main(int argc, char *argv[]) {
    parsearg::parser parser;
    parser.argument("directoryPath", "Directory path", true);
    parser.option("list", "View in list format", false, 'l');
    parser.option("type", "View file type", false, 't');
    parser.option("graphic", "View image file width and height", false, 'g');
    parser.option("length", "View file or folder size", false, 'L');
    parser.option("fullpath", "View full path", false, 'f');
    parser.option("column", "Set column size", true, 'c');
    parser.option("strong", "Highlight files with specified extensions", true, 's');
    parser.option("only", "Show only files with specified extensions", true, 'o');
    parser.option("help", "Show this message", false, 'h');
    parser.option("version", "Show app version info", false, 0);
    parser.parse(argc, argv);

    if (parser.contains_option("version")) {
        std::cout << "fs version " << FSCPP_VERSION << std::endl;
        return 0;
    }

    if (parser.contains_option("help")) {
        parser.print_usage("[directryPath] [options]");
        return 0;
    }

    std::string filepath = ".";
    if (parser.contains_argument("directoryPath")) {
        filepath = parser.parsed_value("directoryPath", false);
    }

    if (!std::filesystem::exists(filepath)) {
        std::cerr << "filesystem cannot find a object -- '" << filepath << "'" << std::endl;
        exit(-1);
    }

    if (!std::filesystem::is_directory(filepath)) {
        std::cerr << "object is not a directory -- '" << filepath << "'" << std::endl;
        exit(-1);
    }

    strong = parser.contains_option("strong");
    if (strong) {
        strong_ext[0] = parser.parsed_value("strong", true);
    }
    only = parser.contains_option("only");
    if (only) {
        only_ext[0] = parser.parsed_value("only", true);
    }

    if (parser.contains_option("list") || parser.contains_option("type") || parser.contains_option("graphic") || parser.contains_option("length") ||
        parser.contains_option("fullpath"))
        return ls_info(filepath, parser);
    else
        return ls_normal(filepath);
}

std::vector<std::filesystem::directory_entry> list_dirent(std::filesystem::directory_iterator dir) {
    std::vector<std::filesystem::directory_entry> pathlist;
    if (pathlist.size() == 0) {
        for (auto e : dir) {
            try {
                std::string dummy = e.path().string();
                pathlist.push_back(e);
            } catch (const std::exception) {
                std::cerr << "invalid Unicode character is in the path." << std::endl;
            }
        }
        std::sort(pathlist.begin(), pathlist.end(), [](std::filesystem::directory_entry &a, std::filesystem::directory_entry &b) {
            if (!a.is_directory() && b.is_directory()) return false;
            if (a.is_directory() && !b.is_directory()) return true;
            return a.path().string().compare(b.path().string()) < 0;
        });
    }
    return pathlist;
}

void print_color(std::string str, int foreground_color, int background_color, bool intensity, bool add_endl) {
#if defined(_WIN32) || defined(_WIN64)
    static HANDLE hc = GetStdHandle(STD_OUTPUT_HANDLE);
    const WORD default_color = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
    WORD color = 0x0000;
    if (foreground_color & COLOR_BLUE) color |= FOREGROUND_BLUE;
    if (foreground_color & COLOR_GREEN) color |= FOREGROUND_GREEN;
    if (foreground_color & COLOR_RED) color |= FOREGROUND_RED;
    if (background_color & COLOR_BLUE) color |= BACKGROUND_BLUE;
    if (background_color & COLOR_GREEN) color |= BACKGROUND_GREEN;
    if (background_color & COLOR_RED) color |= BACKGROUND_RED;
    if (intensity) color |= FOREGROUND_INTENSITY;
    SetConsoleTextAttribute(hc, color);
    std::cout << str;
    if (add_endl) std::cout << std::endl;
    SetConsoleTextAttribute(hc, default_color);
#elif defined(__unix__)
    std::cout << str;
    if (add_endl) std::cout << std::endl;
#endif
}

void out_entry_name(std::filesystem::directory_entry dp, bool full_path) {
    std::string filename = full_path ? std::filesystem::absolute(dp.path()).string() : dp.path().filename().string();
    if (dp.is_directory()) {
        if (USE_COLOR) {
            if (ext_exist(strong_ext, "dir") == 0 && strong)
                print_color("-" + filename, 0, COLOR_GREEN | COLOR_RED);
            else
                print_color("-" + filename, DIR_COLOR, 0);
        } else {
            std::cout << "-" << filename;
        }
    } else {
        auto extension = dp.path().extension().string();
        if (!extension.empty()) {
            if (USE_COLOR && (extension == ".lnk" || extension == ".url")) {
                if (ext_exist(strong_ext, extension) == 0 && strong)
                    print_color(" " + filename, 0, COLOR_GREEN | COLOR_RED);
                else
                    print_color(" " + filename, LINK_COLOR);
            } else {
                if (ext_exist(strong_ext, extension) == 0 && strong)
                    print_color(" " + filename, 0, COLOR_GREEN | COLOR_RED);
                else
                    std::cout << " " + filename;
            }
        } else {
            if (ext_exist(strong_ext, "nope") == 0 && strong)
                print_color(" " + filename, 0, COLOR_GREEN | COLOR_RED);
            else
                std::cout << " " + filename;
        }
    }
}

int ls_normal(std::string filepath) {
    auto dirlist = list_dirent(std::filesystem::directory_iterator(filepath));

    int i = 1;
    int spacenum;

    for (auto dp : dirlist) {
        out_entry_name(dp);

        for (int j = 1; (spacenum = FNAME_MAX * j - (static_cast<int>(dp.path().filename().string().size()) + 1) + (j >= 2 ? 1 : 0)) < 0; j++) {
            i++;
            if (i >= COLUMN_SIZE) {
                i = 0;
                break;
            }
        }
        for (int j = 0; j < spacenum; j++) {
            std::cout << " ";
        }

        if (i % COLUMN_SIZE == 0) {
            i = 0;
            std::cout << std::endl;
        } else {
            std::cout << "|";
        }

        i++;
    }
    return 0;
}

int ls_info(std::string filepath, parsearg::parser &parser) {
    auto dirlist = list_dirent(std::filesystem::directory_iterator(filepath));
    size_t filename_length = FNAME_MAX;
    std::string path;

    for (auto dp : dirlist) {
        path = parser.contains_option("fullpath") ? std::filesystem::absolute(dp.path()).string() : dp.path().filename().string();
        if (filename_length < path.size()) filename_length = path.size();
    }

    for (int i = 0; i < (filename_length - 8) / 2 + 1; i++) std::cout << "-";
    std::cout << "filename";
    for (int i = 0; i < filename_length - (filename_length - 8) / 2 - 8; i++) std::cout << "-";
    if (parser.contains_option("type")) {
        std::cout << "|";
        for (int i = 0; i < (FILETYPE_MAX + 1 - 8) / 2; i++) std::cout << "-";
        std::cout << "filetype";
        for (int i = 0; i < FILETYPE_MAX + 1 - (FILETYPE_MAX + 1 - 8) / 2 - 8; i++) std::cout << "-";
    }
    if (parser.contains_option("graphic")) {
        std::cout << "|-width--";
        std::cout << "|-height-";
    }
    if (parser.contains_option("length")) {
        std::cout << "|--Length---";
    }
    std::cout << "|" << std::endl;

    for (auto dp : dirlist) {
        out_entry_name(dp, parser.contains_option("fullpath"));
        path = parser.contains_option("fullpath") ? std::filesystem::absolute(dp.path()).string() : dp.path().filename().string();
        for (int i = 0; i < filename_length - path.size(); i++) {
            std::cout << " ";
        }
        if (parser.contains_option("type")) out_filetype(dp);
        if (parser.contains_option("graphic")) out_picsize(dp);
        if (parser.contains_option("length")) out_filesize(dp);
        std::cout << "|" << std::endl;
    }
    return 0;
}

void out_filetype(std::filesystem::directory_entry dp) {
    std::string fileinfo;

    if (dp.is_directory()) {
        fileinfo = "directory";
    } else {
        std::string extension = dp.path().extension().string();
        if (!extension.empty()) {
            if (USE_COLOR && (extension == ".lnk" || extension == ".url")) {
                fileinfo = "shortcut";
            } else {
                if (media_file.count(extension)) {
                    fileinfo = media_file.at(extension) + " file";
                } else if (code_file.count(extension)) {
                    fileinfo = code_file.at(extension) + " file";
                } else if (binery_file.count(extension)) {
                    fileinfo = binery_file.at(extension) + " file";
                } else {
                    fileinfo = "file";
                }
            }
        } else {
            fileinfo = "file";
        }
    }

    std::cout << "| " << fileinfo;
    for (int i = 0; i < FILETYPE_MAX - fileinfo.size(); i++) {
        std::cout << " ";
    }
}

void out_picsize(std::filesystem::directory_entry dp) {
    std::filesystem::path path = dp.path();
    Size size;

    size.h = 0;
    size.w = 0;
    if (path.extension().string() == ".jpg" || path.extension().string() == ".jpeg" || path.extension().string() == ".JPG") {  // JPEG
        size = get_jpeg_size(std::filesystem::absolute(path).string());
        std::cout << "| " << size.w;
        for (int i = 0; i < 7 - std::to_string(size.w).size(); i++) {
            std::cout << " ";
        }
        std::cout << "| " << size.h;
        for (int i = 0; i < 7 - std::to_string(size.h).size(); i++) {
            std::cout << " ";
        }
    } else if (path.extension().string() == ".png" || path.extension().string() == ".PNG") {  // PNG
        size = get_png_size(std::filesystem::absolute(path).string());
        std::cout << "| " << size.w;
        for (int i = 0; i < 7 - std::to_string(size.w).size(); i++) {
            std::cout << " ";
        }
        std::cout << "| " << size.h;
        for (int i = 0; i < 7 - std::to_string(size.h).size(); i++) {
            std::cout << " ";
        }
    } else if (path.extension().string() == ".bmp" || path.extension().string() == ".BMP") {  // BMP
        size = get_bmp_size(std::filesystem::absolute(path).string());
        std::cout << "| " << size.w;
        for (int i = 0; i < 7 - std::to_string(size.w).size(); i++) {
            std::cout << " ";
        }
        std::cout << "| " << size.h;
        for (int i = 0; i < 7 - std::to_string(size.h).size(); i++) {
            std::cout << " ";
        }
    } else {
        std::cout << "|";
        for (int i = 0; i < 8; i++) {
            std::cout << " ";
        }
        std::cout << "|";
        for (int i = 0; i < 8; i++) {
            std::cout << " ";
        }
    }
}

unsigned long long get_file_size(std::string filepath) {
    return std::filesystem::file_size(filepath);
}

unsigned long long get_dir_size(std::string folderpath) {
    std::filesystem::directory_iterator dir(folderpath);
    unsigned long long size = 0;
    for (auto entry : dir) {
        if (entry.is_regular_file()) {
            size += entry.file_size();
        } else if (entry.is_directory()) {
            size += get_dir_size(entry.path().string());
        }
    }
    return size;
}

void out_filesize(std::filesystem::directory_entry dp) {
    unsigned long long size;

    if (dp.is_directory()) {
        size = get_dir_size(dp.path().string());
    } else {
        size = get_file_size(dp.path().string());
    }
    std::cout << "| " << size;
    for (int i = 0; i < 10 - std::to_string(size).size(); i++) {
        std::cout << " ";
    }
}

int ext_exist(std::string ext_array[], std::string extension) {
    for (int i = 0; i < EXTENSION_MAXNUM; i++) {
        if (ext_array[i] == extension.substr(1)) {
            return 0;
        }
    }
    return -1;
}

Size get_jpeg_size(const std::string jpg) {
    Size ret = {0, 0};
    unsigned char buf[8];
    std::basic_ifstream<unsigned char> fin(jpg, std::ios::in | std::ios::binary);
    if (!fin) return Size{0, 0};
    while (!fin.eof() && fin.read(buf, 2)) {
        if (buf[0] != 0xff) break;
        if (buf[1] >= 0xc0 && buf[1] <= 0xcf && fin.read(buf, 7)) {
            ret.h = buf[3] * 256 + buf[4];
            ret.w = buf[5] * 256 + buf[6];
        } else if (buf[1] == 0xd8 || (fin.read(buf, 2) && fin.seekg(buf[0] * 256 + buf[1] - 2, std::ios::cur)))
            continue;
        break;
    }
    return ret;
}

Size get_png_size(const std::string png) {
    Size ret = {0, 0};
    unsigned char buf[8];
    int buffer_length;
    std::basic_ifstream<unsigned char> fin(png, std::ios::in | std::ios::binary);
    if (!fin) return Size{0, 0};
    fin.read(buf, 8);
    if (memcmp(buf, "\x89PNG\r\n\x1a\n", 8) != 0) return Size{0, 0};
    while (fin.read(buf, 8)) {
        std::reverse(buf, buf + 4);
        memcpy(&buffer_length, buf, 4);
        if (memcmp(buf + 4, "IHDR", 4) == 0) {
            fin.read(buf, 8);
            std::reverse(buf, buf + 8);
            memcpy(&ret.w, buf + 4, 4);
            memcpy(&ret.h, buf, 4);
            break;
        } else {
            fin.seekg(buffer_length + 4, std::ios::cur);
        }
    }
    return ret;
}

Size get_bmp_size(const std::string bmp) {
    Size ret = {0, 0};
    unsigned char buf[4];
    std::basic_ifstream<unsigned char> fin(bmp, std::ios::in | std::ios::binary);
    if (!fin) return Size{0, 0};
    fin.seekg(18);
    fin.read(buf, 4);
    memcpy(&ret.w, buf, 4);
    fin.read(buf, 4);
    memcpy(&ret.h, buf, 4);
    return ret;
}

std::string scan(std::ifstream &fin) {
    std::string buf = "";
    char next;
    int type = 0;

    for (;;) {
        fin.read(&next, 1);
        if (fin.eof()) return buf;
        if (type == 0 && (next == ' ' || next == '\n')) continue;
        if (type != 0 && (next == ' ' || next == '\n')) return buf;
        if (type == 0) {
            if (isalpha(next)) {
                type = 1;  // alphabet
            } else if (isdigit(next)) {
                type = 2;  // number
            } else {
                type = 3;  // cymbol
            }
        } else {
            if ((type == 1 && (isalpha(next) == 0 && next != '_')) || (type == 2 && isdigit(next) == 0) || (type == 3 && isalnum(next))) {
                fin.putback(next);
                return buf;
            }
        }
        buf += next;
    }
    return buf;
}

void set_fsrc() {
    std::string token;
    std::vector<std::string> fsrc_param = {"FILENAME_MAX", "COLUMN_SIZE", "USE_COLOR", "DIR_COLOR", "LINK_COLOR"};
    std::filesystem::path userProfile;
    std::string param;
    char path[128];

#if defined(_WIN32) || defined(_WIN64)
    ExpandEnvironmentStrings(TEXT("%USERPROFILE%"), TEXT(path), 128);
#elif defined(__unix__)
    char *homedir;
    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }
    memcpy(path, homedir, strlen(homedir));
#endif
    userProfile = path;
    userProfile.append(".fsrc");
    std::ifstream fin(userProfile);
    if (!fin) return;

    while (!(token = scan(fin)).empty()) {
        param = "";
        for (auto p : fsrc_param) {
            if (token == p) {
                param = p;
                break;
            }
        }
        if (param == "") {
            puts("Illegal token has detected in .fsrc!");
            return;
        }
        token = scan(fin);
        if (token.empty()) return;
        if (token != "=") {
            puts("None '=' token after the element.");
            return;
        }
        if ((token = scan(fin)).empty()) return;
        if (param == "FILENAME_MAX") {
            FNAME_MAX = stoi(token);
        } else if (param == "COLUMN_SIZE") {
            COLUMN_SIZE = stoi(token);
        } else if (param == "USE_COLOR") {
            if (token == "TRUE") USE_COLOR = 1;
            if (token == "FALSE") USE_COLOR = 0;
        } else if (param == "DIR_COLOR") {
            if (token == "RED") DIR_COLOR = COLOR_RED;
            if (token == "BLUE") DIR_COLOR = COLOR_BLUE;
            if (token == "GREEN") DIR_COLOR = COLOR_GREEN;
            if (token == "YELLOW") DIR_COLOR = COLOR_RED | COLOR_GREEN;
            if (token == "SKYBLUE") DIR_COLOR = COLOR_BLUE | COLOR_GREEN;
            if (token == "PURPLE") DIR_COLOR = COLOR_RED | COLOR_BLUE;
            if (token == "WHITE") DIR_COLOR = COLOR_RED | COLOR_BLUE | COLOR_GREEN;
        } else if (param == "LINK_COLOR") {
            if (token == "RED") LINK_COLOR = COLOR_RED;
            if (token == "BLUE") LINK_COLOR = COLOR_BLUE;
            if (token == "GREEN") LINK_COLOR = COLOR_GREEN;
            if (token == "YELLOW") LINK_COLOR = COLOR_RED | COLOR_GREEN;
            if (token == "SKYBLUE") LINK_COLOR = COLOR_BLUE | COLOR_GREEN;
            if (token == "PURPLE") LINK_COLOR = COLOR_RED | COLOR_BLUE;
            if (token == "WHITE") LINK_COLOR = COLOR_RED | COLOR_BLUE | COLOR_GREEN;
        }
    }
}