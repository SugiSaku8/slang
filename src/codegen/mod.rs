use crate::ir::*;
use crate::error::Result;
use std::collections::HashMap;

pub struct CodeGenerator {
    module: Module,
    current_function: Option<String>,
    current_block: Option<String>,
    value_map: HashMap<String, String>,
    block_map: HashMap<String, String>,
}

impl CodeGenerator {
    pub fn new(module: Module) -> Self {
        Self {
            module,
            current_function: None,
            current_block: None,
            value_map: HashMap::new(),
            block_map: HashMap::new(),
        }
    }

    pub fn generate(&mut self) -> Result<String> {
        let mut output = String::new();
        
        // グローバル変数の生成
        for global in &self.module.globals {
            output.push_str(&self.generate_global(global)?);
        }

        // 関数の生成
        for function in &self.module.functions {
            output.push_str(&self.generate_function(function)?);
        }

        Ok(output)
    }

    fn generate_global(&self, global: &Global) -> Result<String> {
        let mut output = format!("@{} = ", global.name);
        
        // 優先所有格の処理
        if let Some(priority) = &global.priority {
            output.push_str(&format!("priority({:?}) ", priority));
        }

        // 型の生成
        output.push_str(&self.generate_type(&global.type_)?);
        output.push_str(" ");

        // 値の生成
        output.push_str(&self.generate_constant(&global.value)?);
        output.push_str("\n");

        Ok(output)
    }

    fn generate_function(&mut self, function: &Function) -> Result<String> {
        self.current_function = Some(function.name.clone());
        let mut output = format!("define {} @{}(", 
            self.generate_type(&function.return_type)?,
            function.name
        );

        // パラメータの生成
        let params: Vec<String> = function.params.iter()
            .map(|param| format!("{} %{}", 
                self.generate_type(&param.type_)?,
                param.name
            ))
            .collect();
        output.push_str(&params.join(", "));
        output.push_str(") {\n");

        // ブロックの生成
        for block in &function.blocks {
            output.push_str(&self.generate_block(block)?);
        }

        output.push_str("}\n\n");
        self.current_function = None;
        Ok(output)
    }

    fn generate_block(&mut self, block: &Block) -> Result<String> {
        self.current_block = Some(block.name.clone());
        let mut output = format!("{}:\n", block.name);

        // 命令の生成
        for instruction in &block.instructions {
            output.push_str(&self.generate_instruction(instruction)?);
        }

        // 終端命令の生成
        output.push_str(&self.generate_terminator(&block.terminator)?);
        output.push_str("\n");

        self.current_block = None;
        Ok(output)
    }

    fn generate_instruction(&mut self, instruction: &Instruction) -> Result<String> {
        match instruction {
            Instruction::Alloca(inst) => {
                let name = format!("%{}", inst.name);
                self.value_map.insert(inst.name.clone(), name.clone());
                Ok(format!("  {} = alloca {}\n", 
                    name,
                    self.generate_type(&inst.type_)?))
            }
            Instruction::Load(inst) => {
                let name = format!("%{}", inst.name);
                self.value_map.insert(inst.name.clone(), name.clone());
                Ok(format!("  {} = load {}, {}* {}\n",
                    name,
                    self.generate_type(&inst.type_)?,
                    self.generate_type(&inst.type_)?,
                    self.generate_value(&inst.pointer)?))
            }
            // 他の命令の生成も同様に実装...
            _ => unimplemented!(),
        }
    }

    fn generate_type(&self, type_: &Type) -> Result<String> {
        match type_ {
            Type::Int => Ok("i64".to_string()),
            Type::Float => Ok("double".to_string()),
            Type::Bool => Ok("i1".to_string()),
            Type::Char => Ok("i8".to_string()),
            Type::String => Ok("i8*".to_string()),
            Type::Void => Ok("void".to_string()),
            // 他の型の生成も同様に実装...
            _ => unimplemented!(),
        }
    }

    fn generate_value(&self, value: &Value) -> Result<String> {
        match value {
            Value::Constant(constant) => self.generate_constant(constant),
            Value::Instruction(name) => Ok(self.value_map.get(name)
                .cloned()
                .unwrap_or_else(|| format!("%{}", name))),
            Value::Parameter(name) => Ok(format!("%{}", name)),
            Value::Global(name) => Ok(format!("@{}", name)),
        }
    }

    fn generate_constant(&self, constant: &Constant) -> Result<String> {
        match constant {
            Constant::Int(value) => Ok(value.to_string()),
            Constant::Float(value) => Ok(value.to_string()),
            Constant::Bool(value) => Ok(if *value { "1" } else { "0" }.to_string()),
            Constant::Char(value) => Ok(format!("{}", *value as u8)),
            Constant::String(value) => Ok(format!("c\"{}\"", value)),
            // 他の定数の生成も同様に実装...
            _ => unimplemented!(),
        }
    }

    fn generate_terminator(&self, terminator: &Terminator) -> Result<String> {
        match terminator {
            Terminator::Ret(None) => Ok("  ret void\n".to_string()),
            Terminator::Ret(Some(value)) => Ok(format!("  ret {}\n",
                self.generate_value(value)?)),
            Terminator::Br(label) => Ok(format!("  br label %{}\n", label)),
            Terminator::CondBr(cond, true_label, false_label) => Ok(format!(
                "  br {}, label %{}, label %{}\n",
                self.generate_value(cond)?,
                true_label,
                false_label
            )),
            // 他の終端命令の生成も同様に実装...
            _ => unimplemented!(),
        }
    }
} 