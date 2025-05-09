#include <iostream>
#include <unordered_map>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <sstream>



class FileReader {
public:
    struct FileData {
        int fd;                // 文件描述符
        std::string buffer;    // 文件内容缓冲区
    };

    FileReader(const FileReader&) = delete;
    FileReader& operator=(const FileReader&) = delete;

    static FileReader& getInstance();

    bool openFile(const std::string& filePath);
    const std::string& readFile(const std::string& filePath);
    const FileData* getFileData(const std::string& filePath) const;
    bool closeAndRemoveFd(const std::string& filePath);

private:
    std::unordered_map<std::string, FileData> fdMap;
    FileReader() {}
    ~FileReader();
    void closeAllFiles();
};