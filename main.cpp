// Copyright [2020] [Rimpei Kunimoto]

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

// 	http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "./lib/cmdline.h"
#include "./lib/riff.hpp"
#include "./lib/tamago.hpp"
#include <csignal>
#include <ctime>
#include <fstream>
#include <functional>
#include <iostream>

using namespace std;

sig_atomic_t stopFlag = 0;
void signal_handler(int signal)
{
    stopFlag = 1;
}

bool checkFileExistence(const std::string &str)
{
    std::ifstream ifs(str);
    return ifs.is_open();
}

int main(int argc, char *argv[])
{
    // シグナルハンドラ
    signal(SIGINT, signal_handler);

    // コマンドパーサ
    cmdline::parser p;
    p.add<string>("device", 'd', "PCM Device", true);
    p.add<string>("output", 'o', "Output File Name", true);
    p.add<string>("logOut", 'l', "Log File Name (if this option is not present, log output to strout)", false, "");
    p.add<unsigned int>("bufsize", 'b', "Record Buffer Size (milsec)", false, 1000);
    p.add<unsigned int>("recordTime", 't', "Record Time (milsec)", false, 0);
    p.parse_check(argc, argv);

    const char *recDeviceName = p.get<string>("device").c_str();
    const char *outputFileName = p.get<string>("output").c_str();
    string logFileName = p.get<string>("logOut");
    unsigned int recBufSize = p.get<unsigned int>("bufsize");
    unsigned int recordTime = p.get<unsigned int>("recordTime");

    // ファイルがないか確認
    if (checkFileExistence(outputFileName) || (logFileName != "" && checkFileExistence(logFileName))) {
        throw std::runtime_error("file exist");
    };

    // ログファイル名が指定されている場合、coutをログファイルにリダイレクトする
    ofstream ofs;
    if (logFileName != "") {
        ofs.open(logFileName);
        cout.rdbuf(ofs.rdbuf());
    }

    // 1秒ごとにタマゴから取り込む
    Tamago egg(recDeviceName, recBufSize);

    // ./rec.wav に 8ch 16khz/24bit で書き込む
    RIFF wav(outputFileName, 8, 24, 16000);

    // バッファにたまるたびに呼ばれるコールバック関数
    egg.getBuffer([&](unsigned int readCount, unsigned long long int readLength, char *buffer, int readBuffFrames) {
        time_t t = time(0);

        // Ctrl + C で止められるか、指定時間分のデータを読んだら停止
        if ((recordTime != 0 && readCount * recBufSize > recordTime) || stopFlag != 0) {
            return false;
        } else {
            // error 出たら記録してリトライ
            if (readBuffFrames < 0) {
                cout << t << ",error," << snd_strerror(readBuffFrames) << endl;
                return true;
            }
            // 問題なければ書きこむ
            wav.write(buffer, readBuffFrames);
            cout << t << "," << readCount * recBufSize << "," << readLength << "," << readBuffFrames << endl;
            return true;
        }
    });

    if (logFileName != "") {
        ofs.close();
    }

    return 0;
}