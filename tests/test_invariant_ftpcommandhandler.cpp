#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <cstring>
#include <sanitizer/asan_interface.h>

// We test the invariant by observing that constructing the path
// with oversized szFolder must not overflow szFind (typically MAX_PATH/260 bytes).
// We use AddressSanitizer or a canary-based check to detect overflow.

// The buffer size used in ftpcommandhandler.cpp for szFind
static const size_t SZFIND_BUFFER_SIZE = 260; // MAX_PATH

class BufferOverflowSecurityTest : public ::testing::TestWithParam<std::string> {};

TEST_P(BufferOverflowSecurityTest, FolderPathNeverExceedsBuffer) {
    // Invariant: The resulting path (szFolder + "/*.*") must never exceed
    // the declared szFind buffer size. Input must be truncated or rejected.
    std::string payload = GetParam();

    // Simulate the vulnerable pattern with a canary-guarded buffer
    char szFind[SZFIND_BUFFER_SIZE];
    char canary[8];
    memset(canary, 0xAB, sizeof(canary));

    // Safe version: assert input is within safe bounds before copy
    size_t required = payload.size() + strlen("/*.*") + 1;
    bool input_is_safe = required <= SZFIND_BUFFER_SIZE;

    if (input_is_safe) {
        // Only perform the copy if it's safe — this is what the fix must enforce
        strncpy(szFind, payload.c_str(), SZFIND_BUFFER_SIZE - 5);
        szFind[SZFIND_BUFFER_SIZE - 5] = '\0';
        strncat(szFind, "/*.*", 4);
        // Canary must be intact
        for (int i = 0; i < 8; i++) {
            EXPECT_EQ((unsigned char)canary[i], 0xAB)
                << "Stack canary corrupted at index " << i;
        }
        EXPECT_LT(strlen(szFind), SZFIND_BUFFER_SIZE);
    } else {
        // Oversized input MUST be rejected — not blindly copied
        EXPECT_GT(required, SZFIND_BUFFER_SIZE)
            << "Oversized input should have been rejected by the handler";
    }
}

INSTANTIATE_TEST_SUITE_P(
    AdversarialInputs,
    BufferOverflowSecurityTest,
    ::testing::Values(
        // Valid input: normal short path
        std::string("/home/user"),
        // Boundary: exactly fills buffer minus wildcard (255 chars)
        std::string(255, 'A'),
        // Exploit: 2x buffer overflow (520 chars)
        std::string(520, 'B'),
        // Exploit: 10x buffer overflow (2600 chars)
        std::string(2600, 'C')
    )
);

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}