/**_________________________________________________________________
class:   RawPCCProducerDynVeto.cc

description: Creates a LumiInfo object that will contain the luminosity per bunch crossing,
along with the total luminosity and the statistical error.

authors:Sam Higginbotham (shigginb@cern.ch), Chris Palmer (capalmer@cern.ch), Jose Benitez (jose.benitez@cern.ch)

________________________________________________________________**/
#include <cmath>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>
#include <unordered_set>

#include "CondFormats/DataRecord/interface/LumiCorrectionsRcd.h"
#include "CondFormats/DataRecord/interface/PccVetoListRcd.h"
#include "CondFormats/Luminosity/interface/LumiCorrections.h"
#include "CondFormats/Luminosity/interface/PccVetoList.h"
#include "DataFormats/Luminosity/interface/LumiConstants.h"
#include "DataFormats/Luminosity/interface/LumiInfo.h"
#include "DataFormats/Luminosity/interface/PccVetoListTransient.h"
#include "DataFormats/Luminosity/interface/PixelClusterCounts.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
// #include "FWCore/Framework/interface/global/EDProducer.h"
#include "FWCore/Framework/interface/one/EDProducer.h"
// #include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/LuminosityBlock.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/EDGetToken.h"

// class RawPCCProducerDynVeto : public edm::global::EDProducer<edm::EndLuminosityBlockProducer, edm::BeginRunProducer> {
class RawPCCProducerDynVeto : public edm::one::EDProducer<edm::BeginRunProducer, edm::EndLuminosityBlockProducer> {
  // class RawPCCProducerDynVeto : public edm::stream::EDProducer<edm::BeginRunProducer, edm::EndLuminosityBlockProducer> {
public:
  explicit RawPCCProducerDynVeto(const edm::ParameterSet&);
  ~RawPCCProducerDynVeto() override;

private:
  // void globalEndLuminosityBlockProduce(edm::LuminosityBlock& lumiSeg, const edm::EventSetup& iSetup) const final;
  void endLuminosityBlockProduce(edm::LuminosityBlock& lumiSeg, const edm::EventSetup& iSetup) override;
  // void endLuminosityBlockProduce(edm::LuminosityBlock& lumiSeg, const edm::EventSetup& iSetup) override;
  // void produce(edm::StreamID, edm::Event& iEvent, const edm::EventSetup& iSetup) const final;
  void produce(edm::Event&, edm::EventSetup const&) override {}

  // void globalBeginRunProduce(edm::Run & runSeg, const edm::EventSetup& iSetup) const final;
  void beginRunProduce(edm::Run & runSeg, const edm::EventSetup& iSetup) override;
  // void beginRun(edm::Run & runSeg, const edm::EventSetup& iSetup) override;

  //input object labels
  edm::EDGetTokenT<reco::PixelClusterCounts> pccToken_;
  edm::EDGetTokenT<PccVetoListTransient> dinamicVetoTransientToken_;

  //background corrections from DB
  const edm::ESGetToken<LumiCorrections, LumiCorrectionsRcd> lumiCorrectionsToken_;

  //The list of modules to skip in the lumi calc.
  const std::vector<int> staticModuleVetoList_;
  const edm::ESGetToken<PccVetoList, PccVetoListRcd> dinamicVetoDBToken_;
  
  const bool useDynamicModVetoDB_;
  const bool useDynamicModVetoTransient_;
  PccVetoListTransient vetoObject_;


  //background corrections
  const bool applyCorr_;
  //Output average values
  const std::string takeAverageValue_;

  //output object labels
  const edm::EDPutTokenT<LumiInfo> putToken_;

  //produce csv lumi file
  const bool saveCSVFile_;
  const std::string csvFileName_;
  mutable std::mutex fileLock_;
};

//--------------------------------------------------------------------------------------------------
RawPCCProducerDynVeto::RawPCCProducerDynVeto(const edm::ParameterSet& iConfig)
    : pccToken_(consumes<reco::PixelClusterCounts, edm::InLumi>(
          edm::InputTag(iConfig.getParameter<edm::ParameterSet>("RawPCCProducerDynVetoParameters")
                            .getParameter<std::string>("inputPccLabel"),
                        iConfig.getParameter<edm::ParameterSet>("RawPCCProducerDynVetoParameters")
                            .getParameter<std::string>("ProdInstPCCI")))),
      dinamicVetoTransientToken_(consumes<PccVetoListTransient, edm::InRun>(
          edm::InputTag(iConfig.getParameter<edm::ParameterSet>("RawPCCProducerDynVetoParameters")
                            .getParameter<std::string>("inputDynamicVetoLabel"),
                        iConfig.getParameter<edm::ParameterSet>("RawPCCProducerDynVetoParameters")
                            .getParameter<std::string>("ProdInstDynamicVeto")))),
      lumiCorrectionsToken_(esConsumes<edm::Transition::EndLuminosityBlock>()),
      staticModuleVetoList_(iConfig.getParameter<edm::ParameterSet>("RawPCCProducerDynVetoParameters")
                   .getParameter<std::vector<int>>("staticModuleVetoList")),
      dinamicVetoDBToken_(esConsumes<edm::Transition::EndLuminosityBlock>()),
      useDynamicModVetoDB_(iConfig.getParameter<edm::ParameterSet>("RawPCCProducerDynVetoParameters")
                             .getParameter<bool>("useDynamicModVetoDB")),
      useDynamicModVetoTransient_(iConfig.getParameter<edm::ParameterSet>("RawPCCProducerDynVetoParameters")
                             .getParameter<bool>("useDynamicModVetoTransient")),
      applyCorr_(iConfig.getParameter<edm::ParameterSet>("RawPCCProducerDynVetoParameters")
                     .getUntrackedParameter<bool>("ApplyCorrections", false)),
      takeAverageValue_(iConfig.getParameter<edm::ParameterSet>("RawPCCProducerDynVetoParameters")
                            .getUntrackedParameter<std::string>("OutputValue", std::string("Average"))),
      putToken_(produces<LumiInfo, edm::Transition::EndLuminosityBlock>(
          iConfig.getParameter<edm::ParameterSet>("RawPCCProducerDynVetoParameters")
              .getUntrackedParameter<std::string>("outputProductName", "alcaLumi"))),
      saveCSVFile_(iConfig.getParameter<edm::ParameterSet>("RawPCCProducerDynVetoParameters")
                       .getUntrackedParameter<bool>("saveCSVFile", false)),
      csvFileName_(iConfig.getParameter<edm::ParameterSet>("RawPCCProducerDynVetoParameters")
                       .getUntrackedParameter<std::string>("CsvFileName", std::string("rawPCC.csv"))) {}

//--------------------------------------------------------------------------------------------------
RawPCCProducerDynVeto::~RawPCCProducerDynVeto() {}

// void RawPCCProducerDynVeto::globalBeginRunProduce(edm::Run & runSeg, const edm::EventSetup& iSetup) {
void RawPCCProducerDynVeto::beginRunProduce(edm::Run & runSeg, const edm::EventSetup& iSetup) {
// void RawPCCProducerDynVeto::beginRun(edm::Run & runSeg, const edm::EventSetup& iSetup) {

  if (useDynamicModVetoDB_) {
    if (useDynamicModVetoTransient_) 
      throw std::runtime_error("Cannot use both dynamic veto from DB and Transient");

    const PccVetoList dynamicVeto = iSetup.getData(dinamicVetoDBToken_);
    vetoObject_.setBadModules(dynamicVeto.getBadModules());
    vetoObject_.setResponseFraction(dynamicVeto.getResponseFraction());
  } else if (useDynamicModVetoTransient_){
    if (useDynamicModVetoDB_) 
      throw std::runtime_error("Cannot use both dynamic veto from DB and Transient");

    const edm::Handle<PccVetoListTransient> pccVetoListTransientHandle = runSeg.getHandle(dinamicVetoTransientToken_);
    const PccVetoListTransient& dynamicVetoTransient = *(pccVetoListTransientHandle.product());
    vetoObject_.setBadModules(dynamicVetoTransient.getBadModules());
    vetoObject_.setResponseFraction(dynamicVetoTransient.getResponseFraction());
  }
  else {
    if (vetoObject_.getBadModules().size() == 0){
      vetoObject_.setBadModules(staticModuleVetoList_);
      vetoObject_.setResponseFraction(1.0);
    }
  }

}

//--------------------------------------------------------------------------------------------------
// void RawPCCProducerDynVeto::globalEndLuminosityBlockProduce(edm::LuminosityBlock& lumiSeg, const edm::EventSetup& iSetup) const {
void RawPCCProducerDynVeto::endLuminosityBlockProduce(edm::LuminosityBlock& lumiSeg, const edm::EventSetup& iSetup) {
// void RawPCCProducerDynVeto::endLuminosityBlockProduce(edm::LuminosityBlock& lumiSeg, const edm::EventSetup& iSetup) {
  //The total raw luminosity from the pixel clusters - not scaled
  float totalLumi = 0.0;
  //the statistical error on the lumi - large num ie sqrt(N)
  float statErrOnLumi = 0.0;

  //new vector containing clusters per bxid
  std::vector<int> clustersPerBXOutput(LumiConstants::numBX, 0);
  //new vector containing clusters per bxid with afterglow corrections
  std::vector<float> corrClustersPerBXOutput(LumiConstants::numBX, 0);

  //////////////////////////////////
  /// read input , clusters per module per bx
  /////////////////////////////////
  const edm::Handle<reco::PixelClusterCounts> pccHandle = lumiSeg.getHandle(pccToken_);
  const reco::PixelClusterCounts& inputPcc = *(pccHandle.product());
  //vector with Module IDs 1-1 map to bunch x-ing in clusers
  auto modID = inputPcc.readModID();
  //vector with total events at each bxid.
  auto events = inputPcc.readEvents();
  //cluster counts per module per bx
  auto clustersPerBXInput = inputPcc.readCounts();

  ////////////////////////////
  ///Apply the module veto
  ///////////////////////////
  for (int bx = 0; bx < int(LumiConstants::numBX); bx++) {
    for (unsigned int i = 0; i < modID.size(); i++){
      if (vetoObject_.isBad(modID.at(i)))
        continue;
      clustersPerBXOutput.at(bx) += clustersPerBXInput.at( modID.at(i) * int(LumiConstants::numBX) + bx);
    }
  }

  //////////////////////////////
  //// Apply afterglow corrections
  //////////////////////////////
  std::vector<float> correctionScaleFactors;
  if (applyCorr_) {
    const auto pccCorrections = &iSetup.getData(lumiCorrectionsToken_);
    correctionScaleFactors = pccCorrections->getCorrectionsBX();
  } else {
    correctionScaleFactors.resize(LumiConstants::numBX, 1.0);
  }

  for (unsigned int i = 0; i < clustersPerBXOutput.size(); i++) {
    if (events.at(i) != 0) {
      corrClustersPerBXOutput[i] = clustersPerBXOutput[i] * correctionScaleFactors[i] * vetoObject_.getScaleFactor();
    } else {
      corrClustersPerBXOutput[i] = 0.0;
    }
    totalLumi += corrClustersPerBXOutput[i];
    statErrOnLumi += float(events[i]);
  }

  std::vector<float> errorPerBX;  //Stat error (or number of events)
  errorPerBX.assign(events.begin(), events.end());

  //////////////////////////////////
  /// Compute average number of clusters per event
  ////////////////////////////////
  if (takeAverageValue_ == "Average") {
    unsigned int NActiveBX = 0;
    for (int bx = 0; bx < int(LumiConstants::numBX); bx++) {
      if (events[bx] > 0) {
        NActiveBX++;
        corrClustersPerBXOutput[bx] /= float(events[bx]);
        errorPerBX[bx] = 1 / sqrt(float(events[bx]));
      }
    }
    if (statErrOnLumi != 0) {
      totalLumi = totalLumi / statErrOnLumi * float(NActiveBX);
      statErrOnLumi = 1 / sqrt(statErrOnLumi) * totalLumi;
    }
  }

  ///////////////////////////////////////////////////////
  ///Lumi saved in the LuminosityBlocks
  LumiInfo outputLumiInfo;
  outputLumiInfo.setTotalInstLumi(totalLumi);
  outputLumiInfo.setTotalInstStatError(statErrOnLumi);
  outputLumiInfo.setErrorLumiAllBX(errorPerBX);
  outputLumiInfo.setInstLumiAllBX(corrClustersPerBXOutput);
  lumiSeg.emplace(putToken_, std::move(outputLumiInfo));

  //Lumi saved in the csv file
  if (saveCSVFile_) {
    std::lock_guard<std::mutex> lock(fileLock_);
    std::ofstream csfile(csvFileName_, std::ios_base::app);
    csfile << std::to_string(lumiSeg.run()) << ",";
    csfile << std::to_string(lumiSeg.luminosityBlock()) << ",";
    csfile << std::to_string(totalLumi);

    for (unsigned int bx = 0; bx < LumiConstants::numBX; bx++)
      csfile << "," << std::to_string(corrClustersPerBXOutput[bx]);
    csfile << std::endl;

    csfile.close();
  }
}

DEFINE_FWK_MODULE(RawPCCProducerDynVeto);
