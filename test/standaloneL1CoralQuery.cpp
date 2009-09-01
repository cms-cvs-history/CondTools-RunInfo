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
//fixed size 128+64. Note: position has meanings
typedef std::vector<unsigned int> BITCOUNT;
typedef std::vector<std::string> BITNAME;
typedef std::vector<unsigned int> BITPRESCALE;

typedef unsigned int DEADCOUNT;

typedef std::vector<BITCOUNT> TriggerCountResult;//variable size positioned by sorted lumisections.Position has meanings

typedef std::vector<BITNAME> TriggerNameResult;
typedef std::vector<BITPRESCALE> PrescaleResult;
typedef std::vector<DEADCOUNT> TriggerDeadCountResult;

void printResult(const TriggerCountResult& triggercountresult){
  size_t lumisec=0;
  for(TriggerCountResult::const_iterator it=triggercountresult.begin();it!=triggercountresult.end();++it){
    std::cout<<"lumisec "<<lumisec<<std::endl;
    ++lumisec;
    size_t bitidx=0;
    for(BITCOUNT::const_iterator itt=it->begin();itt!=it->end();++itt){
      std::cout<<"\t bit: "<<bitidx<<" : count : "<<*itt<<std::endl;
      ++bitidx;
    }
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
    **/
    //uncomment if you want to see all the visible views
    /**listofviews.clear();
    listofviews=session->schema(gtschema).listViews();
    for( std::set<std::string>::iterator it=listofviews.begin(); it!=listofviews.end();++it ){
      std::cout<<"view: "<<*it<<std::endl;
    } 
    std::cout<<"commit transaction"<<std::endl;
    transaction.commit();
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
    
    //select counts,lsnr,algobit from cms_gt_mon.gt_mon_trig_algo_view where runnr=:runnumber order by lsnr,algobit;
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
    BITCOUNT mybitcount; 
    TriggerCountResult countresult;
    mybitcount.reserve(128);
    while( c.next() ){
      const coral::AttributeList& row = c.currentRow();     
      //row.toOutputStream( std::cout ) << std::endl;
      unsigned int lsnr=row["lsnr"].data<unsigned int>();
      unsigned int count=row["counts"].data<unsigned int>();
      if(s%128==0&&s!=0){
	countresult.push_back(mybitcount);
	mybitcount.clear();
      }
      mybitcount.push_back(count);
      ++s;
    }
    transaction.commit();
    printResult(countresult);
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
    
    transaction.commit();
    delete session;
    delete conService; 
  }catch(const std::exception& er){
    std::cout<<"caught exception "<<er.what()<<std::endl;
    throw er;
  }
}

