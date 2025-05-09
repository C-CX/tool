#include "FileReader.h"

int main(int argc, char const *argv[]) {
    std::string pid = "774075";
    std::string path = "/proc/"+ pid +"/stat"; //smaps
    
    FileReader::getInstance().openFile(path);
    int index = 0;
    while(1) {
        // if(index++ % 10 == 0) {
        //     const std::string& str = FileReader::getInstance().readFile(path);
        //     printf("str:%s.\n",str.c_str());
        //     printf("size:%ld.\n",str.size());
        // }
        // usleep(1000);

        const std::string& str = FileReader::getInstance().readFile(path);
        printf("str:%s.\n",str.c_str());
        printf("size:%ld.\n",str.size());
    }
    
    return 0;
}