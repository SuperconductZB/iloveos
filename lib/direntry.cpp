/*********************************Hash operation********************************************

********************************************************************************************/
// Hash operation
unsigned int hash(HashTable *h, char *key) {
    unsigned int hashval = 0;
    for (; *key != '\0'; key++) hashval = *key + (hashval << 5) - hashval;
    return hashval % h->size;
}

HashTable *createHashTable(int size) {
    HashTable *newTable = (HashTable *)malloc(sizeof(HashTable));
    newTable->size = size;
    newTable->table = (FileNode **)malloc(sizeof(FileNode *) * size);
    for (int i = 0; i < size; i++) newTable->table[i] = NULL;
    return newTable;
}

FileNode *insertHash(HashTable *h, char *key, TreeNode *subdirectory) {
    unsigned int hashval = hash(h, key);
    FileNode *newNode = (FileNode *)malloc(sizeof(FileNode));
    newNode->name = strdup(key);
    newNode->subdirectory = subdirectory;
    newNode->next = h->table[hashval];
    h->table[hashval] = newNode;
    return newNode;
}

FileNode *lookupHash(HashTable *h, char *key) {
    unsigned int hashval = hash(h, key);
    FileNode *node = h->table[hashval];
    while (node != NULL) {
        if (strcmp(node->name, key) == 0) return node;
        node = node->next;
    }
    return NULL; // Not found
}

void freeHashTable(HashTable *table) {
    if (table == NULL) return;

    for (int i = 0; i < table->size; ++i) {
        FileNode *current = table->table[i];
        while (current != NULL) {
            FileNode *temp = current;
            current = current->next;
            // free(temp->name);
            // free(temp);
        }
    }
    free(table->table);
    free(table);
}

void freeTree(TreeNode *node) {
    //printf("***********************FREE TREE %s**************************\n",node->dirName);
    //printf("***********************FREE TREE **************************\n");
    if (node == NULL) return;

    if (node->contents != NULL) {
        for (int i = 0; i < node->contents->size; ++i) {
            FileNode *current = node->contents->table[i];
            while (current != NULL) {
                FileNode *temp = current;
                current = current->next;

                if (temp->subdirectory != NULL) {
                    freeTree(temp->subdirectory);
                }
                // Free the FileNode if it's not a directory
                printf("free who %s\n",temp->name);
                free(temp->name);
                free(temp);
            }
        }
        //printf("free %s's hash table\n",node->dirName);
        freeHashTable(node->contents);
        //node->contents = NULL;
    }
    //printf("free directory %s\n",node->dirName);
    free(node->dirName);
    node->dirName = NULL;
    free(node);
    node = NULL;
    //printf("***********************END**************************\n");
}

/*********************************Direntry operation******************************************

********************************************************************************************/
int fischl_add_entry(TreeNode *parent, int new_inode_number, const char *fileName, INode *new_inode){
    char *Name = strdup(fileName);
    TreeNode *newDir = NULL;
    /*If directory, malloc TreeNode, and then create filenode that belongs to Parent hash table content*/
    if ((new_inode->permissions & S_IFMT) == S_IFDIR) {
        newDir = (TreeNode *)malloc(sizeof(TreeNode));
        newDir->dirName = Name;
        newDir->contents = createHashTable(20);//hasSize define 20
        newDir->parent = parent;
    }
    FileNode *newFile = insertHash(parent->contents, Name, newDir); //newDir == NULL indicates it's a file
    //assign INode *new_inode metadata to data member in FileNode structure
    newFile->permissions = new_inode->permissions;
    newFile->inode_number = new_inode_number;
    //Diretory have its own file information, that is . here
    if(newDir != NULL)
        newDir->self_info = newFile;
    //free(Name); cannot free name
    return 0;
}

FileNode *fischl_find_entry(TreeNode *root, const char *path){
    //support . and .. function
    char *pathCopy = strdup(path);
    char *segment = strtok(pathCopy, "/");
    TreeNode *current = root;
    FileNode *file = NULL;

    while (segment != NULL && current != NULL) {
        file = lookupHash(current->contents, segment);
        if (file != NULL && file->subdirectory == NULL) {
            free(pathCopy);
            return file; //File found
            //return current; return filenode
        }
        current = file ? file->subdirectory : NULL;
        segment = strtok(NULL, "/");
    }

    free(pathCopy);
    return file; // NULL if not found
    //return current; return filenode
}