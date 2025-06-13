use crate::ir::{IRNode, IRFunction, IRBlock, IRInstruction, IRValue, IRType};
use std::collections::{HashMap, HashSet};

pub struct Optimizer {
    constant_values: HashMap<String, IRValue>,
    priority_ownership: HashMap<String, u32>,
}

impl Optimizer {
    pub fn new() -> Self {
        Self {
            constant_values: HashMap::new(),
            priority_ownership: HashMap::new(),
        }
    }

    pub fn optimize(&mut self, ir: &mut IRNode) {
        self.constant_folding(ir);
        self.dead_code_elimination(ir);
        self.optimize_priority_ownership(ir);
    }

    fn constant_folding(&mut self, ir: &mut IRNode) {
        match ir {
            IRNode::Function(func) => {
                for block in &mut func.blocks {
                    self.fold_constants_in_block(block);
                }
            }
            _ => {}
        }
    }

    fn fold_constants_in_block(&mut self, block: &mut IRBlock) {
        let mut i = 0;
        while i < block.instructions.len() {
            if let Some(folded) = self.try_fold_instruction(&block.instructions[i]) {
                block.instructions[i] = folded;
            }
            i += 1;
        }
    }

    fn try_fold_instruction(&self, inst: &IRInstruction) -> Option<IRInstruction> {
        match inst {
            IRInstruction::BinaryOp { op, left, right, result } => {
                if let (Some(l), Some(r)) = (self.evaluate_constant(left), self.evaluate_constant(right)) {
                    if let (IRValue::Integer(l_val), IRValue::Integer(r_val)) = (l, r) {
                        let result_val = match op.as_str() {
                            "add" => l_val + r_val,
                            "sub" => l_val - r_val,
                            "mul" => l_val * r_val,
                            "div" => l_val / r_val,
                            _ => return None,
                        };
                        return Some(IRInstruction::Assign {
                            value: IRValue::Integer(result_val),
                            target: result.clone(),
                        });
                    }
                }
            }
            _ => {}
        }
        None
    }

    fn evaluate_constant(&self, value: &IRValue) -> Option<IRValue> {
        match value {
            IRValue::Integer(_) | IRValue::Float(_) | IRValue::Boolean(_) => Some(value.clone()),
            IRValue::Variable(name) => self.constant_values.get(name).cloned(),
            _ => None,
        }
    }

    fn dead_code_elimination(&mut self, ir: &mut IRNode) {
        let mut live_vars = HashSet::new();
        self.mark_live_variables(ir, &mut live_vars);
        self.remove_dead_code(ir, &live_vars);
    }

    fn mark_live_variables(&self, ir: &IRNode, live_vars: &mut HashSet<String>) {
        match ir {
            IRNode::Function(func) => {
                for block in &func.blocks {
                    for inst in &block.instructions {
                        match inst {
                            IRInstruction::Assign { target, .. } => {
                                live_vars.insert(target.clone());
                            }
                            IRInstruction::BinaryOp { result, .. } => {
                                live_vars.insert(result.clone());
                            }
                            _ => {}
                        }
                    }
                }
            }
            _ => {}
        }
    }

    fn remove_dead_code(&self, ir: &mut IRNode, live_vars: &HashSet<String>) {
        match ir {
            IRNode::Function(func) => {
                for block in &mut func.blocks {
                    block.instructions.retain(|inst| {
                        match inst {
                            IRInstruction::Assign { target, .. } => live_vars.contains(target),
                            IRInstruction::BinaryOp { result, .. } => live_vars.contains(result),
                            _ => true,
                        }
                    });
                }
            }
            _ => {}
        }
    }

    fn optimize_priority_ownership(&mut self, ir: &mut IRNode) {
        self.analyze_priority_ownership(ir);
        self.apply_priority_optimizations(ir);
    }

    fn analyze_priority_ownership(&mut self, ir: &IRNode) {
        match ir {
            IRNode::Function(func) => {
                for block in &func.blocks {
                    for inst in &block.instructions {
                        if let IRInstruction::PriorityOwnership { variable, priority } = inst {
                            self.priority_ownership.insert(variable.clone(), *priority);
                        }
                    }
                }
            }
            _ => {}
        }
    }

    fn apply_priority_optimizations(&self, ir: &mut IRNode) {
        match ir {
            IRNode::Function(func) => {
                for block in &mut func.blocks {
                    block.instructions.retain(|inst| {
                        if let IRInstruction::PriorityOwnership { variable, priority } = inst {
                            if let Some(current_priority) = self.priority_ownership.get(variable) {
                                return current_priority != priority;
                            }
                        }
                        true
                    });
                }
            }
            _ => {}
        }
    }
} 