#include "core/number-format.hpp"

#include <charconv>
#include <string_view>

namespace amanuensis::core {

void AppendInteger(std::string& output, long long value)
{
  char formatBuffer[32];
  auto [endPointer, errorCode] =
      std::to_chars(formatBuffer, formatBuffer + sizeof(formatBuffer), value);
  output.append(formatBuffer, static_cast<std::size_t>(endPointer - formatBuffer));
}

void AppendDouble(std::string& output, double value)
{
  char formatBuffer[64];
  auto [endPointer, errorCode] =
      std::to_chars(formatBuffer, formatBuffer + sizeof(formatBuffer), value);
  std::string_view formatted(formatBuffer, static_cast<std::size_t>(endPointer - formatBuffer));

  output.append(formatted);
  if (formatted.find_first_of(".eE") == std::string_view::npos) {
    output.append(".0");
  }
}

} // namespace amanuensis::core
