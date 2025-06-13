use std::collections::HashMap;
use std::sync::{Arc, Mutex};
use std::cell::RefCell;

pub struct Runtime {
    memory: MemoryManager,
    priority_ownership: PriorityOwnershipManager,
    standard_library: StandardLibrary,
}

pub struct MemoryManager {
    heap: HashMap<usize, Vec<u8>>,
    stack: Vec<Vec<u8>>,
}

pub struct PriorityOwnershipManager {
    priorities: HashMap<String, Vec<i32>>,
    ownership_chains: HashMap<String, Vec<String>>,
}

pub struct StandardLibrary {
    functions: HashMap<String, Box<dyn Fn(&[Box<dyn std::any::Any>]) -> Box<dyn std::any::Any>>>,
}

impl Runtime {
    pub fn new() -> Self {
        Runtime {
            memory: MemoryManager::new(),
            priority_ownership: PriorityOwnershipManager::new(),
            standard_library: StandardLibrary::new(),
        }
    }

    pub fn allocate(&mut self, size: usize, priority: i32) -> usize {
        self.memory.allocate(size, priority)
    }

    pub fn deallocate(&mut self, address: usize) {
        self.memory.deallocate(address);
    }

    pub fn set_priority(&mut self, name: &str, priority: Vec<i32>) {
        self.priority_ownership.set_priority(name, priority);
    }

    pub fn get_priority(&self, name: &str) -> Option<&Vec<i32>> {
        self.priority_ownership.get_priority(name)
    }

    pub fn call_standard_function(&self, name: &str, args: &[Box<dyn std::any::Any>]) -> Option<Box<dyn std::any::Any>> {
        self.standard_library.call_function(name, args)
    }
}

impl MemoryManager {
    fn new() -> Self {
        MemoryManager {
            heap: HashMap::new(),
            stack: Vec::new(),
        }
    }

    fn allocate(&mut self, size: usize, priority: i32) -> usize {
        let address = self.heap.len();
        self.heap.insert(address, vec![0; size]);
        address
    }

    fn deallocate(&mut self, address: usize) {
        self.heap.remove(&address);
    }
}

impl PriorityOwnershipManager {
    fn new() -> Self {
        PriorityOwnershipManager {
            priorities: HashMap::new(),
            ownership_chains: HashMap::new(),
        }
    }

    fn set_priority(&mut self, name: &str, priority: Vec<i32>) {
        self.priorities.insert(name.to_string(), priority);
    }

    fn get_priority(&self, name: &str) -> Option<&Vec<i32>> {
        self.priorities.get(name)
    }
}

impl StandardLibrary {
    fn new() -> Self {
        let mut functions: HashMap<String, Box<dyn Fn(&[Box<dyn std::any::Any>]) -> Box<dyn std::any::Any>>> = HashMap::new();
        
        // 標準ライブラリ関数の登録
        functions.insert("print".to_string(), Box::new(|args: &[Box<dyn std::any::Any>]| {
            if let Some(arg) = args.get(0) {
                println!("{}", arg);
            }
            Box::new(())
        }));

        functions.insert("len".to_string(), Box::new(|args: &[Box<dyn std::any::Any>]| {
            if let Some(arg) = args.get(0) {
                if let Some(s) = arg.downcast_ref::<String>() {
                    return Box::new(s.len());
                }
            }
            Box::new(0)
        }));

        StandardLibrary { functions }
    }

    fn call_function(&self, name: &str, args: &[Box<dyn std::any::Any>]) -> Option<Box<dyn std::any::Any>> {
        if let Some(func) = self.functions.get(name) {
            Some(func(args))
        } else {
            None
        }
    }
} 