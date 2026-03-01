#pragma once
#include <string>
#include <algorithm>

using namespace std;

string base62_chars =
    "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

string encodeBase62(long long num) {
    string res = "";
    while (num > 0) {
        res += base62_chars[num % 62];
        num /= 62;
    }
    reverse(res.begin(), res.end());
    return res;
}

