// Copyright 2023-2024 VulcanDB

#ifndef EXPRESS_DEFS_H
#define EXPRESS_DEFS_H

#ifdef __cplusplus
extern "C" {

#endif

#include <stdbool.h>
#include <stdlib.h>

typedef enum {
  T_REAL,
  T_NUMBER,
  T_INTEGER,
  T_BOOLEAN,
  T_BINARY,
  T_LOGICAL,
  T_STRING,
  T_ENTITY,
  T_ENUM,
  T_SELECT,
  T_REFERENCE,  // TYPE IfcPositiveLengthMeasure = IfcLengthMeasure;
  T_UNKNOWN
} TypeEnum;

#define MAX_TYPE_NUM 500
#define MAX_ENTITY_NUM 1000
#define MAX_SUBTYPE_NUM 50
#define MAX_ATTR_NUM 50

typedef struct ExpDesc ExpDesc;
typedef struct Type Type;
typedef struct Entity Entity;
typedef struct Schema Schema;

typedef struct ExpDesc {
  Schema* schema;
  int index_in_schema;
  char* name;
  TypeEnum type;
} ExpDesc;

// 用于在语法分析器和解析器之间传递type参数的结构体
typedef struct TypeArgs {
  TypeEnum type;
  char* type_name;
  char* ref_type_name;
  bool is_list;
  size_t lb;
  size_t ub;
  bool fixed_length;
  char** select_list;
  char** enum_list;
  int enum_num;
  int select_num;
} TypeArgs, *TypeArgsPtr;

typedef struct EntityArgs {
  char* entity_name;
  bool is_abstract;       // 标记实体是否为抽象实体
  char** attr_names;      // 用于存储属性的名称
  TypeArgs** attr_types;  // 用于存储属性的类型
  bool* is_optional;      // 用于标记attr是否为可选属性
  char** subtype_names;   // 子类名称
  int subtype_num;
  char* supertype_name;  // 父类名称
  int attr_num;
} EntityArgs, *EntityArgsPtr;

typedef struct Type {
  ExpDesc desc;

  ExpDesc* ref_type;      // 指向派生类型的指针
  bool is_list;           // 标记当前类型是否为list类型
  size_t lb;              // list类型的下界
  size_t ub;              // list类型的上界
  bool fixed_length;      // 标记当前类型是否为固定长度
  char** enum_list;       // 用于存储枚举类型的枚举值
  ExpDesc** select_list;  // 用于存储select类型的可选类型
  int select_num;         // select类型的可选类型的数量
  int enum_num;           // 枚举类型的枚举值的数量
} Type;

typedef struct Attr {
  char* attr_name;
  ExpDesc* attr_type;
  bool is_list;
} Attr;

typedef struct Entity {
  ExpDesc desc;

  bool is_abstract;
  char* attr_name[MAX_ATTR_NUM];     // 属性的名称
  ExpDesc* attr_type[MAX_ATTR_NUM];  // 属性的类型
  bool is_optional[MAX_ATTR_NUM];    // 标记属性是否为可选属性
  size_t attr_num;                   // 属性的数量

  Entity* subtypes[MAX_SUBTYPE_NUM];  // 子类
  size_t subtype_num;                 // 子类数量
  Entity* supertype;                  // 父类
} Entity;

typedef struct Schema {
  char* schema_version;
  Type* types[MAX_TYPE_NUM];
  Entity* entities[MAX_ENTITY_NUM];
  int entity_num;
  int type_num;
} Schema;

Schema* new_schema();
Type* new_type();
Entity* new_entity();
TypeArgs* new_type_name_args(char* const type_name, TypeEnum type);

void add_type_to_schema(Schema* schema, const char* entity_name);
void add_entity_to_schema(Schema* schema, const char* entity_name);

void init_type(void* parser, TypeArgs* args);
void init_entity(void* parser, EntityArgs* args);

void assign_type_args(Type* type, TypeArgs* args);
void assign_entity_args(Entity* entity, EntityArgs* args);

char* serialize_type_tree(ExpDesc* desc);
Entity* find_entity_by_name(Schema* schema, const char* entity_name);
Type* find_type_by_name(Schema* schema, const char* type_name);

void destroy_type_name_args(TypeArgs* args);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // EXPRESS_DEFS_H
