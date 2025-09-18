
Dynamic module veto list for pixel cluster counting lumi

Dear Colleagues, 
this is my first contribution to the CMSSW codebase. I tried to get it in as good a shape as I could, but now it is time to make the PR. Thank you for your input in advance. 

I tried to rebase my commits onto master as per the recommendations. 3 commits showed merge conflicts in files I never touched. I pulled the current version of the files from master to fix that. 

#### PR description:

These changes implement a per-run dynamic selection of the tracker modules based on certain stability criteria for more robust luminosity measurement. The algorithm is described in more detail [here](https://indico.cern.ch/event/1358674/contributions/5725781/attachments/2775100/4836057/PCC_Active_Masking_Dec_19_2023.pdf).

The feature also implements a new record in the ALCA Conditions DB to keep note of the vetoed modules. The details of this were discussed [here](https://indico.cern.ch/event/1504123/#2-new-record-in-alcadb-for-onl).

The record data structure is also used as a volatile/transient structure passed between producers. Not to repeat code, I just included the class defined in `src/CondFormats/Luminosity/interface/PccVetoList.h` to `src/DataFormats/Luminosity/interface/PccVetoListTransient.h`, I hope this is good enough practice. 

The total feature is not activated in the configs, for now the point is to add it to the codebase. 

#### PR validation:

Current tests are [here](https://github.com/nowaythatsok/cmssw/blob/pcc_dyn_veto/Calibration/LumiAlCaRecoProducers/python/test_hlt_2.py) for Run2 and [here](https://github.com/nowaythatsok/cmssw/blob/pcc_dyn_veto/Calibration/LumiAlCaRecoProducers/python/test_hlt_3.py) for Run3 files. 
I am not familiar with thests in "the matrix", here some advice would be welcome. 

@capalmer85 @duff-ae

git checkout official-cmssw/master -- Configuration/Geometry/README.md
git checkout official-cmssw/master -- Configuration/Geometry/python/dict2021Geometry.py
git checkout official-cmssw/master -- Configuration/StandardSequences/python/GeometryConf.py
git checkout official-cmssw/master -- Geometry/CMSCommonData/data/dd4hep/cmsExtendedGeometry2025.xml
git checkout official-cmssw/master -- Geometry/CMSCommonData/python/cmsExtendedGeometry2025XML_cfi.py
git checkout official-cmssw/master -- GeneratorInterface/LHEInterface/data/run_generic_tarball_cvmfs.sh
git checkout official-cmssw/master -- L1Trigger/L1TGlobal/plugins/L1TGlobalProducer.cc

git add --sparse     Configuration/Geometry/README.md Configuration/Geometry/python/dict2021Geometry.py Configuration/StandardSequences/python/GeometryConf.py Geometry/CMSCommonData/data/dd4hep/cmsExtendedGeometry2025.xml Geometry/CMSCommonData/python/cmsExtendedGeometry2025XML_cfi.py GeneratorInterface/LHEInterface/data/run_generic_tarball_cvmfs.sh L1Trigger/L1TGlobal/plugins/L1TGlobalProducer.cc

git filter-repo --path Calibration/LumiAlCaRecoProducers/python/rawPCC.csv --invert-paths --force --refs pcc_dyn_veto_rebase
git filter-repo --path Calibration/LumiAlCaRecoProducers/plugins/DynamicVetoProducer_old.cc --invert-paths --force --refs pcc_dyn_veto_rebase
git filter-repo --path Calibration/LumiAlCaRecoProducers/python/PCC_Run3.root --invert-paths --force --refs pcc_dyn_veto_rebase
git filter-repo --path Calibration/LumiAlCaRecoProducers/plugins/DynamicVetoProducer.bckp --invert-paths --force --refs pcc_dyn_veto_rebase
git filter-repo --path Calibration/LumiAlCaRecoProducers/plugins/try.something --invert-paths --force --refs pcc_dyn_veto_rebase
git filter-repo --path Calibration/LumiAlCaRecoProducers/notes.md --invert-paths --force --refs pcc_dyn_veto_rebase


python3 -m pip install --user git-filter-repo
export PATH=$HOME/.local/bin:$PATH
git filter-repo --force --refs 5ed750c59ce23b8450f8a0d97b0f33f42d262134..931a92b957969ba44606cd12df706e86e3948daa \
  --path Calibration/LumiAlCaRecoProducers/python/rawPCC.csv \
  --path Calibration/LumiAlCaRecoProducers/plugins/DynamicVetoProducer_old.cc \
  --path Calibration/LumiAlCaRecoProducers/plugins/DynamicVetoProducer.bckp \
  --path Calibration/LumiAlCaRecoProducers/plugins/try.something \
  --path Calibration/LumiAlCaRecoProducers/python/PCC_Run3.root \
  --path Calibration/LumiAlCaRecoProducers/notes.md \
  --invert-paths

git filter-repo --force --refs HEAD~13..HEAD \
  --path Calibration/LumiAlCaRecoProducers/python/rawPCC.csv \
  --path Calibration/LumiAlCaRecoProducers/plugins/DynamicVetoProducer_old.cc \
  --path Calibration/LumiAlCaRecoProducers/plugins/DynamicVetoProducer.bckp \
  --path Calibration/LumiAlCaRecoProducers/plugins/try.something \
  --path Calibration/LumiAlCaRecoProducers/python/PCC_Run3.root \
  --path Calibration/LumiAlCaRecoProducers/notes.md \
  --invert-paths

git filter-repo --force --refs HEAD~13..HEAD \
  --path Calibration/LumiAlCaRecoProducers/python/minimal_veto_frac_response-2024.txt \
  --path Calibration/LumiAlCaRecoProducers/python/minimal_veto-2024.txt \
  --path Calibration/LumiAlCaRecoProducers/python/FPix_ring1_modules.txt \
  --invert-paths

git log --name-only --pretty="" | grep PCC_Run3.root
git push --force myRemote pcc_dyn_veto_rebase

git filter-repo --force --refs HEAD~13..HEAD --path .gitignore --invert-paths