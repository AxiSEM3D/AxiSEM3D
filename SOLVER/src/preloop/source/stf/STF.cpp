// STF.cpp
// created by Kuangdai on 9-May-2016
// source time function

#include "STF.h"
#include "SourceTimeFunction.h"
#include "Domain.h"

void STF::release(Domain &domain) const {
    std::vector<Real> ts(mSTF.begin(), mSTF.end());
    domain.setSTF(new SourceTimeFunction(ts, mDeltaT, mShift));
}

#include "XMPI.h"
#include "Parameters.h"
#include "ErfSTF.h"
#include <fstream>
#include "GaussSTF.h"
#include "RickerSTF.h"

void STF::buildInparam(STF *&stf, const Parameters &par, double dt, int verbose) {
    if (stf) delete stf;
    // read half duration
    std::string cmtfile = Parameters::sInputDirectory + "/" + par.getValue<std::string>("SOURCE_FILE");
    double hdur;
    if (XMPI::root()) {
        std::fstream fs(cmtfile, std::fstream::in);
        if (!fs) throw std::runtime_error("STF::buildInparam || "
            "Error opening source data file: ||" + cmtfile);
        std::string line;
        while (getline(fs, line)) {
            try {
                std::vector<std::string> strs;
                boost::trim_if(line, boost::is_any_of("\t "));
                boost::split(strs, line, boost::is_any_of("\t "), boost::token_compress_on);
                if (boost::iequals(strs[0], "half")) {
                    hdur = boost::lexical_cast<double>(strs[strs.size() - 1]);
                    break;
                }
            } catch(std::exception) {
                throw std::runtime_error("STF::buildInparam || "
                    "Error reading half duration in source data file: ||" + cmtfile);
            }
        }
        fs.close();
    }
    XMPI::bcast(hdur);

    if (hdur < 5. * dt) hdur = 5. * dt;
    double decay = 1.628;
    double duration = par.getValue<double>("TIME_RECORD_LENGTH");

    std::string mstf = par.getValue<std::string>("SOURCE_TIME_FUNCTION");
    if (boost::iequals(mstf,"erf")) {
        // Heaviside
        stf = new ErfSTF(dt, duration, hdur, decay);
    } else if (boost::iequals(mstf,"gauss")) {
        // Gaussian
        stf = new GaussSTF(dt, duration, hdur, decay);
    } else if (boost::iequals(mstf,"ricker")){
        // Ricker
        stf = new RickerSTF(dt, duration, hdur, decay);
    } else {
        throw std::runtime_error("STF::buildInparam || Unknown stf type: " + mstf);
    }
    
    // max total steps
    int maxTotalSteps = INT_MAX;
    int enforceMaxSteps = par.getValue<int>("DEVELOP_MAX_TIME_STEPS");
    if (enforceMaxSteps > 0) maxTotalSteps = enforceMaxSteps;
    if (stf->mSTF.size() > maxTotalSteps) stf->mSTF.resize(maxTotalSteps);
    
    // verbose
    if (verbose == 2) XMPI::cout << stf->verbose();
}
