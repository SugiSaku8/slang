# S-Lang
## 主な特徴
S-Langは、C言語の低レベルな制御とRustの安全性を組み合わせた、高性能で安全なプログラミング言語です。

### 言語設計の特徴
1. **C言語とRustの統合**
   - C言語の低レベル制御（ポインタ、メモリ管理）
   - Rustの安全性（所有権、型システム）
   - 両言語の良い部分を選択的に採用

2. **独自の優先所有格システム**
   - 実行時の優先度管理
   - ログ型、関数型、メモリ型の3種類
   - 動的な優先度変更

3. **高度な型システム**
   - 強力な型推論
   - ジェネリクス
   - トレイト
   - 数学的型（Tensor、Vector、Matrix）

4. **安全なメモリ管理**
   - 所有権システム
   - 借用チェッカー
   - 安全なポインタ操作

5. **モダンな構文**
   - パターンマッチング
   - エラーハンドリング
   - マクロシステム

6. **数学的計算のサポート**
   - テンソル演算
   - 行列計算
   - 四元数
   - 複素数

7. **モジュールシステム**
   - 明確なスコープ管理
   - パブリック/プライベートの区別
   - 依存関係の管理

8. **ファイル拡張子の体系**
   - `.sl`: ソースファイル
   - `.sls`: ヘッダファイル
   - `.slu`: 特殊記法ファイル
   - `.slm`: マクロファイル
   - `.slb`: バイナリファイル

### 主な用途
- システムプログラミング
- 数値計算
- 科学技術計算
- パフォーマンスクリティカルなアプリケーション
- メモリ安全性が重要なアプリケーション

## 基本思想
### 言語仕様
C言語 + Rustのいいとこどり 
##### C言語から受け継ぐ主な思想:
- `#include`によるライブラリインポートシステム
- `;`による文末処理
- アドレスとポインタによるメモリ操作
##### Rustから受け継ぐ主な思想:
- 型推論
- マクロとメタプログラミング
- if letなどの構文(if,else,match,loop,while,forなどはもちろん)
- 所有権
- ムーブ
- コンストラクタが無い
- 関数オーバーロードが無い
- 関数のデフォルト引数が無い
##### 独自思想:
- 優先所有格
- マクロは、マクロファイルに記述される。
##### 優先所有格について
優先所有格は、実行時に優先的に保持される関数・変数を設定できるものである。
###### ログ型優先所有格
これは、`Macro:type:log`に分類される、`log`、`warn`、`alert`、`error`、`debug`、`info`に存在する優先所有格のことである。
これらのログマクロには、それぞれ優先所有が存在していて、これは`Macro:type:log:variable_priority`に存在するマクロによって動的に変更されない限り、デフォルトの値を持ち続ける。
所有格は、2次元的な構造を持つ。
`Macro:type:log:log`の型を持つマクロである`log`、`info`、`debug`は、次のような所有格情報を持つ。
```slang_macro
//info!の場合
Macro:type:log:priority:[Log,0]
//log!の場合
Macro:type:log:priority:[Log,1]
//debug!の場合
Macro:type:log:priority:[Log,2]
```
`Macro:type:log:emerg`の型を持つマクロである`warn`、`alert`、`error`は、次のような所有格情報を持つ。
```slang_macro
//warn!の場合
Macro:type:log:priority:[Emerg,0]
//alert!の場合
Macro:type:log:priority:[Emerg,1]
//error!の場合
Macro:type:log:priority:[Emerg,2]
```
###### 関数型優先所有格
これは、`Macro:type:function`に分類される、関数に設定された優先所有格のことである。
これは、`Macro:type:function:variable_priority`に存在するマクロによって動的に変更されない限り、デフォルトの値を持ち続ける。
所有格は、1次元的な構造を持つ。
構造は、次のようなものであることが多い。
```slang_macro
Function:type:priority:0;
//この場合、優先所有格は、0である。
Function:type:priority:most_low;
//この場合、優先所有格は、この時点で最も低いものが設定される。
Function:type:priority:most_high;
//この場合、優先所有格は、この時点で最も高いものが設定される。
```

###### メモリ型優先所有格
これは、`Macro:type:var`に分類される、メモリに設定された優先所有格のことである。
この優先所有格は、`Macro:type:var:variable_priority`に存在するマクロによって動的に変更されない限り、存在しない。
この所有格は、1~4までの変動的な次元を持ち、次のようにアクセスすることができる。
```
Var:type:priority:0;
//この場合、優先所有格は、0である。
Var:type:priority:most_low;
//この場合、優先所有格は、この時点で最も低いものが設定される。
Var:type:priority:most_high;
//この場合、優先所有格は、この時点で最も高いものが設定される。
Var:type:priority:[0,1];
//この場合、優先所有格は、0であり1である。
//→この場合は、0という優先所有格内で競合し、その中で1という優先所有格を持つ。
Var:type:priority:[5,1];
//この場合、優先所有格は、5であり1である。
//→この場合は、5という優先所有格内で競合し、その中で1という優先所有格を持つ。
Var:type:priority:[5,most_high];
//この場合、優先所有格は、5であり、この時点で最も高いものが設定される。
//→この場合は、5という優先所有格内で競合し、その中で最も高いものが設定される。
Var:type:priority:[most_low,most_high;];
//この場合、優先所有格は、この時点で最も低いものが設定される。
//→この場合は、この時点で最も低いものが設定される。
Var:type:priority:[most_low,most_high];
//この場合、優先所有格は、この時点で最も高いものが設定される。
//→この場合は、この時点で最も高いものが設定される。
```
######　優先所有格独自関数について
優先所有格関連の独自関数である、`most_low`、`most_high`は、次のようなものである。
```slang
most_low
//この場合、優先所有格は、この時点で最も低いものが設定される。
most_high
//この場合、優先所有格は、この時点で最も高いものが設定される。
```
これらの関数は、関数特有の記法`()`や`;`を利用しない。
このような関数は、格記法時特殊関数(lattice notation time special function)と呼ばれる。
### ファイル拡張子
- `.sl`→ソースファイル
- `.sls`→ヘッダファイル
- `.slu`→特殊記法ファイル
- `.slm`→マクロファイル
- `.slb`→バイナリファイル

## 基本文法
### 基本関数
#### 標準組み立て
- `log`
優先所有格1のログを出力する。
```slang
log!("Hello, World!");
```
- `info`
優先所有格2のログを出力する。
```slang
info!("Hello, World!");
```
- `debug`
優先所有格3のログを出力する。
```slang
debug!("Hello, World!");
```
- `warn`
優先所有2のログを出力する
- `error`
優先所有1のログを出力する
```slang
warn!("Hello, World!");
```
- `alert`
優先所有2のログを出力する
```slang
alert!("Hello, World!");
```
- `let`
変数を定義。
定義時に型を推論し、所有権を与える(Rustと同じシステム) 
```slang
let any_number = 1;
// xは有効で、xはint型として処理される。
let any_string = "Hello, World!";
// yは有効で、yはstring型として処理される。
let any_boolean = true;
// zは有効で、zはboolean型として処理される。
let any_float = 1.0;
// wは有効で、wはfloat型として処理される。
let any_char = 'a';
// vは有効で、vはchar型として処理される。
```
- `${string}`
S-langでは、${型名称}という方法で、型独自の関数を表す。
`${string}`は、string型の固有関数を表す。
`${string}`には、次のようなものが含まれる。
ほとんどRustと同じ内容。同じ内容を実装すればいい。
```slang
let any_string = "Hello, World!";
any_string.len();
any_string.to_upper();
any_string.to_lower();
any_string.split(' ');
any_string.join(' ');
any_string.replace('Hello', 'Hi');
any_string.contains('Hello');
any_string.starts_with('Hello');
any_string.ends_with('World!');
any_string.push_str("Hello, World!");
any_string.pop();
any_string.clear();
any_string.replace_range(0, 5, "Hi");
any_string.replace_range(0, 5, "Hi");   
//ここまでは、すべて動作する。
any_string.push_int(1);
//このコードは、${string}であるany_stringに、push_int関数は存在しないため、エラーが発生する。
```
- `${int}`
`${int}`は、int型の固有関数を表す。
`${int}`には、次のようなものが含まれる。
```slang
let any_number = 1;
any_number.add(1);
any_number.sub(1);
```
- `${float}`
`${float}`は、float型の固有関数を表す。
`${float}`には、次のようなものが含まれる。
```slang
let any_float = 1.0;
any_float.add(1.0);
any_float.sub(1.0);
```
- `${char}`
`${char}`は、char型の固有関数を表す。
`${char}`には、次のようなものが含まれる。
```slang
let any_char = 'a';
any_char.add('a');
any_char.sub('a');
```
- `${boolean}`
`${boolean}`は、boolean型の固有関数を表す。
`${boolean}`には、次のようなものが含まれる。
```slang
let any_boolean = true;
any_boolean.not();
any_boolean.and(true);
any_boolean.or(true);
any_boolean.xor(true);
any_boolean.not();
any_boolean.and(true);
any_boolean.or(true);
any_boolean.xor(true);
```
- `if`
条件分岐。
```slang
let any_number = 1;
if any_number == 1 {
    log!("any_number is 1");
}
```
- `else`
条件分岐のelse文。
```slang
let any_number = 1;
if any_number == 1 {
    log!("any_number is 1");
}
else {
    log!("any_number is not 1");
}
```
- `match`
条件分岐。
```slang
let any_number = 1;
match any_number {
    1 => log!("any_number is 1"),
    2 => log!("any_number is 2"),
}
```
- `while`
条件分岐。
```slang
let any_number = 1;
while any_number < 10 {
    log!("any_number is less than 10");
}
```
- `for`
条件分岐。
```slang
let any_number = 1;
for any_number in 1..10 {
    log!("any_number is less than 10");
}
```

### 型システム
#### 基本型
- `int`: 64ビット整数
- `float`: 64ビット浮動小数点数
- `bool`: 真偽値
- `char`: UTF-8文字
- `string`: UTF-8文字列
- `void`: 空型（関数の戻り値として使用）

#### 複合型
- 配列: `[T; N]` (Tは型、Nはサイズ)
```slang
let numbers: [int; 5] = [1, 2, 3, 4, 5];
```

- タプル: `(T1, T2, ...)` (T1, T2は型)
```slang
let pair: (int, string) = (1, "hello");
```

- 構造体: `struct`
```slang
struct Person {
    name: string,
    age: int,
    is_active: bool
}
```

### 関数定義
```slang
fn add(a: int, b: int) -> int {
    return a + b;
}

// 型推論を使用した場合
fn multiply(a, b) {
    return a * b;
}
```

### モジュールシステム
```slang
// モジュールの定義
mod math {
    pub fn add(a: int, b: int) -> int {
        return a + b;
    }
}

// モジュールの使用
use math::add;
let result = add(1, 2);
```

### エラーハンドリング
```slang
// Result型を使用したエラーハンドリング
fn divide(a: int, b: int) -> Result<int, string> {
    if b == 0 {
        return Err("Division by zero");
    }
    return Ok(a / b);
}

// エラーハンドリングの使用
match divide(10, 0) {
    Ok(result) => log!("Result: {}", result),
    Err(error) => error!("Error: {}", error)
}
```

### マクロ
```slang
// マクロの定義（.slmファイル内）
macro_rules! custom_log {
    ($level:expr, $msg:expr) => {
        match $level {
            "info" => info!($msg),
            "error" => error!($msg),
            _ => log!($msg)
        }
    }
}

// マクロの使用
custom_log!("info", "This is a custom log message");
```

### 所有権システム
```slang
// ムーブセマンティクス
let s1 = "hello".to_string();
let s2 = s1; // s1の所有権はs2に移動
// log!("{}", s1); // エラー: s1は無効

// 借用
fn print_string(s: &string) {
    log!("{}", s);
}

let s = "hello".to_string();
print_string(&s); // 参照を渡す
```

### ジェネリクス
```slang
// ジェネリック関数
fn swap<T>(a: &mut T, b: &mut T) {
    let temp = *a;
    *a = *b;
    *b = temp;
}

// ジェネリック構造体
struct Container<T> {
    value: T
}
```

### トレイト
```slang
// トレイトの定義
trait Printable {
    fn print(&self);
}

// トレイトの実装
impl Printable for Person {
    fn print(&self) {
        log!("Name: {}, Age: {}", self.name, self.age);
    }
}
```

### パターンマッチング
```slang
// 構造体のパターンマッチング
let person = Person { name: "John", age: 30, is_active: true };
match person {
    Person { name, age: 30, .. } => log!("Found 30-year-old {}", name),
    Person { age: 0..=18, .. } => log!("Minor"),
    _ => log!("Other")
}

// 列挙型のパターンマッチング
enum Status {
    Active,
    Inactive,
    Suspended(string)
}

let status = Status::Suspended("Maintenance".to_string());
match status {
    Status::Active => log!("Active"),
    Status::Inactive => log!("Inactive"),
    Status::Suspended(reason) => log!("Suspended: {}", reason)
}
```

### アドレスとポインタ
#### 基本概念
- `&`: 参照演算子（アドレス取得）
- `*`: 間接参照演算子（ポインタの参照先にアクセス）
- `&mut`: 可変参照演算子（可変アドレス取得）

#### ポインタ型
```slang
// 不変ポインタ
let x = 42;
let p: *const int = &x;  // 不変ポインタ
log!("Value: {}", *p);   // ポインタの参照先にアクセス

// 可変ポインタ
let mut y = 42;
let p: *mut int = &mut y;  // 可変ポインタ
*p = 100;                  // ポインタの参照先を変更
log!("New value: {}", *p);

// ヌルポインタ
let null_ptr: *const int = null;
```

#### ポインタ演算
```slang
// 配列のポインタ演算
let arr = [1, 2, 3, 4, 5];
let p: *const int = &arr[0];
let p2 = p.add(2);  // ポインタに2を加算
log!("Value at p2: {}", *p2);

// ポインタの比較
if p < p2 {
    log!("p is before p2");
}
```

#### 安全なポインタ操作
```slang
// 安全なポインタ操作のための関数
fn safe_deref<T>(ptr: *const T) -> Option<&T> {
    if ptr.is_null() {
        return None;
    }
    unsafe {
        Some(&*ptr)
    }
}

// 使用例
let x = 42;
let p: *const int = &x;
match safe_deref(p) {
    Some(value) => log!("Value: {}", value),
    None => log!("Null pointer")
}
```

#### ポインタと所有権
```slang
// ポインタと所有権の関係
struct Data {
    value: int
}

fn process_data(data: *mut Data) {
    unsafe {
        (*data).value = 100;
    }
}

let mut data = Data { value: 42 };
let p: *mut Data = &mut data;
process_data(p);
log!("New value: {}", data.value);
```

#### ポインタの安全性チェック
```slang
// ポインタの有効性チェック
fn is_valid_ptr<T>(ptr: *const T) -> bool {
    !ptr.is_null()
}

// ポインタの範囲チェック
fn is_in_range<T>(ptr: *const T, start: *const T, end: *const T) -> bool {
    ptr >= start && ptr <= end
}

// 使用例
let arr = [1, 2, 3, 4, 5];
let start = &arr[0];
let end = &arr[4];
let p = &arr[2];

if is_valid_ptr(p) && is_in_range(p, start, end) {
    log!("Pointer is valid and in range");
}
```

#### ポインタとマクロ
```slang
// ポインタ操作のためのマクロ
macro_rules! safe_ptr {
    ($ptr:expr) => {
        if $ptr.is_null() {
            error!("Null pointer dereference");
            return;
        }
        unsafe { &*$ptr }
    }
}

// 使用例
let x = 42;
let p: *const int = &x;
let value = safe_ptr!(p);
log!("Value: {}", value);
```

#### ポインタと優先所有格
```slang
// ポインタの優先所有格
let mut data = Data { value: 42 };
let p: *mut Data = &mut data;

// ポインタの優先所有格を設定
Var:type:priority:[Pointer, 1];
p.priority = 1;

// 優先所有格に基づくポインタ操作
fn process_priority_ptr(ptr: *mut Data) {
    if ptr.priority == 1 {
        unsafe {
            (*ptr).value = 100;
        }
    }
}
```

### 数学的型システム
#### 基本数学型
```slang
// ベクトル型
let v: Vector<3, float> = Vector::new([1.0, 2.0, 3.0]);
let v2: Vector<3, float> = Vector::new([4.0, 5.0, 6.0]);

// ベクトル演算
let sum = v + v2;  // ベクトル加算
let dot = v.dot(v2);  // 内積
let cross = v.cross(v2);  // 外積
let norm = v.norm();  // ノルム

// 行列型
let m: Matrix<2, 2, float> = Matrix::new([
    [1.0, 2.0],
    [3.0, 4.0]
]);

// 行列演算
let m2 = m.transpose();  // 転置
let det = m.determinant();  // 行列式
let inv = m.inverse();  // 逆行列
```

#### テンソル型
```slang
// テンソル型（任意の次元をサポート）
let t: Tensor<[2, 3, 4], float> = Tensor::new([
    [[1.0, 2.0, 3.0, 4.0],
     [5.0, 6.0, 7.0, 8.0],
     [9.0, 10.0, 11.0, 12.0]],
    [[13.0, 14.0, 15.0, 16.0],
     [17.0, 18.0, 19.0, 20.0],
     [21.0, 22.0, 23.0, 24.0]]
]);

// テンソル演算
let t2 = t.transpose([0, 2, 1]);  // 次元の並び替え
let t3 = t.reshape([6, 4]);  // 形状の変更
let t4 = t.slice([0..1, 1..2, 2..3]);  // 部分テンソルの取得
```

#### 四元数型
```slang
// 四元数型
let q: Quaternion<float> = Quaternion::new(1.0, 2.0, 3.0, 4.0);

// 四元数演算
let q2 = q.conjugate();  // 共役
let q3 = q.normalize();  // 正規化
let q4 = q * q2;  // 四元数積
```

#### 複素数型
```slang
// 複素数型
let c: Complex<float> = Complex::new(1.0, 2.0);

// 複素数演算
let c2 = c.conjugate();  // 共役
let c3 = c.abs();  // 絶対値
let c4 = c * c2;  // 複素数積
```

#### 数学的型の優先所有格
```slang
// 数学的型の優先所有格
let v: Vector<3, float> = Vector::new([1.0, 2.0, 3.0]);

// 優先所有格の設定
Var:type:priority:[Math, 1];
v.priority = 1;

// 優先所有格に基づく演算
fn process_math_type<T: MathType>(value: T) {
    if value.priority == 1 {
        // 高優先度の演算
        value.optimize();
    }
}
```

#### 数学的型のマクロ
```slang
// 数学的型のためのマクロ
macro_rules! math_op {
    ($a:expr, $b:expr, $op:tt) => {
        match ($a, $b) {
            (Vector(a), Vector(b)) => Vector(a $op b),
            (Matrix(a), Matrix(b)) => Matrix(a $op b),
            (Tensor(a), Tensor(b)) => Tensor(a $op b),
            _ => error!("Unsupported operation")
        }
    }
}

// 使用例
let v1: Vector<3, float> = Vector::new([1.0, 2.0, 3.0]);
let v2: Vector<3, float> = Vector::new([4.0, 5.0, 6.0]);
let result = math_op!(v1, v2, +);
```

#### 数学的型のトレイト
```slang
// 数学的型のトレイト
trait MathType {
    fn norm(&self) -> float;
    fn conjugate(&self) -> Self;
    fn transpose(&self) -> Self;
    fn optimize(&mut self);
}

// トレイトの実装
impl MathType for Vector<3, float> {
    fn norm(&self) -> float {
        // ノルムの計算
        self.data.iter().map(|x| x * x).sum::<float>().sqrt()
    }

    fn conjugate(&self) -> Self {
        // 共役の計算（実ベクトルの場合は自身を返す）
        self.clone()
    }

    fn transpose(&self) -> Self {
        // 転置の計算（ベクトルの場合は自身を返す）
        self.clone()
    }

    fn optimize(&mut self) {
        // 最適化処理
        self.data.iter_mut().for_each(|x| *x = x.round());
    }
}
```

#### 数学的型のエラーハンドリング
```slang
// 数学的型のエラー型
enum MathError {
    DimensionMismatch,
    SingularMatrix,
    InvalidOperation,
    OutOfRange
}

// エラーハンドリング付きの演算
fn safe_matrix_inverse(m: Matrix<2, 2, float>) -> Result<Matrix<2, 2, float>, MathError> {
    let det = m.determinant();
    if det == 0.0 {
        return Err(MathError::SingularMatrix);
    }
    Ok(m.inverse())
}

// 使用例
match safe_matrix_inverse(matrix) {
    Ok(inv) => log!("Inverse matrix: {}", inv),
    Err(MathError::SingularMatrix) => error!("Matrix is singular"),
    Err(e) => error!("Other error: {:?}", e)
}
```