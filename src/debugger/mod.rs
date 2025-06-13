use std::collections::{HashMap, HashSet};
use std::sync::{Arc, Mutex};
use crate::runtime::Runtime;

pub struct Debugger {
    runtime: Arc<Runtime>,
    breakpoints: Arc<Mutex<HashSet<Breakpoint>>>,
    watched_variables: Arc<Mutex<HashMap<String, WatchConfig>>>,
    priority_watches: Arc<Mutex<HashSet<String>>>,
}

#[derive(Clone, Hash, Eq, PartialEq)]
pub struct Breakpoint {
    file: String,
    line: u32,
    condition: Option<String>,
}

pub struct WatchConfig {
    last_value: String,
    break_on_change: bool,
}

impl Debugger {
    pub fn new(runtime: Arc<Runtime>) -> Self {
        Self {
            runtime,
            breakpoints: Arc::new(Mutex::new(HashSet::new())),
            watched_variables: Arc::new(Mutex::new(HashMap::new())),
            priority_watches: Arc::new(Mutex::new(HashSet::new())),
        }
    }

    pub fn add_breakpoint(&self, file: String, line: u32, condition: Option<String>) {
        let mut breakpoints = self.breakpoints.lock().unwrap();
        breakpoints.insert(Breakpoint {
            file,
            line,
            condition,
        });
    }

    pub fn remove_breakpoint(&self, file: &str, line: u32) {
        let mut breakpoints = self.breakpoints.lock().unwrap();
        breakpoints.retain(|bp| !(bp.file == file && bp.line == line));
    }

    pub fn watch_variable(&self, name: String, break_on_change: bool) {
        let mut watched = self.watched_variables.lock().unwrap();
        watched.insert(name.clone(), WatchConfig {
            last_value: String::new(),
            break_on_change,
        });
    }

    pub fn unwatch_variable(&self, name: &str) {
        let mut watched = self.watched_variables.lock().unwrap();
        watched.remove(name);
    }

    pub fn watch_priority(&self, name: String) {
        let mut priority_watches = self.priority_watches.lock().unwrap();
        priority_watches.insert(name);
    }

    pub fn unwatch_priority(&self, name: &str) {
        let mut priority_watches = self.priority_watches.lock().unwrap();
        priority_watches.remove(name);
    }

    pub fn check_breakpoint(&self, file: &str, line: u32) -> bool {
        let breakpoints = self.breakpoints.lock().unwrap();
        breakpoints.iter().any(|bp| {
            bp.file == file && bp.line == line && {
                if let Some(condition) = &bp.condition {
                    self.evaluate_condition(condition)
                } else {
                    true
                }
            }
        })
    }

    pub fn check_variable_change(&self, name: &str, new_value: &str) -> bool {
        let mut watched = self.watched_variables.lock().unwrap();
        if let Some(config) = watched.get_mut(name) {
            let changed = config.last_value != new_value;
            if changed && config.break_on_change {
                config.last_value = new_value.to_string();
                return true;
            }
            config.last_value = new_value.to_string();
        }
        false
    }

    pub fn check_priority_change(&self, name: &str, new_priority: u32) -> bool {
        let priority_watches = self.priority_watches.lock().unwrap();
        if priority_watches.contains(name) {
            if let Some(old_priority) = self.runtime.get_priority(name) {
                return old_priority != new_priority;
            }
        }
        false
    }

    fn evaluate_condition(&self, condition: &str) -> bool {
        // Simple condition evaluation for now
        // In a real implementation, this would parse and evaluate the condition
        true
    }

    pub fn get_variable_value(&self, name: &str) -> Option<String> {
        let watched = self.watched_variables.lock().unwrap();
        watched.get(name).map(|config| config.last_value.clone())
    }

    pub fn get_priority_value(&self, name: &str) -> Option<u32> {
        self.runtime.get_priority(name)
    }

    pub fn list_breakpoints(&self) -> Vec<Breakpoint> {
        let breakpoints = self.breakpoints.lock().unwrap();
        breakpoints.iter().cloned().collect()
    }

    pub fn list_watched_variables(&self) -> Vec<(String, WatchConfig)> {
        let watched = self.watched_variables.lock().unwrap();
        watched.iter().map(|(k, v)| (k.clone(), v.clone())).collect()
    }

    pub fn list_priority_watches(&self) -> Vec<String> {
        let priority_watches = self.priority_watches.lock().unwrap();
        priority_watches.iter().cloned().collect()
    }
} 