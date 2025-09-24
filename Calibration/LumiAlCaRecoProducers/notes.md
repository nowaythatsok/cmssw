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


So I think I found out what causes my segfault in the end. 
I thought that these were the files Braiden used to develop his code and I tried to use these to see if my code works with Run3 data too. (Nota bene, I used Run2 files to develop my code, that seems to work if all collections are found as expected.)
# 'file:/eos/cms/store/data/Run2024F/AlCaLumiPixelsCountsPrompt/ALCARECO/AlCaPCCZeroBias-PromptReco-v1/000/382/913/00000/242f6268-861c-4231-9cf8-b51c22aa7195.root'
# 'file:/eos/cms/store/data/Run2024F/AlCaLumiPixelsCountsPrompt/ALCARECO/AlCaPCCZeroBias-PromptReco-v1/000/382/913/00000/9491103d-34bf-441c-bd88-b7b3cd3d71a3.root'
# 'file:/eos/cms/store/data/Run2024F/AlCaLumiPixelsCountsPrompt/ALCARECO/AlCaPCCZeroBias-PromptReco-v1/000/382/913/00000/ac0dc5c4-c440-4c03-b876-e7731816355a.root'
# 'file:/eos/cms/store/data/Run2024F/AlCaLumiPixelsCountsPrompt/ALCARECO/AlCaPCCZeroBias-PromptReco-v1/000/382/913/00000/e644fd22-7a9f-4e3e-8edc-626aabea358d.root'
# 'file:/eos/cms/store/data/Run2024F/AlCaLumiPixelsCountsPrompt/ALCARECO/AlCaPCCZeroBias-PromptReco-v1/000/382/913/00000/edc8ddbc-de7e-4896-ad05-b63a1805d011.root'

Now it seems to me that these files actually have zero edm content
[pmajor@lxplus950 python]$ edmDumpEventContent /eos/cms/store/data/Run2024F/AlCaLumiPixelsCountsPrompt/ALCARECO/AlCaPCCZeroBias-PromptReco-v1/000/382/913/00000/242f6268-861c-4231-9cf8-b51c22aa7195.root
Type      Module    Label     Process   
----------------------------------------
[pmajor@lxplus950 python]$ 

And as a result my code reached for some undefined reference somewhere, not yet clear where. I d need some run3 file that has hltFEDSelectorLumiPixels collections I can test with.
Friday
Chris Palmer
4:56 PM

This is what I did to find datasets:

[16:54:26] capalmer@lxplus982 ~ > dasgoclient --query="dataset dataset=/*Lumi*/*/* run=382913"
/AlCaLumiPixelsCountsPrompt/Run2024F-AlCaPCCRandom-PromptReco-v1/ALCARECO
/AlCaLumiPixelsCountsPrompt/Run2024F-AlCaPCCZeroBias-PromptReco-v1/ALCARECO
/AlCaLumiPixelsCountsPrompt/Run2024F-RawPCCProducer-PromptReco-v1/ALCARECO
/AlCaLumiPixelsCountsPrompt/Run2024F-v1/RAW
/StreamALCALumiPixelsCountsExpress/Run2024F-AlCaPCCRandom-Express-v1/ALCARECO
/StreamALCALumiPixelsCountsExpress/Run2024F-Express-v1/DQMIO
/StreamALCALumiPixelsCountsExpress/Run2024F-PromptCalibProdLumiPCC-Express-v1/ALCAPROMPT

I think the one you want is:
/AlCaLumiPixelsCountsPrompt/Run2024F-AlCaPCCRandom-PromptReco-v1/ALCARECO

this will give you a file list:
dasgoclient --query="file dataset=/AlCaLumiPixelsCountsPrompt/Run2024F-AlCaPCCRandom-PromptReco-v1/ALCARECO run=382913" > filelist_Run2024F-AlCaPCCRandom-Prompt_384406.txt

You're in luck. These files are at CERN T2 (so locally available):
[16:59:04] capalmer@lxplus982 ~ > lt /eos/cms/store/data/Run2024F/AlCaLumiPixelsCountsPrompt/ALCARECO/AlCaPCCRandom-PromptReco-v1/000/382/913/00000/
total 12G
-rw-r--r--. 1 cmsprd zh 2.1G Jul 12  2024 01e533af-8d30-445e-ae06-e6556de459d1.root
-rw-r--r--. 1 cmsprd zh 3.2G Jul 12  2024 70db01b0-c42e-45d0-a1bb-b28d4fa4458c.root
-rw-r--r--. 1 cmsprd zh 2.8G Jul 12  2024 1c99fe38-e3dd-4764-a104-d0d0a274c027.root
-rw-r--r--. 1 cmsprd zh 3.2G Jul 12  2024 4150272f-3d98-4d1d-a1a1-8874ca1f155d.root
-rw-r--r--. 1 cmsprd zh 646M Jul 12  2024 f9dea74a-0687-440a-84ba-2488884804e7.root
Braden Kronheim
5:03 PM

I can run over the Random as opposed to the zero bias if you want. I think Peter needs the RAW though as the Alcas don't have the objects he needs
Chris Palmer
5:04 PM

ah

that's why

ok 

makes sense

are those objects in the 2025 alcas?

these datasets should have AVERAGE PCC per module per LS per BCID. Why isn't that enough? 
Braden Kronheim
5:06 PM

Probably not, I doubt hltFEDSelectorLumiPixels would be in an alca
Chris Palmer
5:06 PM

but why do we need that?
Braden Kronheim
5:06 PM

They definitely have enough information, as they are what I use. But maybe they don't have what the CMSSW algorithm has, which was the point of this test, if I understand correctly
Chris Palmer
5:07 PM

the point of the test was to see if the veto list comes out similar enough

but from the module we expect to run
5:08 PM

the input dataset for the Prompt Calibration Loop is after the integration over events in a LS