#pragma once

/* Wrapper header that maps the 'fs' namespace to either 'std::filesystem' or
 * 'std::experimental::filesystem'. Reason is there are still plenty of older c++ standard libraries
 * in circulation that are missing std::filesystem unfortunately.*/

#if __has_include(<filesystem>)

#include <filesystem>
namespace fs = std::filesystem;

#elif __has_include(<experimental/filesystem>)

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#endif
