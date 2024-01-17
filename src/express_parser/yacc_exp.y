
%{

#include "express_defs.h"
#include "yacc_exp.tab.h"
#include "lex_exp.yy.h"

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include<assert.h>
#include<stddef.h>
#include<stdbool.h>

#if YYDEBUG > 0
#define debug_printf  printf
#else
#define debug_printf(...)
#endif // YYDEBUG

/* 获取子串
 * 中提取下标为n1~n2的字符组成一个新字符串，然后返回这个新串的首地址
 * @param s 原字符串
 * @param n1 起始下标
 * @param n2 结束下标
 * @return 新串的首地址
 */
char *substr(const char *s,int n1,int n2) {
  char *sp = malloc(sizeof(char) * (n2 - n1 + 2));
  int i, j = 0;
  for (i = n1; i <= n2; i++) {
    sp[j++] = s[i];
  }
  sp[j] = 0;
  return sp;
}

char** split_string(const char* s, const char delim, size_t* num) {
    char* buffer = strdup(s);
    for (int i = 0; buffer[i] != '\0'; i++) {
        if (buffer[i] == delim) {
            buffer[i] = '\0';
            (*num)++;
        }
    }

    char** result_list = malloc(sizeof(char*) * (*num));
    char* pos = buffer;
    for (int i = 0; i < *num; i++) {
        result_list[i] = strdup(pos);
        pos += strlen(pos) + 1;
    }

    free(buffer);
    return result_list;
}

void* append_array(void** dest, const void* src, size_t dest_len, size_t elem_size) {
    void* temp = realloc(*dest, elem_size * (dest_len + 1));
    if (temp == NULL) {
        return NULL;
    }
    *dest = temp;
    if (src != NULL) {
        memcpy((char*)*dest + dest_len * elem_size, src, elem_size);
    } else {
        memset((char*)*dest + dest_len * elem_size, 0, elem_size);
    }
    return *dest;
}


void yyerror(yyscan_t scanner, const char *str) {
//   ParserContext *context = (ParserContext *)(yyget_extra(scanner));
  printf("parse .exp failed. error=%s\n", str);
}

void clear_entity_args_context(ParserContext* parser_context) {
    assert(parser_context->entity_args.entity_name != NULL);
    // 清空类型名
    free(parser_context->entity_args.entity_name);
    parser_context->entity_args.entity_name = NULL;

    // 清空子类型名
    for(int i = 0; i < parser_context->entity_args.subtype_num; i++) {
        assert(parser_context->entity_args.subtype_names[i] != NULL);
        free(parser_context->entity_args.subtype_names[i]);
    }
    free(parser_context->entity_args.subtype_names);
    parser_context->entity_args.subtype_names = NULL;
    parser_context->entity_args.subtype_num = 0;

    // 清空父类型名
    if(parser_context->entity_args.supertype_name != NULL) {
        free(parser_context->entity_args.supertype_name);
        parser_context->entity_args.supertype_name = NULL;
    }

    // 清空属性
    for(int i = 0; i < parser_context->entity_args.attr_num; i++) {
        assert(parser_context->entity_args.attr_names[i] != NULL);
        free(parser_context->entity_args.attr_names[i]);
        free(parser_context->entity_args.attr_types[i]);
    }
    free(parser_context->entity_args.attr_names);
    free(parser_context->entity_args.is_optional);
    free(parser_context->entity_args.attr_types);
    parser_context->entity_args.attr_names = NULL;
    parser_context->entity_args.is_optional = NULL;
    parser_context->entity_args.attr_types = NULL;
    parser_context->entity_args.attr_num = 0;

    parser_context->entity_args.is_abstract = false;
}

ParserContext *get_context(yyscan_t scanner) {
  return (ParserContext *)yyget_extra(scanner);
}

#define CONTEXT get_context(scanner)

%}

%code requires {
    typedef struct ParserContext {
        void* parser;
        TypeArgs type_args;
        EntityArgs entity_args;
    } ParserContext, *ParserContextPtr;

    typedef struct Range {
        size_t lb;
        size_t ub;
    } Range;
}

%define api.pure full
%lex-param { yyscan_t scanner }
%parse-param { void *scanner }

// 终结符token
%token SCHEMA TYPE END_TYPE ABSTRACT SUPERTYPE SUBTYPE ENTITY END_ENTITY OF ONEOF FOR INVERSE FUNCTION END_FUNCTION RULE END_RULE FIXED DERIVE END_SCHEMA WHERE REAL BOOLEAN STRING INTEGER ENUMERATION LIST SET SELECT LOGICAL SEMICOLON DOT STAR LBRACKET RBRACKET OR COLON_EQ LBRACE RBRACE COMMA EQ LE NE LT GE GT QUESTION COLON AND SSS EXPRESSION SELF IN OPTIONAL ARRAY NUMBER UNIQUE BINARY

%union {
  TypeArgs* type_args;
  Range range;
  char *string;
  int integer;
  float floats;
}

// 终结符类型声明
%token <string> LABEL
%token <integer> INTEGER_VALUE
%token <string> ENTITY_NAME

//非终结符
%type <string> label_list;
%type <string> enum_attr;
%type <string> entity_names;
%type <string> entity_name_list;
%type <type_args> type_name;
%type <range> range;

%%

schema: SCHEMA LABEL SEMICOLON declaration_list END_SCHEMA SEMICOLON {
            debug_printf("parser ended.\n");
            free($2);
        }
      ;

declaration_list: declaration
                | declaration_list declaration
                ;

declaration: 
          type_declaration
        | entity_declaration
        ;

type_declaration: TYPE ENTITY_NAME EQ type_name SEMICOLON END_TYPE SEMICOLON      // TYPE Label = INTEGER;
                {
                    debug_printf("matched %s\n", $2);
                    TypeArgs* args = $4;
                    args->type_name = $2;
                    init_type(CONTEXT->parser, args);
                    destroy_type_name_args(args);
                }
                ;

type_name: ENTITY_NAME
        {
            debug_printf("[DEBUG] type_name: %s\n", $1);       // Ifcxxx
            TypeArgs* args = new_type_name_args(NULL, T_REFERENCE);
            args->ref_type_name = $1;
            $$ = args;
        }
        | STRING LBRACE INTEGER_VALUE RBRACE FIXED      // STRING(22) FIXED
        {
            debug_printf("[DEBUG] type_name: STRING (%d) FIXED\n", $3);
            TypeArgs* args = new_type_name_args(NULL, T_STRING);
            args->ub = $3;
            args->fixed_length = true;
            $$ = args;
        }
        | STRING LBRACE INTEGER_VALUE RBRACE      // STRING(22)
        {
            debug_printf("[DEBUG] type_name: STRING (%d)\n", $3);
            TypeArgs* args = new_type_name_args(NULL, T_STRING);
            args->ub = $3;
            $$ = args;
        }
        | REAL
        {
            debug_printf("[DEBUG] type_name: REAL\n");
            TypeArgs* args = new_type_name_args(NULL, T_REAL);
            $$ = args;
        }
        | INTEGER
        {
            debug_printf("[DEBUG] type_name: matched INTEGER\n");
            $$ = new_type_name_args(NULL, T_INTEGER);
        }
        | LOGICAL
        {
            debug_printf("[DEBUG] type_name: matched LOGICAL\n");
            $$ = new_type_name_args(NULL, T_LOGICAL);

        }
        | BOOLEAN
        {
            debug_printf("[DEBUG] type_name: matched BOOLEAN\n");
            $$ = new_type_name_args(NULL, T_BOOLEAN);
        }
        | NUMBER
        {
            debug_printf("[DEBUG] type_name: matched NUMBER\n");
            $$ = new_type_name_args(NULL, T_NUMBER);
        }
        | STRING
        {
            debug_printf("[DEBUG] type_name: matched STRING\n");
            $$ = new_type_name_args(NULL, T_STRING);
        }
        | BINARY
        {
            debug_printf("[DEBUG] type_name: BINARY\n");
            $$ = new_type_name_args(NULL, T_BINARY);
        }
        | BINARY LBRACE INTEGER_VALUE RBRACE      // BINARY(22)
        {
            debug_printf("[DEBUG] type_name: BINARY  (%d)\n", $3);
            TypeArgs* args = new_type_name_args(NULL, T_BINARY);
            args->ub = $3;
            $$ = args;
        }
        | LIST range OF type_name 
        {
            TypeArgs* args = $4;
            args->is_list = true;
            args->lb = $2.lb;
            args->ub = $2.ub;
            debug_printf("[DEBUG] type_name: LIST [%zu:%zu] OF type_name\n", args->lb, args->ub);
            $$ = args;
        }
        | LIST range OF UNIQUE type_name 
        {
            debug_printf("[DEBUG] type_name: matched LIST range OF type_name\n");
            TypeArgs* args = $5;
            args->is_list = true;
            $$ = args;
        }
        | SET range OF type_name 
        {
            debug_printf("[DEBUG] type_name: matched SET range OF type_name\n");
            TypeArgs* args = $4;
            args->is_list = true;
            $$ = args;
        }
        | ARRAY range OF type_name 
        {
            debug_printf("[DEBUG] type_name: matched ARRAY range OF type_name\n");
            TypeArgs* args = $4;
            args->is_list = true;
            $$ = args;
        }
        | ENUMERATION OF LBRACE enum_attr RBRACE
        {
            TypeArgs* args = new_type_name_args(NULL, T_ENUM);
            char* buffer = $4;
            debug_printf("[DEBUG] type_name: ENUMERATION OF (%s)\n", buffer);
            for (int i = 0; buffer[i] != '\0'; i++) {
                debug_printf("[DEBUG] i=%d: %c\n", i, buffer[i]);
                if (buffer[i] == ',') {
                    debug_printf("[DEBUG] i=%d: got a comma %c\n", i, buffer[i]);
                    buffer[i] = '\0';
                    args->enum_num++;
                }
            }
            args->enum_list = malloc(sizeof(char*) * args->enum_num);
            for (int i = 0; i < args->enum_num; i++) {
                args->enum_list[i] = strdup(buffer);
                buffer += strlen(buffer) + 1;
                debug_printf("[DEBUG] append %d enum (%s)\n", i, args->enum_list[i]);
            }
            free($4);
            $$ = args;
        }
        | SELECT LBRACE entity_names RBRACE 
        {
            debug_printf("[DEBUG] type_name: SELECT OF (%s)\n", $3);
            TypeArgs* args = new_type_name_args(NULL, T_SELECT);
            char* buffer = $3;
            for (int i = 0; buffer[i] != '\0'; i++) {
                if (buffer[i] == ',') {
                    buffer[i] = '\0';
                    args->select_num++;
                }
            }
            args->select_list = malloc(sizeof(char*) * args->select_num);
            for (int i = 0; i < args->select_num; i++) {
                args->select_list[i] = strdup(buffer);
                buffer += strlen(buffer) + 1;
            }
            free($3);
            $$ = args;
        }
        ;

range: LBRACKET INTEGER_VALUE COLON INTEGER_VALUE RBRACKET {
            Range range = {$2, $4};
            $$ = range;
        }
       | LBRACKET INTEGER_VALUE COLON QUESTION RBRACKET {
            Range range = {$2, UINT32_MAX};
            $$ = range;
       }
     ;

enum_attr: LABEL label_list { 
            char* res = $1;
            res = realloc(res, strlen(res) + 1 + ($2 == NULL ? 0 : strlen($2)) + 1);
            $1 = NULL;
            strcat(res, ",");
            if ($2 != NULL) {
                strcat(res, $2);
                free($2);
            } 
            $$ = res;
        }
        ;

label_list: 
        /* empty */ {
            $$ = NULL;
        }
        | COMMA LABEL label_list { 
            char* res = $2;
            res = realloc(res, strlen(res) + 1 + ($3 == NULL ? 0 : strlen($3)) + 1);
            $2 = NULL;
            strcat(res, ",");
            if ($3 != NULL) {
                strcat(res, $3);
                free($3);
            }
            $$ = res;
        }
        ;

entity_names: ENTITY_NAME entity_name_list { 
            char* res = $1;
            size_t entity_list_len = $2 == NULL ? 0 : strlen($2);
            res = realloc(res, strlen(res) + 1 + entity_list_len + 1);
            strcat(res, ",");
            if ($2 != NULL) {
                strcat(res, $2);
                free($2);
            }
            $$ = res;
            debug_printf("[DEBUG] entity_names: %s\n", $$);
        }

entity_name_list: 
    /* empty */ {
        $$ = NULL;
    }
    | COMMA ENTITY_NAME entity_name_list{ 
        char* res = $2;
        size_t list_len = $3 == NULL ? 0 : strlen($3);
        res = realloc(res, strlen(res) + 1 + list_len + 1);
        strcat(res, ",");
        if($3 != NULL) {
            strcat(res, $3);
            free($3);
        }
        $$ = res;
        debug_printf("[DEBUG] entity_list: %s\n", $$);
    }
    ;

entity_declaration: ENTITY ENTITY_NAME supertype_of subtype_of SEMICOLON entity_attr unique_condition END_ENTITY SEMICOLON {
    debug_printf("[DEBUG] entity definitation: %s\n", $2);
    size_t subtype_num = 0;
    EntityArgs* args = &CONTEXT->entity_args;
    args->entity_name = $2;

    init_entity(CONTEXT->parser, args);
    clear_entity_args_context(CONTEXT);
}
    ; 

supertype_of: 
    /* empty */ {
    }
    | SUPERTYPE OF LBRACE ENTITY_NAME RBRACE {
        debug_printf("[DEBUG] supertype_of: matched SUPERTYPE OF LBRACE %s RBRACE SEMICOLON\n", $4);
        CONTEXT->entity_args.subtype_num = 1;
        CONTEXT->entity_args.subtype_names = malloc(sizeof(char*));
        CONTEXT->entity_args.subtype_names[0] = $4;
        CONTEXT->entity_args.is_abstract = false;
    }
    | SUPERTYPE OF LBRACE ONEOF LBRACE entity_names RBRACE RBRACE {
        debug_printf("[DEBUG] supertype_of: matched ABSTRACT SUPERTYPE OF LBRACE ONEOF LBRACE %s RBRACE RBRACE SEMICOLON\n", $6);
        CONTEXT->entity_args.subtype_names = split_string($6, ',', (size_t*)&CONTEXT->entity_args.subtype_num);
        CONTEXT->entity_args.is_abstract = false;
    }
    | ABSTRACT SUPERTYPE OF LBRACE ENTITY_NAME RBRACE {
        debug_printf("[DEBUG] ABSTRACT SUPERTYPE OF LBRACE %s RBRACE SEMICOLON\n", $5);
        CONTEXT->entity_args.subtype_num = 1;
        CONTEXT->entity_args.subtype_names = malloc(sizeof(char*));
        CONTEXT->entity_args.subtype_names[0] = $5;
        CONTEXT->entity_args.is_abstract = true;
    }
    | ABSTRACT SUPERTYPE OF LBRACE ONEOF LBRACE entity_names RBRACE RBRACE {
        debug_printf("[DEBUG] ABSTRACT SUPERTYPE OF LBRACE ONEOF LBRACE %s RBRACE RBRACE SEMICOLON\n", $7);
        CONTEXT->entity_args.subtype_names = split_string($7, ',', (size_t*)&CONTEXT->entity_args.subtype_num);
        CONTEXT->entity_args.is_abstract = true;
    }
    ;

subtype_of: 
    /* empty */ {
    }
    | SUBTYPE OF LBRACE ENTITY_NAME RBRACE {
        debug_printf("[DEBUG] subtype_of: matched SUBTYPE OF LBRACE %s RBRACE SEMICOLON\n", $4);
        CONTEXT->entity_args.supertype_name = $4;
    }
    ;

entity_attr: 
    /* empty */
    | LABEL COLON OPTIONAL type_name SEMICOLON entity_attr_list {
        debug_printf("[DEBUG] append attr %s\n", $1);
        CONTEXT->entity_args.attr_names = (char**)append_array((void**)&CONTEXT->entity_args.attr_names, &$1, CONTEXT->entity_args.attr_num, sizeof(char*));
        
        // 处理属性类型信息
        TypeArgs* args = $4;
        bool flag = true;
        append_array((void**)&CONTEXT->entity_args.attr_types, &args, CONTEXT->entity_args.attr_num, sizeof(char*));
        append_array((void**)&CONTEXT->entity_args.is_optional, &flag, CONTEXT->entity_args.attr_num, sizeof(bool));
        CONTEXT->entity_args.attr_num++;
    }
    | LABEL COLON type_name SEMICOLON entity_attr_list {
        debug_printf("[DEBUG] append attr %s\n", $1);

        append_array((void**)&CONTEXT->entity_args.attr_names, &$1, CONTEXT->entity_args.attr_num, sizeof(char*));

        TypeArgs* args = $3;
        bool flag = false;
        append_array((void**)&CONTEXT->entity_args.attr_types, &args, CONTEXT->entity_args.attr_num, sizeof(char*));
        append_array((void**)&CONTEXT->entity_args.is_optional, &flag, CONTEXT->entity_args.attr_num, sizeof(bool));
        CONTEXT->entity_args.attr_num++;
    }
    ;

entity_attr_list: 
    /* empty */
    | LABEL COLON OPTIONAL type_name SEMICOLON  entity_attr_list {
        debug_printf("[DEBUG] append attr %s\n", $1);

        append_array((void**)&CONTEXT->entity_args.attr_names, &$1, CONTEXT->entity_args.attr_num, sizeof(char*));
        
        // 处理属性类型信息
        TypeArgs* args = $4;
        bool flag = true;
        append_array((void**)&CONTEXT->entity_args.attr_types, &args, CONTEXT->entity_args.attr_num, sizeof(char*));
        CONTEXT->entity_args.is_optional = (bool*)append_array((void**)&CONTEXT->entity_args.is_optional, &flag, CONTEXT->entity_args.attr_num, sizeof(bool));
        CONTEXT->entity_args.attr_num++;
    }
    | LABEL COLON type_name SEMICOLON  entity_attr_list {
        debug_printf("[DEBUG] append attr %s\n", $1);

        append_array((void**)&CONTEXT->entity_args.attr_names, &$1, CONTEXT->entity_args.attr_num, sizeof(char*));

        TypeArgs* args = $3;
        bool flag = false;
        append_array((void**)&CONTEXT->entity_args.attr_types, &args, CONTEXT->entity_args.attr_num, sizeof(char*));
        append_array((void**)&CONTEXT->entity_args.is_optional, &flag, CONTEXT->entity_args.attr_num, sizeof(bool));
        CONTEXT->entity_args.attr_num++;
    }
    ;

unique_condition:
    /* empty */
    | UNIQUE LABEL COLON LABEL label_list SEMICOLON unique_condition_list {
        debug_printf("UNIQUE CONDITION: ignore");
        free($2);
        free($4);
        free($5);
    }
    ;

unique_condition_list:
    /* empty */
    | LABEL COLON LABEL label_list SEMICOLON unique_condition_list {
        debug_printf("UNIQUE CONDITION LIST: ignore");
        free($1);
        free($3);
        free($4);
    }
    ;


%%
//_____________________________________________________________________
extern void scan_string(const char *str, yyscan_t scanner);

int express_parse(const char *s, void* parser){
	ParserContext context;
	memset(&context, 0, sizeof(context));

	yyscan_t scanner;
	yylex_init_extra(&context, &scanner);
    context.parser = parser;
	scan_string(s, scanner);
	int result = yyparse(scanner);
	yylex_destroy(scanner);
	return result;
}
