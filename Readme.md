# TAMAGO-03 Recorder With ALSA


## Files
- main.cpp 
  - 本体
- tamago.hpp
  - ALSA を利用してPCMデータをTAMAGOから取得するライブラリ
- riff.hpp
  - RIFF(WAV)フォーマットで書き出す用のライブラリ

## Usage
```sh
# ALSAのライブラリ x86 以外はパッケージ名が違うかも
apt install alsa arecord libasound2-dev 

# コンパイル
./make.sh 

# TAMAGOのデバイス名を調べる card2 なら hw:2
arecord -l 

# hw:2 のTAMAGOをoutput.wav に Ctrl+C で止めるまで録音
./rec -d hw:2 -o output.wav

# hw:2 のTAMAGOをoutput.wav に 10秒録音
./rec -d hw:2 -o output.wav -t 10000

# hw:2 のTAMAGOをoutput.wav に 0.5秒録音
# バッファサイズの整数倍しか録音できないので注意
./rec -d hw:2 -o output.wav -t 500 -b 100


```