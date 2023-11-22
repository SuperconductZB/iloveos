/***********************************************************
 Directory owns treeNode and FileNode structure, detect S_IFDIR to make treeNode or not (see add_entry Function)
 File owns FileNode structure only, detect !S_IFDIR 

*/
#include <stdio.h>
#include <string>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <gtest/gtest.h>
#include <iostream>
#include "fs.h"
#include "direntry.h"

const char* d;

//global can be taken
TreeNode *root;
const char* target_filepath;
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

int total_number = 0;
int total_free_dir = 0;
int total_free_file = 0;

const char* get_baseName(const char *filename){
    const char* base_name = strrchr(filename, '/');
    if (base_name != NULL) {
        base_name++; // Move past the '/' character
    } else {
        base_name = filename; // No '/' found, use the original string
    }
    return base_name;
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
    fprintf(stderr,"[%s ,%d]\n",__func__,__LINE__);
    // Allocate memory for root
    *root = new dir_test;
    (*root)->name = strdup("/"); // Generate a random name for the directory
    (*root)->inFile = nullptr; // Initialize file list to nullptr
    fprintf(stderr,"[%s ,%d]\n",__func__,__LINE__);
    (*root)->subdir = createDirHierarchy(0, 1);
    (*root)->next = nullptr;
    fprintf(stderr,"[%s ,%d]\n",__func__,__LINE__);
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
void setupTestDirectory_1(dir_test** root) {

    *root = createDirHierarchy(0, 1);
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
        currentFile = currentFile->next;
    }
}

void traverseDirHierarchy(const dir_test* dir, int depth = 0) {
    while (dir != nullptr) {
        // std::cout << "Depth " << depth << ", Directory: " << dir->name << std::endl;
        total_number++;//for debug

        // Print files in this directory
        printFileList(dir->inFile);
        // Recursively traverse subdirectories
        traverseDirHierarchy(dir->subdir, depth + 1);

        // Go to the next directory at the same level
        dir = dir->next;
    }
}

TEST(DirTest, root_test) {
    //Init fake root directory
    INode inode_root;
    u_int64_t file_permissions = 0;
    inode_root.permissions = file_permissions | S_IFDIR;
    root = fischl_init_entry(0, "/", &inode_root);//0 is inode number assigned by inode_allocate()
}
TEST(DirTest, AddFile_test) {
    //assume file and dir itself(content,metadata) same,but different name and inode number
    INode inode_file;
    INode inode_dir;
    u_int64_t file_permissions = 0;
    file_permissions = 0;
    inode_dir.permissions = file_permissions | S_IFDIR;
    fischl_add_entry(root, 2, "file1",&inode_file);
    fischl_add_entry(root, 3, "dir1",&inode_dir);
}
TEST(DirTest, FindFile_test) {
    //find file
    target_filepath = "/file1";
    FileNode *get_file = fischl_find_entry(root,target_filepath);
    EXPECT_TRUE(get_file != NULL);
    EXPECT_STREQ(get_file->name, get_baseName(target_filepath));
    //find dir
    target_filepath = "/dir1/";
    FileNode *get_dir = fischl_find_entry(root,target_filepath);
    EXPECT_TRUE(get_dir != NULL);//detect this should find success 
    EXPECT_STREQ(get_dir->name, "dir1");
    ASSERT_TRUE(get_dir->subdirectory != NULL);//secure it is directory
    //check . function
    get_dir = fischl_find_entry(root,"./");
    EXPECT_TRUE(get_dir != NULL);//detect this should find success 
    EXPECT_STREQ(get_dir->name, "/");
    ASSERT_TRUE(get_dir->subdirectory != NULL);//secure it is directory
    //check .. function
    get_dir = fischl_find_entry(root,"..");
    EXPECT_TRUE(get_dir != NULL);//detect this should find success
    EXPECT_STREQ(get_dir->name, "/");
    ASSERT_TRUE(get_dir->subdirectory != NULL);//secure it is directory
}
TEST(DirTest, Add_FindFile_test) {
    //add file and dir under subdirectory instead of root
    INode inode_file;
    INode inode_dir;
    u_int64_t file_permissions = 0;
    file_permissions = 0;
    inode_dir.permissions = file_permissions | S_IFDIR;

    /*add with subdirectory*/
    //Treenode dir(you cannot find here), you only can get Filenode dir based on fischl_find_entry Function
    //So use Filenode->subdirectory will point to the treenode dir, then can add files
    FileNode *get_dir = fischl_find_entry(root,"/dir1/");  
    fischl_add_entry(get_dir->subdirectory, 4, "file2",&inode_file);
    
    //verfication treeNode and Filenode relationship
    TreeNode *get_dir_tree = find_parentPath(root,"/dir1/file2");
    ASSERT_TRUE(get_dir_tree == get_dir->subdirectory);//treeNode dir should be same as treeNode subdir in that Filenode
 
    //two Ways to get File(include dir itself) information
    FileNode *get_file = NULL;
    //1. absolute path, the root(treeNode) will always exist when initialize
    get_file = fischl_find_entry(root,"/dir1/file2");
    EXPECT_TRUE(get_file != NULL);
    EXPECT_STREQ(get_file->name,"file2");
    //2. relative path, the get_dir1(FileNode)->subdirectory(treeNode), use treeNode(dir) to find 
    get_file = fischl_find_entry(get_dir->subdirectory,"/file2");
    EXPECT_TRUE(get_file != NULL);
    EXPECT_STREQ(get_file->name,"file2");
    /**********************************************************/
    //add one more file under dir1
    fischl_add_entry(get_dir->subdirectory, 5, "file3",&inode_file);
    //add one more directory under dir1
    fischl_add_entry(get_dir->subdirectory, 6, "dir2", &inode_dir);
    //find
    get_file = fischl_find_entry(get_dir->subdirectory,"./file3");
    EXPECT_TRUE(get_file != NULL);
    EXPECT_STREQ(get_file->name,"file3");
    //use .. from dir1 to find file1 
    get_file = fischl_find_entry(get_dir->subdirectory,"../file1");
    EXPECT_TRUE(get_file != NULL);
    EXPECT_STREQ(get_file->name,"file1");
    //check dir1 with .
    get_dir = fischl_find_entry(get_dir->subdirectory,".");
    EXPECT_TRUE(get_dir != NULL);//detect this should find success
    EXPECT_STREQ(get_dir->name, "dir1");
    ASSERT_TRUE(get_dir->subdirectory != NULL);//secure it is directory
    //check root with dir1
    get_dir = fischl_find_entry(get_dir->subdirectory,"..");
    EXPECT_TRUE(get_dir != NULL);//detect this should find success
    EXPECT_STREQ(get_dir->name, "/");
    ASSERT_TRUE(get_dir->subdirectory != NULL);//secure it is directory
    //use .. to access parent directory
    get_dir = fischl_find_entry(root, "/dir1/dir2/..");
    EXPECT_TRUE(get_dir != NULL);
    EXPECT_STREQ(get_dir->name, "dir1");
    FileNode *get_rootdir = fischl_find_entry(root, "/dir1/dir2/../..");
    EXPECT_TRUE(get_rootdir != NULL);
    EXPECT_STREQ(get_rootdir->name, "/");
    EXPECT_TRUE(get_rootdir->subdirectory != NULL);
    EXPECT_TRUE(get_rootdir->subdirectory->self_info == get_rootdir);
}

int main(int argc, char **argv) {
    srand(time(NULL)); // Seed the random number generator
    d = (argc < 2) ? "/dev/vdc" : argv[1];

    dir_test* mock_root = nullptr;
    setupTestDirectory(&mock_root);

    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();

    total_number = 0;
    traverseDirHierarchy(mock_root);//mock_root
    printf("Traverse Dir total %d\n",total_number);
    
    // Cleanup
    freeDirHierarchy(mock_root);//mock_root
    printf("Free Dir total %d\n",total_free_dir);
    printf("Free File total %d\n",total_free_file);
    freeTree(root);
    return result;
}