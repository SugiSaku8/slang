#ifndef SLANG_TYPE_SYSTEM_H
#define SLANG_TYPE_SYSTEM_H

#include "common.h"
#include "ast.h"

// 型の種類
typedef enum {
    TYPE_VOID,
    TYPE_INTEGER,
    TYPE_FLOAT,
    TYPE_BOOLEAN,
    TYPE_STRING,
    TYPE_ARRAY,
    TYPE_STRUCT,
    TYPE_FUNCTION,
    TYPE_TRAIT,
    TYPE_GENERIC,
    TYPE_ERROR
} TypeKind;

// 型の構造体
typedef struct Type {
    TypeKind kind;
    size_t size;
    bool is_mutable;
    union {
        // 配列型
        struct {
            struct Type* element_type;
            size_t length;
        } array;
        
        // 構造体型
        struct {
            char* name;
            Vector* fields;
        } structure;
        
        // 関数型
        struct {
            Vector* parameter_types;
            struct Type* return_type;
        } function;
        
        // トレイト型
        struct {
            char* name;
            Vector* methods;
        } trait;
        
        // ジェネリック型
        struct {
            char* name;
            Vector* type_parameters;
        } generic;
    } as;
} Type;

// 型システムの関数
Type* type_create(TypeKind kind);
void type_destroy(Type* type);
bool type_equals(const Type* a, const Type* b);
Type* type_infer(ASTNode* node);
SlangError type_check(ASTNode* node);
char* type_to_string(const Type* type);

#endif // SLANG_TYPE_SYSTEM_H 