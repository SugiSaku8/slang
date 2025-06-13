# S-Lang Step-by-Step Guide

## 1. Getting Started

### 1.1 First Program
```slang
// hello.sl
fn main() {
    log!("Hello, World!");
}
```

### 1.2 Variables and Types
```slang
// variables.sl
fn main() {
    // Basic types
    let x: int = 42;
    let y: float = 3.14;
    let z: bool = true;
    let s: string = "hello";

    // Type inference
    let a = 42;        // int type
    let b = 3.14;      // float type
    let c = "world";   // string type

    log!("x: {}, y: {}, z: {}, s: {}", x, y, z, s);
}
```

## 2. Priority Ownership System

### 2.1 Log Priority
```slang
// logging.sl
fn main() {
    // Using log levels
    log!("Normal log");     // Priority: [Log, 0]
    info!("Info log");      // Priority: [Log, 1]
    debug!("Debug log");    // Priority: [Log, 2]
    warn!("Warning log");   // Priority: [Emerg, 0]
    alert!("Alert log");    // Priority: [Emerg, 1]
    error!("Error log");    // Priority: [Emerg, 2]
}
```

### 2.2 Function Priority
```slang
// function_priority.sl
Function:type:priority:1;
fn high_priority_function() {
    log!("High priority function");
}

Function:type:priority:0;
fn low_priority_function() {
    log!("Low priority function");
}

fn main() {
    high_priority_function();
    low_priority_function();
}
```

### 2.3 Memory Priority
```slang
// memory_priority.sl
fn main() {
    // Single priority
    Var:type:priority:1;
    let x = 42;

    // Nested priority
    Var:type:priority:[1, 2];
    let y = 100;

    // Mixed priority
    Var:type:priority:[5, most_high];
    let z = 200;

    log!("x: {}, y: {}, z: {}", x, y, z);
}
```

## 3. Composite Types and Data Structures

### 3.1 Arrays and Tuples
```slang
// collections.sl
fn main() {
    // Arrays
    let numbers: [int; 5] = [1, 2, 3, 4, 5];
    let first = numbers[0];
    let last = numbers[4];

    // Tuples
    let person: (string, int) = ("John", 30);
    let (name, age) = person;

    log!("First number: {}, Last number: {}", first, last);
    log!("Name: {}, Age: {}", name, age);
}
```

### 3.2 Structs
```slang
// structs.sl
struct Person {
    name: string,
    age: int,
    is_active: bool
}

fn main() {
    let person = Person {
        name: "Alice",
        age: 25,
        is_active: true
    };

    log!("Name: {}, Age: {}, Active: {}", 
         person.name, person.age, person.is_active);
}
```

### 3.3 Enums
```slang
// enums.sl
enum Status {
    Active,
    Inactive,
    Suspended(string)
}

fn main() {
    let status1 = Status::Active;
    let status2 = Status::Suspended("Maintenance".to_string());

    match status1 {
        Status::Active => log!("Active"),
        Status::Inactive => log!("Inactive"),
        Status::Suspended(reason) => log!("Suspended: {}", reason)
    }
}
```

## 4. Memory Management

### 4.1 Ownership
```slang
// ownership.sl
fn main() {
    let s1 = "hello".to_string();
    let s2 = s1;  // Ownership of s1 moves to s2
    // log!("{}", s1);  // Error: s1 is invalid

    // Borrowing
    let s3 = "world".to_string();
    print_string(&s3);  // Pass reference
    log!("{}", s3);    // Valid
}

fn print_string(s: &string) {
    log!("{}", s);
}
```

### 4.2 Priority-Based Memory Management
```slang
// priority_memory.sl
fn main() {
    // High priority memory allocation
    Var:type:priority:[1, 2];
    let important_data = "Important".to_string();

    // Low priority memory allocation
    Var:type:priority:0;
    let temp_data = "Temporary".to_string();

    // Ownership transfer
    transfer_ownership(&important_data, &temp_data, 1);
}
```

## 5. Mathematical Computations

### 5.1 Vector Operations
```slang
// vectors.sl
fn main() {
    let v1: Vector<3, float> = Vector::new([1.0, 2.0, 3.0]);
    let v2: Vector<3, float> = Vector::new([4.0, 5.0, 6.0]);

    // Vector operations
    let sum = v1 + v2;
    let dot = v1.dot(v2);
    let cross = v1.cross(v2);

    log!("Sum: {:?}, Dot: {}, Cross: {:?}", sum, dot, cross);
}
```

### 5.2 Matrix Operations
```slang
// matrices.sl
fn main() {
    let m1: Matrix<2, 2, float> = Matrix::new([
        [1.0, 2.0],
        [3.0, 4.0]
    ]);

    // Matrix operations
    let m2 = m1.transpose();
    let det = m1.determinant();
    let inv = m1.inverse();

    log!("Transpose: {:?}, Determinant: {}, Inverse: {:?}", 
         m2, det, inv);
}
```

## 6. Debugging

### 6.1 Breakpoints
```slang
// debugging.sl
fn main() {
    let x = 42;
    
    // Setting breakpoint
    breakpoint!("debugging.sl", 5);
    
    let y = x * 2;
    
    // Conditional breakpoint
    breakpoint!("debugging.sl", 9, "y > 80");
    
    log!("y: {}", y);
}
```

### 6.2 Variable Watching
```slang
// watching.sl
fn main() {
    let mut x = 0;
    
    // Watch variable
    watch!("x");
    
    for i in 0..10 {
        x += i;
        // Break on change
        watch!("x", true);
    }
    
    log!("Final x: {}", x);
}
```

### 6.3 Priority Ownership Debugging
```slang
// priority_debug.sl
fn main() {
    Var:type:priority:1;
    let x = 42;
    
    // Watch priority
    watch_priority!("x");
    
    // Change priority
    Var:type:priority:2;
    let x = 100;
    
    // Get priority chain
    let chain = get_priority_chain("x");
    log!("Priority chain: {:?}", chain);
}
```

## 7. Practical Examples

### 7.1 Simple Calculator
```slang
// calculator.sl
fn add(a: int, b: int) -> int {
    return a + b;
}

fn subtract(a: int, b: int) -> int {
    return a - b;
}

fn multiply(a: int, b: int) -> int {
    return a * b;
}

fn divide(a: int, b: int) -> Result<int, string> {
    if b == 0 {
        return Err("Division by zero".to_string());
    }
    return Ok(a / b);
}

fn main() {
    let a = 10;
    let b = 5;

    log!("Addition: {}", add(a, b));
    log!("Subtraction: {}", subtract(a, b));
    log!("Multiplication: {}", multiply(a, b));
    
    match divide(a, b) {
        Ok(result) => log!("Division: {}", result),
        Err(error) => error!("Error: {}", error)
    }
}
```

### 7.2 Priority-Based Task Management
```slang
// task_manager.sl
struct Task {
    name: string,
    priority: int,
    completed: bool
}

fn main() {
    Var:type:priority:[1, 2];
    let mut tasks: [Task; 3] = [
        Task { name: "High priority task".to_string(), priority: 2, completed: false },
        Task { name: "Medium priority task".to_string(), priority: 1, completed: false },
        Task { name: "Low priority task".to_string(), priority: 0, completed: false }
    ];

    // Process tasks
    for task in &mut tasks {
        if task.priority == 2 {
            log!("Processing high priority task: {}", task.name);
            task.completed = true;
        }
    }

    // Display completed tasks
    for task in &tasks {
        if task.completed {
            log!("Completed: {}", task.name);
        }
    }
}
```

### 7.3 Mathematical Computation Example
```slang
// math_example.sl
fn main() {
    // Vector calculations
    let v1: Vector<3, float> = Vector::new([1.0, 2.0, 3.0]);
    let v2: Vector<3, float> = Vector::new([4.0, 5.0, 6.0]);
    
    // Matrix calculations
    let m1: Matrix<2, 2, float> = Matrix::new([
        [1.0, 2.0],
        [3.0, 4.0]
    ]);
    
    // Complex number calculations
    let c1: Complex<float> = Complex::new(1.0, 2.0);
    let c2: Complex<float> = Complex::new(3.0, 4.0);
    
    // Display results
    log!("Vector sum: {:?}", v1 + v2);
    log!("Matrix determinant: {}", m1.determinant());
    log!("Complex product: {:?}", c1 * c2);
}
```

## 8. Best Practices

### 8.1 Priority Ownership Usage
```slang
// best_practices.sl
// 1. Using appropriate priority levels
Var:type:priority:[1, 2];
let important_data = "Critical data".to_string();

// 2. Avoiding priority conflicts
Function:type:priority:1;
fn process_data(data: &string) {
    // Processing
}

// 3. Documenting priority requirements
/// This function needs to be executed with high priority
/// Priority: [1, 2]
fn critical_operation() {
    // Processing
}
```

### 8.2 Memory Management
```slang
// memory_best_practices.sl
// 1. Following ownership rules
fn process_string(s: string) {
    // Take ownership of s
    log!("{}", s);
} // s is dropped here

// 2. Using appropriate borrowing
fn analyze_string(s: &string) {
    // Borrow and use s
    log!("Length: {}", s.len());
}

// 3. Resource cleanup
fn main() {
    let data = "Important data".to_string();
    process_string(data);
    // data is automatically dropped
}
```

### 8.3 Error Handling
```slang
// error_handling.sl
fn safe_divide(a: int, b: int) -> Result<int, string> {
    if b == 0 {
        return Err("Division by zero".to_string());
    }
    return Ok(a / b);
}

fn main() {
    match safe_divide(10, 0) {
        Ok(result) => log!("Result: {}", result),
        Err(error) => error!("Error: {}", error)
    }
}
``` 