# マイクロ構文から考えるテスト
- EOFや分離子のみ(blank.mpl, saparator.mpl)
- 名前に数字を含める(プログラム名)
- キーワード全て(sample011.mpl)
- 空の文字列, 連続する文字列(sample011.mpl)
- 記号全て(sample011.mpl)
- 英字全て(upper.mpl)
- 数字全て(sample011.mpl)
- 注釈全て(sample15.mplに{}あり)

#実装から考えるテスト
- 符号なし整数の値の範囲(outofrange.mpl)
- 文字列の長さ(aaaaa.mpl)
- 一行の長さ(aaaaa.mpl)
- 表示文字てない文字コードの挙動(無視orエラー)(controlchar.mpl)
- ファイルがオープン出来ない場合のエラー(適当な名前のファイル)
- 