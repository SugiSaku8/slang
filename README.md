# S-Lang

S-Langは、C言語の低レベル制御とRustの安全性を組み合わせた、高性能で安全なプログラミング言語です。

## 特徴

- **優先所有格システム**: 実行時の優先度管理による高度なメモリ制御
- **安全なメモリ管理**: 所有権システムと借用チェッカーによる安全性の確保
- **高度な型システム**: 強力な型推論とジェネリクス
- **数学的計算サポート**: テンソル、行列、四元数などの高度な数学型
- **包括的なツールサポート**: デバッガ、フォーマッタ、シンタックスハイライト

## 必要条件

- Rust 1.70.0以上
- LLVM 15.0以上
- CMake 3.20以上

## インストール

```bash
# リポジトリのクローン
git clone https://github.com/sugisaku8/slang.git
cd slang

# ビルド
cargo build --release

# インストール
cargo install --path .
```

## クイックスタート

```slang
// hello.sl
fn main() {
    log!("Hello, World!");
}
```

```bash
# コンパイル
slangc hello.sl

# 実行
./hello
```

## プロジェクト構造

```
slang/
├── src/
│   ├── lexer/      # 字句解析器
│   ├── parser/     # 構文解析器
│   ├── ast/        # 抽象構文木
│   ├── ir/         # 中間表現
│   ├── optimizer/  # 最適化パス
│   ├── codegen/    # コード生成
│   ├── runtime/    # ランタイム
│   └── debugger/   # デバッガ
├── examples/       # サンプルコード
├── tests/         # テスト
└── doc/           # ドキュメント
```

## ドキュメント

- [仕様書 (日本語)](doc/specification_ja.md)
- [Specification (English)](doc/specification.md)
- [API Reference](doc/api.md)

## 開発への参加

1. このリポジトリをフォーク
2. 新しいブランチを作成 (`git checkout -b feature/amazing-feature`)
3. 変更をコミット (`git commit -m 'Add amazing feature'`)
4. ブランチにプッシュ (`git push origin feature/amazing-feature`)
5. プルリクエストを作成

## ライセンス

このプロジェクトはMITライセンスの下で公開されています。詳細は[LICENSE](LICENSE)ファイルを参照してください。


## 謝辞

- [Rust](https://www.rust-lang.org/) - 言語設計のインスピレーション
- [LLVM](https://llvm.org/) - コード生成バックエンド
- [Cargo](https://doc.rust-lang.org/cargo/) - ビルドシステム 