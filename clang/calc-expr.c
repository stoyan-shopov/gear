
#include <stdio.h>
#include "clang.h"
#include "parser.tab.h"
#include "clang-parser-common.h"

long int calc_expr(struct xlat_unit_parse_node * expr)
{
long int res;

	if (expr->type != PARSE_NODE_EXPR)
		panic("");
	switch (expr->expr_node.type)
	{
		default:
			panic("");
			break;
		case EXPR_NODE_INVALID:
			panic("");
			break;
		case EXPR_NODE_IDENT:
			panic("");
			break;
		case EXPR_NODE_CONSTANT:
			res = expr->expr_node.const_val;
			break;
		case EXPR_NODE_STR_LITERAL:
			panic("");
			break;
		case EXPR_NODE_PARENTHESIZED_EXPR:
			res = calc_expr(expr->expr_node.expr[0]);
			break;
		case EXPR_NODE_ARR_SUBSCRIPT:
			panic("");
			break;
		case EXPR_NODE_FUNCTION_CALL:
			panic("");
			break;
		case EXPR_NODE_DOT_MEMBER_SELECT:
			panic("");
			break;
		case EXPR_NODE_PTR_MEMBER_SELECT:
			panic("");
			break;
		case EXPR_NODE_POST_INC:
			panic("");
			break;
		case EXPR_NODE_POST_DEC:
			panic("");
			break;
		case EXPR_NODE_PRE_INC:
			panic("");
			break;
		case EXPR_NODE_PRE_DEC:
			panic("");
			break;
		case EXPR_NODE_UNARY_AMPERSAND:
			panic("");
			break;
		case EXPR_NODE_UNARY_INDIRECT:
			panic("");
			break;
		case EXPR_NODE_UNARY_PLUS:
			res = calc_expr(expr->expr_node.expr[0]);
			break;
		case EXPR_NODE_UNARY_MINUS:
			res = - calc_expr(expr->expr_node.expr[0]);
			break;
		case EXPR_NODE_UNARY_BITWISE_NOT:
			res = ~ calc_expr(expr->expr_node.expr[0]);
			break;
		case EXPR_NODE_UNARY_LOGICAL_NOT:
			res = ! calc_expr(expr->expr_node.expr[0]);
			break;
		case EXPR_NODE_SIZEOF:
			panic("");
			break;
		case EXPR_NODE_TYPE_CAST:
			panic("");
			break;
		case EXPR_NODE_MUL:
			res = calc_expr(expr->expr_node.expr[0])
				* calc_expr(expr->expr_node.expr[1]);
			break;
		case EXPR_NODE_DIV:
			res = calc_expr(expr->expr_node.expr[0])
				/ calc_expr(expr->expr_node.expr[1]);
			break;
		case EXPR_NODE_MOD:
			res = calc_expr(expr->expr_node.expr[0])
				% calc_expr(expr->expr_node.expr[1]);
			break;
		case EXPR_NODE_ADD:
			res = calc_expr(expr->expr_node.expr[0])
				+ calc_expr(expr->expr_node.expr[1]);
			break;
		case EXPR_NODE_SUB:
			res = calc_expr(expr->expr_node.expr[0])
				- calc_expr(expr->expr_node.expr[1]);
			break;
		case EXPR_NODE_LEFT_SHIFT:
			res = calc_expr(expr->expr_node.expr[0])
				<< calc_expr(expr->expr_node.expr[1]);
			break;
		case EXPR_NODE_RIGHT_SHIFT:
			res = calc_expr(expr->expr_node.expr[0])
				>> calc_expr(expr->expr_node.expr[1]);
			break;
		case EXPR_NODE_LESS:
			res = calc_expr(expr->expr_node.expr[0])
				< calc_expr(expr->expr_node.expr[1]);
			break;
		case EXPR_NODE_GREATER:
			res = calc_expr(expr->expr_node.expr[0])
				> calc_expr(expr->expr_node.expr[1]);
			break;
		case EXPR_NODE_LESS_OR_EQUAL:
			res = calc_expr(expr->expr_node.expr[0])
				<= calc_expr(expr->expr_node.expr[1]);
			break;
		case EXPR_NODE_GREATER_OR_EQUAL:
			res = calc_expr(expr->expr_node.expr[0])
				>= calc_expr(expr->expr_node.expr[1]);
			break;
		case EXPR_NODE_EQUAL:
			res = calc_expr(expr->expr_node.expr[0])
				== calc_expr(expr->expr_node.expr[1]);
			break;
		case EXPR_NODE_NOT_EQUAL:
			res = calc_expr(expr->expr_node.expr[0])
				!= calc_expr(expr->expr_node.expr[1]);
			break;
		case EXPR_NODE_BITWISE_AND:
			res = calc_expr(expr->expr_node.expr[0])
				& calc_expr(expr->expr_node.expr[1]);
			break;
		case EXPR_NODE_XOR:
			res = calc_expr(expr->expr_node.expr[0])
				^ calc_expr(expr->expr_node.expr[1]);
			break;
		case EXPR_NODE_BITWISE_OR:
			res = calc_expr(expr->expr_node.expr[0])
				| calc_expr(expr->expr_node.expr[1]);
			break;
		case EXPR_NODE_LOGICAL_AND:
			res = calc_expr(expr->expr_node.expr[0])
				&& calc_expr(expr->expr_node.expr[1]);
			break;
		case EXPR_NODE_LOGICAL_OR:
			res = calc_expr(expr->expr_node.expr[0])
				|| calc_expr(expr->expr_node.expr[1]);
			break;
		case EXPR_NODE_TERNARY_COND:
			res = calc_expr(expr->expr_node.expr[0]) ?
				calc_expr(expr->expr_node.expr[1])
				: calc_expr(expr->expr_node.cond_expr);
			break;
		case EXPR_NODE_ASSIGN:
			panic("");
			break;
		case EXPR_NODE_MUL_ASSIGN:
			panic("");
			break;
		case EXPR_NODE_DIV_ASSIGN:
			panic("");
			break;
		case EXPR_NODE_MOD_ASSIGN:
			panic("");
			break;
		case EXPR_NODE_ADD_ASSIGN:
			panic("");
			break;
		case EXPR_NODE_SUB_ASSIGN:
			panic("");
			break;
		case EXPR_NODE_LEFT_ASSIGN:
			panic("");
			break;
		case EXPR_NODE_RIGHT_ASSIGN:
			panic("");
			break;
		case EXPR_NODE_AND_ASSIGN:
			panic("");
			break;
		case EXPR_NODE_XOR_ASSIGN:
			panic("");
			break;
		case EXPR_NODE_OR_ASSIGN:
			panic("");
			break;
		case EXPR_NODE_COMMA:
			panic("");
			break;
	}
	return res;
}
