#include <stdio.h>
#include <string>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "fs.hpp"
#include "direntry.h"
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

bool removeHash(HashTable *h, char *key) {
    unsigned int hashval = hash(h, key);
    FileNode *node = h->table[hashval];
    if (node == NULL) return false;
    if (strcmp(node->name, key) == 0) {
        h->table[hashval] = node->next;
        return true;
    }
    FileNode *prev = NULL;
    bool foundit = false;
    while (node != NULL) {
        if (strcmp(node->name, key) == 0) break;
        prev = node;
        node = node->next;
    }
    if (node == NULL) {
        return false;
    } else {
        prev->next = node->next;
        return true;
    }
}

TreeNode *createDirectory(const char *dirName, TreeNode *parent, int hashSize) {
    TreeNode *newDir = (TreeNode *)malloc(sizeof(TreeNode));
    newDir->dirName = strdup(dirName);
    newDir->contents = createHashTable(hashSize);
    newDir->parent = parent;
    if (parent) {
        newDir->self_info = insertHash(parent->contents, newDir->dirName, newDir);
    }
    return newDir;
}

TreeNode *find_parentPath(TreeNode *root, const char *path) {
    char *pathCopy = strdup(path);
    char *segment = strtok(pathCopy, "/");
    TreeNode *current = root;
    FileNode *file = NULL;

    while (segment != NULL && current != NULL) {
        file = lookupHash(current->contents, segment);
        if (file != NULL && file->subdirectory == NULL) {
            free(pathCopy); 
            //printf("status current directory %s\n",current->dirName);
            return current; //File found
        }
        current = file ? file->subdirectory : NULL;
        segment = strtok(NULL, "/");
    }

    free(pathCopy);
    return current; // NULL if not found
}

void freeHashTable(HashTable *table) {
    if (table == NULL) return;

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
                // printf("free who %s\n",temp->name);
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
//for fake root (mount point)
TreeNode *fischl_init_entry(int new_inode_number, const char *fileName, INode_Data *new_inode) {
    TreeNode *newDir = (TreeNode *)malloc(sizeof(TreeNode));
    newDir->dirName = strdup(fileName);
    newDir->contents = createHashTable(20);//hashSize define 20
    newDir->parent = newDir;
    FileNode *newFile = (FileNode *)malloc(sizeof(FileNode));
    newFile->name = strdup(fileName);
    newFile->inode_number = new_inode_number;
    newFile->permissions = new_inode->metadata.permissions;
    newFile->subdirectory = newDir;
    newDir->self_info = newFile;
    return newDir;
}

int fischl_add_entry(TreeNode *parent, int new_inode_number, const char *fileName, INode_Data *new_inode){
    char *Name = strdup(fileName);
    TreeNode *newDir = NULL;
    /*If directory, malloc TreeNode, and then create filenode that belongs to Parent hash table content*/
    if ((new_inode->metadata.permissions & S_IFMT) == S_IFDIR) {
        newDir = (TreeNode *)malloc(sizeof(TreeNode));
        newDir->dirName = Name;
        newDir->contents = createHashTable(20);//hasSize define 20
        newDir->parent = parent;
    }
    FileNode *newFile = insertHash(parent->contents, Name, newDir); //newDir == NULL indicates it's a file
    //assign INode *new_inode metadata to data member in FileNode structure
    newFile->permissions = new_inode->metadata.permissions;
    newFile->inode_number = new_inode_number;
    //Diretory have its own file information, that is . here
    if(newDir != NULL)
        newDir->self_info = newFile;
    //free(Name); cannot free name
    return 0;
}

int fischl_rm_entry(TreeNode *parent, const char *fileName) {
    char *fileName_dup = strdup(fileName);
    if (parent->contents == NULL) return -1;
    FileNode *file = NULL;
    file = lookupHash(parent->contents, fileName_dup);
    if (file == NULL) return -1;
    if (file->subdirectory != NULL) freeTree(file->subdirectory);
    removeHash(parent->contents, fileName_dup);
    free(file->name);
    free(file);
    delete fileName_dup;
}


FileNode *fischl_find_entry(TreeNode *root, const char *path){
    //support . and .. function
    char *pathCopy = strdup(path);
    char *segment = strtok(pathCopy, "/");
    TreeNode *current = root;
    FileNode *file = NULL;

    while (segment != NULL && current != NULL) {
        if (strcmp(segment, "..") == 0) {
            // Move up to the parent directory
            current = current->parent;
            if (current == NULL) {
                // If there's no parent, we've reached the top of the tree, but root itself is same
                file = NULL;
                break;
            } else {
                file = current->self_info;
            }
        } else if (strcmp(segment, ".") == 0) {
            // Stay in the current directory (no action needed)
        } 
        else{
            file = lookupHash(current->contents, segment);
            if (file != NULL && file->subdirectory == NULL) {
                free(pathCopy);
                return file; //File found
                //return current; return filenode
            }
            current = file ? file->subdirectory : NULL;
        }
        segment = strtok(NULL, "/");
    }

    free(pathCopy);

    if (current != NULL && file == NULL) {
        // If we've stopped at a directory and not a file, return the directory's self info
        return current->self_info;
    }

    return file; // NULL if not found
}