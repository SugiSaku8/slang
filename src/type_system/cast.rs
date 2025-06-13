use crate::error::{Result, SlangError};
use crate::ast::Literal;
use crate::type_system::Type;

#[derive(Debug, Clone)]
pub struct TypeCast;

impl TypeCast {
    pub fn new() -> Self {
        Self
    }

    pub fn cast_literal(&self, literal: &Literal, target_type: &Type) -> Result<Literal> {
        match (literal, target_type) {
            (Literal::Int(i), Type::Float) => Ok(Literal::Float(*i as f64)),
            (Literal::Float(f), Type::Int) => Ok(Literal::Int(*f as i64)),
            (Literal::Int(i), Type::String) => Ok(Literal::String(i.to_string())),
            (Literal::Float(f), Type::String) => Ok(Literal::String(f.to_string())),
            (Literal::Bool(b), Type::String) => Ok(Literal::String(b.to_string())),
            (Literal::String(s), Type::Int) => {
                s.parse::<i64>()
                    .map(Literal::Int)
                    .map_err(|_| SlangError::Type("Cannot convert string to integer".to_string()))
            }
            (Literal::String(s), Type::Float) => {
                s.parse::<f64>()
                    .map(Literal::Float)
                    .map_err(|_| SlangError::Type("Cannot convert string to float".to_string()))
            }
            (Literal::String(s), Type::Bool) => {
                match s.to_lowercase().as_str() {
                    "true" => Ok(Literal::Bool(true)),
                    "false" => Ok(Literal::Bool(false)),
                    _ => Err(SlangError::Type("Cannot convert string to boolean".to_string())),
                }
            }
            (lit, target) if lit.get_type() == *target => Ok(lit.clone()),
            _ => Err(SlangError::Type(format!(
                "Cannot cast {:?} to {:?}",
                literal, target_type
            ))),
        }
    }

    pub fn is_cast_allowed(&self, from_type: &Type, to_type: &Type) -> bool {
        match (from_type, to_type) {
            (Type::Int, Type::Float) => true,
            (Type::Float, Type::Int) => true,
            (Type::Int, Type::String) => true,
            (Type::Float, Type::String) => true,
            (Type::Bool, Type::String) => true,
            (Type::String, Type::Int) => true,
            (Type::String, Type::Float) => true,
            (Type::String, Type::Bool) => true,
            (a, b) if a == b => true,
            _ => false,
        }
    }

    pub fn get_cast_cost(&self, from_type: &Type, to_type: &Type) -> Option<u32> {
        match (from_type, to_type) {
            (Type::Int, Type::Float) => Some(1),
            (Type::Float, Type::Int) => Some(1),
            (Type::Int, Type::String) => Some(2),
            (Type::Float, Type::String) => Some(2),
            (Type::Bool, Type::String) => Some(2),
            (Type::String, Type::Int) => Some(3),
            (Type::String, Type::Float) => Some(3),
            (Type::String, Type::Bool) => Some(3),
            (a, b) if a == b => Some(0),
            _ => None,
        }
    }
}

impl Literal {
    fn get_type(&self) -> Type {
        match self {
            Literal::Int(_) => Type::Int,
            Literal::Float(_) => Type::Float,
            Literal::Bool(_) => Type::Bool,
            Literal::String(_) => Type::String,
            Literal::Null => Type::Unit,
        }
    }
} 