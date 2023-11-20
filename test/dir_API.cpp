/***********************************************************
 Directory owns treeNode and FileNode structure, detect S_IFDIR to make treeNode or not (see add_entry Function)
 File owns FileNode structure only, detect !S_IFDIR 

*/
#include <stdio.h>
#include <string>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "fs.h"
#include "direntry.h"

int main() {
    TreeNode *root = createDirectory("/", NULL, 20);
    INode inode_file1;
    fischl_add_entry(root, 2, "file1",&inode_file1);
    //I will put this function in create_new_inode function, there will inode number(2) when inode_allocate
    INode inode_dir1;
    //permission is necessary there to create treeNode or not
    u_int64_t ddd_permissions = 0;
    inode_dir1.permissions = ddd_permissions | S_IFDIR;
    fischl_add_entry(root, 3, "dir1",&inode_dir1);
    //find dir file (from root directory view, root contains dir1/ subdirectory)
    FileNode *get_dir1 = fischl_find_entry(root,"/dir1/");
    if(get_dir1 == NULL){
        printf("No dir1 under %s\n",root->dirName);
        return -1;
    }else{
        fprintf(stderr,"[%s ,%d]",__func__,__LINE__);
        printf(" %s under %s\n",get_dir1->name,root->dirName);
    }
    //add file2 under dir1
    INode inode_file2;
    if(get_dir1->subdirectory != NULL){
        //Treenode dir(you cannot find here), you only can get Filenode dir based on fischl_find_entry Function
        //So use Filenode->subdirectory will point to the treenode dir, then can add files
        fischl_add_entry(get_dir1->subdirectory, 4, "file2",&inode_file2);
        printf("add file2 in dir1\n");
    }
    /**********************************************************/
    //This is for debugging, and demonstate to you
    TreeNode *get_dir1_tree = find_parentPath(root,"/dir1/file2");
    if(get_dir1_tree == get_dir1->subdirectory){
        fprintf(stderr,"[%s ,%d]",__func__,__LINE__);
        printf(" [Treenode]get_dir1_tree->dirName (%s) same [Filenode]get_dir1->name (%s)\n",get_dir1_tree->dirName,get_dir1->name);
    }else{
        printf("not same\n");
    }
    /**********************************************************/
    //two Ways to get File(include dir itself) information
    FileNode *get_file2 =NULL;
    //1. absolute path, the root(treeNode) will always exist when initialize
    get_file2 = fischl_find_entry(root,"/dir1/file2");
    if(get_file2 == NULL){
        printf("No dir1 under dir1\n");
        return -1;
    }else{
        fprintf(stderr,"[%s ,%d]",__func__,__LINE__);
        printf(" %s under %sdir1/\n",get_file2->name,root->dirName);
    }
    //2. relative path, the get_dir1(FileNode)->subdirectory(treeNode), use treeNode(dir) to find 
    get_file2 = fischl_find_entry(get_dir1->subdirectory,"/file2");
    if(get_file2 == NULL){
        printf("No dir1 under %s\n",get_dir1->subdirectory->dirName);
        return -1;
    }else{
        fprintf(stderr,"[%s ,%d]",__func__,__LINE__);
        printf(" %s under %s\n",get_file2->name,get_dir1->subdirectory->dirName);
    }
    /**********************************************************/
    //add one more file under dir1
    INode inode_file3;
    if(get_dir1->subdirectory != NULL){
        fischl_add_entry(get_dir1_tree, 5, "file3",&inode_file3);
        printf("add file3 in dir1\n");
    }
    FileNode *get_file3 =NULL;
    //fischl_find_entry(get_dir1->subdirectory,"/file3"); are equivalent
    get_file3 = fischl_find_entry(get_dir1_tree,"/file3");
    if(get_file3 == NULL){
        printf("No dir1 under %s\n",get_dir1_tree->dirName);
        return -1;
    }else{
        printf(" %s under %s\n",get_file3->name,get_dir1_tree->dirName);
    }
    // Cleanup
    freeTree(root);

    return 0;
}