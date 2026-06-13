// 216764803 Yuli Smishkis
// 330829847 Ido Maimon
/*
 *  binary_tree.h
 *  Fixed API for the concurrent binary search tree.
 *
 *  You implement these functions in binary_tree.c. Multiple threads
 *  will call them concurrently on the same tree; your internal
 *  synchronization is what keeps the tree consistent.
 *
 *  You MAY add fields to TreeNode (e.g., an omp_lock_t per node).
 *  If you do, submit the modified binary_tree.h with your solution.
 *  You may NOT change any function signature.
 */

#ifndef BINARY_TREE_H
#define BINARY_TREE_H
#include <stdbool.h>
#include <omp.h>
 
typedef struct TreeNode {
    int data;
    struct TreeNode *left;
    struct TreeNode *right;
    omp_lock_t lock;
    omp_lock_t count_lock;
    int readers_count;
} TreeNode;
 
// Function to create a new binary tree node.
// Returns a pointer to the newly allocated node.
TreeNode* createNode(int data);
 
// Function that insert a new value into the binary search tree.
// Returns a pointer to the (possibly updated) root of the tree.
TreeNode* insertNode(TreeNode* root, int data);
 
// Function that delete a value from the binary search tree, if it exists.
// Returns a pointer to the (possibly updated) root of the tree.
TreeNode* deleteNode(TreeNode* root, int data);
 
// Search for a value in the binary search tree.
// Returns true if 'data' is found, false otherwise.
bool searchNode(TreeNode* root, int data);
 
// Find the node with the minimum value in the tree.
// Returns a pointer to the node containing the smallest key, or NULL
// if the tree is empty.
TreeNode* findMin(TreeNode* root);
 
// Perform an in-order traversal and print every node value to stdout,
// each value followed by a single space. The harness uses this to verify
// correctness, so its output is the sorted list of values in the tree.
void inorderTraversal(TreeNode* root);
 
// Free the entire tree and all its nodes (and destroy every lock).
void freeTree(TreeNode* root);
 
#endif // BINARY_TREE_H
 