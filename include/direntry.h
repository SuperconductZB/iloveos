typedef struct fileNode {
    char *name = NULL;
    int inode_number;
    int permissions;
    char *Symbolink;
    struct treeNode *subdirectory;
    struct fileNode *next;
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

/*root directory have its own initialization, so parent wont be NULL*/
int fischl_add_entry(TreeNode *parent, int new_inode_number, const char *fileName, INode *new_inode);
/*if want to use dir mode use the subdirectory treeNode pointer */
//e.g. FileNode *Dirnode = fischl_find_entry(); can see file inside with Dirnode->subdirectory
//e.g. go to the current Dirnode parent directory, use TreeNode *get_Dir_parent = Dirnode->subdirectory->parent;
FileNode *fischl_find_entry(TreeNode *root, const char *path);