// TransverselyIsotropic3X.h
// created by Kuangdai on 6-Jun-2016 
// isotropic 3D material with topography

#pragma once

#include "Elastic3D.h"
#include "eigenp.h"

class TransverselyIsotropic3X: public Elastic3D {
public:

    TransverselyIsotropic3X(const RDRowN &theta, 
        const RMatXN &A, const RMatXN &C, const RMatXN &F, const RMatXN &L, const RMatXN &N, 
        const RMatXN4 &X, Attenuation3D *att);
    
    // STEP 2: strain ==>>> stress
    void strainToStress(const vec_ar9_CMatPP &strain, vec_ar9_CMatPP &stress, int Nu) const;
    
    // check compatibility
    void checkCompatibility(int Nr, bool isVoigt) const; 
    
    // verbose
    std::string verbose() const {return "TransverselyIsotropic3X";};
                
private:    
    void rotateStrainToTIso(const CMatXN9 &strainCyln, CMatXN9 &strainTIso, int n) const;
    void rotateStressToCyln(const CMatXN9 &stressTIso, CMatXN9 &stressCyln, int n) const;
    void transformStrainX(const RMatXN9 &strain9, RMatXN6 &strain6, int Nu) const;
    void transformStressX(const RMatXN6 &stress6, RMatXN9 &stress9, int Nu) const;
    
    // orientation of axis of symmetry of transverse isotropy
    RMatXN mSin1t;
    RMatXN mCos1t;
    RMatXN mSin2t;
    RMatXN mCos2t;
    
    // Cijkl
    RMatXN mA, mC, mF, mL, mN, mN2;
    
    // particle relabelling
    RMatXN4 mX;
};

