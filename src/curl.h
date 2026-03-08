#pragma once

#include <string>

class Curl
{
public:
    Curl();
    static void clear();
    [[nodiscard]] bool is_ready() const;
private:
    void init();
    bool _ready;
};

[[nodiscard]] std::string curl_wrapper(const std::string& url);
