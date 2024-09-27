#include <comosp.h>
#include <comoobj.h>
#include <gtest/gtest.h>
#include <DebugUtils.h>

TEST(DebugUtils, testDebugUtils1)
{
    como::LinuxMemInfo memInfo;
    como::DebugUtils::GetMemoryInfo(memInfo);

    printf("MemTotal:    %ld kB\n", memInfo.MemTotal);
    printf("MemFree:     %ld kB\n", memInfo.MemFree);
    printf("MemAvailable: %ld kB\n", memInfo.MemAvailable);
    printf("Buffers:     %ld kB\n", memInfo.Buffers);
    printf("Cached:      %ld kB\n", memInfo.Cached);
    printf("SwapTotal:   %ld kB\n", memInfo.SwapTotal);
    printf("SwapFree:    %ld kB\n", memInfo.SwapFree);
}

TEST(DebugUtils, testDebugUtils2)
{
    const char *str = "// Pad out last line if not exactly 16 characters.// Pad "
    "out last line if not exactly 16 characters.// Pad out last line if not "
    "exactly 16 characters. \t\n\r// Pad out last line if not exactly 16 characters.";

    char bufStr[4096];
    int i = como::DebugUtils::HexDump(bufStr, 4096, (void*)str, strlen(str));
    printf("%d\n%s", i, bufStr);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}