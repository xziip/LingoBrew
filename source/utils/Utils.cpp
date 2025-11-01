#include "Utils.hpp"
#include "logger.h"
#include <cstring>

#include <coreinit/debug.h>
#include <coreinit/mcp.h>
#include <coreinit/thread.h>
#include <dirent.h>
#include <fstream>
#include <malloc.h>
#include <mbedtls/aes.h>
#include <mbedtls/sha256.h>
#include <sys/fcntl.h>
#include <sys/unistd.h>

namespace Utils {
    bool CheckFile(const std::string &fullpath) {
        struct stat filestat {};

        char dirnoslash[strlen(fullpath.c_str()) + 2];
        snprintf(dirnoslash, sizeof(dirnoslash), "%s", fullpath.c_str());

        while (dirnoslash[strlen(dirnoslash) - 1] == '/') {
            dirnoslash[strlen(dirnoslash) - 1] = '\0';
        }

        char *notRoot = strrchr(dirnoslash, '/');
        if (!notRoot) {
            strcat(dirnoslash, "/");
        }

        return stat(dirnoslash, &filestat) == 0;
    }

    bool CreateSubfolder(const std::string &fullpath) {
        if (fullpath.empty()) {
            return false;
        }

        char dirnoslash[fullpath.length() + 1];
        strcpy(dirnoslash, fullpath.c_str());

        auto pos = strlen(dirnoslash) - 1;
        while (dirnoslash[pos] == '/') {
            dirnoslash[pos] = '\0';
            pos--;
        }

        if (CheckFile(dirnoslash))
            return true;

        char parentpath[strlen(dirnoslash) + 2];
        strcpy(parentpath, dirnoslash);
        char *ptr = strrchr(parentpath, '/');

        if (!ptr) {
            //! Device root directory (must be with '/')
            strcat(parentpath, "/");
            struct stat filestat {};
            return stat(parentpath, &filestat) == 0;
        }

        ptr++;
        ptr[0] = '\0';

        if (CreateSubfolder(parentpath) == 0)
            return false;

        return mkdir(dirnoslash, 0777) >= 0;
    }

    bool CopyFile(const std::string &in, const std::string &out) {
        try {
            std::ifstream src(in.c_str(), std::ios::binary);
            std::ofstream dst(out.c_str(), std::ios::binary);

            dst << src.rdbuf();
            return true;
        } catch (std::exception &ex) {
            DEBUG_FUNCTION_LINE_ERR("Exception: (Tried to copy %s -> %s): %s", in.c_str(), out.c_str(), ex.what());
        }
        return false;
    }

    bool CopyFolder(const std::string &in, const std::string &out, CopyProgressCallback progressCallback) {
        // First pass: count total files for progress tracking
        int totalFiles = 0;
        if (progressCallback) {
            DIR *countDir = opendir(in.c_str());
            if (countDir) {
                struct dirent *dp;
                while ((dp = readdir(countDir)) != nullptr) {
                    std::string name = dp->d_name;
                    if (name == "." || name == "..") continue;
                    
                    std::string srcPath = in + "/" + name;
                    struct stat filestat {};
                    if (stat(srcPath.c_str(), &filestat) == 0) {
                        if ((filestat.st_mode & S_IFMT) != S_IFDIR) {
                            totalFiles++;
                        }
                    }
                }
                closedir(countDir);
            }
        }

        DIR *srcDir = opendir(in.c_str());
        if (!srcDir) {
            DEBUG_FUNCTION_LINE_ERR("Failed to open source directory %s", in.c_str());
            return false;
        }

        // ensure destination folder exists
        if (!CreateSubfolder(out)) {
            DEBUG_FUNCTION_LINE_ERR("Failed to create destination directory %s", out.c_str());
            closedir(srcDir);
            return false;
        }

        // Report directory creation progress
        if (progressCallback) {
            progressCallback(out, true);
            usleep(1000); // Give UI a chance to update
        }

        struct dirent *dp;
        while ((dp = readdir(srcDir)) != nullptr) {
            std::string name = dp->d_name;
            if (name == "." || name == "..")
                continue;

            std::string srcPath = in + "/" + name;
            std::string dstPath = out + "/" + name;

            struct stat filestat {};
            if (stat(srcPath.c_str(), &filestat) < 0) {
                DEBUG_FUNCTION_LINE_ERR("Failed to stat %s", srcPath.c_str());
                closedir(srcDir);
                return false;
            }

            if ((filestat.st_mode & S_IFMT) == S_IFDIR) {
                // Report directory progress before recursing
                if (progressCallback) {
                    progressCallback(srcPath, true);
                    usleep(1000); // Give UI a chance to update
                }
                // recurse into directory
                if (!CopyFolder(srcPath, dstPath, progressCallback)) {
                    closedir(srcDir);
                    return false;
                }
            } else {
                // Report file progress
                if (progressCallback) {
                    progressCallback(srcPath, false);
                    usleep(1000); // Give UI a chance to update
                }
                // copy file
                if (!CopyFile(srcPath, dstPath)) {
                    DEBUG_FUNCTION_LINE_ERR("Failed to copy file %s -> %s", srcPath.c_str(), dstPath.c_str());
                    closedir(srcDir);
                    return false;
                }
            }
        }

        closedir(srcDir);
        return true;
    }

} // namespace Utils
