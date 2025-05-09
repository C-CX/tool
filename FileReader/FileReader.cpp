#include "FileReader.h"

FileReader& FileReader::getInstance() {
    static FileReader instance;
    return instance;
}

bool FileReader::openFile(const std::string& filePath) {
    if (fdMap.find(filePath) != fdMap.end()) {
        return true; // 文件已打开，无需重复打开
    }

    int fd = open(filePath.c_str(), O_RDONLY);
    if (fd == -1) {
        std::cerr << "Failed to open file: " << filePath << " (errno: " << errno << ")" << std::endl;
        return false;
    }

    fdMap[filePath] = {fd, ""};
    return true;
}

const std::string& FileReader::readFile(const std::string& filePath) {
    static const std::string emptyBuffer = ""; // 静态变量作为默认值
    if (fdMap.find(filePath) == fdMap.end()) {
        std::cerr << "File not opened: " << filePath << std::endl;
        return emptyBuffer;
    }

    FileData& fileData = fdMap[filePath];
    fileData.buffer.clear();
    if (fileData.fd <= 0) {
        std::cerr << "error fd: " << fileData.fd << std::endl;
        return emptyBuffer;
    }

    const size_t bufferSize = 4096; // 每次读取的块大小
    char tempBuffer[bufferSize];
    ssize_t bytesRead = 0;
    while ((bytesRead = read(fileData.fd, tempBuffer, bufferSize)) > 0) {
        fileData.buffer.append(tempBuffer, bytesRead); // 将读取的内容追加到缓冲区
    }

    if (bytesRead == -1) {
        std::cerr << "Failed to read file: " << filePath << " (errno: " << errno << ")" << std::endl;
        return emptyBuffer;
    }

    // 重置文件指针到开头，确保下次读取从头开始
    lseek(fileData.fd, 0, SEEK_SET);
    return fileData.buffer;
}

const FileReader::FileData* FileReader::getFileData(const std::string& filePath) const {
    auto it = fdMap.find(filePath);
    if (it == fdMap.end()) {
        return nullptr;
    }
    
    return &it->second;
}

void FileReader::closeAllFiles() {
    for (auto& entry : fdMap) {
        close(entry.second.fd);
    }
    fdMap.clear();
}

FileReader::~FileReader() {
    closeAllFiles();
}

bool FileReader::closeAndRemoveFd(const std::string& filePath) {
    auto it = fdMap.find(filePath);
    if (it == fdMap.end()) {
        return false;
    }
    
    close(it->second.fd);
    fdMap.erase(it);
    return true;
}
