#include "parse.h"

static GHashTable     *reg = NULL;

struct Functions {
    ParseFunc       f;
    AssignParseFunc af;
} *funcs;

void destshit(gpointer shit) { g_free(shit); }

void parse_startup()
{
    reg = g_hash_table_new_full(g_str_hash, g_str_equal, destshit, destshit);
    funcs = NULL;
}

void parse_shutdown()
{
    g_hash_table_destroy(reg);
}

void parse_reg_section(char *section, ParseFunc func, AssignParseFunc afunc)
{
    struct Functions *f = g_new(struct Functions, 1);
    f->f = func;
    f->af = afunc;
    g_hash_table_insert(reg, g_ascii_strdown(section, -1), f);
}

void parse_free_token(ParseToken *token)
{
    GList *it;

    switch (token->type) {
    case TOKEN_STRING:
        g_free(token->data.string);
        break;
    case TOKEN_IDENTIFIER:
        g_free(token->data.identifier);
        break;
    case TOKEN_LIST:
        for (it = token->data.list; it; it = it->next) {
            parse_free_token(it->data);
            g_free(it->data);
        }
        g_list_free(token->data.list);
        break;
    case TOKEN_REAL:
    case TOKEN_INTEGER:
    case TOKEN_BOOL:
    case TOKEN_LBRACE:
    case TOKEN_RBRACE:
    case TOKEN_COMMA:
    case TOKEN_NEWLINE:
        break;
    }
}

void parse_set_section(char *section)
{
    char *sec;
    sec = g_ascii_strdown(section, -1);
    funcs = g_hash_table_lookup(reg, sec);
    g_free(sec);
}

void parse_token(ParseToken *token)
{
    if (funcs) {
        if (funcs->f)
            funcs->f(token);
        else if (token->type != TOKEN_NEWLINE)
            yyerror("syntax error");
    }
}

void parse_assign(char *name, ParseToken *value)
{
    if (funcs) {
        if (funcs->af)
            funcs->af(name, value);
        else
            yyerror("syntax error");
    }
}
