#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

// Helper to split the enum values and create a map
#define ENUM_WITH_STRING_CONVERSIONS(T, ...) \
    enum class T { __VA_ARGS__, COUNT }; \
    inline std::string ToString(T v) { \
        static const std::vector<std::string> strings = [] { \
            std::string s = #__VA_ARGS__; \
            std::vector<std::string> result; \
            std::istringstream iss(s); \
            std::string item; \
            while (std::getline(iss, item, ',')) { \
                item.erase(std::remove_if(item.begin(), item.end(), ::isspace), item.end()); \
                result.push_back(item); \
            } \
            return result; \
        }(); \
        return strings[static_cast<int>(v)]; \
    } \
    inline T FromString(const std::string& str) { \
        static const std::vector<std::string> strVec = [] { \
            std::string s = #__VA_ARGS__; \
            std::vector<std::string> result; \
            std::istringstream iss(s); \
            std::string item; \
            while (std::getline(iss, item, ',')) { \
                item.erase(std::remove_if(item.begin(), item.end(), ::isspace), item.end()); \
                result.push_back(item); \
            } \
            return result; \
        }(); \
        for (size_t i = 0; i < strVec.size(); ++i) { \
            if (strVec[i] == str) { \
                return static_cast<T>(i); \
            } \
        } \
        throw std::invalid_argument("Invalid enum string"); \
    }

ENUM_WITH_STRING_CONVERSIONS(Color, Red, 
Green,Blue , Yellow)

int main() {
    try {
        Color color = static_cast<Color>(1) ;//Color::Green;
        Color color1 = Color::Blue;
        Color color2 = FromString("Yellow");
        if(color2 == Color::Yellow) {
            std::cout << "FromString:" << ToString(color2)<< std::endl;
        }
        std::cout << "Color:" << ToString(color) << ", Color1:" << ToString(color1) << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}