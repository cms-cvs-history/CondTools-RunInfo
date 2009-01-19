#include "DataFormats/Provenance/interface/LuminosityBlockID.h"
#include "CondCore/DBCommon/interface/Time.h"
#include "CondFormats/Calibration/interface/LuminosityInfo.h"
#include "LumiDummyReader.h"
#include "CondTools/RunInfo/interface/LumiReaderFactory.h"
#include <iostream>
void
lumi::LumiDummyReader::fill(int startRun, 
			    int numberOfRuns, 
    std::vector< std::pair<lumi::LuminosityInfo*,cond::Time_t> >& result){
  //fake 10 runs with 30 lumisection each with 3564 bunchcrossing,100 hlt trigger
  for(int i=startRun; i<=startRun+numberOfRuns; ++i){
    for(int j=1; j<30; ++j){
      edm::LuminosityBlockID lu(i,j);
      cond::Time_t current=(cond::Time_t)(lu.value());
      lumi::LuminosityInfo* l=new lumi::LuminosityInfo;
      l->setLumiSectionId(j);
      l->setDeadtimeNormalization(0.5);
      lumi::LumiAverage avg(1.1,0.2,1,5);
      l->setLumiAverage(avg,lumi::ET);
      l->setLumiAverage(avg,lumi::OCCD1);
      l->setLumiAverage(avg,lumi::OCCD2);
      std::vector<lumi::BunchCrossingInfo> bxinfo;
      bxinfo.reserve(3564);
      for(int bxidx=1;bxidx<=3564;++bxidx){
	bxinfo.push_back(lumi::BunchCrossingInfo(bxidx,2.1,0.6,3,4));
      }
      l->setBunchCrossingData(bxinfo,lumi::ET);
      l->setBunchCrossingData(bxinfo,lumi::OCCD1);
      l->setBunchCrossingData(bxinfo,lumi::OCCD2);
      std::vector<lumi::HLTInfo> hltdata;
      for(int h=0; h<100; ++h){
	lumi::HLTInfo hltinfo(12+h,10+h,2+h);
	hltdata.push_back(hltinfo);
      }
      l->setHLTData(hltdata);
      std::cout<<"current "<<current<<std::endl;
      result.push_back(std::make_pair<lumi::LuminosityInfo*,cond::Time_t>(l,current));
    }
  }
  std::cout<<"result size "<<result.size()<<std::endl;
}
DEFINE_EDM_PLUGIN(lumi::LumiReaderFactory,lumi::LumiDummyReader,"dummy");
