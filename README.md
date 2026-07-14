# スマート図書管理システム（C++ 期末課題）

C++17とWin32 APIで開発したGUI図書在庫管理プログラムです。「コアライブラリ・GUI画面・データ保存」の三層構成を採用し、図書の登録・変更・削除・検索、貸出・返却、在庫統計、CSV形式でのデータ保存に対応しています。

## 特長

- オブジェクト指向設計：`Book`データモデルと`Library`業務サービスの責務を分離
- 厳密な業務規則：図書番号の重複、負数の在庫、総冊数を超える貸出を防止
- 柔軟な検索：図書番号の完全一致、書名・著者の部分一致に対応
- 安全な保存：空白・カンマ・引用符を含むCSVを扱い、一時ファイル経由で保存
- GUI操作：一覧から図書を選択し、ボタン操作で登録・変更・貸出・返却が可能
- CMakeに対応し、Windows用の一括ビルドスクリプトも同梱
- 自動テスト：主要機能、境界値、統計、ファイル入出力を検証
- 継続的インテグレーション：GitHub ActionsでWindowsとUbuntuのビルド・テストを自動実行

## 機能一覧

| 分類 | 機能 |
| --- | --- |
| 図書管理 | 追加、変更、削除、全件表示 |
| 検索 | 図書番号、書名、著者による検索 |
| 貸出管理 | 貸出、返却、在庫不足の検出 |
| 統計 | 書目数、総冊数、貸出中冊数、在庫冊数 |
| データ管理 | 起動時読込、手動保存、終了時保存、CSV検証 |

## 実行方法

### ダウンロード後すぐに実行する（推奨）

GitHubの「Code」からZIPをダウンロードして展開し、次のファイルをダブルクリックしてください。ビルド、MinGWのインストール、DLLの追加は不要です。

```text
release/SmartLibraryManager.exe
```

このEXEはC++ランタイムを静的リンクした配布用GUIアプリです。リポジトリ内では`data/books.csv`を読み書きし、EXEだけを別の場所へコピーした場合は同じフォルダーに`books.csv`を自動作成します。

### Windowsで一括ビルド

開発者がソースコードを変更して再ビルドする場合だけ、MinGW-w64またはMSYS2が必要です。`build_windows.bat`を実行すると、次のGUIプログラムと配布用EXEが生成されます。

```text
build/library_manager.exe
release/SmartLibraryManager.exe
```

ビルドスクリプトは旧版の`library_cli.exe`と`library_tests.exe`を自動的に削除します。利用者向けの実行ファイルは`library_manager.exe`の一つだけです。

プログラムを起動します。

```powershell
.\build\library_manager.exe
```

### CMakeを使用する場合

```bash
cmake -S . -B build
cmake --build build
```

既定ではプロジェクト内の`data/books.csv`を自動的に読み書きします。EXEだけを別の場所へコピーした場合は、EXEと同じフォルダーに`books.csv`を作成します。

## フォルダー構成

```text
.
├── include/library/       # 公開データモデルと業務インターフェース
├── src/                   # コア実装とWin32 GUI画面
├── tests/                 # GitHub Actions用の自動テスト
├── data/books.csv         # サンプルデータ
├── release/               # ビルド不要で直接実行できる配布用EXE
├── docs/DESIGN.md         # 設計および学習項目の説明
├── CMakeLists.txt         # クロスプラットフォーム用ビルド設定
└── build_windows.bat      # Windows用GUI一括ビルド
```

## データ形式

```csv
id,title,author,total,borrowed
1001,The C++ Programming Language,Bjarne Stroustrup,3,1
```

項目は順に、図書番号・書名・著者・総冊数・貸出中冊数です。不正な項目数、整数以外の値、図書番号の重複、在庫規則に反するデータは読み込み時に拒否されます。

## 学習項目

クラスとカプセル化、STLコンテナーとアルゴリズム、ファイルストリーム、例外処理、参照、定数メンバー関数、名前空間、`std::filesystem`、モジュール化、自動テストを総合的に活用しています。詳細は[設計書](docs/DESIGN.md)を参照してください。

## 初期版について

新しいGUIは業務ロジックから分離されており、画面を変更しても在庫管理やCSV保存の規則を再利用できます。自動テストはGitHub Actions内部でのみ生成・実行され、通常のWindowsビルドには含まれません。

## ライセンス

本プロジェクトはMIT Licenseで公開します。
