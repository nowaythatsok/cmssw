https://cms-sw.github.io/faq.html
voms-proxy-init --voms cms
https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookStartingGrid
/eos/home-a/alshevel/CMSSW_14_0_9_patch1/src/Calibration/LumiAlCaRecoProducers/python/input_pcc.root

https://github.com/cms-sw/cmssw
https://github.com/nowaythatsok/cmssw
https://github.com/nowaythatsok/cmssw/tree/master/Calibration/LumiAlCaRecoProducers

https://cms-sw.github.io/tutorial.html
https://cms-sw.github.io/faq.html

cd /eos/user/p/pmajor/pcc_dynamic_veto/
cmsrel CMSSW_14_2_2
cd CMSSW_15_0_0/src
scramv1 runtime -sh 
cmsenv
git cms-addpkg Configuration/StandardSequences 
git cms-addpkg Calibration/LumiAlCaRecoProducers 
scram b -j12
cd Calibration/LumiAlCaRecoProducers/python/
cmsRun test_hlt.py


###
what modules are doing:
https://indico.cern.ch/event/913721/contributions/3842297/attachments/2028065/3393448/PCC_update_April28_2020_test.pdf

# alcaPCCEventProducer
makes reco::PixelClusterCountsInEvent object which contains the number of events per module (optionally per ROC)
# alcaPCCIntegrator
makes reco::PixelClusterCounts
Sum up clusters per module per bunch crossing per lumisection (LS)

the reco::PixelClusterCountsInEvent objects are stored into streamCaches, then handed to reco::PixelClusterCounts' merge which takes care of the per module summation

should the same procedure be used for the per-run summation as well?

# rawPCCProd
makes LumiInfo
this sums the modules in accordance with the veto list
also makes CSV

# CorrPCCProducer 
- the input of this comes from rawPCC
- this computes the afterglow corrections
- it can count the lumi blocks in a run and divide them into 50ish lumisections 
- it outputs to the database
this should be the one I start from
use the putput of alcaPCCIntegrator instead
make it output a csv too
accept a base veto list



### 
cmsenv
scram b -j12 # only in root directory
cd Calibration/LumiAlCaRecoProducers/python/
rm PCC.root rawPCC.csv # if not deleted, there will be an error when writing the output
cmsRun test_hlt.py

creates:
PCC.root
rawPCC.csv

###
cd ../../../
scram b -j12
cd Calibration/LumiAlCaRecoProducers/python/
rm PCC.root rawPCC.csv dynamicVeto.csv
cmsRun test_hlt_2.py

root PCC.root
_file0->ls()
_file0->Print()


### 
cmsenv
git cms-addpkg FWCore 
git cms-addpkg DataFormats/Luminosity
git cms-addpkg DataFormats/Provenance
git cms-addpkg DataFormats/Common
git cms-addpkg CondFormats/Serialization
git cms-addpkg CondFormats/DataRecord
git cms-addpkg CondFormats/Luminosity
git cms-addpkg DQMServices/Core
git cms-addpkg CondCore/DBOutputService

git cms-addpkg DataFormats/TrackerCommon
git cms-addpkg DataFormats/SiPixelDetId




###

/eos/user/p/pmajor/pcc_dynamic_veto/CMSSW_14_2_2/src/
or
cd ../../..
cmsenv
scram b -j16
cd Calibration/LumiAlCaRecoProducers/python/
rm -f *.root *.csv *.png 
cmsRun test_hlt_2.py




###
du -hs .[^.]*


###
---------------------------------
#ifndef CondFormats_Luminosity_PccVetoList_h
#define CondFormats_Luminosity_PccVetoList_h

/** 
 * \class PccVetoList
 * 
 * \author Peter Major
 *  
 */

#include <sstream>
#include <cstring>
#include <algorithm>
#include <vector>
#include <boost/serialization/vector.hpp>
#include "CondFormats/Serialization/interface/Serializable.h"

#define PccVetoListVECTORS

class PccVetoList {
public:

#ifdef PccVetoListVECTORS
  void addToVetoList( const std::vector<int>& modList ){ 
    badModules.reserve(badModules.size()+modList.size()); 
    badModules.insert(badModules.end(), modList.begin(), modList.end()); 
  }
  const std::vector<int>& getBadModules() const { return badModules; }
  bool isBad(int mId)  const { return (std::find(badModules.begin(), badModules.end(), mId) != badModules.end()); };

#else
  void addToVetoList( const std::vector<int>& modList ){ 
    std::copy(badModules.begin(), badModules.end(), std::back_inserter(modList));
  }
  const std::map<int>& getBadModules() const { return badModules; }
  bool isBad(int mId)  const { return (badModules.count(mId)); };

#endif

  bool isGood(int mId) const { return ! this->isBad(mId); };

  double responseFraction = 1.0;
  void generateResponseFraction(std::map<int, float> fractionalResponses){
    responseFraction = 0;
    for (const auto& [modID, frac] : fractionalResponses) {
      if (this->isBad(modID)) continue;
      responseFraction += frac;
    }
  }

private:
#ifdef PccVetoListVECTORS
  std::vector<int> badModules;
#else
  std::set<int> badModules;
#endif
  
  COND_SERIALIZABLE;
};


#endif

-------------------------------------------------------------------