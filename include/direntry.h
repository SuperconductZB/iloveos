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

/*root directory have its own initialization, so parent wont be NULL*/
int fischl_add_entry(TreeNode *parent, int new_inode_number, const char *fileName, INode *new_inode);
int fischl_rm_entry(TreeNode *parent, const char *fileName);
/*if want to use dir mode use the subdirectory treeNode pointer */
//e.g. FileNode *Dirnode = fischl_find_entry(); can see file inside with Dirnode->subdirectory
//e.g. go to the current Dirnode parent directory, use TreeNode *get_Dir_parent = Dirnode->subdirectory->parent;
FileNode *fischl_find_entry(TreeNode *root, const char *path);

void freeTree(TreeNode *node);
/*for debug use*/
TreeNode *createDirectory(const char *dirName, TreeNode *parent, int hashSize);
TreeNode *find_parentPath(TreeNode *root, const char *path);