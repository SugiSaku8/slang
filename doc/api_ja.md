# S-Lang APIリファレンス

## コア言語機能

### 優先所有格システム

#### ログ優先度
```slang
log!("メッセージ");    // 優先度: [Log, 0]
info!("メッセージ");   // 優先度: [Log, 1]
debug!("メッセージ");  // 優先度: [Log, 2]
warn!("メッセージ");   // 優先度: [Emerg, 0]
alert!("メッセージ");  // 優先度: [Emerg, 1]
error!("メッセージ");  // 優先度: [Emerg, 2]
```

#### 関数優先度
```slang
Function:type:priority:0;        // 優先度を0に設定
Function:type:priority:most_low; // 最低優先度に設定
Function:type:priority:most_high;// 最高優先度に設定
```

#### メモリ優先度
```slang
Var:type:priority:0;             // 単一優先度
Var:type:priority:[0,1];         // ネストされた優先度
Var:type:priority:[5,most_high]; // 混合優先度
```

### 型システム

#### 基本型
```slang
let x: int = 42;
let y: float = 3.14;
let z: bool = true;
let c: char = 'a';
let s: string = "hello";
```

#### 複合型
```slang
// 配列
let arr: [int; 5] = [1, 2, 3, 4, 5];

// タプル
let tuple: (int, string) = (42, "hello");

// 構造体
struct Person {
    name: string,
    age: int
}

// 列挙型
enum Status {
    Active,
    Inactive,
    Suspended(string)
}
```

#### 数学的型
```slang
// ベクトル
let v: Vector<3, float> = Vector::new([1.0, 2.0, 3.0]);

// 行列
let m: Matrix<2, 2, float> = Matrix::new([
    [1.0, 2.0],
    [3.0, 4.0]
]);

// テンソル
let t: Tensor<[2, 3, 4], float> = Tensor::new([...]);

// 四元数
let q: Quaternion<float> = Quaternion::new(1.0, 2.0, 3.0, 4.0);

// 複素数
let c: Complex<float> = Complex::new(1.0, 2.0);
```

### メモリ管理

#### 所有権
```slang
let s1 = "hello".to_string();
let s2 = s1; // s1の所有権がs2に移動

// 借用
fn print_string(s: &string) {
    log!("{}", s);
}
```

#### 優先度ベースのメモリ管理
```slang
// メモリ優先度の設定
Var:type:priority:[1, 2];

// 優先度付き所有権の転送
transfer_ownership(source, target, priority);
```

### 標準ライブラリ

#### コア関数
```slang
// 入出力
print!("Hello, World!");
log!("デバッグメッセージ");
error!("エラーメッセージ");

// 文字列操作
let s = "hello".to_string();
let len = s.len();
let upper = s.to_upper();
let lower = s.to_lower();

// コレクション操作
let arr = [1, 2, 3];
let len = arr.len();
let first = arr.first();
let last = arr.last();
```

#### 数学関数
```slang
// ベクトル演算
let v1 = Vector::new([1.0, 2.0, 3.0]);
let v2 = Vector::new([4.0, 5.0, 6.0]);
let sum = v1 + v2;
let dot = v1.dot(v2);
let cross = v1.cross(v2);

// 行列演算
let m1 = Matrix::new([[1.0, 2.0], [3.0, 4.0]]);
let m2 = m1.transpose();
let det = m1.determinant();
let inv = m1.inverse();
```

### デバッグ

#### ブレークポイント
```slang
// ブレークポイントの設定
breakpoint!("file.sl", 42);

// 条件付きブレークポイント
breakpoint!("file.sl", 42, "x > 10");
```

#### 変数の監視
```slang
// 変数の監視
watch!("x");

// 変更時のブレーク
watch!("x", true);
```

#### 優先所有格のデバッグ
```slang
// 優先度の変更監視
watch_priority!("x");

// 優先度チェーンの取得
let chain = get_priority_chain("x");
```

## コンパイラAPI

### 最適化パス

#### 定数畳み込み
```rust
let mut optimizer = Optimizer::new();
optimizer.constant_folding(&mut ir);
```

#### 不要コード削除
```rust
let mut optimizer = Optimizer::new();
optimizer.dead_code_elimination(&mut ir);
```

#### 優先所有格の最適化
```rust
let mut optimizer = Optimizer::new();
optimizer.optimize_priority_ownership(&mut ir);
```

### コード生成

#### LLVMバックエンド
```rust
let mut codegen = CodeGenerator::new();
codegen.generate_llvm_ir(&ir);
```

#### メモリ安全性チェック
```rust
let mut codegen = CodeGenerator::new();
codegen.generate_safety_checks(&ir);
```

## ランタイムAPI

### メモリ管理

#### 割り当て
```rust
let runtime = Runtime::new();
runtime.allocate("x".to_string(), Box::new(42));
```

#### 解放
```rust
runtime.deallocate("x");
```

### 優先所有格管理

#### 優先度の設定
```rust
runtime.set_priority("x".to_string(), 1);
```

#### 優先度の取得
```rust
let priority = runtime.get_priority("x");
```

### 標準ライブラリ統合

#### 関数呼び出し
```rust
let result = runtime.call_standard_lib("print", &[Box::new("hello")]);
```

## デバッガAPI

### ブレークポイント管理

#### ブレークポイントの追加
```rust
let debugger = Debugger::new(runtime);
debugger.add_breakpoint("file.sl".to_string(), 42, None);
```

#### ブレークポイントの削除
```rust
debugger.remove_breakpoint("file.sl", 42);
```

### 変数の監視

#### 監視の追加
```rust
debugger.watch_variable("x".to_string(), true);
```

#### 監視の削除
```rust
debugger.unwatch_variable("x");
```

### 優先所有格のデバッグ

#### 優先度監視の追加
```rust
debugger.watch_priority("x".to_string());
```

#### 優先度監視の削除
```rust
debugger.unwatch_priority("x");
```

#### 優先度情報の取得
```rust
let priority = debugger.get_priority_value("x");
``` 