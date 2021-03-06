// XTimer.h
// created by Kuangdai on 5-Nov-2016 
// Recursive timer

#pragma once

#include <vector>
#include <fstream>
#include <chrono>

class MyBoostTimer {
public:
    void stop();
    void resume();
    void clear();
    double elapsed(); // in seconds
    void start() {clear(); resume();};
    
    bool running() {return mTimePoints.size() % 2 == 1;};
    static double getClockResolution();
    
private:
    std::vector<std::chrono::high_resolution_clock::time_point> mTimePoints;
};


class XTimer {
public:
    static void initialize(std::string fileName, int nLevels);
    static void finalize();
    static void begin(std::string procName, int level, bool barrier = false);
    static void end(std::string procName, int level, bool barrier = true);
    static void enable() {openFile(); mEnabled = true;};
    static void disable() {mEnabled = false;};
    static void pause(int level);
    static void resume(int level);
    
private:
    static void openFile();
    static std::string mFileName;
    static std::vector<MyBoostTimer> mTimers;
    static std::fstream mFile; 
    static bool mEnabled;
};

