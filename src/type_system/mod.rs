use std::collections::HashMap;
use std::fmt;

use crate::ast::*;
use crate::error::{Error, Result};

mod inference;
mod cast;
mod checker;

pub use inference::TypeInference;
pub use cast::TypeCast;
pub use checker::TypeChecker;

#[derive(Debug, Clone, PartialEq)]
pub enum Type {
    Unit,
    Int,
    Float,
    Bool,
    String,
    Char,
    Void,
    Array(Box<Type>),
    Tuple(Vec<Type>),
    Vector(usize, Box<Type>),
    Matrix(usize, usize, Box<Type>),
    Tensor(Vec<usize>, Box<Type>),
    Quaternion(Box<Type>),
    Complex(Box<Type>),
    Function {
        params: Vec<Type>,
        return_type: Box<Type>,
        priority: Option<u32>,
    },
    Pointer(Box<Type>),
    Named(String),
}

impl Type {
    pub fn is_vector(&self) -> bool {
        matches!(self, Type::Vector(_, _))
    }

    pub fn is_matrix(&self) -> bool {
        matches!(self, Type::Matrix(_, _, _))
    }

    pub fn is_tensor(&self) -> bool {
        matches!(self, Type::Tensor(_, _))
    }

    pub fn is_quaternion(&self) -> bool {
        matches!(self, Type::Quaternion(_))
    }

    pub fn is_complex(&self) -> bool {
        matches!(self, Type::Complex(_))
    }

    pub fn is_function(&self) -> bool {
        matches!(self, Type::Function { .. })
    }

    pub fn is_pointer(&self) -> bool {
        matches!(self, Type::Pointer(_))
    }

    pub fn is_numeric(&self) -> bool {
        matches!(self, Type::Int | Type::Float)
    }

    pub fn get_vector_dimension(&self) -> Option<usize> {
        match self {
            Type::Vector(dim, _) => Some(*dim),
            _ => None,
        }
    }

    pub fn get_matrix_dimensions(&self) -> Option<(usize, usize)> {
        match self {
            Type::Matrix(rows, cols, _) => Some((*rows, *cols)),
            _ => None,
        }
    }

    pub fn get_tensor_dimensions(&self) -> Option<&[usize]> {
        match self {
            Type::Tensor(dims, _) => Some(dims),
            _ => None,
        }
    }

    pub fn get_function_signature(&self) -> Option<(&[Type], &Type)> {
        match self {
            Type::Function {
                params,
                return_type,
                ..
            } => Some((params, return_type)),
            _ => None,
        }
    }

    pub fn get_pointer_type(&self) -> Option<&Type> {
        match self {
            Type::Pointer(inner) => Some(inner),
            _ => None,
        }
    }

    pub fn get_priority(&self) -> Option<u32> {
        match self {
            Type::Function { priority, .. } => *priority,
            _ => None,
        }
    }

    pub fn set_priority(&mut self, priority: u32) -> Result<()> {
        match self {
            Type::Function { priority: p, .. } => {
                *p = Some(priority);
                Ok(())
            }
            _ => Err(Error::new(
                "Cannot set priority for non-function type",
                None,
            )),
        }
    }

    pub fn is_compatible_with(&self, other: &Type) -> bool {
        match (self, other) {
            (Type::Int, Type::Float) | (Type::Float, Type::Int) => true,
            (Type::Int, Type::String) | (Type::Float, Type::String) | (Type::Bool, Type::String) => true,
            (Type::String, Type::Int) | (Type::String, Type::Float) | (Type::String, Type::Bool) => true,
            (Type::Array(t1), Type::Array(t2)) => t1.is_compatible_with(t2),
            (Type::Tuple(t1), Type::Tuple(t2)) => {
                t1.len() == t2.len() && t1.iter().zip(t2.iter()).all(|(a, b)| a.is_compatible_with(b))
            }
            (Type::Vector(d1, t1), Type::Vector(d2, t2)) => d1 == d2 && t1.is_compatible_with(t2),
            (Type::Matrix(r1, c1, t1), Type::Matrix(r2, c2, t2)) => {
                r1 == r2 && c1 == c2 && t1.is_compatible_with(t2)
            }
            (Type::Tensor(d1, t1), Type::Tensor(d2, t2)) => {
                d1 == d2 && t1.is_compatible_with(t2)
            }
            (Type::Quaternion(t1), Type::Quaternion(t2)) => t1.is_compatible_with(t2),
            (Type::Complex(t1), Type::Complex(t2)) => t1.is_compatible_with(t2),
            (
                Type::Function {
                    params: p1,
                    return_type: r1,
                    priority: pr1,
                },
                Type::Function {
                    params: p2,
                    return_type: r2,
                    priority: pr2,
                },
            ) => {
                p1.len() == p2.len()
                    && p1.iter().zip(p2.iter()).all(|(a, b)| a.is_compatible_with(b))
                    && r1.is_compatible_with(r2)
                    && pr1 == pr2
            }
            (Type::Pointer(t1), Type::Pointer(t2)) => t1.is_compatible_with(t2),
            (Type::Named(n1), Type::Named(n2)) => n1 == n2,
            (t1, t2) => t1 == t2,
        }
    }

    pub fn can_own(&self, other: &Type) -> bool {
        match (self, other) {
            (Type::Function { priority: p1, .. }, Type::Function { priority: p2, .. }) => {
                p1 > p2
            }
            _ => false,
        }
    }
}

impl fmt::Display for Type {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Type::Unit => write!(f, "()"),
            Type::Int => write!(f, "int"),
            Type::Float => write!(f, "float"),
            Type::Bool => write!(f, "bool"),
            Type::String => write!(f, "string"),
            Type::Char => write!(f, "char"),
            Type::Void => write!(f, "void"),
            Type::Array(t) => write!(f, "[{}]", t),
            Type::Tuple(types) => {
                write!(f, "(")?;
                for (i, t) in types.iter().enumerate() {
                    if i > 0 {
                        write!(f, ", ")?;
                    }
                    write!(f, "{}", t)?;
                }
                write!(f, ")")
            }
            Type::Vector(dim, t) => write!(f, "vec{}<{}>", dim, t),
            Type::Matrix(rows, cols, t) => write!(f, "mat{}x{}<{}>", rows, cols, t),
            Type::Tensor(dims, t) => {
                write!(f, "tensor<")?;
                for (i, dim) in dims.iter().enumerate() {
                    if i > 0 {
                        write!(f, "x")?;
                    }
                    write!(f, "{}", dim)?;
                }
                write!(f, ", {}>", t)
            }
            Type::Quaternion(t) => write!(f, "quat<{}>", t),
            Type::Complex(t) => write!(f, "complex<{}>", t),
            Type::Function {
                params,
                return_type,
                priority,
            } => {
                write!(f, "fn(")?;
                for (i, param) in params.iter().enumerate() {
                    if i > 0 {
                        write!(f, ", ")?;
                    }
                    write!(f, "{}", param)?;
                }
                write!(f, ") -> {}", return_type)?;
                if let Some(p) = priority {
                    write!(f, " @{}", p)?;
                }
                Ok(())
            }
            Type::Pointer(t) => write!(f, "*{}", t),
            Type::Named(name) => write!(f, "{}", name),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_type_compatibility() {
        assert!(Type::Int.is_compatible_with(&Type::Float));
        assert!(Type::Float.is_compatible_with(&Type::Int));
        assert!(Type::Int.is_compatible_with(&Type::String));
        assert!(Type::String.is_compatible_with(&Type::Int));

        let array1 = Type::Array(Box::new(Type::Int));
        let array2 = Type::Array(Box::new(Type::Float));
        assert!(array1.is_compatible_with(&array2));

        let tuple1 = Type::Tuple(vec![Type::Int, Type::String]);
        let tuple2 = Type::Tuple(vec![Type::Float, Type::String]);
        assert!(tuple1.is_compatible_with(&tuple2));

        let vec1 = Type::Vector(3, Box::new(Type::Int));
        let vec2 = Type::Vector(3, Box::new(Type::Float));
        assert!(vec1.is_compatible_with(&vec2));

        let mat1 = Type::Matrix(2, 2, Box::new(Type::Int));
        let mat2 = Type::Matrix(2, 2, Box::new(Type::Float));
        assert!(mat1.is_compatible_with(&mat2));

        let tensor1 = Type::Tensor(vec![2, 3, 4], Box::new(Type::Int));
        let tensor2 = Type::Tensor(vec![2, 3, 4], Box::new(Type::Float));
        assert!(tensor1.is_compatible_with(&tensor2));

        let quat1 = Type::Quaternion(Box::new(Type::Int));
        let quat2 = Type::Quaternion(Box::new(Type::Float));
        assert!(quat1.is_compatible_with(&quat2));

        let complex1 = Type::Complex(Box::new(Type::Int));
        let complex2 = Type::Complex(Box::new(Type::Float));
        assert!(complex1.is_compatible_with(&complex2));

        let func1 = Type::Function {
            params: vec![Type::Int],
            return_type: Box::new(Type::Int),
            priority: Some(1),
        };
        let func2 = Type::Function {
            params: vec![Type::Float],
            return_type: Box::new(Type::Float),
            priority: Some(1),
        };
        assert!(func1.is_compatible_with(&func2));

        let ptr1 = Type::Pointer(Box::new(Type::Int));
        let ptr2 = Type::Pointer(Box::new(Type::Float));
        assert!(ptr1.is_compatible_with(&ptr2));
    }

    #[test]
    fn test_priority_ownership() {
        let high_priority = Type::Function {
            params: vec![Type::Int],
            return_type: Box::new(Type::Int),
            priority: Some(2),
        };
        let low_priority = Type::Function {
            params: vec![Type::Int],
            return_type: Box::new(Type::Int),
            priority: Some(1),
        };

        assert!(high_priority.can_own(&low_priority));
        assert!(!low_priority.can_own(&high_priority));
    }

    #[test]
    fn test_type_display() {
        assert_eq!(format!("{}", Type::Int), "int");
        assert_eq!(format!("{}", Type::Float), "float");
        assert_eq!(format!("{}", Type::Bool), "bool");
        assert_eq!(format!("{}", Type::String), "string");
        assert_eq!(format!("{}", Type::Char), "char");
        assert_eq!(format!("{}", Type::Void), "void");
        assert_eq!(format!("{}", Type::Unit), "()");

        let array = Type::Array(Box::new(Type::Int));
        assert_eq!(format!("{}", array), "[int]");

        let tuple = Type::Tuple(vec![Type::Int, Type::String]);
        assert_eq!(format!("{}", tuple), "(int, string)");

        let vec = Type::Vector(3, Box::new(Type::Float));
        assert_eq!(format!("{}", vec), "vec3<float>");

        let mat = Type::Matrix(2, 2, Box::new(Type::Float));
        assert_eq!(format!("{}", mat), "mat2x2<float>");

        let tensor = Type::Tensor(vec![2, 3, 4], Box::new(Type::Float));
        assert_eq!(format!("{}", tensor), "tensor<2x3x4, float>");

        let quat = Type::Quaternion(Box::new(Type::Float));
        assert_eq!(format!("{}", quat), "quat<float>");

        let complex = Type::Complex(Box::new(Type::Float));
        assert_eq!(format!("{}", complex), "complex<float>");

        let func = Type::Function {
            params: vec![Type::Int, Type::String],
            return_type: Box::new(Type::Bool),
            priority: Some(1),
        };
        assert_eq!(format!("{}", func), "fn(int, string) -> bool @1");

        let ptr = Type::Pointer(Box::new(Type::Int));
        assert_eq!(format!("{}", ptr), "*int");

        let named = Type::Named("MyType".to_string());
        assert_eq!(format!("{}", named), "MyType");
    }
} 