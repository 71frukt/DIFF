#include <stdio.h>
#include <assert.h>

#include "simplifier.h"
#include "operations.h"
#include "diff_tree.h"
#include "diff_debug.h"

void SimplifyExpr(Tree *expr_tree)
{
    fprintf(stderr, "rabotaet yproshalka\n");
    DIFF_DUMP(expr_tree);

    // TakeOutConsts(expr_tree->root_ptr);

    SimplifyConstants(expr_tree, expr_tree->root_ptr);

    DIFF_DUMP(expr_tree);
}

Node *SimplifyConstants(Tree *tree, Node *cur_node)
{
    assert(tree);
    assert(cur_node);

    // DIFF_DUMP(tree);

fprintf(stderr, "type = %d, val = %d\n", cur_node->type, cur_node->value);

    if (cur_node->type == NUM || cur_node->type == VAR)
        return cur_node;

    Node *left_arg  = SimplifyConstants(tree, cur_node->left);
    Node *right_arg = SimplifyConstants(tree, cur_node->right);

    if (left_arg->type == NUM && right_arg->type == NUM)
    {
        const Operation *cur_op = GetOperationByNum((int) cur_node->value);
        assert(cur_op);

        if (cur_op->num == ADD || cur_op->num == SUB || cur_op->num == MUL || (cur_op->num == DEG && CanCalcDeg(right_arg)))
        {
            TreeElem_t new_val = cur_op->op_func(left_arg->value, right_arg->value);

            cur_node->left  = NULL;
            cur_node->right = NULL;

            cur_node->type = NUM;
            cur_node->value = new_val;

            RemoveNode(tree, &left_arg);
            RemoveNode(tree, &right_arg);

            return cur_node;
        }
    }

    if (cur_node->value == DIV)
    {
        TakeOutConsts(cur_node);

        if (cur_node->value == DIV)
            SimplifyFraction(tree, cur_node, left_arg, right_arg);
    }

    else if (cur_node->value == MUL)
    {
        TakeOutConsts(cur_node);                                                            // a * (b * x)  =>  (a * b) * x

        if (cur_node->value == MUL && MulByFraction(cur_node, left_arg, right_arg))         // ���� ���� ��������� �� �����
            SimplifyConstants(tree, cur_node);
    }        

    return cur_node;
}

Node *SimplifyFraction(Tree *tree, Node *op_node, Node *numerator, Node *denominator)
{
    assert(tree);
    assert(numerator);
    assert(denominator);

    if (numerator->type == NUM && numerator->value == 0)
        return FractionBecomesZero(tree, op_node);

    bool numerator_is_int   = IS_INT_VAL(numerator);
    bool denominator_is_int = IS_INT_VAL(denominator);

    bool can_calc = ((!numerator_is_int || !denominator_is_int) && (numerator->type == NUM && denominator->type == NUM)) ||                                                               // ��� double
                   ((numerator_is_int  &&  denominator_is_int)  && (numerator->value % denominator->value) == 0);                // ��� ������� ������

    if (numerator->type == NUM && denominator->type == NUM && can_calc)
    {
        TreeElem_t new_val = numerator->value / denominator->value;
        
        RemoveNode(tree, &op_node->left);
        RemoveNode(tree, &op_node->right);

        op_node->left  = NULL;
        op_node->right = NULL;

        op_node->type = NUM;
        op_node->value = new_val;
    }

    else if ((numerator->type   == OP && numerator->value   == DIV)
          || (denominator->type == OP && denominator->value == DIV))
    {
        FlipDenominator(op_node);
        SimplifyConstants(tree, op_node);
    }

    else if (numerator_is_int && denominator_is_int)   // � ���������, � ����������� �����
    {
        TreeElem_t start_val = MIN(numerator->value, denominator->value) / 2;

        for (TreeElem_t divider = start_val; divider > 0; divider--)
        {
            if (divider > numerator->value)
                divider = numerator->value;
            
            if (numerator->value % divider == 0 && denominator->value % divider == 0)
            {
                numerator->value   /= divider;
                denominator->value /= divider;
            }
        } 
    }

    return op_node;
}

void FlipDenominator(Node *fraction_node)           // ���������� ����� � ��������� ��� � ����������� �����
{
    assert(fraction_node);
    assert(fraction_node->right);
    assert(fraction_node->right);

    Node *denom = fraction_node->right;

    if (denom->type == OP && denom->value == DIV)   // � ����������� �����
    {
        Node *numerator_of_denom = denom->left;

        denom->value = MUL;
        denom->left = fraction_node->left;

        fraction_node->left  = denom;
        fraction_node->right = numerator_of_denom;
    }

    else                                            // � ����������� �� ����� => � ��������� �����
    {
        Node *prev_denom = fraction_node->right;

        fraction_node->right = fraction_node->left;
        fraction_node->right->value = MUL;

        Node *new_denom = fraction_node->right;

        fraction_node->left  = fraction_node->left->left;

        new_denom->left  = new_denom->right;
        new_denom->right = prev_denom;
    }
}

bool MulByFraction(Node *mul_node, Node *left_arg, Node *right_arg)
{
    assert(mul_node);
    assert(left_arg);
    assert(right_arg);

    bool exists_mul_by_frac = true;

    bool left_is_fraction  = (left_arg->type == OP  && left_arg->value  == DIV);
    bool right_is_fraction = (right_arg->type == OP && right_arg->value == DIV);

    if (left_is_fraction && !right_is_fraction)
    {
        mul_node->value = DIV;
        mul_node->left->value = MUL;

        mul_node->right = left_arg->right;
        left_arg->right = right_arg;
    }

    else if (right_is_fraction && !left_is_fraction)
    {
        mul_node->value = DIV;

        mul_node->left = right_arg;
        mul_node->left->value = MUL;

        mul_node->right = right_arg->right;
        right_arg->right = left_arg;
    }

    else if (left_is_fraction && right_is_fraction)
    {
        mul_node->value  = DIV;
        left_arg->value  = MUL;
        right_arg->value = MUL;

        Node *tmp_node = left_arg->right;

        left_arg->right = right_arg->left;
        right_arg->left = tmp_node;
    }

    else
        exists_mul_by_frac = false;

    return exists_mul_by_frac;
}

Node *ComplexToTheRight(Node *mul_node)
{
    assert(mul_node);
    assert(mul_node->left);
    assert(mul_node->right);
    assert(mul_node->type == OP && mul_node->value == MUL);

    if (IsComplex(mul_node->left) && !IsComplex(mul_node->right))
    {
        Node *tmp_node  = mul_node->left;
        mul_node->left  = mul_node->right;
        mul_node->right = tmp_node;
    }

    return mul_node;
}

Node *TakeOutConsts(Node *cur_node)                     // a * (b * x)  =>  (a * b) * x
{                                                       // (a * x) / b  =>  (a / b) * x
    if (cur_node == NULL || !(cur_node->type == OP && (cur_node->value == MUL || cur_node->value == DIV)))
        return cur_node;

    if (cur_node->value == MUL)
        ComplexToTheRight(cur_node);

    Node *left_arg  = TakeOutConsts(cur_node->left);
    Node *right_arg = TakeOutConsts(cur_node->right); 

    if (cur_node->value == MUL)
    {
        if (right_arg->type != OP || right_arg->value != MUL)
            return cur_node;

        if (!IsComplex(left_arg)        && IsComplex(right_arg) &&
            !IsComplex(right_arg->left) && IsComplex(right_arg->right))
        {
            cur_node->right = right_arg->right;
            cur_node->left  = right_arg;

            right_arg->right = left_arg;
        }
    }

    else
    {
        if (left_arg->type != OP || left_arg->value != MUL)
            return cur_node;

        if (IsComplex(left_arg)       && !IsComplex(right_arg) && 
           !IsComplex(left_arg->left) && IsComplex(left_arg->right))
        {
            cur_node->right = left_arg->right;
            left_arg->right = right_arg;

            left_arg->value = DIV;
            cur_node->value = MUL;
        }
    }

    return cur_node;
}

bool CanCalcDeg(Node *degree_node)
{
    return ((IS_INT_TREE && IS_INT_VAL(degree_node) && degree_node->value > 0) || !IS_INT_TREE);
}

Node *FractionBecomesZero(Tree *tree, Node *div_node)
{
    Node *denominator = div_node->right;

    if (denominator->type == NUM && denominator->value == 0)
    {
        fprintf(stderr, "%p\n", div_node);

        fprintf(stderr, "Warning: uncertainty type of '0/0'\n");
        return div_node;
    }

    RemoveNode(tree, &div_node->left);
    RemoveNode(tree, &div_node->right);

    div_node->left  = NULL;
    div_node->right = NULL;

    div_node->type  = NUM;
    div_node->value = 0;

    return div_node;
}

bool IsComplex(Node *node)
{
    return (SubtreeContainsVar(node) || (node->type == OP && node->value != DIV));      // ����� ������� ��� ���������
}