#include "ik/effector.h"
#include "ik/log.h"
#include "ik/memory.h"
#include "ik/node.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>

/* ------------------------------------------------------------------------- */
struct node_t*
node_create(uint32_t guid)
{
    struct node_t* node = (struct node_t*)MALLOC(sizeof *node);
    if(node == NULL)
        return NULL;

    node_construct(node, guid);
    return node;
}

/* ------------------------------------------------------------------------- */
void
node_construct(struct node_t* node, uint32_t guid)
{
    memset(node, 0, sizeof *node);
    bstv_construct(&node->children);
    node->guid = guid;
}

/* ------------------------------------------------------------------------- */
void
node_destruct(struct node_t* node)
{
    BSTV_FOR_EACH(&node->children, struct node_t, guid, child)
        node_destroy(child);
    BSTV_END_EACH

    if(node->effector)
        effector_destroy(node->effector);

    bstv_clear_free(&node->children);
}

/* ------------------------------------------------------------------------- */
void
node_destroy(struct node_t* node)
{
    node_destruct(node);
    FREE(node);
}

/* ------------------------------------------------------------------------- */
void
node_add_child(struct node_t* node, struct node_t* child)
{
    child->parent = node;
    bstv_insert(&node->children, child->guid, child);
}

/* ------------------------------------------------------------------------- */
void
node_remove_child(struct node_t* node)
{
    if(node->parent == NULL)
        return;

    assert(node == bstv_erase(&node->parent->children, node->guid));
    node->parent = NULL;
}

/* ------------------------------------------------------------------------- */
struct node_t*
node_find_child(struct node_t* node, uint32_t guid)
{
    {
        struct node_t* child = bstv_find(&node->children, guid);
        if(child != NULL)
            return child;
    }

    BSTV_FOR_EACH(&node->children, struct node_t, child_guid, child)
        return node_find_child(child, child_guid);
    BSTV_END_EACH

    return NULL;
}

/* ------------------------------------------------------------------------- */
void
node_attach_effector(struct node_t* node, struct effector_t* effector)
{
    node->effector = effector;
}

/* ------------------------------------------------------------------------- */
static void
recursively_dump_dot(FILE* fp, struct node_t* node)
{
    if(node->effector != NULL)
        fprintf(fp, "    %d [color=\"1.0 0.5 1.0\"];\n", node->guid);

    BSTV_FOR_EACH(&node->children, struct node_t, guid, child)
        fprintf(fp, "    %d -- %d;\n", node->guid, guid);
        recursively_dump_dot(fp, child);
    BSTV_END_EACH
}

/* ------------------------------------------------------------------------- */
void
node_dump_to_dot(struct node_t* node, const char* file_name)
{
    FILE* fp = fopen(file_name, "w");
    if(fp == NULL)
    {
        ik_log_message("Failed to open file %s", file_name);
        return;
    }

    fprintf(fp, "graph graphname {\n");
    recursively_dump_dot(fp, node);
    fprintf(fp, "}\n");

    fclose(fp);
}