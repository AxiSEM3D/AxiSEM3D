// SolidFluidPoint.cpp
// created by Kuangdai on 5-Apr-2016 
// solid-fluid boundary condition

#include "SolidFluidPoint.h"
#include "SolidPoint.h"
#include "FluidPoint.h"
#include "SFCoupling.h"
#include "Mass.h"
#include "XTimer.h"

SolidFluidPoint::SolidFluidPoint(SolidPoint *sp, FluidPoint *fp, SFCoupling *couple): 
mSolidPoint(sp), mFluidPoint(fp), mSFCoupling(couple),
Point(sp->getNr(), sp->axial(), sp->getCoords()) {
    mSFCoupling->checkCompatibility(mNr);
    if (sp->getNr() != fp->getNr())
        throw std::runtime_error("SolidFluidPoint::SolidFluidPoint || Incompatible size.");
}

SolidFluidPoint::~SolidFluidPoint() {
    delete mSolidPoint;
    delete mFluidPoint;
    delete mSFCoupling;
}

void SolidFluidPoint::updateNewmark(Real dt) {
    mSolidPoint->updateNewmark(dt);
    mFluidPoint->updateNewmark(dt);
}

bool SolidFluidPoint::stable() const {
    return mSolidPoint->stable() && mFluidPoint->stable();
}

void SolidFluidPoint::resetZero() {
    mSolidPoint->resetZero();
    mFluidPoint->resetZero();
}

void SolidFluidPoint::randomDispl(Real factor, int seed) {
    mSolidPoint->randomDispl(factor, seed);
    mFluidPoint->randomDispl(factor, seed);
}

void SolidFluidPoint::randomStiff(Real factor, int seed) {
    mSolidPoint->randomStiff(factor, seed);
    mFluidPoint->randomStiff(factor, seed);
}

std::string SolidFluidPoint::verbose() const {
    return "SolidFluidPoint$" + mSolidPoint->mMass->verbose() 
        + "$" + mFluidPoint->mMass->verbose() + "$" + mSFCoupling->verbose();
} 

double SolidFluidPoint::measure(int count) {
    double cost = 0.;
    // solid
    cost += mSolidPoint->measure(count);
    // fluid
    cost += mFluidPoint->measure(count);
    // coupling
    cost += measureCoupling(count);
    return cost;
}

void SolidFluidPoint::test() {
    mSolidPoint->test();
    mFluidPoint->test();    
}

int SolidFluidPoint::sizeComm() const {
    return mSolidPoint->sizeComm() + mFluidPoint->sizeComm();
}

void SolidFluidPoint::feedBuffer(CColX &buffer, int &row) {
    mSolidPoint->feedBuffer(buffer, row);
    mFluidPoint->feedBuffer(buffer, row);
}

void SolidFluidPoint::extractBuffer(CColX &buffer, int &row) {
    mSolidPoint->extractBuffer(buffer, row);
    mFluidPoint->extractBuffer(buffer, row);
}

void SolidFluidPoint::scatterDisplToElement(vec_ar3_CMatPP &displ, int ipol, int jpol, int maxNu) const {
    mSolidPoint->scatterDisplToElement(displ, ipol, jpol, maxNu);
}

void SolidFluidPoint::scatterDisplToElement(vec_CMatPP &displ, int ipol, int jpol, int maxNu) const {
    mFluidPoint->scatterDisplToElement(displ, ipol, jpol, maxNu);
}

void SolidFluidPoint::gatherStiffFromElement(const vec_ar3_CMatPP &stiff, int ipol, int jpol) {
    mSolidPoint->gatherStiffFromElement(stiff, ipol, jpol);
}

void SolidFluidPoint::gatherStiffFromElement(const vec_CMatPP &stiff, int ipol, int jpol) {
    mFluidPoint->gatherStiffFromElement(stiff, ipol, jpol);
}

void SolidFluidPoint::addToStiff(const CMatX3 &source) {
    mSolidPoint->addToStiff(source);
}

void SolidFluidPoint::coupleSolidFluid() {
    // this order matters!
    mSFCoupling->coupleSolidToFluid(mSolidPoint->mDispl, mFluidPoint->mStiff);
    mSFCoupling->coupleFluidToSolid(mFluidPoint->mStiff, mSolidPoint->mStiff);
}

double SolidFluidPoint::measureCoupling(int count) {
    mSolidPoint->randomDispl((Real)1e-6);
    MyBoostTimer timer;
    timer.start();
    for (int i = 0; i < count; i++) coupleSolidFluid();
    double elapsed_time = timer.elapsed();
    resetZero();
    return elapsed_time / count;
}

void SolidFluidPoint::learnWisdom(double cutoff) {
    mSolidPoint->learnWisdom(cutoff);
    mFluidPoint->learnWisdom(cutoff);
}

int SolidFluidPoint::getNuWisdom() const {
    return std::max(mSolidPoint->getNuWisdom(), mFluidPoint->getNuWisdom());
}

