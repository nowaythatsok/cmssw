#ifndef CondFormats_Luminosity_PccVetoList_h
#define CondFormats_Luminosity_PccVetoList_h

/** 
 * \class PccVetoList class
 * 
 * \author Peter Major
 *  
 */
#include "CondFormats/Serialization/interface/Serializable.h"
#include <vector>

class PccVetoList {
  public:
    PccVetoList() = default;
    PccVetoList(const std::vector<int>& modVec, double respFrac) : badModules(modVec), responseFraction(respFrac) {}
    void setBadModules(const std::vector<int>& modVec) { badModules = modVec; }
    const std::vector<int>& getBadModules() const { return badModules; }

    void setResponseFraction(double respFrac) { responseFraction = respFrac; }
    double getResponseFraction() const { return responseFraction; }
    
  private:
    std::vector<int> badModules; 
    double responseFraction = 1.0; // is public not to have to use a getter

  COND_SERIALIZABLE;
};

#endif


