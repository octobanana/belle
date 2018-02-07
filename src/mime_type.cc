#include "mime_type.hh"

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <string>

namespace OB
{
namespace Belle
{
  std::string to_lower(std::string s)
  {
    for (char &c : s)
    {
      c = to_lower(c);
    }
    return s;
  }

  std::string to_upper(std::string s)
  {
    for (char &c : s)
    {
      c = to_upper(c);
    }
    return s;
  }

  char to_lower(char c)
  {
    if (c >= 'A' && c <= 'Z')
    {
      c += 'a' - 'A';
    }
    return c;
  }

  char to_upper(char c)
  {
    if (c >= 'a' && c <= 'z')
    {
      c += 'A' - 'a';
    }
    return c;
  }

  bool is_equal(std::string str_1, std::string str_2)
  {
      auto n = str_1.size();
      if(str_2.size() != n)
          return false;
      auto p1 = str_1.data();
      auto p2 = str_2.data();
      char a, b;
      while(n--)
      {
          a = *p1++;
          b = *p2++;
          if(a != b)
              goto slow;
      }
      return true;

      while(n--)
      {
      slow:
          if(to_lower(a) != to_lower(b))
              return false;
          a = *p1++;
          b = *p2++;
      }
      return true;
  }
  std::string mime_type(const std::string file_path)
  {
    const fs::path clean_path {fs::canonical(file_path)};
    std::string ext {clean_path.extension().string()};

    if (ext.at(0) != '.')
    {
      return "application/octet-stream";
    }
    ext.erase(0, 1);

    // TODO optimize in an orderable data container
    if(is_equal(ext, "html")) return "text/html";
    if(is_equal(ext, "htm")) return "text/html";
    if(is_equal(ext, "shtml")) return "text/html";

    if(is_equal(ext, "css")) return "text/css";
    if(is_equal(ext, "xml")) return "text/xml";
    if(is_equal(ext, "gif")) return "image/gif";
    if(is_equal(ext, "jpg")) return "image/jpg";
    if(is_equal(ext, "jpeg")) return "image/jpg";
    if(is_equal(ext, "js")) return "application/javascript";
    if(is_equal(ext, "atom")) return "application/atom+xml";
    if(is_equal(ext, "rss")) return "application/rss+xml";

    if(is_equal(ext, "mml")) return "text/mathml";
    if(is_equal(ext, "txt")) return "text/plain";
    if(is_equal(ext, "jad")) return "text/vnd.sun.j2me.app-descriptor";
    if(is_equal(ext, "wml")) return "text/vnd.wap.wml";
    if(is_equal(ext, "htc")) return "text/x-component";

    if(is_equal(ext, "png")) return "image/png";
    if(is_equal(ext, "tif")) return "image/tiff";
    if(is_equal(ext, "tiff")) return "image/tiff";
    if(is_equal(ext, "wbmp")) return "image/vnd.wap.wbmp";
    if(is_equal(ext, "ico")) return "image/x-icon";
    if(is_equal(ext, "jng")) return "image/x-jng";
    if(is_equal(ext, "bmp")) return "image/x-ms-bmp";
    if(is_equal(ext, "svg")) return "image/svg+xml";
    if(is_equal(ext, "svgz")) return "image/svg+xml";
    if(is_equal(ext, "webp")) return "image/webp";

    if(is_equal(ext, "woff")) return "application/font-woff";
    if(is_equal(ext, "jar")) return "application/java-archive";
    if(is_equal(ext, "war")) return "application/java-archive";
    if(is_equal(ext, "ear")) return "application/java-archive";
    if(is_equal(ext, "json")) return "application/json";
    if(is_equal(ext, "hqx")) return "application/mac-binhex40";
    if(is_equal(ext, "doc")) return "application/msword";
    if(is_equal(ext, "pdf")) return "application/pdf";
    if(is_equal(ext, "ps")) return "application/postscript";
    if(is_equal(ext, "eps")) return "application/postscript";
    if(is_equal(ext, "ai")) return "application/postscript";
    if(is_equal(ext, "rtf")) return "application/rtf";

    if(is_equal(ext, "m3u8")) return "application/vnd.apple.mpegurl";
    if(is_equal(ext, "xls")) return "application/vnd.ms-excel";
    if(is_equal(ext, "eot")) return "application/vnd.ms-fontobject";
    if(is_equal(ext, "ppt")) return "application/vnd.ms-powerpoint";
    if(is_equal(ext, "wmlc")) return "application/vnd.wap.wmlc";
    if(is_equal(ext, "kml")) return "application/vnd.google-earth.kml+xml";
    if(is_equal(ext, "kmz")) return "application/vnd.google-earth.kmz";
    if(is_equal(ext, "7z")) return "application/x-7z-compressed";
    if(is_equal(ext, "cco")) return "application/x-cocoa";
    if(is_equal(ext, "jardiff")) return "application/x-java-archive-diff";
    if(is_equal(ext, "jnlp")) return "application/x-java-jnlp-file";
    if(is_equal(ext, "run")) return "application/x-makeself";
    if(is_equal(ext, "pm")) return "application/x-perl";
    if(is_equal(ext, "pl")) return "application/x-perl";
    if(is_equal(ext, "pdb")) return "application/x-pilot";
    if(is_equal(ext, "prc")) return "application/x-pilot";
    if(is_equal(ext, "rar")) return "application/x-rar-compressed";
    if(is_equal(ext, "rpm")) return "application/x-redhat-package-manager";
    if(is_equal(ext, "sea")) return "application/x-sea";
    if(is_equal(ext, "swf")) return "application/x-shockwave-flash";
    if(is_equal(ext, "sit")) return "application/x-stuffit";
    if(is_equal(ext, "tk")) return "application/x-tcl";
    if(is_equal(ext, "tcl")) return "application/x-tcl";
    if(is_equal(ext, "crt")) return "application/x-x509-ca-cert";
    if(is_equal(ext, "pem")) return "application/x-x509-ca-cert";
    if(is_equal(ext, "der")) return "application/x-x509-ca-cert";
    if(is_equal(ext, "xpi")) return "application/x-xpinstall";
    if(is_equal(ext, "xhtml")) return "application/xhtml+xml";
    if(is_equal(ext, "xspf")) return "application/xspf+xml";
    if(is_equal(ext, "zip")) return "application/zip";

    if(is_equal(ext, "dll")) return "application/octet-stream";
    if(is_equal(ext, "exe")) return "application/octet-stream";
    if(is_equal(ext, "bin")) return "application/octet-stream";
    if(is_equal(ext, "deb")) return "application/octet-stream";
    if(is_equal(ext, "dmg")) return "application/octet-stream";
    if(is_equal(ext, "img")) return "application/octet-stream";
    if(is_equal(ext, "iso")) return "application/octet-stream";
    if(is_equal(ext, "msm")) return "application/octet-stream";
    if(is_equal(ext, "msp")) return "application/octet-stream";
    if(is_equal(ext, "msi")) return "application/octet-stream";

    if(is_equal(ext, "docx")) return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    if(is_equal(ext, "xlsx")) return "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
    if(is_equal(ext, "pptx")) return "application/vnd.openxmlformats-officedocument.presentationml.presentation";

    if(is_equal(ext, "kar")) return "audio/midi";
    if(is_equal(ext, "midi")) return "audio/midi";
    if(is_equal(ext, "mid")) return "audio/midi";
    if(is_equal(ext, "mp3")) return "audio/mpeg";
    if(is_equal(ext, "ogg")) return "audio/ogg";
    if(is_equal(ext, "m4a")) return "audio/x-m4a";
    if(is_equal(ext, "ra")) return "audio/x-realaudio";

    if(is_equal(ext, "3gp")) return "video/3gpp";
    if(is_equal(ext, "3gpp")) return "video/3gpp";
    if(is_equal(ext, "ts")) return "video/mp2t";
    if(is_equal(ext, "mp4")) return "video/mp4";
    if(is_equal(ext, "mpg")) return "video/mpeg";
    if(is_equal(ext, "mpeg")) return "video/mpeg";
    if(is_equal(ext, "mov")) return "video/quicktime";
    if(is_equal(ext, "webm")) return "video/webm";
    if(is_equal(ext, "flv")) return "video/x-flv";
    if(is_equal(ext, "m4v")) return "video/x-m4v";
    if(is_equal(ext, "mng")) return "video/x-mng";
    if(is_equal(ext, "asf")) return "video/x-ms-asf";
    if(is_equal(ext, "asx")) return "video/x-ms-asf";
    if(is_equal(ext, "wmv")) return "video/x-ms-wmv";
    if(is_equal(ext, "avi")) return "video/x-msvideo";

    return "application/octet-stream";
  }
} // namespace Belle
} // namespace OB
