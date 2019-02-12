# 使い方

みなさんwavファイルを読み込みたくなる時、ありますよね。
そんな時にいちいち自分でファイルを解析するプログラムを書くのは面倒だと思いませんか？この`WavFileReader`はあなたをその煩わしさから解放します。単にPCMデータを読み込むだけでいいのならね。本記事ではその使い方をざっくり説明します。ただし対応しているPCMの形式は8bitと16bitのみであり、24bitには対応していないことにご注意ください。
<br/>
<br/>

# 1. ソースを追加してヘッダをインクルードする
githubからWavFileReaderのリポジトリをcloneもしくはダウンロードします。
https://github.com/GoldSakana/WavFileReader

その後`src`ディレクトリ内のファイル（２つしかないけど）をあなたのプロジェクトに追加して、このクラスを使用したいファイルの先頭で`wav_file_reader.h`をインクルードしてください。

```C++
#include"wav_file_reader.h"
```
　　
# 2. `WavFileReader`のインスタンスを作る

```C++
	gold::WavFileReader wfr("test.wav");
```
作ってください。名前空間`gold`をお忘れなく。読み込むファイルもここで指定してください。
　　
# 3. `Read()`関数でPCMデータを読み込む
`int Read(unsigned char *buf, unsigned int count)`
読み込むPCMデータを格納する配列を用意してください。その配列の先頭ポインタを第一引数に渡し、第二引数には読み込みたいデータの個数を渡します。

これで配列には第二引数で指定された個数ぶんのPCMデータが格納されます。C言語の`fread()`関数のような感じです。次のデータが読み込みたければ`fread()`と同様にもう一度`Read()`関数を呼べば次のデータが格納されます。

```C++
	unsigned char buf[44100];
	wfr.Read(buf, 44100);//最初の44100サンプルを読み込み
	wfr.Read(buf, 44100);//次の44100サンプルを読み込み
```

ここで注意して頂きたいのはWAVファイルのフォーマットがステレオの場合は、左右のデータが自動的に平均された値が配列に格納されるということです。左右のデータを別々に取得したい場合は後述する`ReadLR()`関数を使用してください。

`Read()`関数には配列ポインタとして`unsigned char*`以外のデータ型を受け取れるオーバーロードが複数あります。具体的には`signed short*`, `int*`, `double*`, `float*`型が受け取れます。

基本的に読み込んだ値は加工されずにそのまま配列に格納されます。しかし読み込み先が`16bit`で受け取り先が`8bit`の場合のみ例外で、そのままでは溢れてしまうので値は自動的に`8bit`に変換されて格納されます。
　　
# 4. コード全体
以上が最も基本的な使い方です。コードの全体図を以下に示します。

```C++
#include"wav_file_reader.h"

int main(void) {

	gold::WavFileReader wfr("test.wav");
	unsigned char buf[44100];
	
	wfr.Read(buf, 44100);//最初の44100サンプルを読み込み
	wfr.Read(buf, 44100);//次の44100サンプルを読み込み

	return 0;
}
```
簡単ですね。
　　
# 5. おまけ
細かい機能の説明をします。
<br/>

* __`WavFileReader(const char* filename, unsigned int gpBufCnt)`コンストラクタ__  
   
パフォーマンスを最大限に発揮したいという方は、以下のように第二引数に自分が後にRead()関数の第二引数として使用する値と同じ値またはそれより大きな値を指定してください。これはオブジェクト内部で確保されるメモリの大きさに関係します。なので無駄に大きすぎるのも良くないでしょう。

```C++
gold::WavFileReader wfr("test.wav",44100);
```
また、このコンストラクタはファイルが存在しないなどの例外の場合に`wav_file_reader.h`内で定義されている`WFRException`オブジェクトをスローします。
<br/>
<br/>

* __`int Read(unsigned char *buf, unsigned int count)`関数__

上でだいたい説明しました。戻り値は`fread()`関数と同じく読み込みに成功したサンプル数です。通常は第二引数と同じ値が返ります。
<br />
<br />

* __`int ReadLR(unsigned char *bufL, unsigned char *bufR, unsigned int count)`関数__

`Read()`関数とだいたい同じですが、違う点は配列のポインタを2つ受け取るところです。`bufL`に左のPCMデータ、`bufR`に右のPCMデータがそれぞれ`count`個ずつ格納されます。読み込み先がモノラルデータの場合は2つの配列には同じ値が格納されます。この関数にも違う型を扱えるオーバーロードがあります。扱える型の種類は先程上で挙げた`Read()`関数と同様です。

戻り値は読み込みに成功したサンプル数です。左右のセットで1つのサンプル数とカウントします。通常は第3引数と同じ値です。

```C++
	unsigned char bufL[1000];
	unsigned char bufR[1000];

	wfr.ReadLR(bufL, bufR, 1000);
```
<br/>
<br />

* __`int Seek(long offset, int origin)`関数__

C言語の`fseek()`関数とだいたい同じですが、`offset`に指定する値は移動するデータサイズではなくサンプル数です。読み込み先がステレオの場合は`offset`*2サンプル先に進むことになります。

`wav_file_reader`ヘッダ内で`cstdio`をインクルードしているので`origin`には`SEEK_CUR`、`SEEK_SET`、`SEEK_END`が使えます。

戻り値は成功で0、失敗で0以外です。

```C++
	wfr.Seek(5000, SEEK_CUR);
```
<br/>
<br/>

* __WAVフォーマット情報__

インスタンス変数により取得可能です。

```C++
	int v1 = wfr.NumChannels
	int v2 = wfr.SampleRate
	int v3 = wfr.BitsPerSample
```
　　
# 6. 最後に
WavFileReaderは私が自分用に作ったものですが、便利なので他の人にも使ってもらえればという思いでコードを整えて公開致しました。正直言ってgithubに上げるのもQiitaに投稿するのも初めてです。わりと手探りなので至らぬ点があればご容赦願いたいです。改善点などあればご指摘頂ければ幸いです。
