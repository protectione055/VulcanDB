// Copyright 2023-2024 VulcanDB

#include "express_defs.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

const char* TYPE_NAMES[] = {"REAL",   "NUMBER",  "INTEGER", "BOOLEAN",
                            "BINARY", "LOGICAL", "STRING",  "ENTITY",
                            "ENUM",   "SELECT",  "UNKNOWN"};

char* resize_buffer(char* buffer, size_t* buf_size, size_t new_size) {
  if (new_size > *buf_size) {
    *buf_size = new_size;
    buffer = (char*)realloc(buffer, *buf_size);
  }
  return buffer;
}

Schema* new_schema() {
  Schema* schema = (Schema*)malloc(sizeof(Schema));
  schema->schema_version = NULL;
  schema->entity_num = 0;
  schema->type_num = 0;
  return schema;
}

Type* new_type() {
  Type* type = (Type*)malloc(sizeof(Type));
  ExpDesc tmp = {
      .schema = NULL, .index_in_schema = 0, .name = NULL, .type = T_UNKNOWN};
  memcpy(&(type->desc), &tmp, sizeof(ExpDesc));
  type->ref_type = NULL;
  type->is_list = false;
  type->lb = 0;
  type->ub = UINT32_MAX;
  type->fixed_length = false;
  type->enum_list = NULL;
  type->select_list = NULL;
  type->select_num = 0;
  type->enum_num = 0;
  return type;
}

Entity* new_entity() {
  Entity* entity = (Entity*)malloc(sizeof(Entity));
  ExpDesc tmp = {
      .schema = NULL, .index_in_schema = 0, .name = NULL, .type = T_DERIVED};
  memcpy(&(entity->desc), &tmp, sizeof(ExpDesc));
  entity->is_abstract = false;
  entity->subtypes;
  entity->supertype = NULL;
  return entity;
}

TypeArgs* new_type_name_args(char* const type_name, TypeEnum type) {
  TypeArgs* args = malloc(sizeof(TypeArgs));
  args->type_name = type_name;
  args->type = type;
  args->enum_list = NULL;
  args->select_list = NULL;
  args->enum_num = 0;
  args->select_num = 0;
  args->is_list = false;
  args->lb = 0;
  args->ub = UINT32_MAX;
  args->fixed_length = false;
  args->ref_type_name = NULL;
  return args;
}

void add_type_to_schema(Schema* schema, const char* entity_name) {
  int index = schema->type_num;
  assert(index < MAX_ENTITY_NUM);
  Type* type = new_type();
  type->desc.name = strdup(entity_name);
  type->desc.index_in_schema = index;
  type->desc.schema = schema;
  schema->types[index] = type;
  schema->type_num++;
}

void add_entity_to_schema(Schema* schema, const char* entity_name) {
  int index = schema->entity_num;
  assert(index < MAX_ENTITY_NUM);
  Entity* entity = new_entity();
  entity->desc.name = strdup(entity_name);
  entity->desc.index_in_schema = index;
  entity->desc.schema = schema;
  entity->desc.type = T_ENTITY;
  schema->entities[index] = entity;
  schema->entity_num++;
}

Type* find_type_by_name(Schema* schema, const char* type_name) {
  for (int i = 0; i < schema->type_num; i++) {
    if (strcmp(schema->types[i]->desc.name, type_name) == 0) {
      return schema->types[i];
    }
  }

  return NULL;
}

Entity* find_entity_by_name(Schema* schema, const char* entity_name) {
  for (int i = 0; i < schema->entity_num; i++) {
    if (strcmp(schema->entities[i]->desc.name, entity_name) == 0) {
      return schema->entities[i];
    }
  }

  return NULL;
}

void assign_type_args(Type* type_decl, TypeArgs* args) {
  Schema* schema = type_decl->desc.schema;
  char* type_name = args->type_name;
  type_decl->desc.type = args->type;
  type_decl->fixed_length = args->fixed_length;
  type_decl->is_list = args->is_list;
  type_decl->lb = args->lb;
  type_decl->ub = args->ub;
  type_decl->select_num = args->select_num;
  type_decl->enum_num = args->enum_num;

  switch (args->type) {
    case T_DERIVED: {
      // 从schema中查找ref_type_name对应的类型声明，并将其赋值给type_decl->ref_type
      char* ref_type_name = args->ref_type_name;
      Type* ref_type = find_type_by_name(schema, ref_type_name);
      if (ref_type != NULL) {
        Type* ref_type_decl = ref_type;
        type_decl->ref_type = (ExpDesc*)ref_type_decl;
      } else {
        Entity* ref_entity_decl = find_entity_by_name(schema, ref_type_name);
        if (ref_entity_decl != NULL) {
          type_decl->ref_type = (ExpDesc*)ref_entity_decl;
        } else {
          printf("Invalid ref_type name: %s", ref_type_name);
          exit(1);
        }
      }
    } break;

    case T_ENUM: {
      type_decl->enum_list = (char**)malloc(sizeof(char*) * args->enum_num);
      for (int i = 0; i < args->enum_num; i++) {
        type_decl->enum_list[i] = strdup(args->enum_list[i]);
      }
    } break;

    case T_SELECT: {
      type_decl->select_list =
          (ExpDesc**)malloc(sizeof(ExpDesc*) * args->select_num);
      ExpDesc* select_type_decl = NULL;
      for (int i = 0; i < args->select_num; i++) {
        Type* select_it = find_type_by_name(schema, args->select_list[i]);
        if (select_it == NULL) {
          Entity* select_it = find_entity_by_name(schema, args->select_list[i]);
          if (select_it == NULL) {
            printf("Invalid select type name: %s", args->select_list[i]);
            exit(1);
          }
          select_type_decl = (ExpDesc*)select_it;
        } else {
          select_type_decl = (ExpDesc*)select_it;
        }
        type_decl->select_list[i] = select_type_decl;
      }

    } break;

    default:
      break;
  }
}

void assign_entity_args(Entity* entity_decl, EntityArgs* args) {
  Schema* schema = entity_decl->desc.schema;
  entity_decl->is_abstract = args->is_abstract;
  entity_decl->subtype_num = args->subtype_num;
  entity_decl->attr_num = args->attr_num;
  assert(entity_decl->attr_num <= MAX_ENTITY_NUM);

  // 处理子类定义
  assert(entity_decl->subtype_num <= MAX_SUBTYPE_NUM);
  for (int i = 0; i < args->subtype_num; i++) {
    Entity* subtype_decl = find_entity_by_name(schema, args->subtype_names[i]);
    entity_decl->subtypes[i] = subtype_decl;
  }

  // 处理父类定义
  if (args->supertype_name != NULL) {
    Entity* supertype_decl = find_entity_by_name(schema, args->supertype_name);
    assert(supertype_decl != NULL);
    entity_decl->supertype = supertype_decl;
  }

  // 处理属性定义
  for (int i = args->attr_num - 1; i >= 0; i--) {
    TypeArgs* attr_type_args = args->attr_types[i];

    Type* attr_type_decl = NULL;
    if (attr_type_args->ref_type_name == NULL) {
      // 无派生的基本类型，需要创建匿名类型描述结构体Type并使用attr_type_args对其进行赋值
      attr_type_decl = new_type();
      assign_type_args((Type*)attr_type_decl, attr_type_args);
    } else {
      // 已经声明的派生类型，直接从schema中找到类型定义
      attr_type_decl = find_type_by_name(schema, attr_type_args->ref_type_name);
      if (attr_type_decl == NULL) {
        attr_type_decl = new_type();
        attr_type_decl->desc.type = T_REFERENCE;
        attr_type_decl->ref_type = (ExpDesc*)find_entity_by_name(
            schema, attr_type_args->ref_type_name);
        if (attr_type_decl == NULL) {
          printf("Invalid attr type name: %s", attr_type_args->ref_type_name);
          exit(1);
        }
      }
    }
    attr_type_decl->is_list = attr_type_args->is_list;

    // TODO(zzm):
    // 由于是自下而上解析器，列表中的元素按照规约先后顺序会形成倒序，这个问题应该在语法解析器解决，但暂时不管
    size_t insert_idx = args->attr_num - i - 1;
    entity_decl->attr_name[insert_idx] = strdup(args->attr_names[i]);
    entity_decl->is_optional[insert_idx] = args->is_optional[i];
    entity_decl->attr_type[insert_idx] = attr_type_decl;
  }
}

char* serialize_type_tree(Type* type) {
  char* buffer = (char*)malloc(BUFFER_SIZE);
  memset(buffer, 0, BUFFER_SIZE);
  size_t buf_size = BUFFER_SIZE;

  snprintf(buffer + strlen(buffer), buf_size - strlen(buffer), "%s", "[");
  snprintf(buffer + strlen(buffer), buf_size - strlen(buffer), "%s", " ");
  if (type->is_list) {
    snprintf(buffer + strlen(buffer), buf_size - strlen(buffer), "%s",
             "LIST OF ");
  }

  if (type->desc.type == T_REFERENCE) {
    Entity* entity = (Entity*)type->ref_type;
    snprintf(buffer + strlen(buffer), buf_size - strlen(buffer), "ENTITY %s]",
             entity->desc.name);
    return buffer;
  }

  if (type->desc.name != NULL) {
    snprintf(buffer + strlen(buffer), buf_size - strlen(buffer), "%s",
             type->desc.name);
  }

  switch (type->desc.type) {
    case T_DERIVED: {
      assert(type->ref_type != NULL);
      char* child_str = serialize_type_tree((Type*)type->ref_type);
      buffer = resize_buffer(buffer, &buf_size,
                             strlen(buffer) + strlen(child_str) + 2);
      snprintf(buffer + strlen(buffer), buf_size - strlen(buffer), "%s",
               child_str);

      free(child_str);
      return buffer;
    } break;

    case T_SELECT: {
      assert(type->select_num > 0);
      snprintf(buffer + strlen(buffer), buf_size - strlen(buffer), "%s",
               "SELECT OF (");

      for (int i = 0; i < type->select_num; i++) {
        // 打印select的可选类型
        ExpDesc* child = type->select_list[i]->type == T_DERIVED
                             ? ((Type*)(type->select_list[i]))->ref_type
                             : type->select_list[i];
        if (child->type == T_SELECT || child->type == T_ENUM ||
            child->type != T_ENTITY && ((Type*)child)->is_list) {
          char* child_str = serialize_type_tree((Type*)child);
          buffer = resize_buffer(buffer, &buf_size,
                                 strlen(buffer) + strlen(child_str) + 2);
          snprintf(buffer + strlen(buffer), buf_size - strlen(buffer), "%s",
                   child_str);
          free(child_str);
        } else if (child->type == T_ENTITY) {
          buffer = resize_buffer(buffer, &buf_size,
                                 strlen(buffer) + strlen(child->name) + 20);
          snprintf(buffer + strlen(buffer), buf_size - strlen(buffer),
                   "[ENTITY %s]", child->name);
        } else {
          buffer = resize_buffer(buffer, &buf_size,
                                 strlen(buffer) + strlen(child->name) +
                                     strlen(TYPE_NAMES[child->type]) + 5);
          snprintf(buffer + strlen(buffer), buf_size - strlen(buffer),
                   "[%s %s]", child->name, TYPE_NAMES[child->type]);
        }

        // 打印分隔符
        if (i == type->select_num - 1) {
          snprintf(buffer + strlen(buffer), buf_size - strlen(buffer), "%s",
                   ")");
        } else {
          snprintf(buffer + strlen(buffer), buf_size - strlen(buffer), "%s",
                   "|");
        }
      }
    } break;

    case T_ENUM: {
      assert(type->enum_num > 0);
      snprintf(buffer + strlen(buffer), buf_size - strlen(buffer),
               "ENUMERATION OF <");

      for (int i = 0; i < type->enum_num; i++) {
        // make sure the buffer is large enough
        size_t cat_size = strlen(type->enum_list[i]) + 1;
        if (strlen(buffer) + cat_size + 1 > buf_size) {
          size_t new_buf_size =
              (buf_size >= UINT32_MAX >> 1 ? UINT32_MAX : buf_size * 2);
          buffer = (char*)realloc(buffer, new_buf_size);
        }
        snprintf(buffer + strlen(buffer), buf_size - strlen(buffer), "%s",
                 type->enum_list[i]);
        if (i == type->enum_num - 1) {
          snprintf(buffer + strlen(buffer), buf_size - strlen(buffer), "%s",
                   ">");
        } else {
          snprintf(buffer + strlen(buffer), buf_size - strlen(buffer), "%s",
                   ",");
        }
      }
    } break;

    default:
      snprintf(buffer + strlen(buffer), buf_size - strlen(buffer), "%s",
               TYPE_NAMES[type->desc.type]);
      break;
  }
  snprintf(buffer + strlen(buffer), buf_size - strlen(buffer), "%s", "]");
  return buffer;
}

void destroy_type_name_args(TypeArgs* args) {
  if (args->type_name != NULL) {
    free(args->type_name);
  }
  if (args->ref_type_name != NULL) {
    free(args->ref_type_name);
  }
  if (args->enum_list != NULL) {
    for (int i = 0; i < args->enum_num; i++) {
      free(args->enum_list[i]);
    }
    free(args->enum_list);
  }
  if (args->select_list != NULL) {
    for (int i = 0; i < args->select_num; i++) {
      free(args->select_list[i]);
    }
    free(args->select_list);
  }
  free(args);
}

Entity* find_entity_by_id(Schema* schema, int id) {
  if (id < 0 || id >= schema->entity_num) {
    return NULL;
  }
  return schema->entities[id];
}

int get_entity_id(const Entity* entity) {
  if (entity == NULL) return -1;
  return entity->desc.index_in_schema;
}