use std::fmt;

#[derive(Debug, Clone, PartialEq)]
pub enum Type {
    Unit,
    Int,
    Float,
    Bool,
    String,
    Array(Box<Type>),
    Tuple(Vec<Type>),
    Named(String),
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
}

impl fmt::Display for Type {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Type::Unit => write!(f, "()"),
            Type::Int => write!(f, "int"),
            Type::Float => write!(f, "float"),
            Type::Bool => write!(f, "bool"),
            Type::String => write!(f, "string"),
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
        }
    }
} 