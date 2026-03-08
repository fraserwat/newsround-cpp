#pragma once

#include <string>
#include <vector>

class Curl
{
public:
  Curl();
  void clear();

private:
  void init();
};

[[nodiscard]] std::vector<std::string> CurlWrapper(std::vector<std::string> &urls);
