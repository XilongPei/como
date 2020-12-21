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

#include "util/Options.h"
#include "codegen/CodeGenerator.h"
#include "metadata/MetadataUtils.h"
#include "util/Properties.h"
#include "util/StringBuilder.h"
#include <cstdio>

namespace cdlc {

void Options::Parse(
    /* [in] */ int argc,
    /* [in] */ char** argv)
{
    StringBuilder errors;
    mProgram = argv[0];

    int i = 1;
    while (i < argc) {
        String option(argv[i++]);
        if (option.Equals("-dump-ast")) {
            mDoDumpAST = true;
        }
        else if (option.Equals("-dump-metadata")) {
            mDoDumpMetadata = true;
        }
        else if (option.Equals("--help")) {
            mShowUsage = true;
        }
        else if (option.Equals("--version")) {
            mShowVersion = true;
        }
        else if (option.Equals("-c")) {
            mDoCompile = true;
        }
        else if (option.Equals("-d")) {
            mCodegenDir = argv[i++];
        }
        else if (option.Equals("-gen")) {
            mDoGenerateCode = true;
        }
        else if (option.Equals("-i")) {
            Properties::Get().AddSearchPath(argv[i++]);
        }
        else if (option.Equals("-metadata-file")) {
            mMetadataFile = argv[i++];
            mMetadataFileType = MetadataUtils::TYPE_METADATA;
        }
        else if (option.Equals("-metadata-so")){
            mMetadataFile = argv[i++];
            mMetadataFileType = MetadataUtils::TYPE_SO;
        }
        else if (option.Equals("-mode-client")) {
            Properties::Get().ClearMode(Properties::BUILD_MODE_MASK);
            Properties::Get().AddMode(Properties::BUILD_MODE_CLIENT);
        }
        else if (option.Equals("-mode-component")) {
            Properties::Get().ClearMode(Properties::BUILD_MODE_MASK);
            Properties::Get().AddMode(Properties::BUILD_MODE_COMPONENT);
        }
        else if (option.Equals("-mode-runtime")) {
            Properties::Get().ClearMode(Properties::BUILD_MODE_MASK);
            Properties::Get().AddMode(Properties::BUILD_MODE_RUNTIME);
        }
        else if (option.Equals("-save-metadata")) {
            mDoSaveMetadata = true;
            mSaveFile = argv[i++];
        }
        else if (option.Equals("-split")) {
            Properties::Get().AddMode(Properties::CODEGEN_SPLIT);
        }
        else if (!option.StartsWith("-")) {
            mSourceFile = option;
        }
        else {
            errors.Append(option);
            errors.Append(":");
        }
    }

    mIllegalOptions = errors.ToString();
}

bool Options::HasErrors() const
{
    if (!mIllegalOptions.IsEmpty()) {
        return true;
    }

    if (mDoCompile && !mSourceFile.IsEmpty()) {
        return false;
    }

    if (!mMetadataFile.IsEmpty()) {
        return false;
    }

    return true;
}

void Options::ShowErrors() const
{
    if (!mIllegalOptions.IsEmpty()) {
        String options = mIllegalOptions;
        int index;
        while ((index = options.IndexOf(":")) != -1) {
            printf("The Option \"%s\" is illegal.\n", options.Substring(0, index).string());
            options = options.Substring(index + 1);
        }
    }
    printf("Use \"--help\" to show usage.\n");
}

void Options::ShowUsage() const
{
    printf("Compile a .cdl file and generate metadata, or generate C++ codes from the metadata.\n"
            "Usage: cdlc [options] file\n"
            "Options:\n"
            "  --help                   Display command line options\n"
            "  --version                Display version information\n"
            "  -dump-ast                Display the AST of the .cdl file\n"
            "  -dump-metadata           Display the metadata generated from the .cdl file\n"
            "  -c                       Compile the .cdl file\n"
            "  -d <directory>           Place generated C++ files into <directory>\n"
            "  -gen                     Generate C++ files\n"
            "  -i <directory>           Add <directory> to the .cdl files search paths\n"
            "  -metadata-file <file>    Set <file> as the metadata source file\n"
            "  -metadata-so <.so file>  Set <.so file> as the metadata source file\n"
            "  -mode-client             Set the \"client\" mode which is used to generate\n"
            "                           C++ files for components caller\n"
            "  -mode-component          Set the \"component\" mode which is used to compile\n"
            "                           .cdl files of components\n"
            "  -mode-runtime            Set the \"runtime\" mode which is used to compile\n"
            "                           .cdl files of comort\n"
            "  -save-metadata <file>    Save the metadata into <file>\n"
            "  -split                   Generate interface or class declarations into seperate files when\n"
            "                           in the \"component\" mode or the \"client\" mode\n");
}

}
