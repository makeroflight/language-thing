#include <stdio.h>
#include <stdlib.h>

#include "tokens.h"


void print_token(token *token) {
    if (token == NULL) {
        printf("NULL TOKEN\n");
        return;
    }

    if (token->value != NULL) {
        switch (token->type) {
        case T_NUMBER: { printf("(NUMBER, '%s')\n", token->value); break; } 
        case T_IDENTIFIER: { printf("(IDENTIFIER, '%s')\n", token->value); break; }
        case T_STRING: { printf("(STRING, '%s')\n", token->value); break; }
        default: break;
        }
    } else {
        const char *symbol;

        switch(token->type) {
        case T_AND: symbol = "AND"; break;
        case T_OR: symbol = "OR"; break;
        case T_FUN: symbol = "FUN"; break;
        case T_IF: symbol = "IF"; break;
        case T_ELSE: symbol = "ELSE"; break;
        case T_FOR: symbol = "FOR"; break;
        case T_WHILE: symbol = "WHILE"; break;

        case T_LPAREN: symbol = "LPAREN"; break;
        case T_RPAREN: symbol = "RPAREN"; break;
        case T_LBRACKET: symbol = "LBRACKET"; break;
        case T_RBRACKET: symbol = "RBRACKET"; break;
        case T_LCURLY: symbol = "LCURLY"; break;
        case T_RCURLY: symbol = "RCURLY"; break;

        case T_QUOTE: symbol = "QUOTE"; break;
        case T_DBL_QUOTE: symbol = "DBL_QUOTE"; break;
        case T_SEMICOLON: symbol = "SEMICOLON"; break;
        case T_COMMA: symbol = "COMMA"; break;
        case T_DOT: symbol = "DOT"; break;

        case T_HASHTAG: symbol = "HASHTAG"; break;
        case T_CARET: symbol = "CARET"; break;
        case T_PIPE: symbol = "PIPE"; break;
        case T_AMP: symbol = "AMP"; break;
        case T_EQL: symbol = "EQL"; break;
        case T_DBL_EQL: symbol = "DBL_EQL"; break;
        case T_BANG: symbol = "BANG"; break;
        case T_BANG_EQL: symbol = "BANG_EQL"; break;
        case T_GREATER: symbol = "GREATER"; break;
        case T_LESS: symbol = "LESS"; break;
        case T_GREATER_EQL: symbol = "GREATER_EQL"; break;
        case T_LESS_EQL: symbol = "LESS_EQL"; break;

        case T_PLUS: symbol = "PLUS"; break;
        case T_MINUS: symbol = "MINUS"; break;
        case T_ASTERISK: symbol = "ASTERISK"; break;
        case T_SLASH: symbol = "SLASH"; break;
        case T_PLUS_PLUS: symbol = "PLUS_PLUS"; break;
        case T_MINUS_MINUS: symbol = "MINUS_MINUS"; break;
        case T_PLUS_EQL: symbol = "PLUS_EQL"; break;
        case T_MINUS_EQL: symbol = "MINUS_EQL"; break;
        case T_ASTERISK_EQL: symbol = "ASTERISK_EQL"; break;
        case T_SLASH_EQL: symbol = "SLASH_EQL"; break;
        default:
            symbol = "UNDEF";
        }

        printf("(%s, %d)\n", symbol, token->type);
    }
}

/* token ops */
linked_token_list *create_token_list() {
    linked_token_list *ltl = malloc(sizeof(linked_token_list));

    ltl->next = NULL;
    ltl->node = NULL;

    return ltl;
}

void append_to_list(linked_token_list *list, token *node) {
    if (list->node == NULL) {
        list->node = node;
    } else if (list->next == NULL) {
        list->next = create_token_list();
        append_to_list(list->next, node);
    } else {
        append_to_list(list->next, node);
    }
}

token create_token() {
    token t;
    t.value = NULL;
    t.type = T_NONE;
    t.line = 0;

    return t;
}

void free_token(token *t) {
    free(t->value);
    t->value = NULL;
    return;

    if (t == NULL) {
        return;
    }

    if (t->value != NULL) {
        free(t->value);
        t->value = NULL;
    }

    free(t);
    t = NULL;
}

/* dynamic array utils */
void grow_array(token_dynamic_array *array, unsigned int new_size) {
    array->tokens = realloc(array->tokens, sizeof(token) * new_size);
    if (array->tokens == NULL) {
        fputs("error: unable to realloc array\n", stderr);
    }
}


/* public functions */
token_dynamic_array *create_token_dyn_array() {
    token_dynamic_array *array = malloc(sizeof(token_dynamic_array));
    array->tokens = malloc(sizeof(token) * 8);
    array->capacity = 8;
    array->count = 0;
    return array;
}

#define DYN_ARRAY_GROW_BY 2
void append_to_array(token_dynamic_array *array, token *t) {
    if (array->count == array->capacity) {
        array->capacity *= DYN_ARRAY_GROW_BY;
        grow_array(array, array->capacity);

        if (array == NULL) {
            printf("got NULL while performing array realloc\n");
            return;
        }
    }

    array->tokens[array->count] = *t;
    array->count++;
}

void free_array(token_dynamic_array *array) {
    for (int i = 0; i < array->count; i++) {
        free(array->tokens[i].value);
        array->tokens[i].value = NULL;
    }

    free(array->tokens);
    array->tokens = NULL;

    free(array);
    array = NULL;
}