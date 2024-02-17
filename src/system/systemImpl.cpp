//
//  systemImpl.cpp
//  Player
//
//  Created by ゾロアーク on 11/22/20.
//

#include "system.h"

#if defined(__WIN32__)
#include <stdlib.h>
#include <windows.h>
#elif defined(__ANDROID__)
#include <SDL.h>
#include <jni.h>
#else
#include <locale>
#endif

#include <SDL_loadso.h>
#include <cstring>
#include <string>

std::string systemImpl::getSystemLanguage()
{
    static char buf[50] = {0};

#if defined(__WIN32__)
    wchar_t wbuf[50] = {0};
    LANGID lid = GetUserDefaultLangID();
    LCIDToLocaleName(lid, wbuf, sizeof(wbuf), 0);
    wcstombs(buf, wbuf, sizeof(buf));
#elif defined(__ANDROID__)
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jclass cls = env->GetObjectClass(activity);

    jmethodID mIDgetLocale = env->GetStaticMethodID(cls, "getSystemLanguage", "()Ljava/lang/String;");
    jstring strJLocale = (jstring)env->CallStaticObjectMethod(cls, mIDgetLocale);

    const char* strCLocale = env->GetStringUTFChars(strJLocale, 0);
    strncpy(buf, strCLocale, sizeof(buf));

    env->ReleaseStringUTFChars(strJLocale, strCLocale);
    env->DeleteLocalRef(strJLocale);
    env->DeleteLocalRef(cls);
    env->DeleteLocalRef(activity);
#else
    strncpy(buf, std::locale("").name().c_str(), sizeof(buf));
#endif

#ifndef __ANDROID__
    for (int i = 0; (size_t)i < strlen(buf); i++)
    {
#ifdef __WIN32__
        if (buf[i] == '-')
        {
            buf[i] = '_';
#else
        if (buf[i] == '.')
        {
            buf[i] = 0;
#endif
            break;
        }
    }
#endif

    return std::string(buf);
}

std::string systemImpl::getUserName()
{
#ifdef __WIN32__
    // The Ruby binding gets the username from the environment loaded
    // with Ruby instead, should fix getting it from WinAPI at some point.
    return std::string("unused");
#elif defined __ANDROID__
    // There's no way to get actual user's name in Android,
    // so return placeholder.
    return std::string("Android");
#else
    char ret[30];
    char* username = getenv("USER");

    if (username) strncpy(ret, username, sizeof(ret));

    return std::string(ret);
#endif
}

bool systemImpl::isWine()
{
#if MKXPZ_PLATFORM != MKXPZ_PLATFORM_WINDOWS
    return false;
#else
    void* ntdll = SDL_LoadObject("ntdll.dll");
    return SDL_LoadFunction(ntdll, "wine_get_host_version") != 0;
#endif
}

bool systemImpl::isRosetta() { return false; }

systemImpl::WineHostType systemImpl::getRealHostType()
{
#if MKXPZ_PLATFORM != MKXPZ_PLATFORM_WINDOWS
    return WineHostType::Linux;
#else
    void* ntdll = SDL_LoadObject("ntdll.dll");
    void (*wine_get_host_version)(const char**, const char**) =
        (void (*)(const char**, const char**))SDL_LoadFunction(ntdll, "wine_get_host_version");

    if (wine_get_host_version == 0) return WineHostType::Windows;

    const char* kernel = 0;
    wine_get_host_version(&kernel, 0);

    if (!strcmp(kernel, "Darwin")) return WineHostType::Mac;

    return WineHostType::Linux;
#endif
}

// HiDPI scaling not supported outside of macOS for now
int systemImpl::getScalingFactor() { return 1; }
