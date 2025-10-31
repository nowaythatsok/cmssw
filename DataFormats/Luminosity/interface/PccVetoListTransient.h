#ifndef DataFormats_Luminosity_PccVetoListTransient_h
#define DataFormats_Luminosity_PccVetoListTransient_h

/** 
 * \class PccVetoList
 * 
 * \author Peter Major
 *  
 */

#include <algorithm>
#include <boost/serialization/vector.hpp>
#include <cstring>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>

class PccVetoListTransient {
    public:
    
    void addToVetoList(const std::unordered_set<int>& modSet) {
        badModules.reserve(badModules.size() + modSet.size());
        badModules.insert(badModules.end(), modSet.begin(), modSet.end());
    }

    void setBadModules(const std::vector<int>& modVec) { badModules = modVec; }

    const std::vector<int>& getBadModules() const { return badModules; }
    bool isBad(int mId) const { return (std::find(badModules.begin(), badModules.end(), mId) != badModules.end()); };
    bool isGood(int mId) const { return !this->isBad(mId); };

    double getResponseFraction() const { return responseFraction; }
    void setResponseFraction(double respFrac) { responseFraction = respFrac; }
    void generateResponseFraction(const std::unordered_map<int, double>& fractionalResponses) {
        double responseTotal = 0;
        responseFraction = 0;
        for (const auto& [modID, frac] : fractionalResponses) {
        responseTotal += frac;
        if (this->isBad(modID))
            continue;
        responseFraction += frac;
        }
        responseFraction /= responseTotal;
    }
    double getScaleFactor() const {
        if (responseFraction == 0)
            return 1.0;
        return 1.0 / responseFraction;
    }

    private:
        // std::unordered_set<int> badModules; // unordered sets are not serializable 
        std::vector<int> badModules; 
        double responseFraction = 1.0; // is public not to have to use a getter

};

#endif