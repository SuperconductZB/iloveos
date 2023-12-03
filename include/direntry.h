typedef struct fileNode {
    char *name = NULL;
    int inode_number;
    int permissions;
    char *Symbolink;
    struct treeNode *subdirectory;
    struct fileNode *next;
} FileNode;

typedef struct {
    int size;
    FileNode **table;
} HashTable;

typedef struct treeNode {
    char *dirName;
    HashTable *contents;
    struct treeNode *parent;
    FileNode *self_info; //self fileNode infromation
} TreeNode;

/*
typedef struct RenameInfo {
    FileNode *oldFileNode;       // The file node being renamed.
    FileNode *oldParentNode;     // The parent directory of the file node being renamed.
    FileNode *newParentNode;     // The target parent directory where the file node will be moved.
    char *newName;               // The new name of the file node after the rename.
    FileNode *newFileNode;       // The new file node, if one already exists at the target location.
    bool exchangeExist;          // Flag to indicate if the rename should replace an existing file node.
} RenameInfo;*/

/*for root*/
TreeNode *fischl_init_entry(int new_inode_number, const char *fileName, INode_Data *new_inode);
/*the to be added file in add_entry should be parent-child relationship with treenode, otherwise will wrong */
/*see Add_FindFiletest in dir_API.cpp*/
int fischl_add_entry_for_cache(TreeNode *parent, int new_inode_number, const char *fileName, INode_Data *new_inode, FileNode *file);
int fischl_add_entry(TreeNode *parent, int new_inode_number, const char *fileName, INode_Data *new_inode);
int fischl_rm_entry(TreeNode *parent, const char *fileName);
/*if want to use dir mode use the subdirectory treeNode pointer */
//e.g. FileNode *Dirnode = fischl_find_entry(); can see file inside with Dirnode->subdirectory
//e.g. go to the current Dirnode parent directory, use TreeNode *get_Dir_parent = Dirnode->subdirectory->parent;
FileNode *fischl_find_entry(Fs *fs, TreeNode *root, const char *path);

void freeTree(TreeNode *node);
/*for debug use*/
TreeNode *createDirectory(const char *dirName, TreeNode *parent, int hashSize);
TreeNode *find_parentPath(TreeNode *root, const char *path);

struct DirectoryEntry {
    u_int64_t inode_number;
    char file_name[256];
    void serialize(char* buffer) {
        u_int64_t t = inode_number;
        for (int j = 0; j < 8; j++){
            buffer[j] = t & (((u_int64_t)1<<(8))-1);
            t >>= 8;
        }
        strcpy(buffer+8, file_name);
    }
    void deserialize(char* buffer) {
        inode_number = 0;
        for (int j = 0; j < 8; j++)
            inode_number = inode_number | (((u_int64_t)(unsigned char)buffer[j])<<(8*j));
        strcpy(file_name, buffer+8);
    }
};
