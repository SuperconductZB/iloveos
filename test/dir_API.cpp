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

//global can be taken
const char* d;
TreeNode *root;
std::string target_filepath;
dir_test* mock_root = nullptr;

int total_dir_num = 0;
int total_file_num = 0;
int total_free_dir = 0;
int total_free_file = 0;


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

TEST(DirTest, root_test) {
    //Init fake root directory
    INode inode_root;
    u_int64_t file_permissions = 0;
    inode_root.permissions = file_permissions | S_IFDIR;
    root = fischl_init_entry(0, mock_root->name, &inode_root);//0 is inode number assigned by inode_allocate()
}
TEST(DirTest, AddFile_test) {
    //assume file and dir itself(content,metadata) same,but different name and inode number
    INode inode_file;
    INode inode_dir;
    u_int64_t file_permissions = 0;
    file_permissions = 0;
    inode_dir.permissions = file_permissions | S_IFDIR;
    fischl_add_entry(root, 2, mock_root->inFile->name,&inode_file);
    fischl_add_entry(root, 3, mock_root->subdir->name,&inode_dir);
}
TEST(DirTest, FindFile_test) {
    //find file
    target_filepath = std::string("/") + mock_root->inFile->name;
    FileNode *get_file = fischl_find_entry(root,target_filepath.c_str());
    EXPECT_TRUE(get_file != NULL);
    EXPECT_STREQ(get_file->name, mock_root->inFile->name);
    //find dir
    target_filepath = std::string("/") + mock_root->subdir->name + "/";
    FileNode *get_dir = fischl_find_entry(root,target_filepath.c_str());
    EXPECT_TRUE(get_dir != NULL);//detect this should find success 
    EXPECT_STREQ(get_dir->name, mock_root->subdir->name);
    ASSERT_TRUE(get_dir->subdirectory != NULL);//secure it is directory
    //check . function
    get_dir = fischl_find_entry(root,"./");
    EXPECT_TRUE(get_dir != NULL);//detect this should find success 
    EXPECT_STREQ(get_dir->name, mock_root->name);
    ASSERT_TRUE(get_dir->subdirectory != NULL);//secure it is directory
    //check .. function
    get_dir = fischl_find_entry(root,"..");
    EXPECT_TRUE(get_dir != NULL);//detect this should find success
    EXPECT_STREQ(get_dir->name, mock_root->name);
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
    target_filepath = std::string("/") + mock_root->subdir->name + "/";
    FileNode *get_dir = fischl_find_entry(root, target_filepath.c_str());
    fischl_add_entry(get_dir->subdirectory, 4, mock_root->subdir->inFile->name, &inode_file);
    
    //verfication treeNode and Filenode relationship
    target_filepath = std::string("/") + mock_root->subdir->name + "/" + mock_root->subdir->inFile->name;
    TreeNode *get_dir_tree = find_parentPath(root,target_filepath.c_str());
    ASSERT_TRUE(get_dir_tree == get_dir->subdirectory);//treeNode dir should be same as treeNode subdir in that Filenode
 
    //two Ways to get File(include dir itself) information
    FileNode *get_file = NULL;
    //1. absolute path, the root(treeNode) will always exist when initialize
    get_file = fischl_find_entry(root,target_filepath.c_str());
    EXPECT_TRUE(get_file != NULL);
    EXPECT_STREQ(get_file->name,mock_root->subdir->inFile->name);
    //2. relative path, the get_dir(FileNode)->subdirectory(treeNode), use treeNode(dir) to find
    target_filepath = std::string("/") + mock_root->subdir->inFile->name;
    get_file = fischl_find_entry(get_dir->subdirectory,target_filepath.c_str());
    EXPECT_TRUE(get_file != NULL);
    EXPECT_STREQ(get_file->name, mock_root->subdir->inFile->name);
    /**********************************************************/
    //add one more file under fist subdir
    fischl_add_entry(get_dir->subdirectory, 5, mock_root->subdir->inFile->next->name, &inode_file);
    //add one more directory under fist subdir
    fischl_add_entry(get_dir->subdirectory, 6, mock_root->subdir->subdir->name, &inode_dir);
    //find
    target_filepath = std::string("./") + mock_root->subdir->inFile->next->name;
    get_file = fischl_find_entry(get_dir->subdirectory, target_filepath.c_str());
    EXPECT_TRUE(get_file != NULL);
    EXPECT_STREQ(get_file->name, mock_root->subdir->inFile->next->name);
    //use .. from fist subdir to find file1
    target_filepath = std::string("../") + mock_root->inFile->name;
    get_file = fischl_find_entry(get_dir->subdirectory,target_filepath.c_str());
    EXPECT_TRUE(get_file != NULL);
    EXPECT_STREQ(get_file->name,mock_root->inFile->name);
    //check fist subdir with .
    get_dir = fischl_find_entry(get_dir->subdirectory,".");
    EXPECT_TRUE(get_dir != NULL);//detect this should find success
    EXPECT_STREQ(get_dir->name, mock_root->subdir->name);
    ASSERT_TRUE(get_dir->subdirectory != NULL);//secure it is directory
    //check root via fist subdir
    get_dir = fischl_find_entry(get_dir->subdirectory,"..");
    EXPECT_TRUE(get_dir != NULL);//detect this should find success
    EXPECT_STREQ(get_dir->name, mock_root->name);
    ASSERT_TRUE(get_dir->subdirectory != NULL);//secure it is directory
    //use .. to access parent directory
    target_filepath = std::string("/") + mock_root->subdir->name + "/" + mock_root->subdir->subdir->name + "/..";
    get_dir = fischl_find_entry(root, target_filepath.c_str());
    EXPECT_TRUE(get_dir != NULL);
    EXPECT_STREQ(get_dir->name, mock_root->subdir->name);
    target_filepath = std::string("/") + mock_root->subdir->name + "/" + mock_root->subdir->subdir->name + "/../..";
    get_file = fischl_find_entry(root, target_filepath.c_str());
    EXPECT_TRUE(get_file != NULL);
    EXPECT_STREQ(get_file->name, mock_root->name);
    EXPECT_TRUE(get_file->subdirectory != NULL);
    EXPECT_TRUE(get_file->subdirectory->self_info == get_file);
}

// TEST(DirTest, Scale_test){
//     INode inode_file;
//     INode inode_dir;
//     u_int64_t file_permissions = 0;
//     file_permissions = 0;
//     inode_dir.permissions = file_permissions | S_IFDIR;
//     dir_test* temp = mock_root;
//     // First loop: Add files and subdirectories under root
//     file_test* currentFile = temp->inFile;
//     dir_test* currentSubdir = temp->subdir;
//
//     for (int i = 1; i < 7; ++i) {
//         if (currentFile) {
//             //add can still add the same filename and dir name, but it will be linked behind the first added
//             fischl_add_entry(root, i, currentFile->name, &inode_file);
//             currentFile = currentFile->next;
//         }
//         if (currentSubdir) {
//             fischl_add_entry(root, i + 1, currentSubdir->name, &inode_dir);
//             currentSubdir = currentSubdir->next;
//         }
//     }

//     // Second loop: Process each subdir under root
//     temp = mock_root->subdir;
//     while (temp) {
//         target_filepath = "/" + std::string(temp->name) + "/";
//         FileNode* get_dir = fischl_find_entry(root, target_filepath.c_str());

//         ASSERT_TRUE(get_dir != NULL);
//         EXPECT_STREQ(get_dir->name, temp->name);
//         ASSERT_TRUE(get_dir->subdirectory != NULL);
//         // Add files and subdirectories in each subdir
//         file_test* currentFile = temp->inFile;
//         dir_test* currentSubSubdir = temp->subdir;
//         for (int j = 7; j < 13; ++j) {
//             if (currentFile) {
//                 fischl_add_entry(get_dir->subdirectory, j, currentFile->name, &inode_file);
//                 currentFile = currentFile->next;
//             }
//             if (currentSubSubdir) {
//                 fischl_add_entry(get_dir->subdirectory, j + 1, currentSubSubdir->name, &inode_dir);
//                 currentSubSubdir = currentSubSubdir->next;
//             }
//         }

//         temp = temp->next; // Move to next subdir
//     }
// }

int main(int argc, char **argv) {
    srand(time(NULL)); // Seed the random number generator
    d = (argc < 2) ? "/dev/vdc" : argv[1];

    setupTestDirectory(&mock_root);

    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();

    total_dir_num = 0;
    total_file_num = 0;
    traverseDirHierarchy(mock_root);//mock_root
    printf("Traverse Total: Dir %d, File %d\n",total_dir_num, total_file_num);
    
    // Cleanup
    freeDirHierarchy(mock_root);//mock_root
    printf("Free Total: Dir %d, File %d\n",total_free_dir, total_free_file);
    freeTree(root);
    return result;
}