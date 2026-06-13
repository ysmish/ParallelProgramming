// 216764803 Yuli Smishkis
// 330829847 Ido Maimon

#include "binary_tree.h"
#include <stdio.h>
#include <stdlib.h>

// ---------------------------------------------------------------------------
// Per-node reader-writer lock.
// Writers (insert/delete) take node->lock exclusively. Readers
// (search/findMin/traversals) share the node: the FIRST reader takes
// node->lock so writers are blocked, and the LAST reader releases it.
// count_lock protects readers_count. Lock-acquisition order is always
// ancestor-before-descendant (root -> leaf) for every operation, which
// makes the scheme deadlock-free.
// ---------------------------------------------------------------------------

void startRead(TreeNode* node) {
    if (!node) return;
    omp_set_lock(&node->count_lock);
    node->readers_count++;
    // first reader blocks writers on this node
    if (node->readers_count == 1) omp_set_lock(&node->lock);
    omp_unset_lock(&node->count_lock);
}

void endRead(TreeNode* node) {
    if (!node) return;
    omp_set_lock(&node->count_lock);
    node->readers_count--;
    // last reader lets writers back in
    if (node->readers_count == 0) omp_unset_lock(&node->lock);
    omp_unset_lock(&node->count_lock);
}

TreeNode* createNode(int data) {
    TreeNode* newNode = (TreeNode*)malloc(sizeof(TreeNode));
    if (newNode == NULL) {
        perror("create failed");
        return NULL;
    }

    newNode->data = data;
    newNode->left = NULL;
    newNode->right = NULL;
    omp_init_lock(&newNode->lock);
    omp_init_lock(&newNode->count_lock);
    newNode->readers_count = 0;

    return newNode;
}

TreeNode* insertNode(TreeNode* root, int data) {
    // Empty tree: the new node becomes the root.
    if (root == NULL) {
        return createNode(data);
    }

    // Hand-over-hand descent: always hold the current node's write lock.
    omp_set_lock(&root->lock);
    TreeNode* curr = root;

    while (true) {
        if (data < curr->data) {
            if (curr->left == NULL) {
                curr->left = createNode(data);
                omp_unset_lock(&curr->lock);
                break;
            } else {
                TreeNode* next = curr->left;
                omp_set_lock(&next->lock);     // lock child before releasing parent
                omp_unset_lock(&curr->lock);
                curr = next;
            }
        } else if (data > curr->data) {
            if (curr->right == NULL) {
                curr->right = createNode(data);
                omp_unset_lock(&curr->lock);
                break;
            } else {
                TreeNode* next = curr->right;
                omp_set_lock(&next->lock);
                omp_unset_lock(&curr->lock);
                curr = next;
            }
        } else {
            // already present: nothing to do
            omp_unset_lock(&curr->lock);
            break;
        }
    }

    return root;
}

TreeNode* deleteNode(TreeNode* root, int data) {
    if (root == NULL) return NULL;

    // Sentinel acting as a virtual parent of the root, so deleting the
    // root is handled uniformly. We hold its lock like a real parent
    // during the hand-over-hand descent.
    TreeNode sentinel;
    sentinel.data = 0;
    sentinel.left = root;
    sentinel.right = NULL;
    omp_init_lock(&sentinel.lock);
    omp_set_lock(&sentinel.lock);

    omp_set_lock(&root->lock);

    TreeNode* curr   = root;
    TreeNode* parent = &sentinel;
    TreeNode** ptr   = &(sentinel.left);   // the slot in 'parent' that points to 'curr'

    // Locate the node to delete; always hold parent->lock and curr->lock.
    while (curr != NULL) {
        if (data < curr->data) {
            if (curr->left == NULL) {                  // not found
                omp_unset_lock(&curr->lock);
                omp_unset_lock(&parent->lock);
                omp_destroy_lock(&sentinel.lock);
                return sentinel.left;
            }
            omp_set_lock(&curr->left->lock);
            omp_unset_lock(&parent->lock);
            parent = curr;
            curr   = curr->left;
            ptr    = &(parent->left);
        } else if (data > curr->data) {
            if (curr->right == NULL) {                 // not found
                omp_unset_lock(&curr->lock);
                omp_unset_lock(&parent->lock);
                omp_destroy_lock(&sentinel.lock);
                return sentinel.left;
            }
            omp_set_lock(&curr->right->lock);
            omp_unset_lock(&parent->lock);
            parent = curr;
            curr   = curr->right;
            ptr    = &(parent->right);
        } else {
            break;                                      // found: curr is the target
        }
    }

    // curr is the node to delete; we hold parent->lock and curr->lock.

    if (curr->left == NULL || curr->right == NULL) {
        // Zero or one child: splice curr out.
        TreeNode* child = (curr->left == NULL) ? curr->right : curr->left;
        *ptr = child;

        omp_unset_lock(&curr->lock);
        omp_unset_lock(&parent->lock);

        curr->left = NULL;
        curr->right = NULL;
        freeTree(curr);
    } else {
        // Two children: replace curr with its in-order successor
        // (leftmost node of curr's right subtree).
        TreeNode* succParent = curr->right;
        omp_set_lock(&succParent->lock);

        if (succParent->left == NULL) {
            // The right child itself is the successor.
            *ptr = succParent;
            succParent->left = curr->left;

            omp_unset_lock(&succParent->lock);
            omp_unset_lock(&curr->lock);
            omp_unset_lock(&parent->lock);

            curr->left = NULL;
            curr->right = NULL;
            freeTree(curr);
        } else {
            // Descend the left spine hand-over-hand to the leftmost node.
            TreeNode* succ = succParent->left;
            omp_set_lock(&succ->lock);

            while (succ->left != NULL) {
                omp_set_lock(&succ->left->lock);
                omp_unset_lock(&succParent->lock);   // release grandparent
                succParent = succ;
                succ = succ->left;
            }
            // succ is the successor; succParent is its parent.
            // Held locks: parent, curr, succParent, succ (all distinct).

            succParent->left = succ->right;          // unlink successor from its parent
            succ->left  = curr->left;                // successor adopts curr's children
            succ->right = curr->right;
            *ptr = succ;                             // and replaces curr under its parent

            omp_unset_lock(&succ->lock);
            omp_unset_lock(&succParent->lock);
            omp_unset_lock(&curr->lock);
            omp_unset_lock(&parent->lock);

            curr->left = NULL;
            curr->right = NULL;
            freeTree(curr);
        }
    }

    TreeNode* new_root = sentinel.left;
    omp_destroy_lock(&sentinel.lock);
    return new_root;
}

bool searchNode(TreeNode* root, int data) {
    if (root == NULL) return false;

    TreeNode* curr = root;
    startRead(curr);

    while (curr != NULL) {
        if (curr->data == data) {
            endRead(curr);
            return true;
        }

        // choose the next node while still holding curr's read lock
        TreeNode* next = (data < curr->data) ? curr->left : curr->right;

        if (next) startRead(next);   // lock child before releasing parent
        endRead(curr);
        curr = next;
    }
    return false;
}

TreeNode* findMin(TreeNode* root) {
    if (root == NULL) return NULL;

    TreeNode* current = root;
    startRead(current);

    // walk to the leftmost node
    while (current->left != NULL) {
        TreeNode* next = current->left;
        startRead(next);
        endRead(current);
        current = next;
    }

    endRead(current);
    return current;
}

void inorderTraversal(TreeNode* root) {
    if (!root) return;
    startRead(root);
    inorderTraversal(root->left);
    printf("%d ", root->data);
    inorderTraversal(root->right);
    endRead(root);
}

void freeTree(TreeNode* root) {
    if (root == NULL) return;
    freeTree(root->left);
    freeTree(root->right);
    omp_destroy_lock(&root->lock);
    omp_destroy_lock(&root->count_lock);
    free(root);
}