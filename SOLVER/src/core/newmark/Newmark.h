// Newmark.h
// created by Kuangdai on 8-Apr-2016 
// Newmark time scheme

#pragma once
#include "global.h"
class Domain;

class Newmark {
public:
    Newmark(Domain *&domain, int reportInterval, int checkStabInterval);
    
    void solve() const;
    
    // void testStability(int maxStep) const;
    
private:
    Domain *mDomain;
    int mReportInterval;
    int mCheckStabInterval;

};