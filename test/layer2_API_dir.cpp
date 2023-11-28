#include <stdio.h>
#include <string>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <gtest/gtest.h>
#include <iostream>
#include "fs.hpp"
#include "files.h"


typedef struct file_test{
    const char* name;
    file_test* next;//use linked-list to know the file at the same level directory
}file_test;
typedef struct dir_test{
    const char* name;
    file_test*  inFile;
    dir_test*   subdir;
    dir_test*   next;//use linked-list to know the other dir at the same parent dir.
}dir_test;

void setupTestDirectory(dir_test** root);
void freeDirHierarchy(dir_test* dir);
void traverseDirHierarchy(const dir_test* dir, int depth);

//global can be taken
std::string target_filepath;
dir_test* mock_root = nullptr;
RawDisk *H;
Fs *fs;
FilesOperation *fsop;

int total_dir_num = 0;
int total_file_num = 0;
int total_free_dir = 0;
int total_free_file = 0;

TEST(FileOperationTest, MkdirnodTest) {

    fsop->initialize_rootinode();
    struct fuse_file_info fi;

    mode_t mode;//set mode
    mode = S_IRWXU | S_IRWXG | S_IRWXO;//future should test permission
    //S_IRWXU(S_IRUSR | S_IWUSR | S_IXUSR) (owner), S_IRWXG(S_IRGRP | S_IWGRP | S_IXGRP) (group), S_IRWXO(S_IROTH | S_IWOTH | S_IXOTH)
    EXPECT_EQ(fsop->fischl_create("/test", mode, &fi), 0);
    printf("point 1:");
    fsop->printDirectory(1);
    EXPECT_EQ(fsop->fischl_mkdir("/foo", mode), 0);
    printf("point 2:");
    fsop->printDirectory(1);
    EXPECT_EQ(fsop->fischl_mkdir("/foo/bar", mode),0);
    printf("point 3:");
    fsop->printDirectory(1);
    EXPECT_EQ(fsop->fischl_create("/foo/bar/baz", mode, &fi), 0);
    // the following three testcases will fail
    printf("Failing cases\n");
    EXPECT_TRUE(fsop->fischl_mkdir("foo/bar", mode) < 0);
    EXPECT_TRUE(fsop->fischl_mkdir("/doesnt_exist/bar", mode) < 0);
    EXPECT_TRUE(fsop->fischl_mkdir("/test/bar", mode) < 0);
    EXPECT_TRUE(fsop->fischl_mkdir("/test", mode) < 0);
    EXPECT_TRUE(fsop->fischl_mkdir("/foo/bar", mode) < 0);
    EXPECT_TRUE(fsop->fischl_mkdir("/foo/bar/..", mode) < 0);
}

TEST(FileOperationTest, WriteTest) {
    // write to files (TODO: fischl_write)
    // read and write to indirect datablocks are not supported yet
    //get inode info from disk
    char buffer[IO_BLOCK_SIZE] = {0};
    INode_Data inode;
    u_int64_t get_disk_inum;
    struct fuse_file_info fi;
    
    //file test
    get_disk_inum = fsop->disk_namei("/test");
    fsop->fischl_open("/test", &fi);
    EXPECT_EQ(fi.fh, get_disk_inum);
    buffer[0] = '1';
    fsop->fischl_write("/test", buffer, sizeof(buffer), 0, &fi);
    //other file baz
    get_disk_inum = fsop->disk_namei("/foo/bar/baz");
    buffer[0] = '4';
    fsop->fischl_open("/foo/bar/baz", &fi);
    EXPECT_EQ(fi.fh, get_disk_inum);
    fsop->fischl_write("/foo/bar/baz", buffer, sizeof(buffer), 3*IO_BLOCK_SIZE, &fi);
    buffer[0] = '5';
    fsop->fischl_write("/foo/bar/baz", buffer, sizeof(buffer), 101*IO_BLOCK_SIZE, &fi);
}

TEST(FileOperationTest, RamDiskTest) {
    //use find_entry(specify certain files or directory)
    FileNode* get_dir;
    u_int64_t get_disk_inum;

    get_dir = fischl_find_entry(fsop->root_node, "/test");//this is file
    EXPECT_TRUE(get_dir != NULL);//detect this should find success 
    EXPECT_STREQ(get_dir->name, "test");
    get_disk_inum = fsop->disk_namei("/test");
    EXPECT_EQ(get_disk_inum, get_dir->inode_number);

    get_dir = fischl_find_entry(fsop->root_node, "/foo/bar/baz");//this is file
    EXPECT_TRUE(get_dir != NULL);//detect this should find success 
    EXPECT_STREQ(get_dir->name, "baz");
    get_disk_inum = fsop->disk_namei("/foo/bar/baz");
    EXPECT_EQ(get_disk_inum, get_dir->inode_number);

    get_dir = fischl_find_entry(fsop->root_node, "/foo/bar/..");
    EXPECT_TRUE(get_dir != NULL);//detect this should find success 
    EXPECT_STREQ(get_dir->name, "foo");
    ASSERT_TRUE(get_dir->subdirectory != NULL);//secure it is directory
    get_disk_inum = fsop->disk_namei("/foo/bar/..");
    EXPECT_EQ(get_disk_inum, get_dir->inode_number);
    fsop->printDirectory(get_disk_inum);

    get_dir = fischl_find_entry(fsop->root_node, "/foo/bar/.");
    EXPECT_TRUE(get_dir != NULL);//detect this should find success 
    EXPECT_STREQ(get_dir->name, "bar");
    ASSERT_TRUE(get_dir->subdirectory != NULL);//secure it is directory
    get_disk_inum = fsop->disk_namei("/foo/bar/.");
    EXPECT_EQ(get_disk_inum, get_dir->inode_number);
    fsop->printDirectory(get_disk_inum);
}


TEST(FileOperationTest, ReadTest) {
    // read files (TODO: fischl_read)
    char read_buffer[IO_BLOCK_SIZE] = {0};
    INode_Data inode;
    u_int64_t get_file_inum;

    //read test file
    get_file_inum = fsop->namei("/test");
    inode.inode_num = get_file_inum;
    fs->inode_manager->load_inode(&inode);
    fsop->read_datablock(inode, 0, read_buffer);
    EXPECT_EQ(read_buffer[0], '1');

    //read test file again with fischl_read API
    struct fuse_file_info fi;
    fsop->fischl_open("/test", &fi);
    EXPECT_EQ(fi.fh, get_file_inum);
    fsop->fischl_read("/test", read_buffer, sizeof(read_buffer), 0, &fi);
    EXPECT_EQ(read_buffer[0], '1');

    //read baz file
    get_file_inum= fsop->namei("/foo/bar/baz");
    // inode.inode_construct(get_file_inum, *H);
    // fsop->read_datablock(inode, 3, read_buffer);
    // EXPECT_EQ(read_buffer[0], '4');
    // fsop->read_datablock(inode, 101, read_buffer);
    // EXPECT_EQ(read_buffer[0], '5');

    //read baz file again with fischl_read API
    fsop->fischl_open("/foo/bar/baz", &fi);
    EXPECT_EQ(fi.fh, get_file_inum);
    fsop->fischl_read("/foo/bar/baz", read_buffer, sizeof(read_buffer), 3*IO_BLOCK_SIZE, &fi);
    EXPECT_EQ(read_buffer[0], '4');
    fsop->fischl_read("/foo/bar/baz", read_buffer, sizeof(read_buffer), 101*IO_BLOCK_SIZE, &fi);
    EXPECT_EQ(read_buffer[0], '5');
}

TEST(FileOperationTest, PressureTest) {
    mode_t mode;//set mode
    mode = S_IRWXU | S_IRWXG | S_IRWXO;//future should test permission
    EXPECT_EQ(fsop->fischl_mkdir("/pressure", mode), 0);

    u_int64_t inode_numbers[700];
    std::string prefix = "/pressure/No_";
    for(int i=0;i<700;i++){
        EXPECT_EQ(fsop->fischl_mkdir((prefix+std::to_string(i)).c_str(), mode), 0);
    }
    for(int i=0;i<700;i++){
        EXPECT_EQ(fsop->namei((prefix+std::to_string(i)).c_str()),fsop->disk_namei((prefix+std::to_string(i)).c_str()));
    }
}
// TEST(FileOperationTest, UnlinkTest) {
//     printf("=== Part 6: unlink test ===\n");
//     fsop.printDirectory(file_pressure);
//     for(int i=0;i<700;i+=2){
//         assert(!fsop.fischl_unlink((prefix+std::to_string(i)).c_str()));
//     }
//     for(int i=0;i<4;i+=2){
//         assert(fsop.namei((prefix+std::to_string(i)).c_str())==(u_int64_t)(-1));
//     }
//     for(int i=1;i<700;i+=2){
//         u_int64_t inode_number = fsop.namei((prefix+std::to_string(i)).c_str());
//         assert(inode_number == inode_numbers[i]);
//     }
//     fsop.printDirectory(file_pressure);
//     std::string newprefix = "/pressure/New";
//     for(int i=0;i<700;i+=2){
//         inode_numbers[i] = fsop.fischl_mkdir((newprefix+std::to_string(i)).c_str(), 0);
//     }
//     for(int i=0;i<700;i+=2){
//         u_int64_t inode_number = fsop.namei((newprefix+std::to_string(i)).c_str());
//         assert(inode_number == inode_numbers[i]);
//     }
//     fsop.printDirectory(file_pressure);

//     // long filename test
//     std::string longfilename = std::string(255,'A');
//     u_int64_t filelong = fsop.fischl_mknod((std::string("/")+longfilename).c_str(),0);
//     printf("/AAA...AAA is inode %llu, it is a file\n", filelong);
// }


int main(int argc, char **argv) {
    srand(time(NULL)); // Seed the random number generator
    const char* d = (argc < 2) ? "/dev/vdc" : argv[1];

    setupTestDirectory(&mock_root);
    H = new FakeRawDisk(2048);
    fs = new Fs(H);
    fs->format();
    fsop = new FilesOperation(*H, fs);

    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();

    total_dir_num = 0;
    total_file_num = 0;

    traverseDirHierarchy(mock_root, 0);//mock_root
    printf("Traverse Total: Dir %d, File %d\n",total_dir_num, total_file_num);
    
    // Cleanup
    freeDirHierarchy(mock_root);//mock_root
    printf("Free Total: Dir %d, File %d\n",total_free_dir, total_free_file);
    
    freeTree(fsop->root_node);
    delete fsop; // First delete fsop which depends on H
    delete H;

    return result;
}

const char* generateRandomName(size_t length) {
    const std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string randomString;

    for (size_t i = 0; i < length; ++i) {
        randomString += chars[rand() % chars.size()];
    }

    // Allocate memory and copy the string
    char* name = new char[randomString.length() + 1];
    strcpy(name, randomString.c_str());
    return name;
}

// Recursive function to create directory hierarchy
dir_test* createDirHierarchy(int level, int maxLevel) {
    if (level > maxLevel) {
        return nullptr;
    }

    dir_test* head = nullptr;
    dir_test* current = nullptr;

    for (int i = 0; i < 3; ++i) {
        dir_test* newDir = new dir_test;
        newDir->name = generateRandomName(6); // Generate a random name for the directory
        newDir->inFile = nullptr; // Initialize file list to nullptr
        newDir->subdir = createDirHierarchy(level + 1, maxLevel); // Recursively create subdirectories
        newDir->next = nullptr;

        // Create file list for this directory
        file_test* fileHead = nullptr;
        file_test* fileCurrent = nullptr;
        for (int j = 0; j < 3; ++j) {
            file_test* newFile = new file_test;
            newFile->name = generateRandomName(6); // Generate a random name for the file
            newFile->next = nullptr;

            if (!fileHead) {
                fileHead = newFile;
            } else {
                fileCurrent->next = newFile;
            }
            fileCurrent = newFile;
        }
        newDir->inFile = fileHead;

        // Add the new directory to the list
        if (!head) {
            head = newDir;
        } else {
            current->next = newDir;
        }
        current = newDir;
    }

    return head;
}

// Setup function for the test directory
void setupTestDirectory(dir_test** root) {
    // Allocate memory for root
    *root = new dir_test;
    (*root)->name = strdup("/"); // use / as begin
    (*root)->inFile = nullptr; // Initialize file list to nullptr
    (*root)->subdir = createDirHierarchy(0, 1);
    (*root)->next = nullptr;
    file_test* fileHead = nullptr;
    file_test* fileCurrent = nullptr;
    for (int j = 0; j < 3; ++j) {
        file_test* newFile = new file_test;
        newFile->name = generateRandomName(6); // Generate a random name for the file
        newFile->next = nullptr;

        if (!fileHead) {
            fileHead = newFile;
        } else {
            fileCurrent->next = newFile;
        }
        fileCurrent = newFile;
    }
    (*root)->inFile = fileHead;
}

// Function to free a list of files
void freeFileList(file_test* fileList) {
    while (fileList != nullptr) {
        file_test* temp = fileList;
        fileList = fileList->next;
        total_free_file++;//for debug
        delete[] temp->name; // Free the name string
        delete temp; // Free the current file
    }
}

// Recursive function to free the directory hierarchy
void freeDirHierarchy(dir_test* dir) {
    while (dir != nullptr) {
        dir_test* temp = dir;
        dir = dir->next;
        total_free_dir++;//for debug
        freeFileList(temp->inFile);  // Free the list of files in the directory
        freeDirHierarchy(temp->subdir); // Recursively free subdirectories
        delete[] temp->name; // Free the name string
        delete temp; // Free the current directory
    }
}

// Function to print the list of files in a directory
void printFileList(const file_test* fileList) {
    const file_test* currentFile = fileList;
    while (currentFile != nullptr) {
        // std::cout << "  File: " << currentFile->name << std::endl;
        total_file_num++;
        currentFile = currentFile->next;
    }
}

void traverseDirHierarchy(const dir_test* dir, int depth = 0) {
    while (dir != nullptr) {
        // std::cout << "Depth " << depth << ", Directory: " << dir->name << std::endl;
        total_dir_num++;//for debug

        // Print files in this directory
        printFileList(dir->inFile);
        // Recursively traverse subdirectories
        traverseDirHierarchy(dir->subdir, depth + 1);

        // Go to the next directory at the same level
        dir = dir->next;
    }
}