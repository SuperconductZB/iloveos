#include <gtest/gtest.h>
#include "rawdisk.h"
const char* d;
TEST(RawDiskTest, WriteReadTest) {
    RawDisk *H = new RawDisk(d);

    char *buf = "iloveosdfjlseirfnerig";
    char readBuffer[512] = {0};

    // Write test
    for(u_int64_t i = 0; i < 10; i++) {
        H->rawdisk_write(i*512, buf, strlen(buf));
    }

    // Read and verify test
    for(u_int64_t i = 0; i < 10; i++) {
        H->rawdisk_read(i*512, readBuffer, sizeof(readBuffer));
        EXPECT_EQ(strncmp(readBuffer, buf, strlen(buf)), 0);
    }

    delete H;
}

TEST(RawDiskTest, AssertionFailureTest) {
    EXPECT_EQ(2, 3);  // Intentional failure
    EXPECT_EQ(4, 1);  // Another intentional failure
}

int main(int argc, char **argv) {
    d = (argc < 2) ? "/dev/vdc" : argv[1];//how to do with this?
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
