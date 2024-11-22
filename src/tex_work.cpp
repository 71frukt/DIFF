#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "diff_tree.h"

const char *OperationToTex(int node_op)
{
    #define _CASE(operation)                                \
    {                                                       \
        case operation:                                     \
            return operation##_TEX;                         \
    }                                                       

    switch (node_op)
    {
        _CASE(ADD);
        _CASE(SUB);
        _CASE(MUL);
        _CASE(DIV);
        _CASE(DEG);
    
        default:
            fprintf(stderr, "unknown operation in OperationToTex() numbered %d\n", node_op);
            return NULL;
    }

    #undef _CASE
}

const char *GetTexTreeData(Node *start_node, char *dest_str, bool need_brackets)
{
    assert(start_node);

    char node_val_str[LABEL_LENGTH] = {};
    NodeValToStr(start_node->value, start_node->type, node_val_str);

    if (start_node->type != OP)
        sprintf(dest_str + strlen(dest_str), "%s", node_val_str);

    else
    {
        const char *op_tex = OperationToTex((int) start_node->value);

        bool param1_brackets = true;
        bool param2_brackets = true;
        ParamsNeedBrackets((int) start_node->value, &param1_brackets, &param2_brackets);

        if (IsPrefixOperation((int) start_node->value))
        {
            sprintf(dest_str + strlen(dest_str), " %s ", op_tex);

            sprintf(dest_str + strlen(dest_str), "{"); 
            GetTexTreeData(start_node->left,  dest_str + strlen(dest_str), param1_brackets);
            sprintf(dest_str + strlen(dest_str), "} {");

            GetTexTreeData(start_node->right, dest_str + strlen(dest_str), param2_brackets);
            sprintf(dest_str + strlen(dest_str), "}");
        }

        else
        {
            if (need_brackets) 
                sprintf(dest_str + strlen(dest_str), "\\left(");

            sprintf(dest_str + strlen(dest_str), "{"); 

            GetTexTreeData(start_node->left, dest_str, param1_brackets);

            sprintf(dest_str + strlen(dest_str), " %s ", op_tex);

            GetTexTreeData(start_node->right, dest_str + strlen(dest_str), param2_brackets);

            sprintf(dest_str + strlen(dest_str), "}");

            if (need_brackets)
                sprintf(dest_str + strlen(dest_str), "\\right)");
        }    
    }

    return dest_str;
}

bool IsPrefixOperation(int op)
{
    if (op == DIV)
        return true;
    else
        return false;
}

void ParamsNeedBrackets(int operation, bool *param_1, bool *param_2)
{
    if (operation == DIV)
    {
        *param_1 = false;
        *param_2 = false;
    }

    else if (operation == DEG)
    {
        *param_1 = true;
        *param_2 = false;
    }

    else
    {
        *param_1 = true;
        *param_2 = true;
    }
}