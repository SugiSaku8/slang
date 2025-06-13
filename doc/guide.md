# S-Lang ステップバイステップガイド

## 1. はじめに

### 1.1 最初のプログラム
```slang
// hello.sl
fn main() {
    log!("Hello, World!");
}
```

### 1.2 変数と型
```slang
// variables.sl
fn main() {
    // 基本型
    let x: int = 42;
    let y: float = 3.14;
    let z: bool = true;
    let s: string = "hello";

    // 型推論
    let a = 42;        // int型
    let b = 3.14;      // float型
    let c = "world";   // string型

    log!("x: {}, y: {}, z: {}, s: {}", x, y, z, s);
}
```

## 2. 優先所有格システム

### 2.1 ログ優先度
```slang
// logging.sl
fn main() {
    // ログレベルの使用
    log!("通常のログ");     // 優先度: [Log, 0]
    info!("情報ログ");     // 優先度: [Log, 1]
    debug!("デバッグログ"); // 優先度: [Log, 2]
    warn!("警告ログ");     // 優先度: [Emerg, 0]
    alert!("注意ログ");    // 優先度: [Emerg, 1]
    error!("エラーログ");  // 優先度: [Emerg, 2]
}
```

### 2.2 関数優先度
```slang
// function_priority.sl
Function:type:priority:1;
fn high_priority_function() {
    log!("高優先度関数");
}

Function:type:priority:0;
fn low_priority_function() {
    log!("低優先度関数");
}

fn main() {
    high_priority_function();
    low_priority_function();
}
```

### 2.3 メモリ優先度
```slang
// memory_priority.sl
fn main() {
    // 単一優先度
    Var:type:priority:1;
    let x = 42;

    // ネストされた優先度
    Var:type:priority:[1, 2];
    let y = 100;

    // 混合優先度
    Var:type:priority:[5, most_high];
    let z = 200;

    log!("x: {}, y: {}, z: {}", x, y, z);
}
```

## 3. 複合型とデータ構造

### 3.1 配列とタプル
```slang
// collections.sl
fn main() {
    // 配列
    let numbers: [int; 5] = [1, 2, 3, 4, 5];
    let first = numbers[0];
    let last = numbers[4];

    // タプル
    let person: (string, int) = ("John", 30);
    let (name, age) = person;

    log!("First number: {}, Last number: {}", first, last);
    log!("Name: {}, Age: {}", name, age);
}
```

### 3.2 構造体
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

### 3.3 列挙型
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

## 4. メモリ管理

### 4.1 所有権
```slang
// ownership.sl
fn main() {
    let s1 = "hello".to_string();
    let s2 = s1;  // s1の所有権がs2に移動
    // log!("{}", s1);  // エラー: s1は無効

    // 借用
    let s3 = "world".to_string();
    print_string(&s3);  // 参照を渡す
    log!("{}", s3);    // 有効
}

fn print_string(s: &string) {
    log!("{}", s);
}
```

### 4.2 優先度ベースのメモリ管理
```slang
// priority_memory.sl
fn main() {
    // 高優先度のメモリ割り当て
    Var:type:priority:[1, 2];
    let important_data = "Important".to_string();

    // 低優先度のメモリ割り当て
    Var:type:priority:0;
    let temp_data = "Temporary".to_string();

    // 所有権の転送
    transfer_ownership(&important_data, &temp_data, 1);
}
```

## 5. 数学的計算

### 5.1 ベクトル演算
```slang
// vectors.sl
fn main() {
    let v1: Vector<3, float> = Vector::new([1.0, 2.0, 3.0]);
    let v2: Vector<3, float> = Vector::new([4.0, 5.0, 6.0]);

    // ベクトル演算
    let sum = v1 + v2;
    let dot = v1.dot(v2);
    let cross = v1.cross(v2);

    log!("Sum: {:?}, Dot: {}, Cross: {:?}", sum, dot, cross);
}
```

### 5.2 行列演算
```slang
// matrices.sl
fn main() {
    let m1: Matrix<2, 2, float> = Matrix::new([
        [1.0, 2.0],
        [3.0, 4.0]
    ]);

    // 行列演算
    let m2 = m1.transpose();
    let det = m1.determinant();
    let inv = m1.inverse();

    log!("Transpose: {:?}, Determinant: {}, Inverse: {:?}", 
         m2, det, inv);
}
```

## 6. デバッグ

### 6.1 ブレークポイント
```slang
// debugging.sl
fn main() {
    let x = 42;
    
    // ブレークポイントの設定
    breakpoint!("debugging.sl", 5);
    
    let y = x * 2;
    
    // 条件付きブレークポイント
    breakpoint!("debugging.sl", 9, "y > 80");
    
    log!("y: {}", y);
}
```

### 6.2 変数の監視
```slang
// watching.sl
fn main() {
    let mut x = 0;
    
    // 変数の監視
    watch!("x");
    
    for i in 0..10 {
        x += i;
        // 変更時にブレーク
        watch!("x", true);
    }
    
    log!("Final x: {}", x);
}
```

### 6.3 優先所有格のデバッグ
```slang
// priority_debug.sl
fn main() {
    Var:type:priority:1;
    let x = 42;
    
    // 優先度の監視
    watch_priority!("x");
    
    // 優先度の変更
    Var:type:priority:2;
    let x = 100;
    
    // 優先度チェーンの取得
    let chain = get_priority_chain("x");
    log!("Priority chain: {:?}", chain);
}
```

## 7. 実践的な例

### 7.1 簡単な計算機
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

### 7.2 優先度付きタスク管理
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

    // タスクの処理
    for task in &mut tasks {
        if task.priority == 2 {
            log!("Processing high priority task: {}", task.name);
            task.completed = true;
        }
    }

    // 完了したタスクの表示
    for task in &tasks {
        if task.completed {
            log!("Completed: {}", task.name);
        }
    }
}
```

### 7.3 数学的計算の例
```slang
// math_example.sl
fn main() {
    // ベクトル計算
    let v1: Vector<3, float> = Vector::new([1.0, 2.0, 3.0]);
    let v2: Vector<3, float> = Vector::new([4.0, 5.0, 6.0]);
    
    // 行列計算
    let m1: Matrix<2, 2, float> = Matrix::new([
        [1.0, 2.0],
        [3.0, 4.0]
    ]);
    
    // 複素数計算
    let c1: Complex<float> = Complex::new(1.0, 2.0);
    let c2: Complex<float> = Complex::new(3.0, 4.0);
    
    // 結果の表示
    log!("Vector sum: {:?}", v1 + v2);
    log!("Matrix determinant: {}", m1.determinant());
    log!("Complex product: {:?}", c1 * c2);
}
```

## 8. ベストプラクティス

### 8.1 優先所有格の使用
```slang
// best_practices.sl
// 1. 適切な優先度レベルの使用
Var:type:priority:[1, 2];
let important_data = "Critical data".to_string();

// 2. 優先度の競合を避ける
Function:type:priority:1;
fn process_data(data: &string) {
    // 処理
}

// 3. 優先度要件の文書化
/// この関数は高優先度で実行される必要があります
/// 優先度: [1, 2]
fn critical_operation() {
    // 処理
}
```

### 8.2 メモリ管理
```slang
// memory_best_practices.sl
// 1. 所有権ルールの遵守
fn process_string(s: string) {
    // sの所有権を受け取る
    log!("{}", s);
} // sはここで解放される

// 2. 適切な借用の使用
fn analyze_string(s: &string) {
    // sを借用して使用
    log!("Length: {}", s.len());
}

// 3. リソースのクリーンアップ
fn main() {
    let data = "Important data".to_string();
    process_string(data);
    // dataは自動的に解放される
}
```

### 8.3 エラーハンドリング
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