#ifndef OB_MIME_TYPE_HH
#define OB_MIME_TYPE_HH

#include <string>
#include <map>

namespace OB
{
namespace Belle
{
  char to_lower(char c);
  char to_upper(char c);

  std::string to_lower(std::string s);
  std::string to_upper(std::string s);

  bool is_equal(std::string str_1, std::string str_2);

  std::string mime_type(const std::string file_path);

} // namespace Belle
} // namespace OB

#endif // OB_MIME_TYPE_HH
