use std::fmt;

#[derive(Debug, Clone, PartialEq)]
pub enum Type {
    Unit,
    Int,
    Float,
    Bool,
    Char,
    String,
    Void,
    Array(Box<Type>),
    Tuple(Vec<Type>),
    Named(String),
    Vector(usize, Box<Type>),
    Matrix(usize, usize, Box<Type>),
    Tensor(Vec<usize>, Box<Type>),
    Quaternion(Box<Type>),
    Complex(Box<Type>),
    Function {
        params: Vec<Type>,
        return_type: Box<Type>,
        priority: Option<i32>,
    },
    Pointer(Box<Type>),
}

impl Type {
    pub fn is_numeric(&self) -> bool {
        matches!(self, Type::Int | Type::Float)
    }

    pub fn is_boolean(&self) -> bool {
        matches!(self, Type::Bool)
    }

    pub fn is_string(&self) -> bool {
        matches!(self, Type::String)
    }

    pub fn is_array(&self) -> bool {
        matches!(self, Type::Array(_))
    }

    pub fn is_tuple(&self) -> bool {
        matches!(self, Type::Tuple(_))
    }

    pub fn is_named(&self) -> bool {
        matches!(self, Type::Named(_))
    }

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

    pub fn get_array_element_type(&self) -> Option<&Type> {
        if let Type::Array(element_type) = self {
            Some(element_type)
        } else {
            None
        }
    }

    pub fn get_tuple_types(&self) -> Option<&Vec<Type>> {
        if let Type::Tuple(types) = self {
            Some(types)
        } else {
            None
        }
    }

    pub fn get_named_type(&self) -> Option<&str> {
        if let Type::Named(name) = self {
            Some(name)
        } else {
            None
        }
    }

    pub fn get_vector_dimension(&self) -> Option<usize> {
        if let Type::Vector(dim, _) = self {
            Some(*dim)
        } else {
            None
        }
    }

    pub fn get_matrix_dimensions(&self) -> Option<(usize, usize)> {
        if let Type::Matrix(rows, cols, _) = self {
            Some((*rows, *cols))
        } else {
            None
        }
    }

    pub fn get_tensor_dimensions(&self) -> Option<&Vec<usize>> {
        if let Type::Tensor(dims, _) = self {
            Some(dims)
        } else {
            None
        }
    }

    pub fn get_function_signature(&self) -> Option<(&Vec<Type>, &Type, Option<i32>)> {
        if let Type::Function { params, return_type, priority } = self {
            Some((params, return_type, *priority))
        } else {
            None
        }
    }

    pub fn get_pointer_type(&self) -> Option<&Type> {
        if let Type::Pointer(pointee_type) = self {
            Some(pointee_type)
        } else {
            None
        }
    }

    pub fn is_compatible_with(&self, other: &Type) -> bool {
        match (self, other) {
            (Type::Unit, Type::Unit) => true,
            (Type::Int, Type::Int) => true,
            (Type::Float, Type::Float) => true,
            (Type::Bool, Type::Bool) => true,
            (Type::Char, Type::Char) => true,
            (Type::String, Type::String) => true,
            (Type::Void, Type::Void) => true,
            (Type::Array(t1), Type::Array(t2)) => t1.is_compatible_with(t2),
            (Type::Tuple(t1), Type::Tuple(t2)) => {
                t1.len() == t2.len() && t1.iter().zip(t2.iter()).all(|(a, b)| a.is_compatible_with(b))
            }
            (Type::Named(n1), Type::Named(n2)) => n1 == n2,
            (Type::Vector(d1, t1), Type::Vector(d2, t2)) => d1 == d2 && t1.is_compatible_with(t2),
            (Type::Matrix(r1, c1, t1), Type::Matrix(r2, c2, t2)) => {
                r1 == r2 && c1 == c2 && t1.is_compatible_with(t2)
            }
            (Type::Tensor(d1, t1), Type::Tensor(d2, t2)) => {
                d1 == d2 && t1.is_compatible_with(t2)
            }
            (Type::Quaternion(t1), Type::Quaternion(t2)) => t1.is_compatible_with(t2),
            (Type::Complex(t1), Type::Complex(t2)) => t1.is_compatible_with(t2),
            (Type::Function { params: p1, return_type: r1, priority: pr1 },
             Type::Function { params: p2, return_type: r2, priority: pr2 }) => {
                p1.len() == p2.len() &&
                p1.iter().zip(p2.iter()).all(|(a, b)| a.is_compatible_with(b)) &&
                r1.is_compatible_with(r2) &&
                pr1 == pr2
            }
            (Type::Pointer(t1), Type::Pointer(t2)) => t1.is_compatible_with(t2),
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
            Type::Char => write!(f, "char"),
            Type::String => write!(f, "string"),
            Type::Void => write!(f, "void"),
            Type::Array(element_type) => write!(f, "[{}]", element_type),
            Type::Tuple(types) => {
                write!(f, "(")?;
                for (i, type_) in types.iter().enumerate() {
                    if i > 0 {
                        write!(f, ", ")?;
                    }
                    write!(f, "{}", type_)?;
                }
                write!(f, ")")
            }
            Type::Named(name) => write!(f, "{}", name),
            Type::Vector(dim, element_type) => write!(f, "Vector<{}, {}>", dim, element_type),
            Type::Matrix(rows, cols, element_type) => {
                write!(f, "Matrix<{}, {}, {}>", rows, cols, element_type)
            }
            Type::Tensor(dims, element_type) => {
                write!(f, "Tensor<{:?}, {}>", dims, element_type)
            }
            Type::Quaternion(element_type) => write!(f, "Quaternion<{}>", element_type),
            Type::Complex(element_type) => write!(f, "Complex<{}>", element_type),
            Type::Function { params, return_type, priority } => {
                write!(f, "fn(")?;
                for (i, param) in params.iter().enumerate() {
                    if i > 0 {
                        write!(f, ", ")?;
                    }
                    write!(f, "{}", param)?;
                }
                write!(f, ") -> {}", return_type)?;
                if let Some(pri) = priority {
                    write!(f, " [priority: {}]", pri)?;
                }
                Ok(())
            }
            Type::Pointer(pointee_type) => write!(f, "*{}", pointee_type),
        }
    }
} 