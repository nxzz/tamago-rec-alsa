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

# hw:2 のTAMAGOをoutput.wav に録音
./rec -d hw:2 -o output.wav
```