use crate::error::{Result, SlangError};
use std::any::Any;
use std::collections::HashMap;
use std::fmt;

pub struct Runtime {
    memory_manager: MemoryManager,
    priority_ownership_manager: PriorityOwnershipManager,
    standard_library: StandardLibrary,
}

impl Runtime {
    pub fn new() -> Self {
        Self {
            memory_manager: MemoryManager::new(),
            priority_ownership_manager: PriorityOwnershipManager::new(),
            standard_library: StandardLibrary::new(),
        }
    }

    pub fn execute(&mut self, ir: &crate::ir::IR) -> Result<()> {
        for function in &ir.functions {
            self.execute_function(function)?;
        }
        Ok(())
    }

    fn execute_function(&mut self, function: &crate::ir::IRFunction) -> Result<()> {
        for block in &function.blocks {
            for instruction in &block.instructions {
                self.execute_instruction(instruction)?;
            }
        }
        Ok(())
    }

    fn execute_instruction(&mut self, instruction: &crate::ir::IRInstruction) -> Result<()> {
        match instruction {
            crate::ir::IRInstruction::Let { name, value } => {
                let value = self.evaluate_value(value)?;
                self.memory_manager.allocate_value(name.clone(), value);
            }
            crate::ir::IRInstruction::Return(value) => {
                let value = self.evaluate_value(value)?;
                println!("Return value: {:?}", value);
            }
            crate::ir::IRInstruction::Expression(value) => {
                let value = self.evaluate_value(value)?;
                println!("Expression value: {:?}", value);
            }
        }
        Ok(())
    }

    fn evaluate_value(&mut self, value: &crate::ir::IRValue) -> Result<Box<dyn Any>> {
        match value {
            crate::ir::IRValue::Constant(constant) => match constant {
                crate::ir::IRConstant::Integer(i) => Ok(Box::new(*i)),
                crate::ir::IRConstant::Float(f) => Ok(Box::new(*f)),
                crate::ir::IRConstant::String(s) => Ok(Box::new(s.clone())),
                crate::ir::IRConstant::Boolean(b) => Ok(Box::new(*b)),
            },
            crate::ir::IRValue::Variable(name) => {
                self.memory_manager.get_value(name).ok_or_else(|| {
                    SlangError::Runtime(format!("Variable not found: {}", name))
                })
            }
            crate::ir::IRValue::BinaryOp { left, op, right } => {
                let left = self.evaluate_value(left)?;
                let right = self.evaluate_value(right)?;
                self.execute_binary_op(op, left, right)
            }
            crate::ir::IRValue::UnaryOp { op, expr } => {
                let expr = self.evaluate_value(expr)?;
                self.execute_unary_op(op, expr)
            }
            crate::ir::IRValue::Call { function, arguments } => {
                let args = arguments
                    .iter()
                    .map(|arg| self.evaluate_value(arg))
                    .collect::<Result<Vec<_>>>()?;
                self.execute_function_call(function, args)
            }
            crate::ir::IRValue::Assignment { name, value } => {
                let value = self.evaluate_value(value)?;
                self.memory_manager.allocate_value(name.clone(), value);
                Ok(Box::new(()))
            }
        }
    }

    fn execute_binary_op(
        &mut self,
        op: &crate::ir::IRBinaryOperator,
        left: Box<dyn Any>,
        right: Box<dyn Any>,
    ) -> Result<Box<dyn Any>> {
        match op {
            crate::ir::IRBinaryOperator::Add => {
                if let (Some(l), Some(r)) = (
                    left.downcast_ref::<i64>(),
                    right.downcast_ref::<i64>(),
                ) {
                    Ok(Box::new(l + r))
                } else if let (Some(l), Some(r)) = (
                    left.downcast_ref::<f64>(),
                    right.downcast_ref::<f64>(),
                ) {
                    Ok(Box::new(l + r))
                } else {
                    Err(SlangError::Runtime("Invalid operands for addition".to_string()))
                }
            }
            crate::ir::IRBinaryOperator::Subtract => {
                if let (Some(l), Some(r)) = (
                    left.downcast_ref::<i64>(),
                    right.downcast_ref::<i64>(),
                ) {
                    Ok(Box::new(l - r))
                } else if let (Some(l), Some(r)) = (
                    left.downcast_ref::<f64>(),
                    right.downcast_ref::<f64>(),
                ) {
                    Ok(Box::new(l - r))
                } else {
                    Err(SlangError::Runtime("Invalid operands for subtraction".to_string()))
                }
            }
            crate::ir::IRBinaryOperator::Multiply => {
                if let (Some(l), Some(r)) = (
                    left.downcast_ref::<i64>(),
                    right.downcast_ref::<i64>(),
                ) {
                    Ok(Box::new(l * r))
                } else if let (Some(l), Some(r)) = (
                    left.downcast_ref::<f64>(),
                    right.downcast_ref::<f64>(),
                ) {
                    Ok(Box::new(l * r))
                } else {
                    Err(SlangError::Runtime("Invalid operands for multiplication".to_string()))
                }
            }
            crate::ir::IRBinaryOperator::Divide => {
                if let (Some(l), Some(r)) = (
                    left.downcast_ref::<i64>(),
                    right.downcast_ref::<i64>(),
                ) {
                    if *r == 0 {
                        Err(SlangError::Runtime("Division by zero".to_string()))
                    } else {
                        Ok(Box::new(l / r))
                    }
                } else if let (Some(l), Some(r)) = (
                    left.downcast_ref::<f64>(),
                    right.downcast_ref::<f64>(),
                ) {
                    if *r == 0.0 {
                        Err(SlangError::Runtime("Division by zero".to_string()))
                    } else {
                        Ok(Box::new(l / r))
                    }
                } else {
                    Err(SlangError::Runtime("Invalid operands for division".to_string()))
                }
            }
            crate::ir::IRBinaryOperator::Modulo => {
                if let (Some(l), Some(r)) = (
                    left.downcast_ref::<i64>(),
                    right.downcast_ref::<i64>(),
                ) {
                    if *r == 0 {
                        Err(SlangError::Runtime("Modulo by zero".to_string()))
                    } else {
                        Ok(Box::new(l % r))
                    }
                } else {
                    Err(SlangError::Runtime("Invalid operands for modulo".to_string()))
                }
            }
            crate::ir::IRBinaryOperator::Equals => {
                if let (Some(l), Some(r)) = (
                    left.downcast_ref::<i64>(),
                    right.downcast_ref::<i64>(),
                ) {
                    Ok(Box::new(l == r))
                } else if let (Some(l), Some(r)) = (
                    left.downcast_ref::<f64>(),
                    right.downcast_ref::<f64>(),
                ) {
                    Ok(Box::new(l == r))
                } else if let (Some(l), Some(r)) = (
                    left.downcast_ref::<String>(),
                    right.downcast_ref::<String>(),
                ) {
                    Ok(Box::new(l == r))
                } else if let (Some(l), Some(r)) = (
                    left.downcast_ref::<bool>(),
                    right.downcast_ref::<bool>(),
                ) {
                    Ok(Box::new(l == r))
                } else {
                    Err(SlangError::Runtime("Invalid operands for equality".to_string()))
                }
            }
            crate::ir::IRBinaryOperator::NotEquals => {
                if let (Some(l), Some(r)) = (
                    left.downcast_ref::<i64>(),
                    right.downcast_ref::<i64>(),
                ) {
                    Ok(Box::new(l != r))
                } else if let (Some(l), Some(r)) = (
                    left.downcast_ref::<f64>(),
                    right.downcast_ref::<f64>(),
                ) {
                    Ok(Box::new(l != r))
                } else if let (Some(l), Some(r)) = (
                    left.downcast_ref::<String>(),
                    right.downcast_ref::<String>(),
                ) {
                    Ok(Box::new(l != r))
                } else if let (Some(l), Some(r)) = (
                    left.downcast_ref::<bool>(),
                    right.downcast_ref::<bool>(),
                ) {
                    Ok(Box::new(l != r))
                } else {
                    Err(SlangError::Runtime("Invalid operands for inequality".to_string()))
                }
            }
            crate::ir::IRBinaryOperator::LessThan => {
                if let (Some(l), Some(r)) = (
                    left.downcast_ref::<i64>(),
                    right.downcast_ref::<i64>(),
                ) {
                    Ok(Box::new(l < r))
                } else if let (Some(l), Some(r)) = (
                    left.downcast_ref::<f64>(),
                    right.downcast_ref::<f64>(),
                ) {
                    Ok(Box::new(l < r))
                } else {
                    Err(SlangError::Runtime("Invalid operands for less than".to_string()))
                }
            }
            crate::ir::IRBinaryOperator::GreaterThan => {
                if let (Some(l), Some(r)) = (
                    left.downcast_ref::<i64>(),
                    right.downcast_ref::<i64>(),
                ) {
                    Ok(Box::new(l > r))
                } else if let (Some(l), Some(r)) = (
                    left.downcast_ref::<f64>(),
                    right.downcast_ref::<f64>(),
                ) {
                    Ok(Box::new(l > r))
                } else {
                    Err(SlangError::Runtime("Invalid operands for greater than".to_string()))
                }
            }
            crate::ir::IRBinaryOperator::LessThanEquals => {
                if let (Some(l), Some(r)) = (
                    left.downcast_ref::<i64>(),
                    right.downcast_ref::<i64>(),
                ) {
                    Ok(Box::new(l <= r))
                } else if let (Some(l), Some(r)) = (
                    left.downcast_ref::<f64>(),
                    right.downcast_ref::<f64>(),
                ) {
                    Ok(Box::new(l <= r))
                } else {
                    Err(SlangError::Runtime("Invalid operands for less than or equal".to_string()))
                }
            }
            crate::ir::IRBinaryOperator::GreaterThanEquals => {
                if let (Some(l), Some(r)) = (
                    left.downcast_ref::<i64>(),
                    right.downcast_ref::<i64>(),
                ) {
                    Ok(Box::new(l >= r))
                } else if let (Some(l), Some(r)) = (
                    left.downcast_ref::<f64>(),
                    right.downcast_ref::<f64>(),
                ) {
                    Ok(Box::new(l >= r))
                } else {
                    Err(SlangError::Runtime("Invalid operands for greater than or equal".to_string()))
                }
            }
            _ => todo!(),
        }
    }

    fn execute_unary_op(
        &mut self,
        op: &crate::ir::IRUnaryOperator,
        expr: Box<dyn Any>,
    ) -> Result<Box<dyn Any>> {
        match op {
            crate::ir::IRUnaryOperator::Negate => {
                if let Some(i) = expr.downcast_ref::<i64>() {
                    Ok(Box::new(-i))
                } else if let Some(f) = expr.downcast_ref::<f64>() {
                    Ok(Box::new(-f))
                } else {
                    Err(SlangError::Runtime("Invalid operand for negation".to_string()))
                }
            }
            crate::ir::IRUnaryOperator::Not => {
                if let Some(b) = expr.downcast_ref::<bool>() {
                    Ok(Box::new(!b))
                } else {
                    Err(SlangError::Runtime("Invalid operand for logical not".to_string()))
                }
            }
            _ => todo!(),
        }
    }

    fn execute_function_call(
        &mut self,
        function: &str,
        arguments: Vec<Box<dyn Any>>,
    ) -> Result<Box<dyn Any>> {
        if let Some(func) = self.standard_library.get_function(function) {
            func(&arguments)
        } else {
            Err(SlangError::Runtime(format!("Function not found: {}", function)))
        }
    }
}

struct MemoryManager {
    heap: HashMap<String, Box<dyn Any>>,
    stack: Vec<Box<dyn Any>>,
}

impl MemoryManager {
    fn new() -> Self {
        Self {
            heap: HashMap::new(),
            stack: Vec::new(),
        }
    }

    fn allocate(&mut self, size: usize, _priority: i32) -> usize {
        let address = self.stack.len();
        self.stack.push(Box::new(vec![0u8; size]));
        address
    }

    fn allocate_value(&mut self, name: String, value: Box<dyn Any>) {
        self.heap.insert(name, value);
    }

    fn get_value(&self, name: &str) -> Option<Box<dyn Any>> {
        self.heap.get(name).cloned()
    }

    fn deallocate(&mut self, address: usize) {
        if address < self.stack.len() {
            self.stack.remove(address);
        }
    }
}

struct PriorityOwnershipManager {
    priorities: HashMap<String, Vec<i32>>,
}

impl PriorityOwnershipManager {
    fn new() -> Self {
        Self {
            priorities: HashMap::new(),
        }
    }

    fn set_priority(&mut self, name: String, priority: i32) {
        self.priorities.insert(name, vec![priority]);
    }

    fn get_priority(&self, name: &str) -> Option<&Vec<i32>> {
        self.priorities.get(name)
    }

    fn transfer_ownership(&mut self, from: &str, to: &str) -> Result<()> {
        if let Some(priority) = self.priorities.remove(from) {
            self.priorities.insert(to.to_string(), priority);
            Ok(())
        } else {
            Err(SlangError::Runtime(format!("No priority found for: {}", from)))
        }
    }
}

struct StandardLibrary {
    functions: HashMap<String, Box<dyn Fn(&[Box<dyn Any>]) -> Result<Box<dyn Any>>>>,
}

impl StandardLibrary {
    fn new() -> Self {
        let mut functions = HashMap::new();
        functions.insert(
            "print".to_string(),
            Box::new(|args: &[Box<dyn Any>]| {
                for arg in args {
                    println!("{:?}", arg);
                }
                Ok(Box::new(()))
            }),
        );
        Self { functions }
    }

    fn get_function(&self, name: &str) -> Option<&dyn Fn(&[Box<dyn Any>]) -> Result<Box<dyn Any>>> {
        self.functions.get(name).map(|f| f.as_ref())
    }
}

impl fmt::Debug for Runtime {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "Runtime")
    }
}

impl fmt::Debug for MemoryManager {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "MemoryManager")
    }
}

impl fmt::Debug for PriorityOwnershipManager {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "PriorityOwnershipManager")
    }
}

impl fmt::Debug for StandardLibrary {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "StandardLibrary")
    }
} 