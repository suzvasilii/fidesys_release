#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <QDebug>
#include "parse_module.h"

using namespace std;
using int8 = int8_t;
using int32 = int32_t;
using int64 = int64_t;
using float64 = double;

string base64_encode(const string& data) {
    static const char* b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    string result;
    int i = 0;
    unsigned char arr3[3], arr4[4];
    for (unsigned char c : data) {
        arr3[i++] = c;
        if (i == 3) {
            arr4[0] = (arr3[0] & 0xfc) >> 2;
            arr4[1] = ((arr3[0] & 0x03) << 4) + ((arr3[1] & 0xf0) >> 4);
            arr4[2] = ((arr3[1] & 0x0f) << 2) + ((arr3[2] & 0xc0) >> 6);
            arr4[3] = arr3[2] & 0x3f;
            for (int j = 0; j < 4; j++) result += b64[arr4[j]];
            i = 0;
        }
    }
    if (i) {
        for (int j = i; j < 3; j++) arr3[j] = 0;
        arr4[0] = (arr3[0] & 0xfc) >> 2;
        arr4[1] = ((arr3[0] & 0x03) << 4) + ((arr3[1] & 0xf0) >> 4);
        arr4[2] = ((arr3[1] & 0x0f) << 2) + ((arr3[2] & 0xc0) >> 6);
        arr4[3] = arr3[2] & 0x3f;
        for (int j = 0; j < i + 1; j++) result += b64[arr4[j]];
        while (i++ < 3) result += '=';
    }
    return result;
}

string base64_decode(const string& data) {
    static const string b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    string result;
    vector<unsigned char> arr4(4), arr3(3);
    int i = 0;
    for (char c : data) {
        if (c == '=') break;
        size_t pos = b64.find(c);
        if (pos == string::npos) continue;
        arr4[i++] = pos;
        if (i == 4) {
            arr3[0] = (arr4[0] << 2) + ((arr4[1] & 0x30) >> 4);
            arr3[1] = ((arr4[1] & 0xf) << 4) + ((arr4[2] & 0x3c) >> 2);
            arr3[2] = ((arr4[2] & 0x3) << 6) + arr4[3];
            result.append(arr3.begin(), arr3.begin() + 3);
            i = 0;
        }
    }
    if (i > 0) {
        for (int j = i; j < 4; j++) arr4[j] = 0;
        arr3[0] = (arr4[0] << 2) + ((arr4[1] & 0x30) >> 4);
        arr3[1] = ((arr4[1] & 0xf) << 4) + ((arr4[2] & 0x3c) >> 2);
        result.append(arr3.begin(), arr3.begin() + i - 1);
    }
    return result;
}

template<typename T>
string encode_vector(const vector<T>& data) {
    if (data.empty()) return "";
    string bytes((const char*)data.data(), data.size() * sizeof(T));
    return base64_encode(bytes);
}

template<typename T>
vector<T> decode_vector(const string& src) {
    if (src.empty()) return {};
    string decoded = base64_decode(src);
    vector<T> result(decoded.size() / sizeof(T));
    memcpy(result.data(), decoded.data(), decoded.size());
    return result;
}

bool isBase64(const string& s) {
    if (s == "all") return false;
    try {
        string dec = base64_decode(s);
        string enc = base64_encode(dec);
        return enc == s;
    }
    catch (...) {
        return false;
    }
}

vector<string> encode_vector_elements(const vector<float64>& data) {
    vector<string> result;
    for (float64 val : data) {
        // Превращаем одно число в строку байт
        string bytes((const char*)&val, sizeof(val));
        // Кодируем в base64
        result.push_back(base64_encode(bytes));
    }
    return result;
}


template string encode_vector<int32>(const vector<int32>&);
template string encode_vector<float64>(const vector<float64>&);
template string encode_vector<int64>(const vector<int64>&);
template string encode_vector<int8>(const vector<int8>&);

template vector<int32> decode_vector<int32>(const string&);
template vector<float64> decode_vector<float64>(const string&);
template vector<int64> decode_vector<int64>(const string&);
template vector<int8> decode_vector<int8>(const string&);