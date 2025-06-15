use crate::ast::*;
use crate::error::{Result, SlangError};
use crate::type_system::Type;
use std::collections::HashMap;

#[derive(Debug, Clone)]
pub struct TypeChecker {
    type_vars: HashMap<String, Type>,
    current_function: Option<Type>,
    type_definitions: HashMap<String, TypeDefinition>,
}

impl TypeChecker {
    pub fn new() -> Self {
        Self {
            type_vars: HashMap::new(),
            current_function: None,
            type_definitions: HashMap::new(),
        }
    }

    pub fn check_ast(&mut self, ast: &AST) -> Result<()> {
        // 型定義を収集
        for type_def in &ast.type_definitions {
            self.type_definitions.insert(type_def.name.clone(), type_def.clone());
        }

        // 関数をチェック
        for function in &ast.functions {
            self.check_function(function)?;
        }

        Ok(())
    }

    fn check_function(&mut self, function: &Function) -> Result<()> {
        // 関数の型を設定
        let function_type = Type::Function {
            params: function.parameters.iter().map(|p| p.type_annotation.clone()).collect(),
            return_type: Box::new(function.return_type.clone()),
            priority: Some(function.priority as u32),
        };
        self.current_function = Some(function_type);

        // パラメータの型を登録
        for param in &function.parameters {
            self.type_vars.insert(param.name.clone(), param.type_annotation.clone());
        }

        // 関数本体をチェック
        self.check_block(&function.body)?;

        // 関数の型をクリア
        self.current_function = None;

        Ok(())
    }

    fn check_block(&mut self, block: &Block) -> Result<()> {
        for statement in &block.statements {
            self.check_statement(statement)?;
        }
        Ok(())
    }

    fn check_statement(&mut self, statement: &Statement) -> Result<()> {
        match statement {
            Statement::Let(stmt) => {
                let value_type = self.check_expression(&stmt.value)?;
                if let Some(annotated_type) = &stmt.type_annotation {
                    if !value_type.is_compatible_with(annotated_type) {
                        return Err(SlangError::Type(format!(
                            "Type mismatch in let statement: expected {:?}, got {:?}",
                            annotated_type, value_type
                        )));
                    }
                }
                self.type_vars.insert(stmt.name.clone(), value_type);
            }
            Statement::Return(stmt) => {
                if let Some(value) = &stmt.value {
                    let value_type = self.check_expression(value)?;
                    if let Some(function_type) = &self.current_function {
                        if let Type::Function { return_type, .. } = function_type {
                            if !value_type.is_compatible_with(return_type) {
                                return Err(SlangError::Type(format!(
                                    "Return type mismatch: expected {:?}, got {:?}",
                                    return_type, value_type
                                )));
                            }
                        }
                    }
                }
            }
            Statement::If(stmt) => {
                let condition_type = self.check_expression(&stmt.condition)?;
                if !condition_type.is_compatible_with(&Type::Bool) {
                    return Err(SlangError::Type("If condition must be boolean".to_string()));
                }
                self.check_block(&stmt.then_block)?;
                if let Some(else_block) = &stmt.else_block {
                    self.check_block(else_block)?;
                }
            }
            Statement::While(stmt) => {
                let condition_type = self.check_expression(&stmt.condition)?;
                if !condition_type.is_compatible_with(&Type::Bool) {
                    return Err(SlangError::Type("While condition must be boolean".to_string()));
                }
                self.check_block(&stmt.body)?;
            }
            Statement::For(stmt) => {
                let iterator_type = self.check_expression(&stmt.iterator)?;
                if let Type::Array(element_type) = iterator_type {
                    self.type_vars.insert(stmt.variable.clone(), *element_type);
                    self.check_block(&stmt.body)?;
                } else {
                    return Err(SlangError::Type("For iterator must be an array".to_string()));
                }
            }
            Statement::Match(stmt) => {
                let value_type = self.check_expression(&stmt.expression)?;
                for arm in &stmt.arms {
                    self.check_pattern(&arm.pattern, &value_type)?;
                    self.check_block(&arm.body)?;
                }
            }
            Statement::Expression(expr) => {
                self.check_expression(expr)?;
            }
        }
        Ok(())
    }

    fn check_expression(&mut self, expression: &Expression) -> Result<Type> {
        match expression {
            Expression::Literal(lit) => Ok(self.get_literal_type(lit)),
            Expression::Identifier(name) => {
                self.type_vars.get(name)
                    .cloned()
                    .ok_or_else(|| SlangError::Type(format!("Undefined variable: {}", name)))
            }
            Expression::BinaryOp(op) => {
                let left_type = self.check_expression(&op.left)?;
                let right_type = self.check_expression(&op.right)?;
                self.check_binary_operation(&op.op, left_type, right_type)
            }
            Expression::UnaryOp(op) => {
                let expr_type = self.check_expression(&op.right)?;
                self.check_unary_operation(&op.op, expr_type)
            }
            Expression::Call(call) => {
                let function_type = self.check_expression(&Expression::Identifier(call.function.clone()))?;
                let arg_types: Vec<Type> = call.arguments
                    .iter()
                    .map(|arg| self.check_expression(arg))
                    .collect::<Result<Vec<_>>>()?;
                self.check_function_call(function_type, arg_types)
            }
            Expression::Assignment(assign) => {
                let value_type = self.check_expression(&assign.value)?;
                let target_type = self.type_vars.get(&assign.target)
                    .cloned()
                    .ok_or_else(|| SlangError::Type(format!("Undefined variable: {}", assign.target)))?;
                if !value_type.is_compatible_with(&target_type) {
                    return Err(SlangError::Type(format!(
                        "Assignment type mismatch: expected {:?}, got {:?}",
                        target_type, value_type
                    )));
                }
                Ok(Type::Unit)
            }
        }
    }

    fn get_literal_type(&self, literal: &Literal) -> Type {
        match literal {
            Literal::Int(_) => Type::Int,
            Literal::Float(_) => Type::Float,
            Literal::Bool(_) => Type::Bool,
            Literal::String(_) => Type::String,
            Literal::Null => Type::Unit,
        }
    }

    fn check_binary_operation(&mut self, op: &BinaryOperator, left: Type, right: Type) -> Result<Type> {
        match op {
            BinaryOperator::Add | BinaryOperator::Sub | BinaryOperator::Mul | BinaryOperator::Div => {
                if left.is_numeric() && right.is_numeric() {
                    if left == Type::Float || right == Type::Float {
                        Ok(Type::Float)
                    } else {
                        Ok(Type::Int)
                    }
                } else {
                    Err(SlangError::Type("Numeric operation requires numeric operands".to_string()))
                }
            }
            BinaryOperator::Mod => {
                if left.is_numeric() && right.is_numeric() {
                    Ok(Type::Int)
                } else {
                    Err(SlangError::Type("Modulo operation requires numeric operands".to_string()))
                }
            }
            BinaryOperator::Eq | BinaryOperator::Neq | BinaryOperator::Equals | BinaryOperator::NotEquals => {
                if left.is_compatible_with(&right) {
                    Ok(Type::Bool)
                } else {
                    Err(SlangError::Type("Cannot compare incompatible types".to_string()))
                }
            }
            BinaryOperator::Lt | BinaryOperator::Lte | BinaryOperator::Gt | BinaryOperator::Gte |
            BinaryOperator::LessThan | BinaryOperator::LessThanEquals | BinaryOperator::GreaterThan | BinaryOperator::GreaterThanEquals => {
                if left.is_numeric() && right.is_numeric() {
                    Ok(Type::Bool)
                } else {
                    Err(SlangError::Type("Comparison requires numeric operands".to_string()))
                }
            }
            BinaryOperator::And | BinaryOperator::Or => {
                if left == Type::Bool && right == Type::Bool {
                    Ok(Type::Bool)
                } else {
                    Err(SlangError::Type("Logical operation requires boolean operands".to_string()))
                }
            }
            BinaryOperator::Divide | BinaryOperator::Modulo => {
                if left.is_numeric() && right.is_numeric() {
                    if left == Type::Float || right == Type::Float {
                        Ok(Type::Float)
                    } else {
                        Ok(Type::Int)
                    }
                } else {
                    Err(SlangError::Type("Numeric operation requires numeric operands".to_string()))
                }
            }
        }
    }

    fn check_unary_operation(&mut self, op: &UnaryOperator, expr: Type) -> Result<Type> {
        match op {
            UnaryOperator::Neg | UnaryOperator::Negate => {
                if expr.is_numeric() {
                    Ok(expr)
                } else {
                    Err(SlangError::Type("Negation requires numeric operand".to_string()))
                }
            }
            UnaryOperator::Not => {
                if expr == Type::Bool {
                    Ok(Type::Bool)
                } else {
                    Err(SlangError::Type("Logical not requires boolean operand".to_string()))
                }
            }
        }
    }

    fn check_function_call(&mut self, function_type: Type, arg_types: Vec<Type>) -> Result<Type> {
        if let Type::Function { params, return_type, .. } = function_type {
            if params.len() != arg_types.len() {
                return Err(SlangError::Type("Wrong number of arguments".to_string()));
            }
            for (param_type, arg_type) in params.iter().zip(arg_types.iter()) {
                if !arg_type.is_compatible_with(param_type) {
                    return Err(SlangError::Type(format!(
                        "Argument type mismatch: expected {:?}, got {:?}",
                        param_type, arg_type
                    )));
                }
            }
            Ok(*return_type)
        } else {
            Err(SlangError::Type("Not a function".to_string()))
        }
    }

    fn check_pattern(&mut self, pattern: &Pattern, value_type: &Type) -> Result<()> {
        match pattern {
            Pattern::Identifier(name) => {
                self.type_vars.insert(name.clone(), value_type.clone());
                Ok(())
            }
            Pattern::Literal(lit) => {
                let lit_type = self.get_literal_type(lit);
                if !lit_type.is_compatible_with(value_type) {
                    return Err(SlangError::Type(format!(
                        "Pattern type mismatch: expected {:?}, got {:?}",
                        value_type, lit_type
                    )));
                }
                Ok(())
            }
            Pattern::Wildcard => Ok(()),
            Pattern::Tuple(patterns) => {
                if let Type::Tuple(types) = value_type {
                    if patterns.len() != types.len() {
                        return Err(SlangError::Type(format!(
                            "Tuple pattern length mismatch: expected {}, got {}",
                            types.len(), patterns.len()
                        )));
                    }
                    for (pattern, type_) in patterns.iter().zip(types.iter()) {
                        self.check_pattern(pattern, type_)?;
                    }
                    Ok(())
                } else {
                    Err(SlangError::Type("Expected tuple type".to_string()))
                }
            }
            Pattern::Struct { name, fields } => {
                if let Some(type_def) = self.type_definitions.get(name) {
                    let mut field_types = type_def.fields.clone();
                    for field in fields {
                        if let Some(field_type) = field_types.iter()
                            .find(|f| f.name == field.name)
                            .map(|f| f.type_.clone())
                        {
                            self.check_pattern(&field.pattern, &field_type)?;
                        } else {
                            return Err(SlangError::Type(format!(
                                "Unknown field: {} in type {}",
                                field.name, name
                            )));
                        }
                    }
                    Ok(())
                } else {
                    Err(SlangError::Type(format!("Unknown type: {}", name)))
                }
            }
        }
    }

    fn check_struct_pattern(&mut self, name: &str, fields: &[FieldPattern]) -> Result<()> {
        if let Some(type_def) = self.type_definitions.get(name) {
            let field_types: HashMap<_, _> = type_def.fields.iter()
                .map(|field| (field.name.clone(), field.type_annotation.clone()))
                .collect();

            for field in fields {
                if let Some(field_type) = field_types.get(&field.name) {
                    self.check_pattern(&field.pattern, field_type.clone())?;
                } else {
                    return Err(SlangError::Type(format!("Field '{}' not found in struct '{}'", field.name, name)));
                }
            }
            Ok(())
        } else {
            Err(SlangError::Type(format!("Type '{}' not found", name)))
        }
    }
} 