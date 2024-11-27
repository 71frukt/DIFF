#ifndef SIMPLIFIER_H
#define SIMPLIFIER_H

#include "diff_tree.h"
#include "operations.h"

void  SimplifyExpr        (Tree *expr_tree);
Node *SimplifyConstants   (Tree *tree, Node *cur_node);
Node *SimplifyFraction    (Tree *tree, Node *op_node, Node *numerator, Node *denominator);
bool  MulByFraction       (Node *mul_node, Node *left_arg, Node *right_arg);
Node *ComplexToTheRight   (Node *cur_node);
Node *TakeOutConsts       (Tree *tree, Node *cur_node);                     // a * (b * x)  =>  (a * b) * x
void  FlipDenominator     (Node *fraction_node);
void  ExpandBrackets      (Tree *tree, Node *mul_node);
bool  CanCalcDeg          (Node *degree_node);

void  SubToAdd            (Tree *tree);
void  AddToSub            (Tree *tree);                                       // 8 + (-1) * x  =>  8 - x

Node *FractionBecomesZero (Tree *tree, Node *div_node);
bool IsComplex(Node *node);

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define IS_INT_VAL(node)  (((long long int) node->value == node->value) && (node->type == NUM))

#endif