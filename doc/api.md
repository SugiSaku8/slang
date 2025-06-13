# S-Lang API Reference

## Core Language Features

### Priority Ownership System

#### Log Priority
```slang
log!("message");    // Priority: [Log, 0]
info!("message");   // Priority: [Log, 1]
debug!("message");  // Priority: [Log, 2]
warn!("message");   // Priority: [Emerg, 0]
alert!("message");  // Priority: [Emerg, 1]
error!("message");  // Priority: [Emerg, 2]
```

#### Function Priority
```slang
Function:type:priority:0;        // Set priority to 0
Function:type:priority:most_low; // Set to lowest priority
Function:type:priority:most_high;// Set to highest priority
```

#### Memory Priority
```slang
Var:type:priority:0;             // Single priority
Var:type:priority:[0,1];         // Nested priority
Var:type:priority:[5,most_high]; // Mixed priority
```

### Type System

#### Basic Types
```slang
let x: int = 42;
let y: float = 3.14;
let z: bool = true;
let c: char = 'a';
let s: string = "hello";
```

#### Composite Types
```slang
// Arrays
let arr: [int; 5] = [1, 2, 3, 4, 5];

// Tuples
let tuple: (int, string) = (42, "hello");

// Structs
struct Person {
    name: string,
    age: int
}

// Enums
enum Status {
    Active,
    Inactive,
    Suspended(string)
}
```

#### Mathematical Types
```slang
// Vectors
let v: Vector<3, float> = Vector::new([1.0, 2.0, 3.0]);

// Matrices
let m: Matrix<2, 2, float> = Matrix::new([
    [1.0, 2.0],
    [3.0, 4.0]
]);

// Tensors
let t: Tensor<[2, 3, 4], float> = Tensor::new([...]);

// Quaternions
let q: Quaternion<float> = Quaternion::new(1.0, 2.0, 3.0, 4.0);

// Complex numbers
let c: Complex<float> = Complex::new(1.0, 2.0);
```

### Memory Management

#### Ownership
```slang
let s1 = "hello".to_string();
let s2 = s1; // s1's ownership moves to s2

// Borrowing
fn print_string(s: &string) {
    log!("{}", s);
}
```

#### Priority-based Memory Management
```slang
// Set memory priority
Var:type:priority:[1, 2];

// Transfer ownership with priority
transfer_ownership(source, target, priority);
```

### Standard Library

#### Core Functions
```slang
// I/O
print!("Hello, World!");
log!("Debug message");
error!("Error message");

// String operations
let s = "hello".to_string();
let len = s.len();
let upper = s.to_upper();
let lower = s.to_lower();

// Collection operations
let arr = [1, 2, 3];
let len = arr.len();
let first = arr.first();
let last = arr.last();
```

#### Mathematical Functions
```slang
// Vector operations
let v1 = Vector::new([1.0, 2.0, 3.0]);
let v2 = Vector::new([4.0, 5.0, 6.0]);
let sum = v1 + v2;
let dot = v1.dot(v2);
let cross = v1.cross(v2);

// Matrix operations
let m1 = Matrix::new([[1.0, 2.0], [3.0, 4.0]]);
let m2 = m1.transpose();
let det = m1.determinant();
let inv = m1.inverse();
```

### Debugging

#### Breakpoints
```slang
// Set breakpoint
breakpoint!("file.sl", 42);

// Conditional breakpoint
breakpoint!("file.sl", 42, "x > 10");
```

#### Variable Watching
```slang
// Watch variable
watch!("x");

// Watch with break on change
watch!("x", true);
```

#### Priority Ownership Debugging
```slang
// Watch priority changes
watch_priority!("x");

// Get priority chain
let chain = get_priority_chain("x");
```

## Compiler API

### Optimization Passes

#### Constant Folding
```rust
let mut optimizer = Optimizer::new();
optimizer.constant_folding(&mut ir);
```

#### Dead Code Elimination
```rust
let mut optimizer = Optimizer::new();
optimizer.dead_code_elimination(&mut ir);
```

#### Priority Ownership Optimization
```rust
let mut optimizer = Optimizer::new();
optimizer.optimize_priority_ownership(&mut ir);
```

### Code Generation

#### LLVM Backend
```rust
let mut codegen = CodeGenerator::new();
codegen.generate_llvm_ir(&ir);
```

#### Memory Safety Checks
```rust
let mut codegen = CodeGenerator::new();
codegen.generate_safety_checks(&ir);
```

## Runtime API

### Memory Management

#### Allocation
```rust
let runtime = Runtime::new();
runtime.allocate("x".to_string(), Box::new(42));
```

#### Deallocation
```rust
runtime.deallocate("x");
```

### Priority Ownership Management

#### Setting Priority
```rust
runtime.set_priority("x".to_string(), 1);
```

#### Getting Priority
```rust
let priority = runtime.get_priority("x");
```

### Standard Library Integration

#### Function Calls
```rust
let result = runtime.call_standard_lib("print", &[Box::new("hello")]);
```

## Debugger API

### Breakpoint Management

#### Adding Breakpoints
```rust
let debugger = Debugger::new(runtime);
debugger.add_breakpoint("file.sl".to_string(), 42, None);
```

#### Removing Breakpoints
```rust
debugger.remove_breakpoint("file.sl", 42);
```

### Variable Watching

#### Adding Watches
```rust
debugger.watch_variable("x".to_string(), true);
```

#### Removing Watches
```rust
debugger.unwatch_variable("x");
```

### Priority Ownership Debugging

#### Adding Priority Watches
```rust
debugger.watch_priority("x".to_string());
```

#### Removing Priority Watches
```rust
debugger.unwatch_priority("x");
```

#### Getting Priority Information
```rust
let priority = debugger.get_priority_value("x");
``` 