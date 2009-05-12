#include <stdexcept>
#include <string>
#include <iostream>
#include <map>
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/LuminosityBlock.h"

#include "FWCore/Framework/interface/EventSetup.h"

#include "CondFormats/RunInfo/interface/LuminosityInfo.h"
#include "CondFormats/DataRecord/interface/LuminosityInfoRcd.h"

using namespace std;

namespace edmtest
{
  class LuminosityInfoAnalyzer : public edm::EDAnalyzer
  {
  public:
    explicit  LuminosityInfoAnalyzer(edm::ParameterSet const& p) 
    { 
      std::cout<<"LuminosityInfoAnalyzer"<<std::endl;
    }
    explicit  LuminosityInfoAnalyzer(int i) 
    { std::cout<<"LuminosityInfoAnalyzer "<<i<<std::endl; }
    virtual ~LuminosityInfoAnalyzer() {  
      std::cout<<"~LuminosityInfoAnalyzer "<<std::endl;
    }
    virtual void beginJob(const edm::EventSetup& context);
    virtual void beginRun(const edm::Run&, const edm::EventSetup& context);
    virtual void beginLuminosityBlock(edm::LuminosityBlock const& iLBlock,
				      edm::EventSetup const& context);
    virtual void analyze(const edm::Event& e, const edm::EventSetup& c);

  private:
  };
  void
  LuminosityInfoAnalyzer::beginRun(const edm::Run&, const edm::EventSetup& context){
    /*std::cout<<"###LuminosityInfoAnalyzer::beginRun"<<std::endl;
    edm::ESHandle<lumi::LuminosityInfo> pPeds;
    std::cout<<"got eshandle"<<std::endl;
    context.get<LuminosityInfoRcd>().get(pPeds);
    */
  }
  void
  LuminosityInfoAnalyzer::beginJob(const edm::EventSetup& context){
    std::cout<<"###LuminosityInfoAnalyzer::beginJob"<<std::endl;
  }
  void
  LuminosityInfoAnalyzer::beginLuminosityBlock(edm::LuminosityBlock const& iLBlock,
					       edm::EventSetup const& context){
    std::cout<<"###LuminosityInfoAnalyzer::beginLuminosityBlock"<<std::endl;
    using namespace edm::eventsetup;
    // Context is not used.
    //std::cout <<" I AM IN RUN NUMBER "<<e.id().run() <<std::endl;
    //    std::cout <<" ---EVENT NUMBER "<<e.id().event() <<std::endl;
    std::cout<<"lumiblock id value "<<iLBlock.id().value()<<std::endl;
    std::cout<<"lumiblock in run "<<iLBlock.run()<<std::endl;
    std::cout<<"lumiblock number "<<iLBlock.id().luminosityBlock()<<std::endl;
    edm::eventsetup::EventSetupRecordKey recordKey(edm::eventsetup::EventSetupRecordKey::TypeTag::findType("LuminosityInfoRcd"));
    if( recordKey.type() == edm::eventsetup::EventSetupRecordKey::TypeTag()) {
      //record not found
      std::cout <<"Record \"LuminosityInfoRcd"<<"\" does not exist "<<std::endl;
    }
    edm::ESHandle<lumi::LuminosityInfo> pPeds;
    context.get<LuminosityInfoRcd>().get(pPeds);
    const lumi::LuminosityInfo* myped=pPeds.product();
    if(myped->isNullData()){
      std::cout<<"null data"<<std::endl;
    }else{
      std::cout<<"\n Dumping summary info lumi::LuminosityInfo* "<<myped<<"\n";
      std::cout<<"lum section number "<<myped->lumisectionID()<<"\n";
      std::cout<<"nBunchCrossing  "<<myped->nBunchCrossing()<<"\n";
      std::cout<<"deadtime fraction  "<<myped->deadFraction()<<"\n";
      std::cout<<"lumi average value "<<myped->lumiAverage()<<"\n";
      std::cout<<"lumi average error "<<myped->lumiError()<<"\n";
      std::cout<<"lumi average quality "<<myped->lumiquality()<<"\n";
      std::cout<<"\n Dumping detail info of BX 1 "<<myped<<"\n";
      std::cout<<"ET lumi value  "<<myped->bunchCrossingInfo(1,lumi::ET).lumivalue<<"\n";
      std::cout<<"ET lumi error "<<myped->bunchCrossingInfo(1,lumi::ET).lumierr<<"\n";
      std::cout<<"ET lumi quality "<<myped->bunchCrossingInfo(1,lumi::ET).lumiquality<<"\n";
      std::cout<<"OCCD1 lumi value  "<<myped->bunchCrossingInfo(1,lumi::OCCD1).lumivalue<<"\n";
      std::cout<<"OCCD1 lumi error "<<myped->bunchCrossingInfo(1,lumi::OCCD1).lumierr<<"\n";
      std::cout<<"OCCD1 lumi quality "<<myped->bunchCrossingInfo(1,lumi::OCCD1).lumiquality<<"\n";
    } 
  }

  void
  LuminosityInfoAnalyzer::analyze(const edm::Event& e, const edm::EventSetup& context){
  }
  DEFINE_FWK_MODULE(LuminosityInfoAnalyzer);
}
