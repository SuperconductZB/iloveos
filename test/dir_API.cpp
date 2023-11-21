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
#include "fs.h"
#include "direntry.h"

const char* d;

TreeNode *root;//global can be taken

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
    FileNode *get_file = fischl_find_entry(root,"/file1");
    EXPECT_TRUE(get_file != NULL);
    EXPECT_STREQ(get_file->name,"file1");
    //find dir
    FileNode *get_dir = fischl_find_entry(root,"/dir1/");
    EXPECT_TRUE(get_dir != NULL);//detect this should find success 
    EXPECT_STREQ(get_dir->name, "dir1");
    ASSERT_TRUE(get_dir->subdirectory != NULL);//secure it is directory
    //check . function
    get_dir = fischl_find_entry(root,"./");
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
    //find
    get_file = fischl_find_entry(get_dir_tree,"./file3");
    EXPECT_TRUE(get_file != NULL);
    EXPECT_STREQ(get_file->name,"file3");
    //use .. from dir1 to find file1 
    get_file = fischl_find_entry(get_dir_tree,"../file1");
    EXPECT_TRUE(get_file != NULL);
    EXPECT_STREQ(get_file->name,"file1");
    
}

int main(int argc, char **argv) {
    d = (argc < 2) ? "/dev/vdc" : argv[1];//how to do with this?
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    // Cleanup
    freeTree(root);
    return result;
}