#include "./lib/cmdline.h"
#include "./lib/riff.hpp"
#include "./lib/tamago.hpp"
#include <csignal>
#include <fstream>
#include <functional>
#include <iostream>

using namespace std;

sig_atomic_t stopFlag = 0;
void signal_handler(int signal)
{
    stopFlag = 1;
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

    // 1秒ごとに取り込む
    Tamago egg(recDeviceName, recBufSize);

    // ./rec.wav に 8ch 16khz/24bit で書き込む
    RIFF wav(outputFileName, 8, 24, 16000);

    egg.getBuffer([&](unsigned int timeStart, unsigned int timeEnd, char *buffer, unsigned int bufferSize) {
        wav.write(buffer, bufferSize);
        if ((recordTime != 0 && timeEnd > recordTime) || stopFlag != 0) {
            return false;
        } else {
            cout << timeEnd << endl;
            return true;
        }
    });

    return 0;
}