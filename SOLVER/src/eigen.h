// eigen.h
// created by Kuangdai on 14-Aug-2016 
// Eigen3 header

#pragma once

// no dynamic allocation once time loop begins
#ifndef NDEBUG
    #define EIGEN_RUNTIME_NO_MALLOC
#endif    

// disable static alignment to enhance code portability
// the extra cost is marginal unless one uses nPol=3
#define EIGEN_DONT_ALIGN_STATICALLY

#include <Eigen/Dense>

const Eigen::IOFormat EIGEN_FMT(Eigen::StreamPrecision, 0, " ", "\n", " ", "", "", "");

