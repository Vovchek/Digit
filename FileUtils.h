#pragma once

#include <string>
#include <string_view>
#include <filesystem>
#include <algorithm>
#include "AppDef.h"

namespace fs = std::filesystem;
using namespace std;

inline int FileType(std::string_view path)
{
    fs::path p(path);

    if (!fs::exists(p) || fs::is_directory(p))
        return -1;

    std::string ext = p.extension().string();
    if (ext.empty())
        return T_EMP;

    // remove the leading dot and convert to lowercase
    if (ext.front() == '.')
        ext.erase(ext.begin());
    std::transform(ext.begin(), ext.end(), ext.begin(),
        [](unsigned char c) { return std::tolower(c); });

    if (ext == "bmp")  return T_BMP;
    if (ext == "pcx")  return T_PCX;
    if (ext == "jpg" || ext == "jpeg") return T_JPG;
    if (ext == "gif")  return T_GIF;
    if (ext == "tga")  return T_TGA;
    if (ext == "tif" || ext == "tiff") return T_TIF;

    if (ext == "pic")  return T_PIC;
    if (ext == "frn")  return T_FRN;
    if (ext == "zap")  return T_ZAP;
    if (ext == "pol")  return T_POL;
    if (ext == "mtr")  return T_MTR;
    if (ext == "dat")  return T_DAT;
    if (ext == "ave")  return T_AVE;
    if (ext == "dsp")  return T_DSP;

    return T_EMP; // unknown or empty extension
}
