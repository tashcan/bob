#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>

inline bool ascii_isspace(unsigned char c)
{
  return std::isspace(static_cast<unsigned char>(c));
}

inline std::string_view StripTrailingAsciiWhitespace(std::string_view str)
{
  auto it = std::find_if_not(str.rbegin(), str.rend(), ascii_isspace);
  return str.substr(0, static_cast<size_t>(str.rend() - it));
}

inline std::string_view StripLeadingAsciiWhitespace(std::string_view str)
{
  auto it = std::find_if_not(str.begin(), str.end(), ascii_isspace);
  return str.substr(static_cast<size_t>(it - str.begin()));
}

inline std::string_view StripAsciiWhitespace(std::string_view str)
{
  return StripTrailingAsciiWhitespace(StripLeadingAsciiWhitespace(str));
}

inline std::string AsciiStrToUpper(std::string_view s)
{
  std::string str = s.data();
  std::transform(str.begin(), str.end(), str.begin(), ::toupper);
  return str;
}

inline std::vector<std::string> StrSplit(const std::string& input, char delimiter)
{
  std::vector<std::string> result;
  int                      last_pos = 0;
  for (int i = 0; i < input.length(); i++) {
    if (input[i] != delimiter) {
      continue;
    }

    if (i - last_pos > 0) {
      result.emplace_back(input.substr(last_pos, i - last_pos));
    }
    last_pos = i + 1;
  }

  if (last_pos != input.length()) {
    auto sp = input.substr(last_pos, input.length() - last_pos);
    sp      = StripAsciiWhitespace(sp);
    if (!sp.empty()) {
      result.emplace_back(sp);
    }
  }

  return result;
}