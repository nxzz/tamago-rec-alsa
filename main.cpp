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
    signal(SIGINT, signal_handler);

    cmdline::parser p;
    p.add<string>("device", 'd', "PCM Device", true);
    p.add<string>("output", 'o', "Output File Name", true);
    p.add<unsigned int>("bufsize", 'b', "Record Buffer Size (milsec)", false, 1000);
    p.add<unsigned int>("recordTime", 't', "Record Time (milsec)", false, 0);
    p.parse_check(argc, argv);

    const char *recDeviceName = p.get<string>("device").c_str();
    const char *outputFileName = p.get<string>("output").c_str();
    unsigned int recBufSize = p.get<unsigned int>("bufsize");
    unsigned int recordTime = p.get<unsigned int>("recordTime");

    // ファイルがないか確認
    if (checkFileExistence(outputFileName)) {
        throw std::runtime_error("file exist");
    };

    // 1秒ごとに取り込む
    Tamago egg(recDeviceName, recBufSize);

    // ./rec.wav に 8ch 16khz/24bit で書き込む
    RIFF wav(outputFileName, 8, 24, 16000);

    egg.getBuffer([&](unsigned int readCount, unsigned long long int readLength, char *buffer, int readBuffFrames) {
        time_t t = time(0);
        if (readBuffFrames < 0) {
            // error 出たら中止
            cout << t << ",error," << snd_strerror(readBuffFrames / 24) << endl;
            return false;
        }
        wav.write(buffer, readBuffFrames);
        if ((recordTime != 0 && readCount * recBufSize > recordTime) || stopFlag != 0) {
            return false;
        } else {
            cout << t << "," << readCount * recBufSize << "," << readLength << "," << readBuffFrames << endl;
            return true;
        }
    });

    return 0;
}