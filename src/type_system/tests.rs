#[cfg(test)]
mod tests {
    use super::*;
    use crate::ast::*;
    use crate::error::Result;

    #[test]
    fn test_basic_types() -> Result<()> {
        let mut checker = TypeChecker::new();
        let mut inference = TypeInference::new();

        // 基本型のテスト
        let ast = AST {
            functions: vec![
                Function {
                    name: "test_basic_types".to_string(),
                    parameters: vec![
                        Parameter {
                            name: "x".to_string(),
                            type_annotation: Type::Int,
                        },
                        Parameter {
                            name: "y".to_string(),
                            type_annotation: Type::Float,
                        },
                    ],
                    return_type: Type::Bool,
                    priority: 0,
                    body: Block {
                        statements: vec![
                            Statement::Let(LetStatement {
                                name: "a".to_string(),
                                type_annotation: Some(Type::Int),
                                value: Box::new(Expression::Literal(Literal::Int(42))),
                            }),
                            Statement::Let(LetStatement {
                                name: "b".to_string(),
                                type_annotation: Some(Type::Float),
                                value: Box::new(Expression::Literal(Literal::Float(3.14))),
                            }),
                            Statement::Let(LetStatement {
                                name: "c".to_string(),
                                type_annotation: Some(Type::Bool),
                                value: Box::new(Expression::Literal(Literal::Bool(true))),
                            }),
                            Statement::Return(ReturnStatement {
                                value: Some(Box::new(Expression::Literal(Literal::Bool(true)))),
                            }),
                        ],
                    },
                },
            ],
            type_definitions: vec![],
        };

        checker.check_ast(&ast)?;
        inference.infer_types(&ast)?;
        Ok(())
    }

    #[test]
    fn test_composite_types() -> Result<()> {
        let mut checker = TypeChecker::new();
        let mut inference = TypeInference::new();

        // 複合型のテスト
        let ast = AST {
            functions: vec![
                Function {
                    name: "test_composite_types".to_string(),
                    parameters: vec![],
                    return_type: Type::Unit,
                    priority: 0,
                    body: Block {
                        statements: vec![
                            Statement::Let(LetStatement {
                                name: "arr".to_string(),
                                type_annotation: Some(Type::Array(Box::new(Type::Int))),
                                value: Box::new(Expression::Literal(Literal::Int(42))),
                            }),
                            Statement::Let(LetStatement {
                                name: "tuple".to_string(),
                                type_annotation: Some(Type::Tuple(vec![Type::Int, Type::String])),
                                value: Box::new(Expression::Literal(Literal::Int(42))),
                            }),
                        ],
                    },
                },
            ],
            type_definitions: vec![
                TypeDefinition {
                    name: "Person".to_string(),
                    fields: vec![
                        Field {
                            name: "name".to_string(),
                            type_annotation: Type::String,
                        },
                        Field {
                            name: "age".to_string(),
                            type_annotation: Type::Int,
                        },
                    ],
                },
            ],
        };

        checker.check_ast(&ast)?;
        inference.infer_types(&ast)?;
        Ok(())
    }

    #[test]
    fn test_mathematical_types() -> Result<()> {
        let mut checker = TypeChecker::new();
        let mut inference = TypeInference::new();

        // 数学的型のテスト
        let ast = AST {
            functions: vec![
                Function {
                    name: "test_mathematical_types".to_string(),
                    parameters: vec![],
                    return_type: Type::Unit,
                    priority: 0,
                    body: Block {
                        statements: vec![
                            Statement::Let(LetStatement {
                                name: "vec".to_string(),
                                type_annotation: Some(Type::Vector(3, Box::new(Type::Float))),
                                value: Box::new(Expression::Literal(Literal::Float(1.0))),
                            }),
                            Statement::Let(LetStatement {
                                name: "mat".to_string(),
                                type_annotation: Some(Type::Matrix(2, 2, Box::new(Type::Float))),
                                value: Box::new(Expression::Literal(Literal::Float(1.0))),
                            }),
                            Statement::Let(LetStatement {
                                name: "tensor".to_string(),
                                type_annotation: Some(Type::Tensor(vec![2, 3, 4], Box::new(Type::Float))),
                                value: Box::new(Expression::Literal(Literal::Float(1.0))),
                            }),
                        ],
                    },
                },
            ],
            type_definitions: vec![],
        };

        checker.check_ast(&ast)?;
        inference.infer_types(&ast)?;
        Ok(())
    }

    #[test]
    fn test_function_types() -> Result<()> {
        let mut checker = TypeChecker::new();
        let mut inference = TypeInference::new();

        // 関数型のテスト
        let ast = AST {
            functions: vec![
                Function {
                    name: "test_function_types".to_string(),
                    parameters: vec![
                        Parameter {
                            name: "f".to_string(),
                            type_annotation: Type::Function {
                                params: vec![Type::Int],
                                return_type: Box::new(Type::Int),
                                priority: Some(1),
                            },
                        },
                    ],
                    return_type: Type::Int,
                    priority: 2,
                    body: Block {
                        statements: vec![
                            Statement::Return(ReturnStatement {
                                value: Some(Box::new(Expression::Call(CallExpression {
                                    function: "f".to_string(),
                                    arguments: vec![Box::new(Expression::Literal(Literal::Int(42)))],
                                }))),
                            }),
                        ],
                    },
                },
            ],
            type_definitions: vec![],
        };

        checker.check_ast(&ast)?;
        inference.infer_types(&ast)?;
        Ok(())
    }

    #[test]
    fn test_type_casting() -> Result<()> {
        let type_cast = TypeCast::new();

        // 数値型の変換
        assert!(type_cast.is_cast_allowed(&Type::Int, &Type::Float));
        assert!(type_cast.is_cast_allowed(&Type::Float, &Type::Int));

        // 文字列への変換
        assert!(type_cast.is_cast_allowed(&Type::Int, &Type::String));
        assert!(type_cast.is_cast_allowed(&Type::Float, &Type::String));
        assert!(type_cast.is_cast_allowed(&Type::Bool, &Type::String));

        // 文字列からの変換
        assert!(type_cast.is_cast_allowed(&Type::String, &Type::Int));
        assert!(type_cast.is_cast_allowed(&Type::String, &Type::Float));
        assert!(type_cast.is_cast_allowed(&Type::String, &Type::Bool));

        // 変換のコスト
        assert_eq!(type_cast.get_cast_cost(&Type::Int, &Type::Float), Some(1));
        assert_eq!(type_cast.get_cast_cost(&Type::String, &Type::Int), Some(3));

        // リテラルの変換
        let int_lit = Literal::Int(42);
        let float_lit = type_cast.cast_literal(&int_lit, &Type::Float)?;
        assert!(matches!(float_lit, Literal::Float(_)));

        let string_lit = type_cast.cast_literal(&int_lit, &Type::String)?;
        assert!(matches!(string_lit, Literal::String(_)));

        Ok(())
    }

    #[test]
    fn test_type_inference() -> Result<()> {
        let mut inference = TypeInference::new();

        // 型推論のテスト
        let ast = AST {
            functions: vec![
                Function {
                    name: "test_type_inference".to_string(),
                    parameters: vec![],
                    return_type: Type::Int,
                    priority: 0,
                    body: Block {
                        statements: vec![
                            Statement::Let(LetStatement {
                                name: "x".to_string(),
                                type_annotation: None,
                                value: Box::new(Expression::Literal(Literal::Int(42))),
                            }),
                            Statement::Let(LetStatement {
                                name: "y".to_string(),
                                type_annotation: None,
                                value: Box::new(Expression::BinaryOp(Box::new(BinaryOpExpression {
                                    left: Box::new(Expression::Identifier("x".to_string())),
                                    op: BinaryOperator::Add,
                                    right: Box::new(Expression::Literal(Literal::Int(1))),
                                }))),
                            }),
                            Statement::Return(ReturnStatement {
                                value: Some(Box::new(Expression::Identifier("y".to_string()))),
                            }),
                        ],
                    },
                },
            ],
            type_definitions: vec![],
        };

        inference.infer_types(&ast)?;
        Ok(())
    }

    #[test]
    fn test_type_checker() -> Result<()> {
        let mut checker = TypeChecker::new();

        // 型チェッカーのテスト
        let ast = AST {
            functions: vec![
                Function {
                    name: "test_type_checker".to_string(),
                    parameters: vec![
                        Parameter {
                            name: "x".to_string(),
                            type_annotation: Type::Int,
                        },
                    ],
                    return_type: Type::Int,
                    priority: 0,
                    body: Block {
                        statements: vec![
                            Statement::Let(LetStatement {
                                name: "y".to_string(),
                                type_annotation: Some(Type::Int),
                                value: Box::new(Expression::BinaryOp(Box::new(BinaryOpExpression {
                                    left: Box::new(Expression::Identifier("x".to_string())),
                                    op: BinaryOperator::Add,
                                    right: Box::new(Expression::Literal(Literal::Int(1))),
                                }))),
                            }),
                            Statement::Return(ReturnStatement {
                                value: Some(Box::new(Expression::Identifier("y".to_string()))),
                            }),
                        ],
                    },
                },
            ],
            type_definitions: vec![],
        };

        checker.check_ast(&ast)?;
        Ok(())
    }

    #[test]
    fn test_priority_ownership() -> Result<()> {
        let mut checker = TypeChecker::new();
        let mut inference = TypeInference::new();

        // 優先所有格システムのテスト
        let ast = AST {
            functions: vec![
                Function {
                    name: "high_priority_function".to_string(),
                    parameters: vec![
                        Parameter {
                            name: "x".to_string(),
                            type_annotation: Type::Int,
                        },
                    ],
                    return_type: Type::Int,
                    priority: 2,
                    body: Block {
                        statements: vec![
                            Statement::Return(ReturnStatement {
                                value: Some(Box::new(Expression::Identifier("x".to_string()))),
                            }),
                        ],
                    },
                },
                Function {
                    name: "low_priority_function".to_string(),
                    parameters: vec![
                        Parameter {
                            name: "x".to_string(),
                            type_annotation: Type::Int,
                        },
                    ],
                    return_type: Type::Int,
                    priority: 0,
                    body: Block {
                        statements: vec![
                            Statement::Return(ReturnStatement {
                                value: Some(Box::new(Expression::Identifier("x".to_string()))),
                            }),
                        ],
                    },
                },
            ],
            type_definitions: vec![
                TypeDefinition {
                    name: "PriorityData".to_string(),
                    fields: vec![
                        Field {
                            name: "value".to_string(),
                            type_annotation: Type::Int,
                        },
                        Field {
                            name: "priority".to_string(),
                            type_annotation: Type::Int,
                        },
                    ],
                },
            ],
        };

        checker.check_ast(&ast)?;
        inference.infer_types(&ast)?;
        Ok(())
    }
} 