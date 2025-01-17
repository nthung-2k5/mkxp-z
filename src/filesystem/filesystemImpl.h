//
//  filesystemImpl.h
//  Player
//
//  Created by ゾロアーク on 11/21/20.
//

#ifndef filesystemImpl_h
#define filesystemImpl_h

#include <SDL_video.h>
#include <string>

namespace filesystemImpl
{
    bool fileExists(const char* path);
    bool directoryExists(const char* path);

    std::string contentsOfFileAsString(const char* path);

    bool setCurrentDirectory(const char* path);
    std::string getCurrentDirectory();

    std::string normalizePath(const char* path, bool preferred, bool absolute);

    std::string getDefaultGameRoot();

#ifdef MKXPZ_BUILD_XCODE
    std::string getPathForAsset(const char* baseName, const char* ext);
    std::string contentsOfAssetAsString(const char* baseName, const char* ext);
    std::string getResourcePath();
    std::string selectPath(SDL_Window* win, const char* msg, const char* prompt);
#endif
}; // namespace filesystemImpl

#endif // filesystemImpl_h
