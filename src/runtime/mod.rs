use std::collections::HashMap;
use std::sync::{Arc, Mutex};
use std::cell::RefCell;

pub struct Runtime {
    memory: Arc<Mutex<MemoryManager>>,
    priority_ownership: Arc<Mutex<PriorityOwnershipManager>>,
    standard_lib: StandardLibrary,
}

pub struct MemoryManager {
    heap: HashMap<String, Box<dyn std::any::Any>>,
    stack: Vec<HashMap<String, Box<dyn std::any::Any>>>,
}

pub struct PriorityOwnershipManager {
    priorities: HashMap<String, u32>,
    ownership_chains: HashMap<String, Vec<String>>,
}

pub struct StandardLibrary {
    functions: HashMap<String, Box<dyn Fn(&[Box<dyn std::any::Any>]) -> Box<dyn std::any::Any>>,
}

impl Runtime {
    pub fn new() -> Self {
        Self {
            memory: Arc::new(Mutex::new(MemoryManager::new())),
            priority_ownership: Arc::new(Mutex::new(PriorityOwnershipManager::new())),
            standard_lib: StandardLibrary::new(),
        }
    }

    pub fn allocate(&self, name: String, value: Box<dyn std::any::Any>) {
        let mut memory = self.memory.lock().unwrap();
        memory.allocate(name, value);
    }

    pub fn deallocate(&self, name: &str) {
        let mut memory = self.memory.lock().unwrap();
        memory.deallocate(name);
    }

    pub fn set_priority(&self, name: String, priority: u32) {
        let mut ownership = self.priority_ownership.lock().unwrap();
        ownership.set_priority(name, priority);
    }

    pub fn get_priority(&self, name: &str) -> Option<u32> {
        let ownership = self.priority_ownership.lock().unwrap();
        ownership.get_priority(name)
    }

    pub fn call_standard_lib(&self, name: &str, args: &[Box<dyn std::any::Any>]) -> Box<dyn std::any::Any> {
        self.standard_lib.call(name, args)
    }
}

impl MemoryManager {
    pub fn new() -> Self {
        Self {
            heap: HashMap::new(),
            stack: vec![HashMap::new()],
        }
    }

    pub fn allocate(&mut self, name: String, value: Box<dyn std::any::Any>) {
        self.heap.insert(name, value);
    }

    pub fn deallocate(&mut self, name: &str) {
        self.heap.remove(name);
    }

    pub fn push_stack_frame(&mut self) {
        self.stack.push(HashMap::new());
    }

    pub fn pop_stack_frame(&mut self) {
        self.stack.pop();
    }
}

impl PriorityOwnershipManager {
    pub fn new() -> Self {
        Self {
            priorities: HashMap::new(),
            ownership_chains: HashMap::new(),
        }
    }

    pub fn set_priority(&mut self, name: String, priority: u32) {
        self.priorities.insert(name, priority);
    }

    pub fn get_priority(&self, name: &str) -> Option<u32> {
        self.priorities.get(name).copied()
    }

    pub fn add_ownership_chain(&mut self, owner: String, owned: String) {
        self.ownership_chains
            .entry(owner)
            .or_insert_with(Vec::new)
            .push(owned);
    }
}

impl StandardLibrary {
    pub fn new() -> Self {
        let mut functions = HashMap::new();
        
        // Add standard library functions
        functions.insert("print".to_string(), Box::new(|args| {
            if let Some(arg) = args.get(0) {
                println!("{}", arg);
            }
            Box::new(())
        }));

        functions.insert("len".to_string(), Box::new(|args| {
            if let Some(arg) = args.get(0) {
                if let Some(s) = arg.downcast_ref::<String>() {
                    return Box::new(s.len());
                }
            }
            Box::new(0)
        }));

        Self { functions }
    }

    pub fn call(&self, name: &str, args: &[Box<dyn std::any::Any>]) -> Box<dyn std::any::Any> {
        if let Some(func) = self.functions.get(name) {
            func(args)
        } else {
            panic!("Unknown standard library function: {}", name);
        }
    }
} 