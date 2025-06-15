use crate::ast::*;
use crate::error::{Result, SlangError};
use crate::type_system::Type;
use std::collections::HashMap;

#[derive(Debug, Clone)]
pub struct TypeInference {
    type_vars: HashMap<String, Type>,
    constraints: Vec<TypeConstraint>,
    type_definitions: HashMap<String, TypeDefinition>,
}

#[derive(Debug, Clone)]
pub struct TypeConstraint {
    pub left: Type,
    pub right: Type,
}

impl TypeInference {
    pub fn new() -> Self {
        Self {
            type_vars: HashMap::new(),
            constraints: Vec::new(),
            type_definitions: HashMap::new(),
        }
    }

    pub fn infer_types(&mut self, ast: &AST) -> Result<()> {
        for function in &ast.functions {
            self.infer_function(function)?;
        }
        for type_def in &ast.type_definitions {
            self.infer_type_definition(type_def)?;
        }
        self.solve_constraints()
    }

    fn infer_function(&mut self, function: &Function) -> Result<()> {
        // 関数の型を推論
        let param_types: Vec<Type> = function.parameters
            .iter()
            .map(|p| p.type_annotation.clone())
            .collect();

        let return_type = function.return_type.clone();
        let function_type = Type::Function {
            params: param_types,
            return_type: Box::new(return_type),
            priority: Some(function.priority as u32),
        };

        // 関数本体の型を推論
        self.infer_block(&function.body)?;

        Ok(())
    }

    fn infer_type_definition(&mut self, type_def: &TypeDefinition) -> Result<()> {
        // 型定義の型を推論
        let type_name = type_def.name.clone();
        let type_fields: Vec<Type> = type_def.fields
            .iter()
            .map(|f| f.type_annotation.clone())
            .collect();

        let struct_type = Type::Named(type_name);
        self.type_vars.insert(type_def.name.clone(), struct_type);
        self.type_definitions.insert(type_def.name.clone(), type_def.clone());

        Ok(())
    }

    fn infer_block(&mut self, block: &Block) -> Result<()> {
        for statement in &block.statements {
            self.infer_statement(statement)?;
        }
        Ok(())
    }

    fn infer_statement(&mut self, statement: &Statement) -> Result<()> {
        match statement {
            Statement::Let(stmt) => {
                let value_type = self.infer_expression(&stmt.value)?;
                if let Some(annotated_type) = &stmt.type_annotation {
                    self.add_constraint(value_type.clone(), annotated_type.clone())?;
                }
                self.type_vars.insert(stmt.name.clone(), value_type);
            }
            Statement::Return(stmt) => {
                if let Some(value) = &stmt.value {
                    let value_type = self.infer_expression(value)?;
                    // 戻り値の型を関数の戻り値の型と一致させる
                    if let Some(return_type) = self.get_current_return_type() {
                        self.add_constraint(value_type, return_type)?;
                    }
                }
            }
            Statement::If(stmt) => {
                let condition_type = self.infer_expression(&stmt.condition)?;
                self.add_constraint(condition_type, Type::Bool)?;
                self.infer_block(&stmt.then_block)?;
                if let Some(else_block) = &stmt.else_block {
                    self.infer_block(else_block)?;
                }
            }
            Statement::While(stmt) => {
                let condition_type = self.infer_expression(&stmt.condition)?;
                self.add_constraint(condition_type, Type::Bool)?;
                self.infer_block(&stmt.body)?;
            }
            Statement::For(stmt) => {
                let iterator_type = self.infer_expression(&stmt.iterator)?;
                // イテレータの型は配列またはイテレータ型である必要がある
                if let Type::Array(element_type) = iterator_type {
                    self.type_vars.insert(stmt.variable.clone(), *element_type);
                } else {
                    return Err(SlangError::Type("Iterator must be an array".to_string()));
                }
                self.infer_block(&stmt.body)?;
            }
            Statement::Match(stmt) => {
                let value_type = self.infer_expression(&stmt.expression)?;
                for arm in &stmt.arms {
                    self.infer_pattern(&arm.pattern, &value_type)?;
                    self.infer_block(&arm.body)?;
                }
            }
            Statement::Expression(expr) => {
                self.infer_expression(expr)?;
            }
        }
        Ok(())
    }

    fn infer_expression(&mut self, expression: &Expression) -> Result<Type> {
        match expression {
            Expression::Literal(lit) => Ok(self.infer_literal(lit)),
            Expression::Identifier(name) => {
                self.type_vars.get(name)
                    .cloned()
                    .ok_or_else(|| SlangError::Type(format!("Undefined variable: {}", name)))
            }
            Expression::BinaryOp(op) => {
                let left_type = self.infer_expression(&op.left)?;
                let right_type = self.infer_expression(&op.right)?;
                self.infer_binary_operation(&op.op, left_type, right_type)
            }
            Expression::UnaryOp(op) => {
                let expr_type = self.infer_expression(&op.right)?;
                self.infer_unary_operation(&op.op, expr_type)
            }
            Expression::Call(call) => {
                let function_type = self.infer_expression(&Expression::Identifier(call.function.clone()))?;
                let arg_types: Vec<Type> = call.arguments
                    .iter()
                    .map(|arg| self.infer_expression(arg))
                    .collect::<Result<Vec<_>>>()?;
                self.infer_function_call(function_type, arg_types)
            }
            Expression::Assignment(assign) => {
                let value_type = self.infer_expression(&assign.value)?;
                let target_type = self.type_vars.get(&assign.target)
                    .cloned()
                    .ok_or_else(|| SlangError::Type(format!("Undefined variable: {}", assign.target)))?;
                self.add_constraint(value_type, target_type)?;
                Ok(Type::Unit)
            }
        }
    }

    fn infer_literal(&self, literal: &Literal) -> Type {
        match literal {
            Literal::Int(_) => Type::Int,
            Literal::Float(_) => Type::Float,
            Literal::Bool(_) => Type::Bool,
            Literal::String(_) => Type::String,
            Literal::Null => Type::Unit,
        }
    }

    fn infer_binary_operation(&mut self, op: &BinaryOperator, left: Type, right: Type) -> Result<Type> {
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
            BinaryOperator::Eq | BinaryOperator::Neq => {
                self.add_constraint(left.clone(), right.clone())?;
                Ok(Type::Bool)
            }
            BinaryOperator::Lt | BinaryOperator::Lte | BinaryOperator::Gt | BinaryOperator::Gte => {
                if left.is_numeric() && right.is_numeric() {
                    Ok(Type::Bool)
                } else {
                    Err(SlangError::Type("Comparison requires numeric operands".to_string()))
                }
            }
            BinaryOperator::And | BinaryOperator::Or => {
                self.add_constraint(left.clone(), Type::Bool)?;
                self.add_constraint(right.clone(), Type::Bool)?;
                Ok(Type::Bool)
            }
            _ => todo!(),
        }
    }

    fn infer_unary_operation(&mut self, op: &UnaryOperator, expr: Type) -> Result<Type> {
        match op {
            UnaryOperator::Neg => {
                if expr.is_numeric() {
                    Ok(expr)
                } else {
                    Err(SlangError::Type("Negation requires numeric operand".to_string()))
                }
            }
            UnaryOperator::Not => {
                self.add_constraint(expr, Type::Bool)?;
                Ok(Type::Bool)
            }
            _ => todo!(),
        }
    }

    fn infer_function_call(&mut self, function_type: Type, arg_types: Vec<Type>) -> Result<Type> {
        if let Type::Function { params, return_type, .. } = function_type {
            if params.len() != arg_types.len() {
                return Err(SlangError::Type("Wrong number of arguments".to_string()));
            }
            for (param_type, arg_type) in params.iter().zip(arg_types.iter()) {
                self.add_constraint(arg_type.clone(), param_type.clone())?;
            }
            Ok(*return_type)
        } else {
            Err(SlangError::Type("Not a function".to_string()))
        }
    }

    fn infer_pattern(&mut self, pattern: &Pattern, value_type: &Type) -> Result<()> {
        match pattern {
            Pattern::Identifier(name) => {
                self.type_vars.insert(name.clone(), value_type.clone());
                Ok(())
            }
            Pattern::Struct { name, fields } => {
                if let Type::Named(type_name) = value_type {
                    let field_types = self.get_field_types(type_name)?;
                    for field in fields {
                        if let Some(field_type) = field_types.get(&field.name) {
                            self.infer_pattern(&field.pattern, field_type)?;
                        } else {
                            return Err(SlangError::Type(format!("Field '{}' not found in struct '{}'", field.name, name)));
                        }
                    }
                    Ok(())
                } else {
                    Err(SlangError::Type(format!("Expected struct type, got {}", value_type)))
                }
            }
            Pattern::Wildcard => Ok(()),
            Pattern::Tuple(patterns) => {
                if let Type::Tuple(types) = value_type {
                    if patterns.len() != types.len() {
                        return Err(SlangError::Type(format!("Expected {} tuple elements, got {}", types.len(), patterns.len())));
                    }
                    for (pattern, type_) in patterns.iter().zip(types.iter()) {
                        self.infer_pattern(pattern, type_)?;
                    }
                    Ok(())
                } else {
                    Err(SlangError::Type(format!("Expected tuple type, got {}", value_type)))
                }
            }
        }
    }

    fn get_field_types(&self, type_name: &str) -> Result<HashMap<String, Type>> {
        if let Some(type_def) = self.type_definitions.get(type_name) {
            let mut field_types = HashMap::new();
            for field in &type_def.fields {
                field_types.insert(field.name.clone(), field.type_annotation.clone());
            }
            Ok(field_types)
        } else {
            Err(SlangError::Type(format!("Type '{}' not found", type_name)))
        }
    }

    fn add_constraint(&mut self, left: Type, right: Type) -> Result<()> {
        self.constraints.push(TypeConstraint { left, right });
        Ok(())
    }

    fn solve_constraints(&mut self) -> Result<()> {
        let mut constraints = std::mem::take(&mut self.constraints);
        while !constraints.is_empty() {
            let mut new_constraints = Vec::new();
            for constraint in constraints {
                if let Some(new_constraint) = self.unify(&constraint.left, &constraint.right)? {
                    new_constraints.push(new_constraint);
                }
            }
            constraints = new_constraints;
        }
        Ok(())
    }

    fn unify(&mut self, left: &Type, right: &Type) -> Result<Option<TypeConstraint>> {
        match (left, right) {
            (Type::Named(name), other) | (other, Type::Named(name)) => {
                if let Some(defined_type) = self.type_vars.get(name) {
                    Ok(Some(TypeConstraint {
                        left: defined_type.clone(),
                        right: other.clone(),
                    }))
                } else {
                    Ok(None)
                }
            }
            (Type::Array(t1), Type::Array(t2)) => {
                Ok(Some(TypeConstraint {
                    left: *t1.clone(),
                    right: *t2.clone(),
                }))
            }
            (Type::Tuple(t1), Type::Tuple(t2)) => {
                if t1.len() != t2.len() {
                    return Err(SlangError::Type("Tuple length mismatch".to_string()));
                }
                for (a, b) in t1.iter().zip(t2.iter()) {
                    if let Some(constraint) = self.unify(a, b)? {
                        return Ok(Some(constraint));
                    }
                }
                Ok(None)
            }
            (Type::Function { params: p1, return_type: r1, priority: pr1 },
             Type::Function { params: p2, return_type: r2, priority: pr2 }) => {
                if p1.len() != p2.len() {
                    return Err(SlangError::Type("Function parameter count mismatch".to_string()));
                }
                for (a, b) in p1.iter().zip(p2.iter()) {
                    if let Some(constraint) = self.unify(a, b)? {
                        return Ok(Some(constraint));
                    }
                }
                if let Some(constraint) = self.unify(r1, r2)? {
                    return Ok(Some(constraint));
                }
                if pr1 != pr2 {
                    return Err(SlangError::Type("Function priority mismatch".to_string()));
                }
                Ok(None)
            }
            (a, b) if a == b => Ok(None),
            _ => Err(SlangError::Type("Type mismatch".to_string())),
        }
    }

    fn get_current_return_type(&self) -> Option<Type> {
        // 現在の関数の戻り値の型を取得
        // 実装は関数のコンテキスト管理に依存
        None
    }
} 