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
#include "RelationalAccess/ITable.h"
#include <iostream>
#include <exception>
int main(){
  /**retrieve hlt info with 2 queries
     select count(distinct PATHNAME ) as npath from HLT_SUPERVISOR_LUMISECTIONS_V2;
     select l.PATHNAME,l.LSNUMBER,l.L1PASS,l.PACCEPT,m.PSVALUE from hlt_supervisor_lumisections_v2 l, hlt_supervisor_scalar_map m where l.RUNNR=m.RUNNR and l.PSINDEX=m.PSINDEX and l.PATHNAME=m.PATHNAME and l.RUNNR=83037 order by l.LSNUMBER;
  **/
  std::string serviceName("oracle://cms_rcms/CMS_RUNINFO");
  std::string authName("/nfshome0/xiezhen/authentication.xml");
  int startRun=108239;
  int numberOfRuns=1;
  std::string tabname("HLT_SUPERVISOR_LUMISECTIONS_V2");
  std::string maptabname("HLT_SUPERVISOR_SCALAR_MAP");
  try{
    coral::ConnectionService* conService = new coral::ConnectionService();
    coral::Context::instance().PropertyManager().property("AuthenticationFile")->set(authName);
    conService->configuration().setAuthenticationService("CORAL/Services/XMLAuthenticationService");
    coral::MessageStream::setMsgVerbosity(coral::Error);
    coral::ISessionProxy* session = conService->connect( serviceName, coral::ReadOnly);
    coral::ITransaction& transaction=session->transaction();
    transaction.start(true); //true means readonly transaction

    coral::AttributeList bindVariableList;
    bindVariableList.extend("runnumber",typeid(int));
    int stopRun=startRun+numberOfRuns;
    std::cout<<"schema name "<<session->nominalSchema().schemaName()<<std::endl;
      //uncomment if you want to see all the visible tables
       std::set<std::string> listoftabs;
       listoftabs=session->nominalSchema().listTables();
       for( std::set<std::string>::iterator it=listoftabs.begin(); it!=listoftabs.end();++it ){
       std::cout<<"tab: "<<*it<<std::endl;
       } 
    
    
    if( !session->nominalSchema().existsTable(tabname) ) throw std::runtime_error("table not found");
    
    for( int currentRun=startRun;currentRun<stopRun;++currentRun){
      bindVariableList["runnumber"].data<int>()=currentRun;
      int npath=0;
      coral::IQuery* query0 = session->nominalSchema().tableHandle(tabname).newQuery();
      coral::AttributeList nls;
      nls.extend("npath",typeid(unsigned int));
      query0->addToOutputList("count(distinct PATHNAME)","npath");
      query0->setCondition("RUNNR =:runnumber",bindVariableList);
      query0->defineOutput(nls);
      coral::ICursor& c=query0->execute();
      if( !c.next() ){
	std::cout<<"requested run "<<currentRun<<" doesn't exist, do nothing"<<std::endl;
	continue;
      }else{
	npath=c.currentRow()["npath"].data<unsigned int>();
	std::cout<<"number of paths "<<npath<<std::endl;
	c.close();
	delete query0;
	if (npath==0) { 
	  std::cout<<"requested run is empty "<<currentRun<<" do nothing"<<std::endl;
	  continue;
	}
      }
      //queries per run
      coral::IQuery* query1 = session->nominalSchema().newQuery();
      query1->addToTableList(tabname,"l");
      query1->addToTableList(maptabname,"m");
      query1->addToOutputList("l.LSNUMBER","lsnumber");
      query1->addToOutputList("l.PATHNAME","pathname");
      query1->addToOutputList("l.L1PASS","hltinput");
      query1->addToOutputList("l.PACCEPT","hltratecounter");
      query1->addToOutputList("m.PSVALUE","prescale");
      query1->setCondition("l.RUNNR=m.RUNNR AND l.PSINDEX=m.PSINDEX AND l.PATHNAME=m.PATHNAME AND l.RUNNR =:runnumber",bindVariableList);
      query1->addToOrderList("l.LSNUMBER");
      query1->setRowCacheSize( 10692 ); //inmemory resultset cache
      coral::ICursor& cursor1 = query1->execute();
      
      int currentLumiSection=1;
      int currentPath=1;
      while( cursor1.next() ){
	const coral::AttributeList& row=cursor1.currentRow();
	currentLumiSection=(int)row["lsnumber"].data<long long>();
	std::cout<<"current run "<<currentRun<<std::endl;
	std::cout<<"currentLumiSection "<<currentLumiSection<<std::endl;
	std::cout<<"current path number "<<currentPath<<std::endl;
	std::cout<<"hltinput "<<row["hltinput"].data<long long>()<<std::endl;
	std::cout<<"hltratecounter "<<row["hltratecounter"].data<long long>()<<std::endl;
	std::cout<<"hltpathname "<<row["pathname"].data<std::string>()<<std::endl;
	std::cout<<"hltprescale "<<row["prescale"].data<long long>()<<std::endl;
	if(currentPath==npath){
	  std::cout<<"=====This is the end of lumisection===="<<currentRun<<":"<<currentLumiSection<<std::endl;
	  currentPath=0;
	}//end if it's last path in the current lumisection	 
	++currentPath;
      }
      cursor1.close();
      delete query1;
    }
    std::cout<<"commit transaction"<<std::endl;
    transaction.commit();
    delete session;
    delete conService; 
  }catch(const std::exception& er){
    std::cout<<"caught exception "<<er.what()<<std::endl;
    throw er;
  }
}

