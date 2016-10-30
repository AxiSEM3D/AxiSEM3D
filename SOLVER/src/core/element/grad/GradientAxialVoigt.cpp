// GradientAxialVoigt.cpp
// created by Kuangdai on 8-May-2016 
// elemental gradient using Voigt notation

#include "GradientAxialVoigt.h"

GradientAxialVoigt::GradientAxialVoigt(const RMatPP &dsdxii, const RMatPP &dsdeta, 
                                       const RMatPP &dzdxii, const RMatPP &dzdeta, const RMatPP &inv_s):
GradientAxial(dsdxii, dsdeta, dzdxii, dzdeta, inv_s) {
    // nothing
}

void GradientAxialVoigt::gradVector(const vec_ar3_CMatPP &ui, vec_ar9_CMatPP &eij, int Nu, int nyquist) const {
    // hardcode for alpha = 0
    static RMatPP GU0R, GU1R, GU2R, UG0R, UG1R, UG2R;
    GU0R = sGT_GLJ * ui[0][0].real();  
    GU1R = sGT_GLJ * ui[0][1].real();  
    GU2R = sGT_GLJ * ui[0][2].real();  
    UG0R = ui[0][0].real() * sG_GLL;
    UG1R = ui[0][1].real() * sG_GLL;
    UG2R = ui[0][2].real() * sG_GLL;
    eij[0][0].real() = mDzDeta.schur(GU0R) + mDzDxii.schur(UG0R);
    eij[0][1].real() = mInv_s.schur(ui[0][0].real()); 
    eij[0][2].real() = mDsDeta.schur(GU2R) + mDsDxii.schur(UG2R);
    eij[0][3].real() = mDsDeta.schur(GU1R) + mDsDxii.schur(UG1R);
    eij[0][4].real() = mDsDeta.schur(GU0R) + mDsDxii.schur(UG0R) + mDzDeta.schur(GU2R) + mDzDxii.schur(UG2R);
    eij[0][5].real() = mDzDeta.schur(GU1R) + mDzDxii.schur(UG1R) - mInv_s.schur(ui[0][1].real());
    eij[0][1].row(0).real() += mDzDeta.row(0).schur(sGT_GLJ.row(0) * ui[0][0].real());
    eij[0][5].row(0).real() -= mDzDeta.row(0).schur(sGT_GLJ.row(0) * ui[0][1].real());
    
    // alpha > 0
    static CMatPP v0, v1, v2, GU0, GU1, GU2, UG0, UG1, UG2;
    for (int alpha = 1; alpha <= Nu - nyquist; alpha++) {        
        Complex iialpha = (Real)alpha * ii;
        v0 = ui[alpha][0] + iialpha * ui[alpha][1];
        v1 = iialpha * ui[alpha][0] - ui[alpha][1];
        v2 = iialpha * ui[alpha][2];
        GU0 = sGT_GLJ * ui[alpha][0];  
        GU1 = sGT_GLJ * ui[alpha][1];  
        GU2 = sGT_GLJ * ui[alpha][2];  
        UG0 = ui[alpha][0] * sG_GLL;
        UG1 = ui[alpha][1] * sG_GLL;
        UG2 = ui[alpha][2] * sG_GLL;
        eij[alpha][0] = mDzDeta.schur(GU0) + mDzDxii.schur(UG0);
        eij[alpha][1] = mInv_s.schur(v0); 
        eij[alpha][2] = mDsDeta.schur(GU2) + mDsDxii.schur(UG2);
        eij[alpha][3] = mDsDeta.schur(GU1) + mDsDxii.schur(UG1) + mInv_s.schur(v2);
        eij[alpha][4] = mDsDeta.schur(GU0) + mDsDxii.schur(UG0) + mDzDeta.schur(GU2) + mDzDxii.schur(UG2);
        eij[alpha][5] = mDzDeta.schur(GU1) + mDzDxii.schur(UG1) + mInv_s.schur(v1);
        eij[alpha][1].row(0) += mDzDeta.row(0).schur(sGT_GLJ.row(0) * v0);
        eij[alpha][5].row(0) += mDzDeta.row(0).schur(sGT_GLJ.row(0) * v1);
        eij[alpha][3].row(0) += mDzDeta.row(0).schur(sGT_GLJ.row(0) * v2);
        if (alpha == 1) {
            eij[alpha][1].row(0) += mDzDxii.row(0).schur(v0.row(0) * sG_GLL);
            eij[alpha][5].row(0) += mDzDxii.row(0).schur(v1.row(0) * sG_GLL);
        }
    }    
    
    // mask Nyquist
    if (nyquist) {
        eij[Nu][0].setZero();
        eij[Nu][1].setZero();
        eij[Nu][2].setZero();
        eij[Nu][3].setZero();
        eij[Nu][4].setZero();
        eij[Nu][5].setZero();
    }   
}

void GradientAxialVoigt::quadVector(const vec_ar9_CMatPP &sij, vec_ar3_CMatPP &fi, int Nu, int nyquist) const {
    // hardcode for mbeta = 0
    static RMatPP X0R, X1R, X2R, Y0R, Y1R, Y2R; 
    X0R = mDzDeta.schur(sij[0][0].real()) + mDsDeta.schur(sij[0][4].real());
    X1R = mDzDeta.schur(sij[0][5].real()) + mDsDeta.schur(sij[0][3].real());
    X2R = mDzDeta.schur(sij[0][4].real()) + mDsDeta.schur(sij[0][2].real());
    Y0R = mDzDxii.schur(sij[0][0].real()) + mDsDxii.schur(sij[0][4].real());
    Y1R = mDzDxii.schur(sij[0][5].real()) + mDsDxii.schur(sij[0][3].real());
    Y2R = mDzDxii.schur(sij[0][4].real()) + mDsDxii.schur(sij[0][2].real());
    fi[0][0].real() = sG_GLJ * X0R + Y0R * sGT_GLL + mInv_s.schur(sij[0][1].real());
    fi[0][1].real() = sG_GLJ * X1R + Y1R * sGT_GLL - mInv_s.schur(sij[0][5].real());
    fi[0][2].real() = sG_GLJ * X2R + Y2R * sGT_GLL; 
    fi[0][0].real() += sG_GLJ.col(0) * mDzDeta.row(0).schur(sij[0][1].real().row(0));
    fi[0][1].real() -= sG_GLJ.col(0) * mDzDeta.row(0).schur(sij[0][5].real().row(0));
    
    // mbeta > 0
    static CMatPP g0, g1, g2, X0, X1, X2, Y0, Y1, Y2;
    for (int mbeta = 1; mbeta <= Nu - nyquist; mbeta++) {
        Complex iibeta = - (Real)mbeta * ii; 
        g0 = sij[mbeta][1] + iibeta * sij[mbeta][5];
        g1 = iibeta * sij[mbeta][1] - sij[mbeta][5];
        g2 = iibeta * sij[mbeta][3];    
        X0 = mDzDeta.schur(sij[mbeta][0]) + mDsDeta.schur(sij[mbeta][4]);
        X1 = mDzDeta.schur(sij[mbeta][5]) + mDsDeta.schur(sij[mbeta][3]);
        X2 = mDzDeta.schur(sij[mbeta][4]) + mDsDeta.schur(sij[mbeta][2]);
        Y0 = mDzDxii.schur(sij[mbeta][0]) + mDsDxii.schur(sij[mbeta][4]);
        Y1 = mDzDxii.schur(sij[mbeta][5]) + mDsDxii.schur(sij[mbeta][3]);
        Y2 = mDzDxii.schur(sij[mbeta][4]) + mDsDxii.schur(sij[mbeta][2]);
        fi[mbeta][0] = sG_GLJ * X0 + Y0 * sGT_GLL + mInv_s.schur(g0);
        fi[mbeta][1] = sG_GLJ * X1 + Y1 * sGT_GLL + mInv_s.schur(g1);
        fi[mbeta][2] = sG_GLJ * X2 + Y2 * sGT_GLL + mInv_s.schur(g2);
        fi[mbeta][0] += sG_GLJ.col(0) * mDzDeta.row(0).schur(g0.row(0));
        fi[mbeta][1] += sG_GLJ.col(0) * mDzDeta.row(0).schur(g1.row(0));
        fi[mbeta][2] += sG_GLJ.col(0) * mDzDeta.row(0).schur(g2.row(0));
        if (mbeta == 1) {
            fi[mbeta][0].row(0) += mDzDxii.row(0).schur(g0.row(0)) * sGT_GLL;
            fi[mbeta][1].row(0) += mDzDxii.row(0).schur(g1.row(0)) * sGT_GLL;
        }
    }
    
    // mask Nyquist
    if (nyquist) {
        fi[Nu][0].setZero();
        fi[Nu][1].setZero();
        fi[Nu][2].setZero();
    }
}

