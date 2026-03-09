#ifndef PARSE_MODULE_H
#define PARSE_MODULE_H

#include <string>
#include <vector>
#include <cstdint>
#include <fstream>
#include <optional>
#include <variant>
#include <QHash>

using int8 = int8_t;
using int32 = int32_t;
using int64 = int64_t;
using float64 = double;

// Base64 функции
std::string base64_encode(const std::string& data);
std::string base64_decode(const std::string& data);
bool isBase64(const std::string& s);

// Векторные функции
template<typename T>
std::string encode_vector(const std::vector<T>& data);

template<typename T>
std::vector<T> decode_vector(const std::string& src);
std::vector<std::string> encode_vector_elements(const std::vector<float64>& data);

struct FCLoad {
    std::variant<std::vector<int32>, std::string> apply_to;
    int apply_dim = 0;
    std::optional<int> cs;
    std::string name;
    int type = 0;
    int id = 0;
    std::vector<float64> data;
    std::vector<int> dependency_type;
};

struct FCRestraint {
    std::variant<std::vector<int32>, std::string> apply_to;
    std::optional<int> cs;
    std::string name;
    int id = 0;
    std::vector<float64> data;
};

struct ParseResults {
    QHash<int, FCLoad> pressures_map;
    QHash<int, FCLoad> forces_map;
    QHash<int, FCLoad> forcesRasp_map;
    QHash<int, FCRestraint> restraints_map;
};

#endif // PARSE_MODULE_H
