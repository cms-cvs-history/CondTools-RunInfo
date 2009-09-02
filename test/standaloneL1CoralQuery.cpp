#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Exception.h"
#include "CoralBase/TimeStamp.h"
#include "CoralBase/MessageStream.h"
#include "CoralKernel/Context.h"
#include "CoralKernel/IHandle.h"
#include "CoralKernel/IProperty.h"
#include "CoralKernel/IPropertyManager.h"
#include "RelationalAccess/AccessMode.h"
#include "RelationalAccess/ConnectionService.h"
#include "RelationalAccess/ISessionProxy.h"
#include "RelationalAccess/IConnectionServiceConfiguration.h"
#include "RelationalAccess/ITransaction.h"
#include "RelationalAccess/ITypeConverter.h"
#include "RelationalAccess/IQuery.h"
#include "RelationalAccess/ICursor.h"
#include "RelationalAccess/ISchema.h"
#include "RelationalAccess/IView.h"
#include "RelationalAccess/ITable.h"
#include <iostream>
#include <exception>
#include <vector>

//per lumisection information
typedef unsigned int DEADCOUNT;
typedef std::vector<DEADCOUNT> TriggerDeadCountResult;
//per lumisection information aggregate by trigger bit
typedef std::vector<unsigned int> BITCOUNT;
typedef std::vector<unsigned int> BITPRESCALE;
typedef std::vector<BITCOUNT> TriggerCountResult_Algo;
typedef std::vector<BITCOUNT> TriggerCountResult_Tech;
typedef std::vector<BITPRESCALE> PrescaleResult_Algo;//NB.constant in run
typedef std::vector<BITPRESCALE> PrescaleResult_Tech;//NB.constant in run
//per run information
typedef std::vector<std::string> TriggerNameResult_Algo;
typedef std::vector<std::string> TriggerNameResult_Tech;

void printCountResult(const TriggerCountResult_Algo& algo,
		      const TriggerCountResult_Tech& tech){
  size_t lumisec=0;
  std::cout<<"===Algorithm trigger counts==="<<std::endl;
  for(TriggerCountResult_Algo::const_iterator it=algo.begin();it!=algo.end();++it){
    std::cout<<"lumisec "<<lumisec<<std::endl;
    ++lumisec;
    size_t bitidx=0;
    for(BITCOUNT::const_iterator itt=it->begin();itt!=it->end();++itt){
      std::cout<<"\t bit: "<<bitidx<<" : count : "<<*itt<<std::endl;
      ++bitidx;
    }
  }
  std::cout<<"===Technical trigger counts==="<<std::endl;
  lumisec=0;//reset lumisec counter
  for(TriggerCountResult_Tech::const_iterator it=tech.begin();it!=tech.end();++it){
    std::cout<<"lumisec "<<lumisec<<std::endl;
    ++lumisec;
    size_t bitidx=0;
    for(BITCOUNT::const_iterator itt=it->begin();itt!=it->end();++itt){
      std::cout<<"\t bit: "<<bitidx<<" : count : "<<*itt<<std::endl;
      ++bitidx;
    }
  }
}

void printDeadTimeResult(const TriggerDeadCountResult& result){
  size_t lumisec=0;
  std::cout<<"===Deadtime counts==="<<std::endl;
  for(TriggerDeadCountResult::const_iterator it=result.begin();it!=result.end();++it){
    std::cout<<"lumisec "<<lumisec<<" : counts : "<<*it<<std::endl;
    ++lumisec;
  }
}

void printTriggerNameResult(const TriggerNameResult_Algo& algonames,
			    const TriggerNameResult_Tech& technames){
  size_t bitidx=0;
  std::cout<<"===Algorithm trigger bit name==="<<std::endl;
  for(TriggerNameResult_Algo::const_iterator it=algonames.begin();
      it!=algonames.end();++it){
    std::cout<<"\t bit: "<<bitidx<<" : name : "<<*it<<std::endl;
    ++bitidx;    
  }
  bitidx=0;
  std::cout<<"===Tech trigger bit name==="<<std::endl;
  for(TriggerNameResult_Tech::const_iterator it=technames.begin();
      it!=technames.end();++it){
    std::cout<<"\t bit: "<<bitidx<<" : name : "<<*it<<std::endl;
    ++bitidx;    
  }
}

int main(){
  std::string serviceName("oracle://cms_omds_lb/CMS_GT_MON");
  std::string authName("/nfshome0/xiezhen/authentication.xml");
  int run=110823;
  //two blocks of views in schema cms_gt_mon&cms_gt
  std::string gtmonschema("CMS_GT_MON");
  std::string algoviewname("GT_MON_TRIG_ALGO_VIEW");
  std::string techviewname("GT_MON_TRIG_TECH_VIEW");
  std::string deadviewname("GT_MON_TRIG_DEAD_VIEW");

  std::string gtschema("CMS_GT");
  std::string runtechviewname("GT_RUN_TECH_VIEW");
  std::string runalgoviewname("GT_RUN_ALGO_VIEW");
  std::string runprescalgoviewname("GT_RUN_PRESC_ALGO_VIEW");
  std::string runpresctechviewname("GT_RUN_PRESC_TECH_VIEW");
  
  try{
    coral::ConnectionService* conService = new coral::ConnectionService();
    coral::Context::instance().PropertyManager().property("AuthenticationFile")->set(authName);
    conService->configuration().setAuthenticationService("CORAL/Services/XMLAuthenticationService");
    conService->configuration().disablePoolAutomaticCleanUp();
    conService->configuration().setConnectionTimeOut(0);
    coral::MessageStream::setMsgVerbosity(coral::Error);
    coral::ISessionProxy* session = conService->connect( serviceName, coral::ReadOnly);
    coral::ITransaction& transaction=session->transaction();
    transaction.start(true); //true means readonly transaction
    coral::AttributeList bindVariableList;
    bindVariableList.extend("runnumber",typeid(int));
    bindVariableList["runnumber"].data<int>()=run;
    std::cout<<"schema name "<<session->schema(gtmonschema).schemaName()<<std::endl;
    //uncomment if you want to see all the visible views
    /**std::set<std::string> listofviews;
       listofviews=session->schema(gtmonschema).listViews();
       for( std::set<std::string>::iterator it=listofviews.begin(); it!=listofviews.end();++it ){
       std::cout<<"view: "<<*it<<std::endl;
       } 
       std::cout<<"schema name "<<session->schema(gtschema).schemaName()<<std::endl;
       listofviews.clear();
       listofviews=session->schema(gtschema).listViews();
       for( std::set<std::string>::iterator it=listofviews.begin(); it!=listofviews.end();++it ){
       std::cout<<"view: "<<*it<<std::endl;
       } 
       std::cout<<"commit transaction"<<std::endl;
    **/
    transaction.commit();
    /**
       Part I
       query tables in schema cms_gt_mon
    **/
    transaction.start(true);
    coral::ISchema& gtmonschemaHandle=session->schema(gtmonschema);

    if(!gtmonschemaHandle.existsView(algoviewname)){
      throw std::runtime_error(std::string("non-existing view ")+algoviewname);
    }
    if(!gtmonschemaHandle.existsView(techviewname)){
      throw std::runtime_error(std::string("non-existing view ")+techviewname);
    }
    if(!gtmonschemaHandle.existsView(deadviewname)){
      throw std::runtime_error(std::string("non-existing view ")+deadviewname);
    }
    //
    //select counts,lsnr,algobit from cms_gt_mon.gt_mon_trig_algo_view where runnr=:runnumber order by lsnr,algobit;
    //
    coral::IQuery* Queryalgoview=gtmonschemaHandle.newQuery();
    Queryalgoview->addToTableList(algoviewname);
    coral::AttributeList qalgoOutput;
    qalgoOutput.extend("counts",typeid(unsigned int));
    qalgoOutput.extend("lsnr",typeid(unsigned int));
    qalgoOutput.extend("algobit",typeid(unsigned int));
    Queryalgoview->addToOutputList("counts");
    Queryalgoview->addToOutputList("lsnr");
    Queryalgoview->addToOutputList("algobit");
    Queryalgoview->setCondition("RUNNR =:runnumber",bindVariableList);
    Queryalgoview->addToOrderList("lsnr");
    Queryalgoview->addToOrderList("algobit");
    Queryalgoview->defineOutput(qalgoOutput);
    coral::ICursor& c=Queryalgoview->execute();
    if( !c.next() ){
      std::cout<<"requested run "<<run<<" doesn't exist, do nothing"<<std::endl;
      c.close();
      delete Queryalgoview;
      transaction.commit();
      return 0;
    }
    unsigned int s=0;
    BITCOUNT mybitcount_algo; 
    mybitcount_algo.reserve(128);
    TriggerCountResult_Algo countresult_algo;
    while( c.next() ){
      const coral::AttributeList& row = c.currentRow();     
      //row.toOutputStream( std::cout ) << std::endl;
      //unsigned int lsnr=row["lsnr"].data<unsigned int>();
      unsigned int count=row["counts"].data<unsigned int>();
      if(s%128==0&&s!=0){
	countresult_algo.push_back(mybitcount_algo);
	mybitcount_algo.clear();
      }
      mybitcount_algo.push_back(count);
      ++s;
    }
    delete Queryalgoview;
    //
    //select counts,lsnr,techbit from cms_gt_mon.gt_mon_trig_tech_view where runnr=:runnumber order by lsnr,techbit;
    //
    TriggerCountResult_Tech countresult_tech;
    BITCOUNT mybitcount_tech; 
    mybitcount_tech.reserve(64);
    coral::IQuery* Querytechview=gtmonschemaHandle.newQuery();
    Querytechview->addToTableList(techviewname);
    coral::AttributeList qtechOutput;
    qtechOutput.extend("counts",typeid(unsigned int));
    qtechOutput.extend("lsnr",typeid(unsigned int));
    qtechOutput.extend("techbit",typeid(unsigned int));
    Querytechview->addToOutputList("counts");
    Querytechview->addToOutputList("lsnr");
    Querytechview->addToOutputList("techbit");
    Querytechview->setCondition("RUNNR =:runnumber",bindVariableList);
    Querytechview->addToOrderList("lsnr");
    Querytechview->addToOrderList("techbit");
    Querytechview->defineOutput(qtechOutput);
    coral::ICursor& techcursor=Queryalgoview->execute();
    if( !techcursor.next() ){
      std::cout<<"requested run "<<run<<" doesn't exist, do nothing"<<std::endl;
      techcursor.close();
      delete Querytechview;
      transaction.commit();
      return 0;
    }
    s=0;
    while( techcursor.next() ){
      const coral::AttributeList& row = techcursor.currentRow();     
      //row.toOutputStream( std::cout ) << std::endl;
      //unsigned int lsnr=row["lsnr"].data<unsigned int>();
      unsigned int count=row["counts"].data<unsigned int>();
      if(s%64==0&&s!=0){
	countresult_tech.push_back(mybitcount_tech);
	mybitcount_tech.clear();
      }
      mybitcount_tech.push_back(count);
      ++s;
    }
    delete Querytechview;

    //
    //select counts,lsnr from cms_gt_mon.gt_mon_trig_dead_view where runnr=:runnumber and deadcounter=:countername order by lsnr;
    //
    TriggerDeadCountResult deadresult;
    coral::IQuery* Querydeadview=gtmonschemaHandle.newQuery();
    Querydeadview->addToTableList(deadviewname);
    coral::AttributeList qdeadOutput;
    qdeadOutput.extend("counts",typeid(unsigned int));
    qdeadOutput.extend("lsnr",typeid(unsigned int));
    Querydeadview->addToOutputList("counts");
    Querydeadview->addToOutputList("lsnr");
    coral::AttributeList bindVariablesDead;
    bindVariablesDead.extend("runnumber",typeid(int));
    bindVariablesDead.extend("countername",typeid(std::string));
    bindVariablesDead["runnumber"].data<int>()=run;
    bindVariablesDead["countername"].data<std::string>()=std::string("Deadtime");
    Querydeadview->setCondition("RUNNR =:runnumber AND DEADCOUNTER =:countername",bindVariablesDead);
    Querydeadview->addToOrderList("lsnr");
    Querydeadview->defineOutput(qdeadOutput);
    coral::ICursor& deadcursor=Querydeadview->execute();
    if( !deadcursor.next() ){
      std::cout<<"requested run "<<run<<" doesn't exist, do nothing"<<std::endl;
      deadcursor.close();
      delete Querydeadview;
      transaction.commit();
      return 0;
    }
    s=0;
    TriggerDeadCountResult deadtimeresult;
    while( deadcursor.next() ){
      const coral::AttributeList& row = deadcursor.currentRow();     
      //row.toOutputStream( std::cout ) << std::endl;
      //unsigned int lsnr=row["lsnr"].data<unsigned int>();
      unsigned int count=row["counts"].data<unsigned int>();
      deadtimeresult.push_back(count);
      ++s;
    }
    delete Querydeadview;
    transaction.commit();
    printCountResult(countresult_algo,countresult_tech);
    printDeadTimeResult(deadtimeresult);
    /**
       Part II
       query tables in schema cms_gt
     **/
    transaction.start(true);
    coral::ISchema& gtschemaHandle=session->schema(gtschema);
    if(!gtschemaHandle.existsView(runtechviewname)){
      throw std::runtime_error(std::string("non-existing view ")+runtechviewname);
    }
    if(!gtschemaHandle.existsView(runalgoviewname)){
      throw std::runtime_error(std::string("non-existing view ")+runalgoviewname);
    }
    if(!gtschemaHandle.existsView(runprescalgoviewname)){
      throw std::runtime_error(std::string("non-existing view ")+runprescalgoviewname);
    }
    if(!gtschemaHandle.existsView(runpresctechviewname)){
      throw std::runtime_error(std::string("non-existing view ")+runpresctechviewname);
    }
    //
    //select algo_index,name from cms_gt.gt_run_algo_view where runnumber=:runnumber order by algo_index;
    //
    std::map<unsigned int,std::string> triggernamemap;
    coral::IQuery* QueryName=gtschemaHandle.newQuery();
    QueryName->addToTableList(runalgoviewname);
    coral::AttributeList qAlgoNameOutput;
    qAlgoNameOutput.extend("algo_index",typeid(unsigned int));
    qAlgoNameOutput.extend("name",typeid(std::string));
    QueryName->addToOutputList("algo_index");
    QueryName->addToOutputList("name");
    QueryName->setCondition("runnumber =:runnumber",bindVariableList);
    QueryName->addToOrderList("algo_index");
    QueryName->defineOutput(qAlgoNameOutput);
    coral::ICursor& algonamecursor=QueryName->execute();
    while( algonamecursor.next() ){
      const coral::AttributeList& row = algonamecursor.currentRow();     
      //row.toOutputStream( std::cout ) << std::endl;
      unsigned int algo_index=row["algo_index"].data<unsigned int>();
      std::string algo_name=row["name"].data<std::string>();
      triggernamemap.insert(std::make_pair(algo_index,algo_name));
    }
    delete QueryName;

    //
    //select techtrig_index,name from cms_gt.gt_run_tech_view where runnumber=:runnumber order by techtrig_index;
    //
    std::map<unsigned int,std::string> techtriggernamemap;
    coral::IQuery* QueryTechName=gtschemaHandle.newQuery();
    QueryTechName->addToTableList(runtechviewname);
    coral::AttributeList qTechNameOutput;
    qTechNameOutput.extend("techtrig_index",typeid(unsigned int));
    qTechNameOutput.extend("name",typeid(std::string));
    QueryTechName->addToOutputList("techtrig_index");
    QueryTechName->addToOutputList("name");
    QueryTechName->setCondition("runnumber =:runnumber",bindVariableList);
    QueryTechName->addToOrderList("techtrig_index");
    QueryTechName->defineOutput(qTechNameOutput);
    coral::ICursor& technamecursor=QueryTechName->execute();
    while( technamecursor.next() ){
      const coral::AttributeList& row = technamecursor.currentRow();     
      //row.toOutputStream( std::cout ) << std::endl;
      unsigned int tech_index=row["techtrig_index"].data<unsigned int>();
      std::string tech_name=row["name"].data<std::string>();
      techtriggernamemap.insert(std::make_pair(tech_index,tech_name));
    }
    delete QueryTechName;
    transaction.commit();

    //reprocess Algo name result filling unallocated trigger bit with string "False"
    TriggerNameResult_Algo nameresult_algo;    
    nameresult_algo.reserve(128);
    for(size_t algoidx=0;algoidx<128;++algoidx){
      std::map<unsigned int,std::string>::iterator pos=triggernamemap.find(algoidx);
      if(pos!=triggernamemap.end()){
	nameresult_algo.push_back(pos->second);
      }else{
	nameresult_algo.push_back("False");
      }
    }
    //reprocess Tech name result filling unallocated trigger bit with string "False"
    TriggerNameResult_Tech nameresult_tech;
    nameresult_tech.reserve(64);
    for(size_t techidx=0;techidx<64;++techidx){
      std::map<unsigned int,std::string>::iterator pos=techtriggernamemap.find(techidx);
      if(pos!=techtriggernamemap.end()){
	nameresult_tech.push_back(pos->second);
      }else{
	nameresult_tech.push_back("False");
      }
    }
    
    printTriggerNameResult(nameresult_algo,nameresult_tech);

    delete session;
    delete conService; 
  }catch(const std::exception& er){
    std::cout<<"caught exception "<<er.what()<<std::endl;
    throw er;
  }
}

