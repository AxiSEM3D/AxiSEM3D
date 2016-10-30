// ExodusModel.h
// created by Kuangdai on 2-May-2016 
// read Exodus mesh file

// Adapted from salvus project 
// ExodusModel.h by Michael Afanasiev

#pragma once

#include <string>
#include <array>
#include <vector>
#include <map>

class Parameters;
class AttParameters;

extern "C" {
#include "exodusII.h"
};

class ExodusModel {
    
public:
    ExodusModel(const std::string &fileName);
    void initialize();
    void bcast();
    
    bool isIsotropic() const {return mElementalVariables.find("VP_0") != mElementalVariables.end();};
    int getNumQuads() const {return mNumQuads;};
    int getNumNodes() const {return mNumNodes;};
    const std::vector<std::array<int, 4>> &getConnectivity() const {return mConnectivity;};
    const std::vector<double> &getNodalS() const {return mNodalS;};
    const std::vector<double> &getNodalZ() const {return mNodalZ;};
    const std::map<std::string, std::vector<double>> &getElementalVariables() const {return mElementalVariables;};
    const std::map<std::string, std::vector<std::array<int, 2>>> &getSideSets() const {return mSideSets;};
    double getDistTolerance() const {return mDistTolerance;};
    double getROuter() const {return mROuter;};
    bool isCartesian() const {return mCartesian;};
    const std::vector<std::array<int, 2>> &getSideSetAxis() const {return mSideSets.at(mSSNameAxis);};
    const std::vector<std::array<int, 2>> &getSideSetSurface() const {return mSideSets.at(mSSNameSurface);};
    const std::vector<std::array<int, 2>> &getSideSetSolidFluid() const {
        if (mSideSets.find("solid_fluid_boundary") != mSideSets.end())
            return mSideSets.at("solid_fluid_boundary");
        static const std::vector<std::array<int, 2>> empty;
        return empty;    
    };
    const std::vector<double> &getAveGLLSpacing() const {return mAveGLLSpacing;};
    const std::vector<std::array<int, 5>> &getVicinalAxis() const {return mVicinalAxis;};
    
    std::string verbose() const;
    
    static void buildInparam(ExodusModel *&exModel, const Parameters &par, 
        AttParameters *&attPar, int verbose);
    
private:
    
    void readGlobalVariables();
    void readConnectivity();
    void readCoordinates();
    void readElementalVariables();
    void readSideSets();
    void readExternalH5();
    void finishReading();
    
    static void exodusError(const int retval, const std::string &func_name);
    static void hdf5Error(const int retval, const std::string &func_name);
    
    // file name
    std::string mExodusFileName;

    // file properties
    int mExodusId = -1;
    float mExodusVersion;
    char mExodusTitle[MAX_LINE_LENGTH + 1];
    
    // numbers 
    int mNumNodes;
    int mNumQuads;
    
    // global variables (dt)
    std::map<std::string, double> mGlobalVariables;
    std::map<std::string, std::string> mGlobalRecords;
    
    // connectivity
    std::vector<std::array<int, 4>> mConnectivity;
    std::vector<double> mNodalS;
    std::vector<double> mNodalZ;
    
    // elemental variables
    std::map<std::string, std::vector<double>> mElementalVariables;
    
    // side sets
    int mNumSideSets;
    std::map<std::string, std::vector<std::array<int, 2>>> mSideSets;
    
    // others
    double mDistTolerance;
    double mROuter;
    
    // for Nr map
    std::vector<double> mAveGLLSpacing;
    std::vector<std::array<int, 5>> mVicinalAxis; 
    
    bool mCartesian = false;
    std::string mSSNameAxis = "t0";
    std::string mSSNameSurface = "r1";
    
    std::vector<double> mEllipKnots;
    std::vector<double> mEllipCoeffs;
};


