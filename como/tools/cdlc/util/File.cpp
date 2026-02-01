//=========================================================================
// Copyright (C) 2018 The C++ Component Model(COMO) Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//=========================================================================

#include "util/File.h"
#include "util/Properties.h"
#include <cstdlib>
#include <unistd.h>

namespace cdlc {

File::File(
    /* [in] */ const String& path,
    /* [in] */ int mode)
    : mMode(mode)
{
    if (path.IsEmpty()) {
        return;
    }
    if (mMode != WRITE) {
        if (path.StartsWith("/")) {
            if (access(path.string(), F_OK) == 0) {
                char* canonicalPath = realpath(path.string(), nullptr);
                if (nullptr == canonicalPath) {
                    return;
                }
                mPath = canonicalPath;
                free(canonicalPath);
            }
        }
        else if (path.StartsWith(".") || path.StartsWith("..")) {
            char* cwd = getcwd(nullptr, 0);
            String absolutePath = String(cwd) + "/" + path;
            if (access(absolutePath.string(), F_OK) == 0) {
                char* canonicalPath = realpath(absolutePath.string(), nullptr);
                if (nullptr == canonicalPath) {
                    return;
                }
                mPath = canonicalPath;
                free(canonicalPath);
            }
            free(cwd);
        }
        else {
            for (String searchPath : Properties::Get().GetSearchPaths()) {
                String absolutePath = searchPath + "/" + path;
                if (access(absolutePath.string(), F_OK) == 0) {
                    char* canonicalPath = realpath(absolutePath.string(), nullptr);
                    if (nullptr == canonicalPath) {
                        return;
                    }
                    mPath = canonicalPath;
                    free(canonicalPath);
                    break;
                }
            }
        }
    }
    else {
        if (path.StartsWith("/")) {
            mPath = path;
        }
        else {
            char* cwd = getcwd(nullptr, 0);
            if (nullptr == cwd) {
                return;
            }
            mPath = String(cwd) + "/" + path;
            free(cwd);
        }
    }


    OpenFile();
}

File::~File()
{
    Close();
    if (mLine != nullptr) {
        free(mLine);
    }
}

void File::OpenFile()
{
    if (mMode & APPEND) {
        mFd = fopen(mPath, "a+");
    }
    else if (mMode & WRITE) {
        mFd = fopen(mPath, "w+");
        if (mFd != nullptr) {
            char* canonicalPath = realpath(mPath.string(), nullptr);
            if (nullptr == canonicalPath) {
                return;
            }
            mPath = canonicalPath;
            free(canonicalPath);
        }
    }
    else {
        mFd = fopen(mPath, "r");
    }
}

String File::RawGetLine()
{
    if (getline(&mLine, &mSizeLine, mFd) != -1) {
        return String(mLine);
    }
    else {
        if (feof(mFd) != 0) {
            mEof = true;
        }
        else {
            mError = ferror(mFd) != 0;
        }
        return nullptr;
    }
}

size_t File::Read(
    /* [out] */ void* data,
    /* [in] */ size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return 0;
    }

    if (!IsValid()) {
        return 0;
    }

    return fread(data, 1, size, mFd);
}

bool File::Write(
    /* [in] */ const void* data,
    /* [in] */ size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return true;
    }

    if (!IsValid()) {
        return false;
    }

    if (mMode & (WRITE | APPEND)) {
        return fwrite(data, 1, size, mFd) == size;
    }
    return true;
}

bool File::Flush()
{
    if (!IsValid()) {
        return false;
    }

    if (mMode & (WRITE | APPEND)) {
        return fflush(mFd) == 0;
    }
    return true;
}

bool File::Seek(
    /* [in] */ long pos,
    /* [in] */ int whence)
{
    if (!IsValid()) {
        return false;
    }

    switch (whence) {
        case SEEK_FROM_BEGIN:
            return fseek(mFd, pos, SEEK_SET) == 0;
        case SEEK_FROM_END:
            return fseek(mFd, pos, SEEK_END) == 0;
        case SEEK_FROM_CURRENT:
            return fseek(mFd, pos, SEEK_CUR) == 0;
        default:
            break;
    }
    return false;
}

void File::Close()
{
    if (mFd != nullptr) {
        fclose(mFd);
        mFd = nullptr;
    }
}

bool File::EndsWith(const std::string& s, const std::string& suffix)
{
    return s.size() >= suffix.size() &&
           s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string File::AddComoToPath(const std::string& path)
{
    std::string::size_type sep = path.find_last_of("/\\");
    std::string dir  = (sep == std::string::npos) ? "" : path.substr(0, sep + 1);
    std::string file = (sep == std::string::npos) ? path : path.substr(sep + 1);

    if (file.empty()) {
        return path;
    }

    std::string name;
    std::string ext;

    std::string::size_type dot = file.find('.');
    if ((dot == std::string::npos) || (0 == dot)) {
        name = file;
        ext  = "";
    }
    else {
        name = file.substr(0, dot);
        ext  = file.substr(dot);
    }

    if (EndsWith(name, "_como")) {
        return path;
    }

    return dir + name + "_como" + ext;
}

} // namespace cdlc
