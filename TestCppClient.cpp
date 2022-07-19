/* Copyright (C) 2019 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */
#include "StdAfx.h"

#include "TestCppClient.h"

#include "EClientSocket.h"
#include "EPosixClientSocketPlatform.h"

#include "Contract.h"
#include "Order.h"
#include "OrderState.h"
#include "Execution.h"
#include "CommissionReport.h"
#include "ContractSamples.h"
#include "OrderSamples.h"
#include "ScannerSubscription.h"
#include "ScannerSubscriptionSamples.h"
#include "executioncondition.h"
#include "PriceCondition.h"
#include "MarginCondition.h"
#include "PercentChangeCondition.h"
#include "TimeCondition.h"
#include "VolumeCondition.h"
#include "AvailableAlgoParams.h"
#include "FAMethodSamples.h"
#include "CommonDefs.h"
#include "AccountSummaryTags.h"
#include "Utils.h"

#include <stdio.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <ctime>
#include <fstream>
#include <cstdint>
#include "biglog.h"

const int PING_DEADLINE = 2; // seconds
const int SLEEP_BETWEEN_PINGS = 30; // seconds

///////////////////////////////////////////////////////////
// member funcs
//! [socket_init]
TestCppClient::TestCppClient() :
      m_osSignal(2000)//2-seconds timeout
    , m_pClient(new EClientSocket(this, &m_osSignal))
	, m_state(ST_CONNECT)
	, m_sleepDeadline(0)
	, m_orderId(0)
    , m_extraAuth(false)
{
}
//! [socket_init]
TestCppClient::~TestCppClient()
{
	// destroy the reader before the client
	if( m_pReader )
		m_pReader.reset();

	delete m_pClient;
}

bool TestCppClient::connect(const char *host, int port, int clientId)
{
	// trying to connect
	printf( "Connecting to %s:%d clientId:%d\n", !( host && *host) ? "127.0.0.1" : host, port, clientId);
	
	//! [connect]
	bool bRes = m_pClient->eConnect( host, port, clientId, m_extraAuth);
	//! [connect]
	
	if (bRes) {
		printf( "Connected to %s:%d clientId:%d\n", m_pClient->host().c_str(), m_pClient->port(), clientId);
		//! [ereader]
		m_pReader = std::unique_ptr<EReader>( new EReader(m_pClient, &m_osSignal) );
		m_pReader->start();
		//! [ereader]
	}
	else
		printf( "Cannot connect to %s:%d clientId:%d\n", m_pClient->host().c_str(), m_pClient->port(), clientId);

	return bRes;
}

void TestCppClient::disconnect() const
{
	m_pClient->eDisconnect();

	printf ( "Disconnected\n");
}

bool TestCppClient::isConnected() const
{
	return m_pClient->isConnected();
}

void TestCppClient::setConnectOptions(const std::string& connectOptions)
{
	m_pClient->setConnectOptions(connectOptions);
}

void TestCppClient::processMessages()
{
	time_t now = time(NULL);

	/*****************************************************************/
    /* Below are few quick-to-test examples on the IB API functions grouped by functionality. Uncomment the relevant methods. */
    /*****************************************************************/
	switch (m_state) {
		case ST_PNLSINGLE:
			pnlSingleOperation();
			break;
		case ST_PNLSINGLE_ACK:
			break;
		case ST_PNL:
			pnlOperation();
			break;
		case ST_PNL_ACK:
			break;
		case ST_TICKDATAOPERATION:
			tickDataOperation();
			break;
		case ST_TICKDATAOPERATION_ACK:
			break;
		case ST_TICKOPTIONCOMPUTATIONOPERATION:
			tickOptionComputationOperation();
			break;
		case ST_TICKOPTIONCOMPUTATIONOPERATION_ACK:
			break;
		case ST_DELAYEDTICKDATAOPERATION:
			delayedTickDataOperation();
			break;
		case ST_DELAYEDTICKDATAOPERATION_ACK:
			break;
		case ST_MARKETDEPTHOPERATION:
			marketDepthOperations();
			break;
		case ST_MARKETDEPTHOPERATION_ACK:
			break;
		case ST_REALTIMEBARS:
			realTimeBars();
			break;
		case ST_REALTIMEBARS_ACK:
			break;
		case ST_MARKETDATATYPE:
			marketDataType();
			break;
		case ST_MARKETDATATYPE_ACK:
			break;
		case ST_HISTORICALDATAREQUESTS:
			historicalDataRequests();
			break;
		case ST_HISTORICALDATAREQUESTS_ACK:
			break;
		case ST_OPTIONSOPERATIONS:
			optionsOperations();
			break;
		case ST_OPTIONSOPERATIONS_ACK:
			break;
		case ST_CONTRACTOPERATION:
			contractOperations();
			break;
		case ST_CONTRACTOPERATION_ACK:
			break;
		case ST_MARKETSCANNERS:
			marketScanners();
			break;
		case ST_MARKETSCANNERS_ACK:
			break;
		case ST_FUNDAMENTALS:
			fundamentals();
			break;
		case ST_FUNDAMENTALS_ACK:
			break;
		case ST_BULLETINS:
			bulletins();
			break;
		case ST_BULLETINS_ACK:
			break;
		case ST_ACCOUNTOPERATIONS:
			accountOperations();
			break;
		case ST_ACCOUNTOPERATIONS_ACK:
			break;
		case ST_ORDEROPERATIONS:
			orderOperations();
			break;
		case ST_ORDEROPERATIONS_ACK:
			break;
		case ST_OCASAMPLES:
			ocaSamples();
			break;
		case ST_OCASAMPLES_ACK:
			break;
		case ST_CONDITIONSAMPLES:
			conditionSamples();
			break;
		case ST_CONDITIONSAMPLES_ACK:
			break;
		case ST_BRACKETSAMPLES:
			bracketSample();
			break;
		case ST_BRACKETSAMPLES_ACK:
			break;
		case ST_HEDGESAMPLES:
			hedgeSample();
			break;
		case ST_HEDGESAMPLES_ACK:
			break;
		case ST_TESTALGOSAMPLES:
			testAlgoSamples();
			break;
		case ST_TESTALGOSAMPLES_ACK:
			break;
		case ST_FAORDERSAMPLES:
			financialAdvisorOrderSamples();
			break;
		case ST_FAORDERSAMPLES_ACK:
			break;
		case ST_FAOPERATIONS:
			financialAdvisorOperations();
			break;
		case ST_FAOPERATIONS_ACK:
			break;
		case ST_DISPLAYGROUPS:
			testDisplayGroups();
			break;
		case ST_DISPLAYGROUPS_ACK:
			break;
		case ST_MISCELANEOUS:
			miscelaneous();
			break;
		case ST_MISCELANEOUS_ACK:
			break;
		case ST_FAMILYCODES:
			reqFamilyCodes();
			break;
		case ST_FAMILYCODES_ACK:
			break;
		case ST_SYMBOLSAMPLES:
			reqMatchingSymbols();
			break;
		case ST_SYMBOLSAMPLES_ACK:
			break;
		case ST_REQMKTDEPTHEXCHANGES:
			reqMktDepthExchanges();
			break;
		case ST_REQMKTDEPTHEXCHANGES_ACK:
			break;
		case ST_REQNEWSTICKS:
			reqNewsTicks();
			break;
		case ST_REQNEWSTICKS_ACK:
			break;
		case ST_REQSMARTCOMPONENTS:
			reqSmartComponents();
			break;
		case ST_REQSMARTCOMPONENTS_ACK:
			break;
		case ST_NEWSPROVIDERS:
			reqNewsProviders();
			break;
		case ST_NEWSPROVIDERS_ACK:
			break;
		case ST_REQNEWSARTICLE:
			reqNewsArticle();
			break;
		case ST_REQNEWSARTICLE_ACK:
			break;
		case ST_REQHISTORICALNEWS:
			reqHistoricalNews();
			break;
		case ST_REQHISTORICALNEWS_ACK:
			break;
		case ST_REQHEADTIMESTAMP:
			reqHeadTimestamp();
			break;
		case ST_REQHISTOGRAMDATA:
			reqHistogramData();
			break;
		case ST_REROUTECFD:
			rerouteCFDOperations();
			break;
		case ST_MARKETRULE:
			marketRuleOperations();
			break;
		case ST_CONTFUT:
			continuousFuturesOperations();
			break;
        case ST_REQHISTORICALTICKS:
            reqHistoricalTicks();
            break;
        case ST_REQHISTORICALTICKS_ACK:
            break;
		case ST_REQTICKBYTICKDATA:
			reqTickByTickData();
			break;
		case ST_REQTICKBYTICKDATA_ACK:
			break;
		case ST_WHATIFSAMPLES:
			whatIfSamples();
			break;
		case ST_WHATIFSAMPLES_ACK:
			break;
		case ST_PING:
			reqCurrentTime();
			break;
		case ST_PING_ACK:
			if( m_sleepDeadline < now) {
				disconnect();
				return;
			}
			break;
		case ST_IDLE:
			if( m_sleepDeadline < now) {
				m_state = ST_PING;
				return;
			}
			break;
	}

	m_osSignal.waitForSignal();
	errno = 0;
	m_pReader->processMsgs();
}

//////////////////////////////////////////////////////////////////
// methods
//! [connectack]
void TestCppClient::connectAck() {
	if (!m_extraAuth && m_pClient->asyncEConnect())
        m_pClient->startApi();
}
//! [connectack]

void TestCppClient::reqCurrentTime()
{
	printf( "Requesting Current Time\n");

	// set ping deadline to "now + n seconds"
	m_sleepDeadline = time( NULL) + PING_DEADLINE;

	m_state = ST_PING_ACK;

	m_pClient->reqCurrentTime();
}

void TestCppClient::pnlOperation()
{
	//! [reqpnl]
    m_pClient->reqPnL(7001, "DUD00029", "");
	//! [reqpnl]
	
    std::this_thread::sleep_for(std::chrono::seconds(2));

	//! [cancelpnl]
    m_pClient->cancelPnL(7001);
	//! [cancelpnl] 
	
    m_state = ST_PNL_ACK;
}

void TestCppClient::pnlSingleOperation()
{
	//! [reqpnlsingle]
    m_pClient->reqPnLSingle(7002, "DUD00029", "", 268084);
	//! [reqpnlsingle]
	
    std::this_thread::sleep_for(std::chrono::seconds(2));

	//! [cancelpnlsingle]
    m_pClient->cancelPnLSingle(7002);
	//! [cancelpnlsingle]
	
    m_state = ST_PNLSINGLE_ACK;
}

void TestCppClient::tickDataOperation()
{
	/*** Requesting real time market data ***/
    std::this_thread::sleep_for(std::chrono::seconds(1));
    //! [reqmktdata]
	m_pClient->reqMktData(1001, ContractSamples::StockComboContract(), "", false, false, TagValueListSPtr());
	m_pClient->reqMktData(1002, ContractSamples::OptionWithLocalSymbol(), "", false, false, TagValueListSPtr());
	//! [reqmktdata]
	//! [reqmktdata_snapshot]
	m_pClient->reqMktData(1003, ContractSamples::FutureComboContract(), "", true, false, TagValueListSPtr());
	//! [reqmktdata_snapshot]

	/*
	//! [regulatorysnapshot]
	// Each regulatory snapshot incurs a fee of 0.01 USD
	m_pClient->reqMktData(1013, ContractSamples::USStock(), "", false, true, TagValueListSPtr());
	//! [regulatorysnapshot]
	*/
	
	//! [reqmktdata_genticks]
	//Requesting RTVolume (Time & Sales) and shortable generic ticks
	m_pClient->reqMktData(1004, ContractSamples::USStockAtSmart(), "233,236", false, false, TagValueListSPtr());
	//! [reqmktdata_genticks]

	//! [reqmktdata_contractnews]
	// Without the API news subscription this will generate an "invalid tick type" error
	m_pClient->reqMktData(1005, ContractSamples::USStock(), "mdoff,292:BZ", false, false, TagValueListSPtr());
	m_pClient->reqMktData(1006, ContractSamples::USStock(), "mdoff,292:BT", false, false, TagValueListSPtr());
	m_pClient->reqMktData(1007, ContractSamples::USStock(), "mdoff,292:FLY", false, false, TagValueListSPtr());
	m_pClient->reqMktData(1008, ContractSamples::USStock(), "mdoff,292:MT", false, false, TagValueListSPtr());
	//! [reqmktdata_contractnews]
	//! [reqmktdata_broadtapenews]
	m_pClient->reqMktData(1009, ContractSamples::BTbroadtapeNewsFeed(), "mdoff,292", false, false, TagValueListSPtr());
	m_pClient->reqMktData(1010, ContractSamples::BZbroadtapeNewsFeed(), "mdoff,292", false, false, TagValueListSPtr());
	m_pClient->reqMktData(1011, ContractSamples::FLYbroadtapeNewsFeed(), "mdoff,292", false, false, TagValueListSPtr());
	//! [reqmktdata_broadtapenews]

	//! [reqoptiondatagenticks]
	//Requesting data for an option contract will return the greek values
	m_pClient->reqMktData(1013, ContractSamples::USOptionContract(), "", false, false, TagValueListSPtr());
	//! [reqoptiondatagenticks]
	
	//! [reqfuturesopeninterest]
	//Requesting data for a futures contract will return the futures open interest
	m_pClient->reqMktData(1014, ContractSamples::SimpleFuture(), "mdoff,588", false, false, TagValueListSPtr());
	//! [reqfuturesopeninterest]

	//! [reqpreopenbidask]
	//Requesting data for a futures contract will return the pre-open bid/ask flag
	m_pClient->reqMktData(1015, ContractSamples::SimpleFuture(), "", false, false, TagValueListSPtr());
	//! [reqpreopenbidask]

	//! [reqavgoptvolume]
	//Requesting data for a stock will return the average option volume
	m_pClient->reqMktData(1016, ContractSamples::USStockAtSmart(), "mdoff,105", false, false, TagValueListSPtr());
	//! [reqavgoptvolume]

	//! [reqetfticks]
	//Requesting data for an ETF will return the ETF ticks
	m_pClient->reqMktData(1017, ContractSamples::etf(), "mdoff,576,577,578,614,623", false, false, TagValueListSPtr());
	//! [reqetfticks]

	std::this_thread::sleep_for(std::chrono::seconds(1));
	/*** Canceling the market data subscription ***/
	//! [cancelmktdata]
	m_pClient->cancelMktData(1001);
	m_pClient->cancelMktData(1002);
	m_pClient->cancelMktData(1003);
	m_pClient->cancelMktData(1014);
	m_pClient->cancelMktData(1015);
	m_pClient->cancelMktData(1016);
	m_pClient->cancelMktData(1017);
	//! [cancelmktdata]

	m_state = ST_TICKDATAOPERATION_ACK;
}

void TestCppClient::tickOptionComputationOperation()
{
	/*** Requesting real time market data ***/
	std::this_thread::sleep_for(std::chrono::seconds(1));

	m_pClient->reqMarketDataType(4);

	//! [reqmktdata]
	m_pClient->reqMktData(2001, ContractSamples::OptionWithLocalSymbol(), "", false, false, TagValueListSPtr());
	//! [reqmktdata]

	std::this_thread::sleep_for(std::chrono::seconds(10));
	/*** Canceling the market data subscription ***/
	//! [cancelmktdata]
	m_pClient->cancelMktData(2001);
	//! [cancelmktdata]

	m_state = ST_TICKOPTIONCOMPUTATIONOPERATION_ACK;
}

void TestCppClient::delayedTickDataOperation()
{
	/*** Requesting delayed market data ***/

	//! [reqmktdata_delayedmd]
	//m_pClient->reqMarketDataType(4); // send delayed-frozen (4) market data type
	//m_pClient->reqMktData(1013, ContractSamples::HKStk(), "", false, false, TagValueListSPtr());
	m_pClient->reqMktData(1014, ContractSamples::USOptionContract(), "", false, false, TagValueListSPtr());
	//! [reqmktdata_delayedmd]

	std::this_thread::sleep_for(std::chrono::seconds(8));

	/*** Canceling the delayed market data subscription ***/
	//! [cancelmktdata_delayedmd]
	//m_pClient->cancelMktData(1013);
	m_pClient->cancelMktData(1014);
	//! [cancelmktdata_delayedmd]

	m_state = ST_DELAYEDTICKDATAOPERATION_ACK;
}

void TestCppClient::marketDepthOperations()
{
	/*** Requesting the Deep Book ***/
	//! [reqmarketdepth]
	m_pClient->reqMktDepth(2001, ContractSamples::EurGbpFx(), 5, false, TagValueListSPtr());
	//! [reqmarketdepth]
	std::this_thread::sleep_for(std::chrono::seconds(2));
	/*** Canceling the Deep Book request ***/
	//! [cancelmktdepth]
	m_pClient->cancelMktDepth(2001, false);
	//! [cancelmktdepth]

	/*** Requesting the Deep Book ***/
	//! [reqmarketdepth]
	m_pClient->reqMktDepth(2002, ContractSamples::EuropeanStock(), 5, true, TagValueListSPtr());
	//! [reqmarketdepth]
	std::this_thread::sleep_for(std::chrono::seconds(5));
	/*** Canceling the Deep Book request ***/
	//! [cancelmktdepth]
	m_pClient->cancelMktDepth(2002, true);
	//! [cancelmktdepth]

	m_state = ST_MARKETDEPTHOPERATION_ACK;
}

void TestCppClient::realTimeBars()
{
	/*** Requesting real time bars ***/
	//! [reqrealtimebars]
	m_pClient->reqRealTimeBars(3001, ContractSamples::EurGbpFx(), 5, "MIDPOINT", true, TagValueListSPtr());
	//! [reqrealtimebars]
	std::this_thread::sleep_for(std::chrono::seconds(2));
	/*** Canceling real time bars ***/
    //! [cancelrealtimebars]
	m_pClient->cancelRealTimeBars(3001);
    //! [cancelrealtimebars]

	m_state = ST_REALTIMEBARS_ACK;
}

void TestCppClient::marketDataType()
{
	//! [reqmarketdatatype]
	/*** By default only real-time (1) market data is enabled
		 Sending frozen (2) enables frozen market data
		 Sending delayed (3) enables delayed market data and disables delayed-frozen market data
		 Sending delayed-frozen (4) enables delayed and delayed-frozen market data
		 Sending real-time (1) disables frozen, delayed and delayed-frozen market data ***/
	m_pClient->reqMarketDataType(2);
	//! [reqmarketdatatype]

	m_state = ST_MARKETDATATYPE_ACK;
}

void TestCppClient::historicalDataRequests()
{
	/*** Requesting historical data ***/
	//! [reqhistoricaldata]
	std::time_t rawtime;
    std::tm* timeinfo;
    char queryTime [80];

	std::time(&rawtime);
    timeinfo = std::localtime(&rawtime);
	std::strftime(queryTime, 80, "%Y%m%d %H:%M:%S", timeinfo);

	m_pClient->reqHistoricalData(4001, ContractSamples::EurGbpFx(), queryTime, "1 M", "1 day", "MIDPOINT", 1, 1, false, TagValueListSPtr());
	m_pClient->reqHistoricalData(4002, ContractSamples::EuropeanStock(), queryTime, "10 D", "1 min", "TRADES", 1, 1, false, TagValueListSPtr());
	//! [reqhistoricaldata]
	std::this_thread::sleep_for(std::chrono::seconds(2));
	/*** Canceling historical data requests ***/
	m_pClient->cancelHistoricalData(4001);
	m_pClient->cancelHistoricalData(4002);

	m_state = ST_HISTORICALDATAREQUESTS_ACK;
}

void TestCppClient::optionsOperations()
{
	//! [reqsecdefoptparams]
	//m_pClient->reqSecDefOptParams(0, "IBM", "", "STK", 8314);
	m_pClient->reqSecDefOptParams(1, "AAPL", "", "STK", 265598);
	//! [reqsecdefoptparams]

	//! [calculateimpliedvolatility]
	m_pClient->calculateImpliedVolatility(5001, ContractSamples::OptionWithLocalSymbol(), 0.5, 55, TagValueListSPtr());
	//! [calculateimpliedvolatility]

	//** Canceling implied volatility ***
	m_pClient->cancelCalculateImpliedVolatility(5001);

	//! [calculateoptionprice]
	m_pClient->calculateOptionPrice(5002, ContractSamples::OptionWithLocalSymbol(), 0.6, 55, TagValueListSPtr());
	//! [calculateoptionprice]

	//** Canceling option's price calculation ***
	m_pClient->cancelCalculateOptionPrice(5002);

	//! [exercise_options]
	//** Exercising options ***
	m_pClient->exerciseOptions(5003, ContractSamples::OptionWithTradingClass(), 1, 1, "", 1);
	//! [exercise_options]

	m_state = ST_OPTIONSOPERATIONS_ACK;
}


char StockNameListE[][64] =
{
	"GOOG","TSLA","QQQ","MSFT","CRWD","META",
	"WHR","BIDU","AAPL","GME","BABA","AMZN",
	"ABNB","DHI","DASH","JD","PDD","CRSP",
	"IBKR","GTLB","FUTU","ARKK","U","ENB",

	"RBLX","FVRR","ZLAB","CPE","INTC","XPEV",
	"RIVN","AR","PBF","UPST","BILI","PARA",
	"DWAC","LAC","LMND","PATH","BTU","BE",
	"M","BEKE","CLF","ICPT","EDIT","AMC",

	"AAL","LYFT","DWNG","TWOU","FGEN","OZON",
	"YANG","CXW","ACCD","PTON","CEE","RDFN",
	"FSR","GTBIF","MARA","DFFN","COTY","KOSS",
	"SPCE","GDRX","SOFI","RSX","NKLA","TME",

	"VIRI","TIGR","RKLB","YALA","CAN","GNFT",
	"TLRY","CRON","CRLBF","FUBO","XYF","CGC",
	"GOEV","GRAB","DBVT","AMRS","YSG","RLX",
	"BQ","PSFE","OUST","XNET","BKYI","ZY",

	"ZH","GTH","SOLO","ASTR","ACB","SKLZ",
	"CTRM","BEST","PHUN","GVP","BVXV","OGZPY",
	"CLVR","OGI","VLDR","RAAS","BXRX","TIRX",
	"BTCM","EBON","RMO","MTCR","MUX","MF",


	"SNDL","CRBP","NAK","HEXO","KERN","ORPHY",
	"COST","ADBE","ARGX","MA","MDB","MCD",
	"ISRG","BGNE","NFLX","MRNA","CRM","AXP",
	"PWR","WMT","ZM","XOP","OAS","NKE",

	"OKTA","ROKU","CVS","PM","TWLO","SAP",
	"NRGD","ETSY","XEC","MPC","TSM","PCAR",
	"AMD","ATVI","SE","MS","NVAX","ROST",
	"DASH","PYPL","EQR","WLL","LPI","SQ",

	"CLR","SPTY","SCHW","KO","OXY","WYNN",
	"LEGN","W","NET","NWN","AIG","YUMC",
	"C","KRA","AA","OVV","HP","PLCE",
	"TDOC","BTI","WFC","KHC","ATHM","TWTR",


	"UAL","SAGE","SHOP","Z","RCL","EQT",
	"SM","HOG","HSBS","SPR","VIAC","MGM",
	"KSS","AVID","ZNH","STOR","BZ","TCOM",
	"RRC","WES","CVNA","AFRM","NIO","UBER",

	"COG","SSL","SVMK","VREX","RMBS","SOHOO",
	"PLUG","CEA","SSPK","TRIP","GOOS","X",
	"NOV","CNX","CNK","SNAP","DVAX","MCS",
	"ARCE","VALE","GBTC","DKNG","ALT","CRK",

	"F","NCLH","IMAB","GT","MSOS","PCG",
	"VIPS","QS","BTWN","PTGX","AM","MAC",
	"CCL","HOOD","STNE","GHL","INSE","FTCH",
	"HDSN","NTZ","BCS","NIU","RAD","AHT",

	"AZUL","LUKOY","JMIA","API","SWN","CDEV",
	"MLCO","SFIX","TEDU","BLUE","MOMO","NXTC",
	"TAL","PRVB","FINV","GRWG","FCEL","GRCL",
	"IQ","AEG","HUYA","UK","NMR","BNR",

	"NERV","HRVSF","RIG","VFF","PHX","BCLI",
	"IH","CRTX","BKKT","HTBX","SLQT","DIDI",
	"CJJD","CMRX","OVID","NM","OCFT","DUO",
	"CDXC","EDSA","SXTC","RIDE","BYSI","WISH",

	"AMRN","UTME","ADVM","BLCT","MDVL","METX",
	"IMRA","FAMI","MLND","KRKR","APRE","CNTB",
	"GOVX","TISI","CMCM","BEDU","CYCN","WNW",
	"EZGO","VTVT","HOFV","UBX","GOCO","KRBP",

	"MKD","CLRB","FMCC","QH","YCBD","TRX",
	"PIXY","GLG","MTC","NTES","SIMO","JKS",
	"DQ","PTR","SNP","CHT","HTHT","LI",
	"RGC","CSIQ","GDS","RENN","YY","ZTO",

	"EDU","WB","HUDI","HNP","NOAH","SHI",
	"SOHU","QFIN","HCM","HOLI","FEDU","HLG",
	"BZUN","ACH","EH","CYD","YMM","DADA",
	"PUYI","SEED","LFC","CD","MNSO","UMC",

	"HYW","HIMX","SVA","NAAS","DDL","LU",
	"VNET","CGA","STG","ASX","FANH","SOL",
	"FFHL","HKIT","FENG","OPRA","DAO","KC",
	"EM","NTP","GHG","DOYU","UXIN","GOTU",

	"BEAM","NTLA","SGMO","LIND","UAL","HA"


};

char StockNameListEx2[][64] =
{
	"CRSP","BLUE","BEAM","NTLA","EDIT","SGMO",
	"CCL","NCLH","RCL","LIND","AAL","UAL",
	"HA","AZUL","DAL","LUV","CPA","SKYW",
	"ALGT","MESA","ULCC","ALK","GOL","JBLU",
	"VLRS","RYAAY","CEA","ZNH","SNCY","SAVE",
	"ALGT","MESA","ULCC","ALK","GOL","JBLU",

	"UP","JOBY","MHK","BSET","PRPL","LEG",
	"LZB","LIVE","TPX","LOVE","ETD","HOFT",
	"FLXS","DXYN","NVFY","NTZ","AXL","ANDT",
	"XT","DAN","GTX","REE","LEA","UFAB",
	"MGA","CPS","XPEL","APTV","KNDI","ALV",
	"VC","SLDP","LCII","MOD","LAZR","BWA",

	"QS","MPAA","GTXAP","FOXF","HLLY","THRM",
	"GNTX","SMP","SRI","FRSX","TEN","CAAS",
	"DORM","PATK","STRT","SYPR","SUP","HZN",
	"GME","CONN","RCII","BBY","EM","AER",
	"AL","AIR","ATSG","AIRT","RMO","VLDR",
	"MVIS","GT","LCII","MOD","LAZR","BWA",
};
char StockNameListt[][64] =
{
	"SPSM","LMND"
};

char StockNameList[][64] =
{
	"AAPL","MSFT","GOOG","AMZN","TSLA","BRK.A",
	"AVGOP","UNH","JNI","META","V","TSM",
	"NVDA","XOM","PG","SPY","VMT","JPM",
	"MA","HD","BABA","PEE","LLY","IVV",
	"CVX","KO","ABBV","BAC","VTI","NVO",
	"VOO","MRK","PEP","COST","TM","VZ",

	"TMO","AZN","AVGO","PCGU","ABT","ORCL",
	"MCD","NVS","TBS","DHR","TBB","CMCSA",
	"CSCO","ADBE","SHEL","ASML","ACN","DIS",
	"TMUS","CRM","NKE","BMY","NEE","UPS",
	"QQQ","INTC","WFC","QCOM","T","PM",
	"TXN","RTX","RY","LIN","MS","AMGN",

	"UNP","BHP","SNY","IBM","CVS","HSBS",
	"AMD","TEE","AMT","HON","SCHW","MDT",
	"SPGI","UL","LOW","ELV","TD","LMT",
	"INTU","HDB","BUD","COP","EQNR","GS",
	"GSK","AXP","SONY","SAP","DEO","VTV",
	"JD","CAT","RIO","BLK","BTI","PLD",

	"DE","VEA","C","SBUX","ADP","CI",
	"EL","BA","MDLZ","NOW","BP","CHTR",
	"ENB","IEFA","DUK","PYPL","ZTS","CB",
	"PTR","AGG","BND","GILD","NFLX","AMAT",
	"MMC","ADI","MO","CNI","SO","INFY",
	"VRTX","MMM","CCI","SYK","PBR","CME",


	"NOC","ISRG","PDD","BKNG","BAM","GE",
	"VWO","TJX","MRNA","BNS","USB","VUG",
	"PGR","BX","BDX","PBR.A","MUFG","TGT",
	"IBN","REGN","CP","CL","MU","PNC",
	"BMO","D","SHW","IEMG","AMX","VALE",
	"TFC","WM","AMOV","CSX","ABNB","HUM",

	"VIG","IJR","GD","EW","FISV","ATVI",
	"GJR","IWF","EOG","AESC","FDX","GJO",
	"AON","EQIX","FIS","ITW","GJP","GLD",
	"IJH","LRCX","DG","CNQ","STZ.B","PSA",
	"EPD","OXY","NTES","UBS","SNP","NSC",
	"BSX","ICE","GJT","KDP","APD","RELX",

	"MNST","TRI","IWD","PXD","CNC","MCO",
	"ETN","ENBA","IWM","HCA","MET","ABB",
	"BIDU","TRP","PANW","TEAM","AEP","FTNT",
	"KHC","NGG","VO","MCK","EMR","SNPS",
	"KLAC","VMW","F","SLB","SNOW","NEM",
	"SRE","GM","KMB","MAR","GIS","NIMC",

	"HSY","ECL","LHX","MPC","BCE","EFA",
	"TAK","LFC","VYM","SYY","STZ","CM",
	"VXUS","EXC","VOD","ORLY","VLO","SU",
	"CNDS","MHNC","UBER","COF","AZO","O",
	"ROP","PAYX","HMC","SMFG","NTR","ADM",
	"SHOP","SAN","ITOT","SE","RSG","AIG",

	"CRWD","IQV","NXPI","ITUB","ABEV","VB",
	"TRV","BNTX","E","PSX","MRVL","CTAS",
	"STLA","XLK","APH","BSV","WDS","VNQ",
	"FCX","SQ","CTVA","BSBR","XLV","XEL",
	"KMI","WMB","ADSK","WELL","SCCO","HBANO",
	"DLTR","DOW","SPG","TEL","DLR","PRU",

	"AFL","LI","LULU","A","HBANM","CMG",
	"ALL","VGT","WDAY","MSI","SCHD","AJG",
	"RACE","ING","NIO","ALC","SBAC","DVN",
	"MSCI","EA","KR","LYG","CTSH","WBM",
	"BK","BF.F","XLE","BBD","MFC","YUM",
	"SGEN","CVE","MELI","ED","PUK","MCHP",

	"WBA","BF.A","LQD","JCI","BAX","RTN",
	"HPQ","CHT","LCID","ZM","PH","DELL",
	"WEC","RMD","LNG","BIIB","GPN","WCN",
	"HLT","VEEV","TIP","PEG","DDOG","CARR",
	"DXCM","TU","VEU","IDXX","TT","VICI",
	"ANET","TSN","ET","IFF","BCS","ODFL",

	"MFG","AQNU","ABC","TDG","ILMN","MPLX",
	"HES","RSP","APO","GOLD","OTIS","NUE",
	"MUB","XLE","FAST","ES","IVW","ORAN",
	"CMI","MTB","HBANP","DASH","DD","SABRP",
	"BKR","LYB","IAU","STM","VIT","DFS",
	"TEF","PCAR","EQR","KKR","NWG","VRSK",

	"PPG","BBDO","SCHX","GLW","IMO","RIVN",
	"MTD","AWK","FRC","SHY","AVB","DIA",
	"CPRT","USMV","SLF","IIVIP","CPNG","CRH",
	"TLK","ROST","IWB","HRL","DHI","IXUS",
	"TWTR","IWR","TROW","RPRX","PARAP","HAL",
	"WY","NOK","ENPH","NDAQ","AME","AMP",

	"BBVA","LVS","KEYS","EEM","SCHF","XPEV",
	"FNV","PCG","K","OKE","SLMBP","CBRE",
	"ERIC","SIRI","GJH","APTV","DTE","GWW",
	"SIVB","EBAY","VV","EIX","CSGP","CAJ",
	"EXR","ARE","IVE","RCI","FERG","SQM",
	"SOJD","ROK","SNAP","FITB","CHD","ALB",

	"ON","EFX","STT","LUV","LEN","CCEP",
	"RBLX","IBKR","AEE","ETR","CLR","IEF",
	"GFS","LH","BLL","FTS","CHNGU","DRE",
	"QRTEP","VBR","MKC","DGRO","TSCO","WST",
	"TTD","WTW","VT","GMAB","BALL","ZBH",
	"FMX","ZS","INVH","MBB","CDW","ESGU",

	"FE","HIG","ZTO","CTRA","IGSB","DVY",
	"SHV","PSNY","BIL","TLT","STE","TTM",
	"ANSS","VTR","SDY","CQP","WAT","EC",
	"VCSH","MAA","NTRS","PPL","AEM","DAL",
	"SUI","MDB","UGIC","IX","MTCH","VMC",
	"GFLU","JPST","GRMN","FTV","ALGN","MLM",

	"ULTA","YUMC","SCHB","PBA","RJF","PLTR",
	"GPC","FANG","CEG","GIB","ARGX","SPOT",
	"QUAL","IT","CMS","VRSN","TTWO","BEKE",
	"LBRKD","HZNP","CTLT","CINF","LYV","EBR.B",
	"LBRDA","MT","DUKB","CNP","FOXA","BNGE",
	"AMCR","NU","LEN.B","PWR","PHG","HEI",

	"PARAA","BYYPP","VTEB","RDOG","MPWR","URI",
	"CHWY","GOVT","AVTR","CFG","ESS","CF",
	"INCY","HBAN","RF","MKL","ARES","CLX",
	"ROL","BPYPO","HOLX","ALNY","TDY","VFC",
	"AGR","NYSE","BIP","WRB","FITBI","PKI",
	"EPAM","DOV","EBR","IUSB","PAYC","RKT",

	"JBHT","BR","FITBP","CAG","FOX","HPE",
	"ICLR","MDY","IR","ACWI","MOH","BRO",
	"PPD","PFG","MGA","ACGL","BPY","FLT",
	"PARA","VAR","BMRN","MOS","UMC","SWK",
	"EXPD","KEY","J","XLU","VCIT","STX",
	"VHT","TW","DB","WPC","BBY","IP",

	"SCHP","QSR","TCOM","DGX","CHKP","WPM",
	"TRU","BPYPN","PODD","ATO","IEP","SWKS",
	"NET","ZBRA","RYAAY","PFF","SYF","FITBO",
	"UDR","TWLO","XLP","OKTA","UI","EDR",
	"TS","WAB","CNHI","BIO","POOL","VIV",
	"EVRG","FDS","SSNC","SPLK","VGK","SEDG",

	"CPB","COO","HEI.A","MRO","HYG","KMX",
	"LNT","CAH","ZI","EXPE","DRI","PEAK",
	"WDC","TRMB","TER","NVR","CPT","AKAM",
	"OWL","LKQ","SHG","NTAP","LPLA","L",
	"ELS","SJM","BDXB","TECK","XLY","BIO.B",
	"DPZ","CS","ACI","AES","GNRC","HWM",

	"FMS","EMB","IEX","IRM","TECH","XYL",
	"EFV","BXP","ENTG","KB","FWONK","LDOS",
	"AVY","SSL","TRGP","TYL","JKHY","SCHG",
	"PULS","BG","SJR","AQNA","OMC","ELAT",
	"NLOK","WMG","TXT","HUBS","SCHA","FMC",
	"CLS","APP","BEPC","LSXMB","HDV","CTXS",

	"PKX","PKG","AQNB","BILL","CBOE","SC",
	"VLYPP","EQT","VXF","FWONA","DOCU","GLPI",
	"NICE","UAL","KIM","MAS","TAP","IWS",
	"SPYV","STLD","STIP","VTRS","MGM","CHRW",
	"PINS","ICL","WLK","AMH","LU","CE",
	"XLI","LSXMA","PTC","HTHT","COIN","LSXMK",

	"BEN","TPL","IEI","VBK","BAH","CNA",
	"SPYG","ROKU","WTRG","SUZ","NDSN","VLYPO",
	"SNN","U","SBNY","FVD","AFG","ABMD",
	"EMN","TAP.A","NI","FHN","AZPN","MINT",
	"IWN","TXF","CG","HST","GDDY","VOE",
	"KOF","IUSV","HAS","IWP","LUMN","Y",

	"NMR","APA","LBTYB","AGNCO","CSLC","CRL",
	"AAP","AEG","SPDW","BIV","ALLY","IPG",
	"SCI","LBTYK","KEP","RE","IUSG","FCNCA",
	"ETSY","UTHR","ASX","BLDR","RS","CCK",
	"LW","FICO","SWT","SNA","BZ","QGEN",
	"AGL","FNF","DT","PATH","BSY","AGNCM",

	"JHX","HSIC","RPM","REG","PHM","LBTYA",
	"PLUG","CCL","AGNCP","OVV","GME","TQQQ",
	"GDX","VRK","OTEX","BHVN","HRC","JEPI",
	"RYAN","WES","WPP","QRVO","REXR","MTUM",
	"MMP","DINO","DOX","SCZ","BIPH","IHG",
	"IWV","ERIE","CHK","XP","GGG","HUBB",

	"DTP","GL","EQH","MORN","SLV","BKI",
	"ESLT","GG","VST","PCTY","YINN","MKTX",
	"JAZZ","GRAB","BURL","AER","NOBL","SPLV",
	"CMA","AAL","IGIB","BRKR","AR","BAP",
	"MYMTO","BEP","LSI","EWJ","NLY","MPW",
	"ZNH","ELAN","BILI","ARCC","FLOT","XLC",

	"DAR","EWBC","CUK","AIZ","SPLG","OLPX",
	"CUBE","NWS","YMM","WHR","DISH","BNDX",
	"RDY","BJ","ZEN","LAMR","JNPR","SKM",
	"FFIV","ACC","ACM","VLUE","AQN","RGEN",
	"HGH","NRG","SCHO","FNDX","WSO","SUB",
	"WSM","AOS","LOGI","XEC","JLL","WSO.B",

	"SCHM","VVX","GFL","LEGN","WOLF","ALLE",
	"CCJ","MTN","RCL","HII","ZG","VFH",
	"Z","HNP","RHI","NLSN","UGI","OGN",
	"COLD","COR","CIG.C","BWA","SWCH","SEE",
	"SCHE","PAG","LNC","BCH","EXAS","MCHI",
	"H","FBHS","NFE","IBB","NYMTM","TTC",

	"MHK","DBX","GFI","AMC","CABO","DOCS",
	"LAD","RDVY","PNW","IJS","G","GTLB",
	"EFG","CBSH","DVA","WMS","TFII","OC",
	"CLF","KNX","NWL","VOT","TPR","RGA",
	"TRT","FRT","PDBC","MAT","ARMK","LEA",
	"AA","ONBPP","SPYD","TEVA","FTCS","LII",

	"NNN","WAL","CAE","OGE","WBS","TPG",
	"DLO","CZR","FND","NYMTL","XM","PNR",
	"ZION","ARKK","UHS","XBI","USFR","CGNX",
	"NVCR","PSTG","ONBPO","CFR","CHE","RRX",
	"XRAY","OEF","SCHZ","PFGC","STOR","TME",
	"CDAY","CHDN","ARW","CAR","TOST","WFG",

		
	"SPSB","GLOB","HESM","RBA","AVLR","HOOD",
	"CHNG","SEIC","MASI","FSLR","MANH","DLB",
	"ACGLN","NYMTZ","TTEK","PPC","TIXI","IVZ",
	"SYNH","CASY","GGB","GRFS","ATCOL","JBL",
	"GUNR","SCHR","CEA","LECO","OLN","CMSA",
	"SBSW","USFD","WSC","DECK","WEX","CFLT",

	"AIRC","EGP","SREA","MRVI","SWAV","PAC",
	"PAA","CIEN","AGCO","FNDF","OHI","FUTU",
	"HTZ","CWEN","SRPT","CMSD","BERY","RGLD",
	"IOT","QDEL","CMSC","CIB","DKS","HALO",
	"VDE","MIDD","BSAC","YNDX","ORI","NXST",
	"MGK","IDEV","CACC","EXEL","IJJ","IAC",

	"PSO","KT","ICSH","LEVI","USHY","AN",
	"TX","CACI","USIG","UNM","S","GNTX",
	"KBR","FLEX","ALV","FIVE","STWD","ATR",
	"ESTC","ACHC","DXC","RRC","AXON","JEF",
	"PSB","LSCC","BEAV","HTA","PII","VONG",
	"PLNT","SWN","QTS","RL","CNXC","IHI",

		
	"FR","RNR","IYW","RH","FIVN","PHI",
	"VIPS","TDOC","BBJP","TREX","FPE","BSMX",
	"CWEN.A","VDC","KWEB","VOOG","PRTC","MUSA",
	"AFRM","GPK","ESGD","WU","COTY","ST",
	"PB","HCP","WYNN","FCN","SPAB","JNK",
	"SOXX","WF","IJK","NEP","DNB","PCOR",

	"KRC","CHH","BRX","NVST","HUB","BBWI",
	"ACH","DGRW","CRSP","DCP","CPRI","FYBR",
	"USG","VOYA","GWRE","AU","ADT","RCM",
	"VNO","WH","XLB","SPTL","UWMC","AGNC",
	"COWZ","NFG","DCI","SAIL","BROS","NOV",
	"BBCA","SPIB","FSK","FAF","SCHH","HRB",
	
	"CASN","LFUS","SF","VONV","MSP","BPOP",
	"BLCO","BLD","THC","FLO","GMED","REYN",
	"EFAV","INGR","INFA","FSV","FFIN","SWX",
	"GTLS","SSB","CX","ROLL","AMLP","EEMV",
	"ISTB","XPO","PRGO","PRF","SON","SGMS",
	"ESGV","CCCS","VPU","SHLX","AZTA","FXI",

	"WWD","SOFI","ASR","ITT","CIG","SKX",
	"ADC","MKSI","PGX","TOL","SITE","MDLA",
	"BYD","CELH","NVAX","SPTI","SRC","SAIA",
	"ONON","MDU","PNFP","ASH","LSTR","TXG",
	"STAG","FTSM","OSH","TXRH","HLI","GBCI",
	"WCC","BC","LITE","RYN","MSPR","W",

	"PDCE","COKE","INSP","VLY","CNM","SBS",
	"IDA","ZIM","FIG","DNA","DSGX","OSK",
	"RUN","MTZ","JSM","SHC","ALK","OPCH",
	"SNV","VVV","SIGI","SPEM","CYBR","IONS",
	"PLTK","GDS","SPTM","RLI","AYI","XLRE",
	"JIRE","MP","MTDR","LMBS","TENB","EME",

	"FNDA","FTEC","VPL","THG","MNDT","ITCI",
	"NVT","APLS","DOOO","AXTA","SAIC","POST",
	"KNSL","BOKF","ALKS","TSEM","CR","OLED",
	"CW","TMX","VTWO","ASND","BWXT","CIBR",
	"BFAM","ZWS","UNMA","BAK","IIVI","GIL",
	"CIGI","AMN","ORCC","OMCL","HYQ","DKNG",

	"BBEU","LEG","CC","TNET","VAC","PENN",
	"GH","HOG","EEFT","LHCG","DQ","NYT",
	"SHM","STN","VG","GLDM","IRDM","ROG",
	"MEDP","OMF","NCLH","RNG","WTFC","MSA",
	"IPGP","DAVA","QLYS","ICLN","EHC","WB",
	"MTCN","TRQ","SAFM","NTLA","UBSI","IJT",

	"PEN","BIPC","GXO","BXMT","WWE","CLDR",
	"LPX","NTRSO","EXLS","TWKS","EZU","VMI",
	"EXPO","PRI","ASGN","AXS","SYNA","ARIA",
	"COLM","CBPO","CLH","IRT","EWZ","BKH",
	"DTM","SPMD","DRVN","RARE","KNTK","SMH",
	"SHYG","IGV","MBT","SLAB","X","M",

	"HXL","DTW","FIZZ","LNTH","PYCR","AUY",
	"SSD","OZK","NSA","MNDY","SMG","SIM",
	"MQ","EQY","STVN","IWY","ONB","NEAR",
	"AMG","COUP","HGV","NAV","IAA","HOMB",
	"UFPI","NRZ","GPT","TV","SLGN","ESI",
	"HE","NVEI","THO","LYFT","ARWR","BMBL",

	"PINC","ATHM","TGNA","EXP","IART","ACWV",
	"SEB","POR","NYCB","ORA","OLLI","AM",
	"OGS","MUR","CADE","ZIONL","EWT","EVO",
	"TRNO","SFBS","IDV","MGV","NOVT","ESGE",
	"AVT","IYR","POWI","UMBF","KGC","FIBK",
	"CUZ","CVNA","LIT","SLM","DHCNL","XOP",

	"LBRDP","GO","WEN","ESNT","NJR","LNW",
	"PSN","FNDE","FTGC","RHP","VNOM","SJI",
	"VNE","FDN","BL","MSM","HP","MAN",
	"WTS","SNDR","NATI","AB","TPX","ATC",
	"PNM","LPL","CIVI","DNLI","BCPC","EVR",
	"CMC","NCR","MTG","VGLT","VIR","ENSG",

	"QS","UNVR","SMAR","CHPT","ATUS","DBEF",
	"BRZE","AVAL","CRUS","INDA","SRCL","PSTH",
	"ARKG","TRK","RPD","AMKR","PECO","VMBS",
	"WOOF","ENLC","AQUA","IRTC","SPSM","UAA",
	"AMED","CERE","ALTR","WE","YETI","GPR.U",
	"DOC","DOCN","DEI","ICUI","MANT","TDC",

	"STRZB","ACWX","DHCNI","BXSL","SM","KRTX",
	"ACAS","KRG","BKLN","EVA","FNB","BBAX",
	"PEGA","PAAS","NSP","SPHD","ALIT","ZIONO",
	"HWC","JHG","ELY","VCR","SAM","PVH",
	"CYTK","BNL","DNP","HR","DH","VYMI",
	"SPSC","CWST","HBI","CWB","BECN","ALGM",

	"VIRT","CPG","MMS","HGTY","ASAI","DUOL",
	"CWEB","BOX","INDB","TPTX","GLBE","NARI",
	"FRSH","LTHM","NEA","SGOV","KSS","IQLT",
	"AL","ITA","UMPQ","ZD","FOXF","FLS",
	"NTRA","ATKR","WIX","SLYV","DV","ZLAB",
	"CBT","EWC","TFSL","AY","NTCO","ONEQ",

	"ABG","ALSN","SGFY","IAGG","SR","R",
	"WTM","DBC","CHX","FIXD","WNS","ACT",
	"IEUR","EDU","EWU","BEAM","AWI","CERT",
	"APG","HIW","VNQI","MSGS","HYLB","HLNE",
	"EPR","SMPL","AVNT","MRCY","IGT","IXJ",
	"GLPG","VTIP","AIT","VIGI","APPN","NEWR",

	"RDN","BLV","UA","VNT","VNT","PAVE",
	"AGO","LANC","PRVA","DSI","CEF","STAA",
	"IGM","DORM","NCNO","AMBP","TNDM","DDS",
	"FTSL","HEFA","PAGS","IMPPP","PPS","ESGR",
	"APLE","OXLCP","RRR","SKYY","VSS","ASAN",
	"SSRM","CVBF","JBT","STRZA","TRTN","SID",

	"KEX","ELP","CWK","AAXJ","HHC","WBT",
	"JMST","TFI","CBU","FELE","TNL","IOO",
	"NYMTN","BTG","OXLCO","MCW","PBF","MRTX",
	"IGF","HAE","FLR","UNF","AEL","PRV",
	"SPHQ","SPR","SXT","SMTC","SJNK","DLN",
	"SEM","FULTP","SBRA","NNI","NTNX","WK",

	"ALE", "AYX", "CROX", "NOMD", "IQ", "JWN",
	"OPEN", "LAZ", "UCBI", "BPMC", "DBRG", "VRT",
	"VIS", "WD", "PS", "LIVN", "ALRM", "SUSA",
	"SPTS", "PK", "HELE", "GATX", "BB", "JBGS",
	"ABCL", "APPF", "MTSI", "ASO", "ONTO", "ABCM",
	"GNR", "BOND", "GOLF", "BRBR", "FLRN", "KMPR",

	"SPT","HRMY","CNS","FCFS","SQSP","SRLN",
	"SPB","EBC","AJRD","JKS","IXN","MPLN",
	"INST","FUL","HAYW","SCHC","BHC","CNX",
	"GDXJ","ROIV","HYD","ANGL","VRNS","BSCO",
	"AVUV","FOLD","PCH","VET","CNR","SKY",
	"TMHC","PRFT","TAC","YOU","PTON","PACW",

	"PTCT", "BE", "JAMF", "EQC", "KFY", "NWE",
	"ZIONP", "IIPR", "BHF", "SUSL", "ACIW", "JOBY",
	"GTES", "XELAP", "SITM", "GT", "CVI", "BRFS",
	"CRI", "ISIL", "NEU", "SEAS", "MGV", "VC",
	"AVA", "BANF", "GPS", "GFNCP", "SUN", "TTEC",
	"LXP", "FN", "FIX", "KRE", "VIAV", "PZZA",

	"GEF.B", "CRC", "NSIT", "GEF", "DEN", "AWR",
	"ARNC", "LCII", "SLG", "NAD", "POWWP", "MTH",
	"BOH", "USSG", "MAIN", "MLI", "CATY", "VAL",
	"CWT", "MATX", "FRHC", "RUSHB", "PACWP", "OFC",
	"RLX", "BTU", "RTLPO", "MMSI", "FDL", "NVG",
	"AAON", "XT", "ATCO", "IBTX", "REZI", "CWAN",

	"ENOV", "TWNK", "SFM", "CVET", "RTLPP", "DON",
	"PBH", "ERF", "BRP", "EMLC", "PHB", "IHS",
	"APAM", "AXNX", "EPRT", "QLD", "DIOD", "BLKB",
	"REET", "FATE", "BKU", "PDCO", "ANDT", "ROLLP",
	"SSO", "FHI", "CVLT", "MC", "SIMO", "EWY",
	"ABM", "ASHR", "PPBI", "IYH", "SVXY", "LESL",

	"PTEN", "LSPD", "FHLC", "SPIP", "PSEC", "KNBE",
	"VOOV", "COOP", "ABCB", "SQQQ", "HI", "TBIO",
	"TCN", "VNLA", "SPWR", "SUM", "QTEC", "BSM",
	"STEP", "VSGX", "CCOI", "PHYS", "DY", "ENV",
	"FALN", "ATI", "ETRN", "UFS", "NHI", "LOPE",
	"CORT", "AGI", "AEIS", "MMYT", "CRK", "UPWK",

	"MGC", "AZEK", "SH", "OTTR", "HCXY", "MGEE",
	"VCRA", "WHD", "FOCS", "TCBI", "FORM", "CVT",
	"GHC", "LAC", "INSM", "GBIL", "CNMD", "PNGY",
	"SFNC", "OUT", "RUSHA", "SHLS", "ESMT", "VRNT",
	"ASB", "CXM", "VOT", "FSR", "BCO", "EVH",
	"HRI", "FTI", "RNW", "UCBIO", "CARG", "PJT",

	"IBP", "VSH", "GPI", "AGTI", "JBLU", "STNE",
	"KBH", "BHFAL", "TAL", "WING", "CPA", "FTCH",
	"SGRY", "IFS", "SAVE", "FPB", "BBIN", "CANO",
	"MEOH", "SLQD", "KW", "CD", "JJSF", "SITC",
	"PFSI", "FOUR", "SHOO", "AHCO", "VONE", "ARVN",
	"GSIE", "ALHC", "COMT", "HASI", "WERN", "WLY",

	"TRUP", "MTOR", "CALM", "COMT", "MXL", "EVTC",
	"GDRX", "WSFS", "WLYB", "PWSC", "NVMI", "AUB",
	"LTH", "CRVL", "GBTG", "CALX", "JETS", "NUVA",
	"MDC", "ARLP", "PCRX", "IBOC", "WBMD", "ACAD",
	"BDC", "ARCH", "ZGN", "MCY", "FV", "VRRM",
	"SANM", "MOG.A", "SIG", "USM", "AIN", "TRIP",
		
	"ICF", "CVAC", "NFRA", "IYE", "KWR", "AMBA",
	"ESAB", "NGVT", "HUBG", "SONO", "UGP", "NEOG",
	"KLIC", "ABR", "THS", "SGOL", "SBLKZ", "BCC",
	"TROX", "XSOE", "OIH", "TR", "HOME", "EXG",
	"CLBK", "ULCC", "FWRD", "OZON", "SPXC", "TAN",
	"HTIA", "SAFE", "VAW", "EMLP", "UTF", "TIGO",
		
	"MLCO", "OMI", "FUN", "CUB", "KOS", "CENT",
	"FL", "NTCT", "CMTG", "ENS", "HPK", "GVI",
	"EURN", "CEQP", "FNDC", "GCP", "QYLD", "OMAB",
	"FULT", "WEBR", "VSCO", "RMBS", "CWEI", "VICR",
	"DNUT", "CHGG", "GBDC", "DFAU", "PEB", "EYE",
	"AMRC", "WDFC", "NOVA", "JOE", "JHMM", "HCM",

	"PD", "GSY", "BMI", "LGIH", "ARKW", "AAT",
	"GKOS", "UNFI", "AUR", "TWST", "GLNG", "FLYW",
	"OZKAP", "FRPT", "COLB", "LOCK", "SBLK", "AIMC",
	"TKC", "FBND", "EMXC", "GBT", "IS", "IPAR",
	"OTLY", "UPST", "IOSP", "PGY", "SRAD", "TOTL",
	"ETWO", "ITGR", "AMJ", "HLF", "BRC", "CENTA",

	"PFHC", "BUR", "COUR", "SMR", "INMD", "DGS",
	"SDGR", "KEN", "HWCPL", "GSAT", "CPK", "KD",
	"UNIT", "SCL", "CPE", "MGPI", "AFGB", "AMR",
	"XRX", "FTAIN", "RPG", "QTWO", "LBRT", "FCPT",
	"NKLA", "SWC", "HPP", "WOR", "FROG", "EE",
	"MSTR", "ACA", "AX", "MNSO", "TLND", "PLXS",

	"AVUS", "QFIN", "JXN", "CIM", "EPP", "SGHC",
	"FTAIP", "FSS", "OAS", "LFST", "CYXT", "NAVI",
	"NAPA", "FRME", "PBUS", "PBUS", "CSQ", "NUS",
	"EQRX", "FTDR", "PRMW", "SSTK", "TDTT", "HTH",
	"ROIC", "INNO", "AFGD", "SMCI", "KAI", "FISK",
	"LAZR", "MLKN", "STR", "UTG", "RLAY", "DAN",

	"CBZ", "KBE", "SAGE", "SHO", "BCRX", "DADA",
	"CCU", "FTAIO", "ALGT", "CRGY", "SABR", "DSEY",
	"DKL", "CNO", "EVOP", "EXPI", "ITRI", "WIRE",
	"WMK", "AI", "ICVT", "RVLV", "AMEH", "ATSG",
	"IBA", "URTH", "XENE", "MAC", "THRM", "SBCF",
	"MOAT", "DXJ", "OLK", "HAIN", "BFH", "HL",

	"BOOT", "GOGL", "BLDP", "HLIO", "MED", "AFGC",
	"CBRL", "SCHK", "PAX", "CIXX", "RCUS", "PZA",
	"ENR", "PSMT", "AEO", "MTX", "CEM", "SLYG",
	"OMFL", "MD", "HEP", "PRFZ", "USRT", "PAGP",
	"WAFD", "ATRC", "VSAT", "NEX", "TEX", "ESBA",
	"CDC", "ANGI", "BANR", "CVCO", "TOWN", "GNW",

	"KRO", "GMS", "OI", "PTLC", "PRK", "FMB",
	"CWH", "BYND", "XLG", "YELP", "EBND", "AVAV",
	"KTB", "LZ", "PIPR", "UPRO", "ASZ.U", "TPH",
	"ASZ", "FTAI", "IMCR", "PRGS", "GSHD", "DCT",
	"PNFPP", "KMT", "AUBAP", "TSP", "FBC", "BHFAO",
	"ONEM", "UUP", "IOVA", "SKIN", "SI", "ZIP",

	"RYT", "AOR", "EPC", "HMY", "IXC", "IDCC",
	"SJW", "RAMP", "KAR", "WSBC", "CSIQ", "OPK",
	"YY", "IYF", "RQI", "NUV", "FBK", "BOWL",
	"LAUR", "RIG", "DOOR", "CSTM", "NMRK", "EVCM",
	"XME", "BHFAP", "GOOS", "NZF", "MAXR", "ARCB",
	"SPXL", "GDV", "TRMK", "NSS", "VCLT", "TFLO",

	"FFBC", "IRWD", "VGSH", "RKLB", "QCLN", "FA",
	"CNK", "TTGT", "CTRE", "XNCR", "RLJ", "SOLN",
	"DVAX", "MGRC", "TRN", "UE", "CSGS", "LGF.A",
	"FRO", "VCYT", "COMP", "MWA", "ADX", "KRYS",
	"OGCP", "RTLR", "VRP", "OR", "ETY", "SYBT",
	"WRE", "STNG", "EVT", "IVT", "BAB", "TLN",

	"NVEE", "AFGE", "SIX", "TASK", "CEIX", "NOG",
	"SPCE", "BRCC", "AG", "NWN", "TEO", "ARKQ",
	"NAC", "URBN", "IBRX", "BIGZ", "SHI", "EAF",
	"FUTY", "DRH", "TDS", "IBDO", "CMF", "HTLF",
	"CGAU", "FORG", "SASR", "DEM", "AMPH", "FLGT",
	"GSG", "FLNC", "GAB", "SWI", "NUTX", "KBWB",

	"MANU", "ACLS", "XYLD", "XHR", "ICFI", "FFIE",
	"DEA", "PGRE", "ITM", "GOGO", "ITCB", "ESE",
	"BMEZ", "RWR", "SHAK", "RZB", "IFRA", "EWA",
	"KAHC.U", "DES", "LKFN", "BRDG", "AAWW", "IBDP",
	"CYRX", "NXE", "IMKTA", "CSWI", "LRN", "MSGE",
	"BDJ", "CVII.U", "ALVO", "SEMR", "SCPL", "KAHC",

	"CVII", "GSBD", "HTGC", "FDML", "HOPE", "ARRY",
	"OLO", "PFS", "EVEX", "BBUC", "AUPH", "IVOL",
	"WGO", "USAC", "EVGO", "ATGE", "FAS", "RVMD",
	"LFG", "AIA", "PRM", "ALLO", "TELL", "BCAT",
	"NE", "TWO", "MDRX", "PI", "USA", "SJIJ",
	"AMPL", "HYLS", "STRA", "HYMB", "FREL", "NPO",

	"XMTR", "WWW", "BGS", "CNNE", "DK", "HLMN",
	"KTOS", "AXSM", "NBTB", "ELF", "GPRE", "CRDO",
	"VCTR", "RNST", "DFAI", "CCS", "PTVE", "AKO.B",
	"CODI", "NVRO", "TBK", "NWBI", "LILA", "VERX",
	"PAY", "LILAK", "SKT", "NABL", "CCXI", "KN",
	"PCY", "TLRY", "APPS", "STER", "TDIV", "FIGS",

	"VXX", "LGND", "GTN", "BTT", "POLY", "TEN",
	"HACK", "GGP", "SNEX", "DBA", "EVRI", "PMD",
	"VGR", "BND", "FORTY", "AKR", "TVTX", "SEAT",
	"CDEV", "GPOR", "EBS", "SATS", "SLY", "VRTV",
	"VBTX", "WTFCP", "SG", "NTB", "MYGN", "NS",
	"ZNTL", "KOMP", "AIRM", "B", "STPZ", "PING",

	"ETV", "HMN", "RZA", "USPH", "SUPN", "BFS",
	"STGW", "LTC", "TCBK", "EFSC", "ENIC", "BVN",
	"BAMR", "XPER", "CDMOP", "CTKB", "PAYO", "VZIO",
	"PHO", "GXC", "GET", "RADI", "MSEX", "IEV",
	"ILCG", "PRAA", "ODP", "SFB", "MOO", "WOW",
	"IAS", "AOM", "NXRT", "WTFCM", "FCEL", "SBGI",

	"RFP", "ECPG", "QDF", "PLAY", "VSTO", "ERJ",
	"OCFCP", "IBDN", "UDMY", "EGBN", "EDE", "CWI",
	"KRA", "JPS", "ARI", "WABC", "AVDV", "DIVO",
	"BHFAN", "MNRO", "OSIS", "MTRN", "GTN.A", "FEZ",
	"NG", "CHEF", "BBU", "HCC", "SES", "IHF",
	"TIPX", "RVT", "GNL", "BLMN", "PLMR", "NMIH",

	"TSE", "FXZ", "HNI", "XPEL", "CRCT", "FXN",
	"EXFY", "PDI", "GGR", "MYRG", "MNRL", "SAH",
	"BSTZ", "FNCL", "SANA", "AIR", "FXR", "LDUR",
	"SGML", "IPOF.U", "MMI", "BBMC", "VIVO", "NOAH",
	"FSLY", "CAKE", "SAFT", "IWX", "FLCB", "TSLX",
	"RELY", "AKO.A", "IPOF", "FXH", "TARO", "FNA",

	"MDYV", "GFF", "KIND", "ECAT", "FDP", "NULV",
	"TECL", "EMBC", "BBIO", "IYY", "SWTX", "LNN",
	"HEDJ", "PTY", "SOVO", "TY", "PRTA", "RXT",
	"TILT", "PSQ", "IYG", "RC", "AOA", "IPAC",
	"GDOT", "JELD", "LOB", "MLNK", "ADPT", "PX",
	"RODM", "PRCT", "MRTN", "FLNG", "MNTK", "AVDE",

	"ENVX", "BATRA", "OXM", "RXRX", "USMC", "ADUS",
	"PCTTU", "SPNS", "PSFE", "ESML", "PLUS", "ISEE",
	"STC", "ALG", "LYEL", "BGCP", "FINV", "EWL",
	"DDL", "CRTO", "LADR", "ARCO", "BKE", "WBX",
	"NG", "CHEF", "BBU", "HCC", "SES", "IHF",
	"TDCX", "FBT", "UCTT", "BIGC", "SIVBP", "IVLU",

	"PMT", "UVV", "INT", "IGLB", "JBI", "PPA",
	"SILK", "PTNR", "BXMX", "CMRE", "MEI", "WRBY",
	"MHLA", "IYK", "PCVX", "EWG", "FENY", "NEGG",
	"RES", "ROCC", "BOTZ", "CXW", "VGIT", "AGIO",
	"HNDL", "BATRK", "MNTV", "CNDA", "ECVT", "CINT",
	"DAWN", "SLVM", "SYM", "WFRD", "ASTL", "PGF",

	"HLLY", "CRS", "SBH", "IBDQ", "TGH", "ESTA",
	"HCSG", "AVDX", "ME", "ROCK", "EXAI", "CSR",
	"URA", "BUSE", "NAAS", "GCMG", "GVA", "FRG",
	"USNA", "MDYG", "COMM", "AMK", "ADV", "VRTS",
	"LAW", "VCEL", "ITB", "COHU", "ALEX", "PHX",
	"FVRR", "FCF", "JWSM.U", "RCB", "RPAY", "RWL",

	"PRA", "JWSM", "TBT", "PEY", "WAFDP", "BPYUP",
	"TCBIP", "AVNS", "LC", "KFRC", "BHFAM", "TCBIL",
	"YSG", "DDD", "RGNX", "KREF", "PRLB", "PHR",
	"RYLD", "GIC", "HURN", "DSL", "VRE", "JACK",
	"GDEV", "OFG", "PATK", "AMCX", "RTPYU", "GTY",
	"VHI", "MCG", "CTOS", "TUYA", "LPRO", "NMFC",

	"MHO", "FBNC", "UITB", "XAR", "RETA", "NGM",
	"KNL", "MIR", "EQX", "OXSQZ", "AMLX", "OCSL",
	"CRSR", "AAC", "RILY", "AROC", "OXSQL", "DFAE",
	"ARQT", "CFFN", "ETG", "CLDX", "RWO", "RIGP",
	"MYOV", "RCII", "STEM", "LMND", "INFN", "BPFH",
	"CHCO", "PL", "AVEM", "EXTR", "UCON", "DAC",

	"CLS", "SCS", "TTMI", "ESTE", "VERU", "KRNT",
	"HOUS", "AVID", "FRGE", "SDOG", "SBSI", "MODV",
	"SOXL", "MBUU", "AMWL", "KURA", "AGYS", "ENFN",
	"ASTS", "MUC", "VORB", "GUSA", "CASH", "KALU",
	"ALX", "DCOM", "ESRT", "ROBO", "KMG", "MDGL",
	"UTZ", "YPF", "KYMR", "EVV", "HYT", "CLOV",

	"SCHL", "HYDW", "SPYX", "FDVV", "NMZ", "SBNYP",
	"ALKT", "FNKO", "STEW", "PKW", "COPX", "FBRT",
	"SFL", "SSYS", "NXGN", "QRTEB", "ZUO", "BHLB",
	"PAM", "KSA", "PLAB", "STAR", "IMGN", "FPF",
	"PRO", "RSI", "TBBK", "CUBI", "MGR", "EIG",
	"LXU", "MFA", "PACB", "FLTR", "DHS", "CNXN",

	"NBHC", "XMLV", "CAMT", "TRS", "BHG", "AHH",
	"PRIM", "OCFC", "FGEN", "SRVR", "HIMX", "HYS",
	"CRON", "PICK", "NTUS", "SIGIP", "GDEN", "NFJ",
	"FXO", "BBN", "MATV", "EAI", "IHDG", "TALO",
	"DFH", "TOP", "KYN", "CDMO", "HTLD", "RGR",
	"BANC", "LZB", "IBDM", "HRT", "SNCY", "GDYN",

	"EWCZ", "JZGG", "MLPA", "CLNE", "CLM", "FIW",
	"SRCE", "HIMS", "CMP", "RXDX", "DIN", "DRBN",
	"VALN", "KNDY", "ATRI", "SPTN", "BV", "STBA",
	"LPI", "APOL", "OSTK", "TCBIO", "PRPB.U", "PXF",
	"SKYW", "FFWM", "PGTI", "SLP", "DLS", "EPAC",
	"MGNI", "FTA", "TNC", "KXI", "NEO", "MFGP",
		
	"DNOW", "CRNX", "HTLFP", "MNINP", "CIICU", "BUG",
	"PCT", "BALY", "PRPB", "ERII", "OFLX", "MTTR",
	"BBUS", "BRG", "PSK", "VCSA", "CTS", "NHC",
	"PLYA", "SAND", "RVNC", "HERA", "NYMT", "REGL",
	"SNDX", "VLRS", "NRGU", "DWAC", "MAG", "CPUH.U",
	"LRGF", "QURE", "RCKT", "MCRI", "CPUH", "ARKO",

	"CQQQ", "LICY", "ENTA", "SHEN", "JCPI", "XPRO",
	"CLBT", "SSP", "ACVA", "BBDC", "PDP", "IRBT",
	"IYJ", "LWLG", "SLDP", "TMP", "NRGV", "FXL",
	"EVBG", "CRHC.U", "AGM", "USO", "PAR", "GRBK",
	"LPSN", "RNP", "TRMD", "MLAB", "AIV", "FSTA",
	"FDIS", "ROAD", "ATEN", "MNKD", "BLU", "IVOO",

	"FEX", "IHRT", "CRHC", "PTRA", "MRUS", "BTZ",
	"ILF", "MEG", "EGO", "INTR", "DWACU", "NBR",
	"GEM", "GABC", "DOMO", "ACRS", "IMCG", "ANDE",
	"FPX", "BRKL", "DTD", "IBDR", "AVO", "UMH",
	"GSM", "EELV", "EB", "PBW", "BRSP", "GLTR",
	"AMPS", "LABU", "INVA", "PEJ", "ACEL", "PFXF",

	"LMAT", "HEES", "CGC", "PFBC", "IDU", "PBFX",
	"KRP", "TNA", "AACQU", "EAT", "SRNE", "DHT",
	"REPL", "FLQL", "PROC", "CRNC", "CERS", "SMP",
	"IAUM", "IGOV", "OSCR", "AZZ", "RNDT", "WISH",
	"NTST", "CTR", "OII", "EDIT", "QQEW", "SPH",
	"NRK", "VMEO", "SXI", "ZH", "PPLT", "MBINN",

	"AMTB", "ENVA", "SVC", "RBCAA", "UAN", "HSKA",
	"SWIR", "ECCC", "OPI", "BBRE", "KC", "DSGR",
	"ADTN", "HTRB", "RTL", "MBIN", "AOK", "VERV",
	"FRGAP", "GIII", "MGI", "NIC", "PSLV", "CAL",
	"BLTE", "AUS", "ASTE", "GEL", "CFNL", "CLFD",
	"AMRX", "MQY", "PTA", "GOODN", "CNOB", "STKL",

	"MBINO", "GGAL", "AGGY", "AMSF", "QCRH", "RDWR",
	"AVXL", "PRG", "VVX", "GES", "XHB", "MORF",
	"OEC", "EUFN", "DFIN", "INSW", "CCSI", "DSGN",
	"LBAI", "BAR", "CET", "ASIX", "MKTW", "SIGA",
	"RWT", "BCI", "HOLI", "INTA", "EHAB", "IMAX",
	"JPRE", "SUSB", "EFC", "ALLG", "XSD", "MOMO",

	"OBNK", "UTL", "HLIT", "SUSC", "BPYU", "ARVL",
	"SAVA", "GBX", "IVOG", "CONX", "UEC", "DMLP",
	"ALEC", "CTBB", "FTXN", "CXSE", "GOEV", "ILPT",
	"RDFN", "CCRN", "SCHN", "MODN", "MGIC", "OXLC",
	"VECO", "NRC", "LANDO", "NUSC", "GOODM", "TMDX",
	"PIII", "PGPG", "PUMP", "MERC", "MLPX", "ACMR",


	"TH", "HFWA", "MYTE", "ETW", "FREY", "PARR",
	"SCRMU", "GB", "DCPH", "BCSF", "DBI", "DLX",
	"PFC", "SLGC", "AIC", "CNOBP", "BY", "CGW",
	"ZETA", "SCRM", "SIVR", "SMRT", "ENTR", "IYM",
	"PRS", "SMLF", "DCBO", "TMCI", "SA", "IWL",
	"VIOO", "AOSL", "FNX", "COWN", "QRTEA", "BRMK",

	"REZ", "TWI", "AXL", "SECT", "HQH", "SUMO",
	"CFO", "IONQ", "IWC", "RMR", "SBR", "HCAT",
	"SCWX", "CNDT", "QLTA", "SAAS", "BTRS", "RNA",
	"DCFC", "RYH", "CAAP", "HBM", "DJP", "CDZIP",
	"GAM", "SSNI", "TGLS", "HDEF", "FIVG", "FTC",
	"TGI", "SPLP", "GPRO", "WLKP", "JRVR", "CHY",

	"RPT", "BSCL", "THQ", "LANDM", "UCO", "BCOR",
	"JBSS", "CLTL", "MRC", "FORR", "NSSC", "UIS",
	"ANF", "SII", "PAYA", "MUNI", "PNTM", "APGB.U",
	"CKH", "MATW", "GROV", "ARIS", "PNTM", "KIDS",
	"UFCS", "IGMS", "ITOS", "CALF", "NNDM", "AOD",
	"IMXI", "EDV", "APOG", "HYGV", "APGB", "LGV",

	"EOS", "RYI", "AWF", "VREX", "HZO", "NPTN",
	"SGH", "CRBN", "IGE", "WASH", "IAT", "GOODO",
	"LQDH", "VIASP", "GTO", "CHCT", "OM", "RA",
	"DRSK", "GAINL", "FWRG", "LEV", "QQQX", "AVPT",
	"CLMT", "DOLE", "IIIV", "DGII", "KAMN", "PUBM",
	"QUS", "IGR", "AGRO", "CHI", "CBD", "DRQ",

	"IMAB", "HUYA", "VNET", "OCFT", "TIGR", "RENN",
	"API", "BZUN", "CAN", "SVA", "SOHU", "YALA",
	"RERE", "DAO", "OPRA", "NIU", "PUYI", "WDH",
	"RGC", "EH", "GOTU", "LX", "MSC", "GHG",
	"DOYU", "XNET", "IH", "CMCM", "SY", "BNR",
	"UNIX", "CO", "CANG", "SOL", "GRCL", "EM",

	"TWOU", "ACCD", "MARA", "KOSS", "VIRI", "FUBO",
	"DBVT", "AMRS", "ZY", "OUST", "GTH", "ACB",
	"SKLZ", "OGI", "VLDR", "SNDL", "NAK", "PLCE",
	"SOHOO", "BTWN", "PTGX", "GHL", "INSE", "AHT",
	"HDSN", "RAD", "AZUL", "JMIA", "BLUE", "SFIX",
	"PRVB", "GRWG", "VFF", "BKKT", "CRTX", "OVID",

	"SLQT", "CMRX", "RIDE", "EDSA", "ADVM", "CDXC",
	"BYSI", "AMRN", "MDVL", "CLVR", "CNTB", "NVTA",
	"LDI", "BLI", "BLFS", "UFPT", "BVS", "LUNG",
	"NUVL", "ATRA", "ACLX", "CPRX", "MIRM", "YMAB",
	"AGEN", "ERAS", "NRIX", "TGTX", "PMVP", "KNSA",
	"MOR", "CTIC", "TIL", "ADCT", "KROS", "GERN",

	"PNT", "ZEAL", "ACET", "NKTX", "COGT", "OGGO",
	"SGMO", "CHRS", "RPTX", "VNDA", "INBX", "FHTX",
	"CGEM", "KZR", "BCYC", "CELU", "JANX", "PAPT",
	"OCGN", "IDYA", "STOK", "IMTX", "EGRX", "AMAB",
	"VXRT", "GOSS", "IPSC", "KNET", "TSVT", "IMGO",
	"IMVT", "MRSN", "PHAR", "CNTA", "CMPS", "ALT",

	"ARCT", "RDUS", "KOD", "GLUE", "ALBO", "CCCC",
	"DYN", "MCRB", "INO", "ADGI", "PTGX", "AFMD",
	"ADMA", "ICPT", "BTAI", "LXRX", "AURA", "SRRK",
	"ALXO", "HLVX", "CRBU", "TYRA", "TNGX", "GBIO",
	"FMTX", "MDXG", "KPTI", "AKRO", "VERA", "ABUS",
	"AVTE", "EIGR", "MGTX", "HUMA", "MESO", "VRDN",

	"CLVS", "BMEA", "TRDA", "RYTM", "PGEN", "ANIK",
	"ZYME", "SLN", "ICVX", "XOMA", "HRTX", "ALVR",
	"ALDX", "TCRT", "KALV", "XOMAP", "GRNA", "XOMAO",
	"STRO", "ADAP", "CMPX", "GTHX", "OPT", "LCTX",
	"VKTX", "AUTL", "AADI", "FDMT", "ALPN", "VECT",
	"SELB", "VYGR", "KRON", "RLYB", "OTLK", "LIAN",

	"GEO", "MVST", "PBI", "BBBY", "MNMD", "EXPR",
	"NGMS", "CNTY", "INSE", "GMBLP", "AGS", "FLL",
	"CDRO", "GAN", "CPHC", "BRAG", "TBP", "XXII",
	"FSM", "EXK", "SVM", "NEWP", "GATO", "XXII",
	"MVIS", "WKHS", "KNDI", "CENN", "SEV", "BIRD",
	"CBD", "CENX", "AVD", "IPI", "NVX", "MASS",

	"MX", "HA", "GOL", "UP", "BLDE", "NWLI",
	"CIA", "NSTG", "MXCT", "AKYA", "BNGO", "QSI",
	"NAUT", "OMIC", "ABSI", "NOTV", "TKNO", "SNCE",
	"PRID", "PSNL", "CGEN", "HBIO", "ISO", "LAB",
	"CDXC", "CSBR", "IPA", "COCO", "ZVIA", "SKYH",
	"CLW", "GLT", "ITP", "PACK", "PTSI", "SWVL",

};
//char StockNameList[][64] =
//{
//	"BLUE"
//};
double StrikeList[20][6000];
double NowPrice[20][6000];
bool   bFalg[20][6000];

double bidPriceList[20][6000];
double askPriceList[20][6000];
long long llReqTick[20][6000];

bool   bReqSuc[20][6000];

//char OptionDataList[][32] =
//{
//	"20220819", 
//	"20220916",
//	"20221021",
//	"20221118",
//	"20230120",
//	"20230616",
//	"20240119"
//};
char OptionDataList[][32] =
{
	"20220819",
	"20230120",
	"20240119"
};
time_t convert(int year, int month, int day)
{
	tm info = { 0 };
	info.tm_year = year - 1900;
	info.tm_mon = month - 1;
	info.tm_mday = day;
	return mktime(&info);

}
int  get_days(const char* from, const char* to)
{
	int year, month, day;
	sscanf(from, "%4d%2d%2d", &year, &month, &day);
	int fromSecond = (int)convert(year, month, day);
	sscanf(to, "%4d%2d%2d", &year, &month, &day);
	int toSecond = (int)convert(year, month, day);
	return (toSecond - fromSecond) / 24 / 3600;
}
int GetStockCount(int m)
{
	int nIndex;
	char pszFileName[256];
	sprintf_s(pszFileName, 256, "C:\\bighouse\\波动率探索器\\%s\\索引.txt", OptionDataList[m]);
	HANDLE hLog = gamelog::OpenLogFile(pszFileName, 2);
	DWORD dwSize = GetFileSize(hLog, 0);
	char *pBuf = (char *)malloc(dwSize);
	DWORD dwRead;
	ReadFile(hLog, pBuf, dwSize, &dwRead, 0);
	nIndex = atoi(pBuf);
	CloseHandle(hLog);
	free(pBuf);
	
	
	return nIndex;
}
#include <iostream> 
#include <algorithm> 


void WriteRateToFile(int mIndex,int nStockIndex,double price)
{
	SYSTEMTIME currentTime = { 0 };
	GetLocalTime(&currentTime);
	char pszInitDate[32] = "";
	sprintf_s(pszInitDate, 32, "%04d%02d%02d", currentTime.wYear, currentTime.wMonth, currentTime.wDay);
	int days = get_days(pszInitDate, OptionDataList[mIndex]) + 1;
	double fddd = StrikeList[mIndex][nStockIndex] - price;
	double fRa = price / fddd;
	double fRate = fRa * 365 / days * 100;
	char pszWrite[1024];
	char pszFileName[256];
	sprintf_s(pszFileName, 256, "C:\\bighouse\\波动率探索器\\%s_%s.txt", "期权利率", OptionDataList[mIndex]);

	sprintf_s(pszWrite, 1024, "%s,%g,%0.2f\n", StockNameList[nStockIndex], StrikeList[mIndex][nStockIndex], fRate);
	gamelog::WriteLog(pszFileName, pszWrite);
	sprintf_s(pszFileName, 256, "C:\\bighouse\\波动率探索器\\利率\\%s\\%s_%s.txt", OptionDataList[mIndex], OptionDataList[mIndex], pszInitDate);
	gamelog::WriteLog(pszFileName, pszWrite);
}
DWORD WINAPI RepDataThread(LPVOID lpParam)
{
	TestCppClient *pp = (TestCppClient *)lpParam;
	char pszFileName[256];
	

	char pszDir[MAX_PATH];
	sprintf_s(pszDir, 256, "C:\\bighouse\\波动率探索器\\利率");
	CreateDirectory(pszDir, NULL);
	int nMktId;
	int nEachSelect = 36;
	int nDataCount = sizeof(OptionDataList) / 32;
	memset(bReqSuc, 0x00, sizeof(bReqSuc));
	memset(askPriceList, 0x00, sizeof(askPriceList));
	memset(bidPriceList, 0x00, sizeof(bidPriceList));
	//int nStockCount = sizeof(StockNameList) / 64;
	for (int m = 0; m < nDataCount; m++)
	{
		sprintf_s(pszFileName, 256, "C:\\bighouse\\波动率探索器\\%s_%s.txt", "期权利率", OptionDataList[m]);
		HANDLE hLog = gamelog::OpenLogFile(pszFileName, 0);
		CloseHandle(hLog);

		sprintf_s(pszDir, 256, "C:\\bighouse\\波动率探索器\\利率\\%s", OptionDataList[m]);
		CreateDirectory(pszDir, NULL);
		//int nStockCount = GetStockCount(m);
		int nStockCount = (std::min)((int)(sizeof(StockNameList) / 64), GetStockCount(m));
		
		for (int k = 0; k < nStockCount; k++)
		{
			bFalg[m][k] = false;
			nMktId = 10000 * m + k;
			pp->m_pClient->reqMktData(nMktId, ContractSamples::StockForQuery(StockNameList[k]), "", false, false, TagValueListSPtr());
			if ((k + 1) % nEachSelect == 0)
			{
				std::this_thread::sleep_for(std::chrono::seconds(10));
				for (int j = nMktId; j > nMktId - nEachSelect; j--)
				{
					
					if (bReqSuc[m][j%10000] == false && bFalg[m][j % 10000] == true)
					{
						double price = (bidPriceList[m][j % 10000] + askPriceList[m][j % 10000]) / 2;
						WriteRateToFile(m, j % 10000, price);
					}
					pp->m_pClient->cancelMktData(j);
				}
			}
			/*nMktId++;*/
		}
		if (nStockCount%nEachSelect != 0)
		{
			std::this_thread::sleep_for(std::chrono::seconds(10));
			for (int k = nMktId ; k > nMktId  - (nStockCount%nEachSelect); k--)
			{
			

				if (bReqSuc[m][k % 10000] == false && bFalg[m][k % 10000] == true)
				{
					double price = (bidPriceList[m][k % 10000] + askPriceList[m][k % 10000]) / 2;
					WriteRateToFile(m, k % 10000, price);
					
				}
				pp->m_pClient->cancelMktData(k);
			}

		}
	}
	return true;
}
//DWORD WINAPI GetOptionStrikeListThread(LPVOID lpParam)
//{
//	TestCppClient *pp = (TestCppClient *)lpParam;
//	int nDataCount = sizeof(OptionDataList) / 32;
//	int nStockCount = sizeof(StockNameList) / 64;
//
//	int mIndexList[20];
//	memset(mIndexList, 0x00, sizeof(mIndexList));
//	//先创建目录
//	for (int m = 0; m < nDataCount; m++)
//	{
//		char pszDir[MAX_PATH];
//		sprintf_s(pszDir, 256, "C:\\bighouse\\波动率探索器\\%s", OptionDataList[m]);
//		CreateDirectory(pszDir, NULL);
//		char pszFileName[256];
//		sprintf_s(pszFileName, 256, "C:\\bighouse\\波动率探索器\\%s\\索引.txt", OptionDataList[m]);
//		HANDLE hLog = gamelog::OpenLogFile(pszFileName, 2);
//		DWORD dwSize = GetFileSize(hLog, 0);
//	    char *pBuf = (char *)malloc(dwSize);
//	    DWORD dwRead;
//	    ReadFile(hLog, pBuf, dwSize, &dwRead, 0);
//		mIndexList[m] = atoi(pBuf);
//		CloseHandle(hLog);
//		free(pBuf);
//		/*sprintf_s(pszDir, 256, "C:\\bighouse\\波动率探索器\\%s\\strike", OptionDataList[m]);
//		CreateDirectory(pszDir, NULL);*/
//	}
//	for (int m = 0; m < nDataCount; m++)
//	{
//		for (int k = mIndexList[m]; k < nStockCount; k++)
//		{
//			//如果没有找到这股票，返回200错误代码,
//			char pszFileName[256];
//			sprintf_s(pszFileName, 256, "C:\\bighouse\\波动率探索器\\%s\\%s.txt", OptionDataList[m], StockNameList[k]);
//			HANDLE hLog = gamelog::OpenLogFile(pszFileName, 0);
//			CloseHandle(hLog);
//			pp->m_pClient->reqContractDetails( k + m * 10000, ContractSamples::OptionForQuery(StockNameList[k], OptionDataList[m]));
//			if ((k + 1) % 36 == 0)
//				std::this_thread::sleep_for(std::chrono::seconds(5));
//		}
//	}
//	return true;
//}
//
struct StListInfo
{
	TestCppClient *pp;
	int mIndex;
};
DWORD WINAPI GetOptionStrikeListThread(LPVOID lpParam)
{
	StListInfo *pInfo = (StListInfo *)lpParam;
	TestCppClient *pp = pInfo->pp;
	int nDataCount = sizeof(OptionDataList) / 32;
	int nStockCount = sizeof(StockNameList) / 64;
	//先创建目录
	int m = pInfo->mIndex;
	int mIndexList[20];
	memset(mIndexList, 0x00, sizeof(mIndexList));
	char pszDir[MAX_PATH];
	sprintf_s(pszDir, 256, "C:\\bighouse\\波动率探索器\\%s", OptionDataList[m]);
	CreateDirectory(pszDir, NULL);
		/*sprintf_s(pszDir, 256, "C:\\bighouse\\波动率探索器\\%s\\strike", OptionDataList[m]);
		CreateDirectory(pszDir, NULL);*/
	mIndexList[m] = GetStockCount(m);


	
	for (int k = mIndexList[m]; k < nStockCount; k++)
	{
			//如果没有找到这股票，返回200错误代码,
			char pszFileName[256];
			sprintf_s(pszFileName, 256, "C:\\bighouse\\波动率探索器\\%s\\%s.txt", OptionDataList[m], StockNameList[k]);
			HANDLE hLog = gamelog::OpenLogFile(pszFileName, 0);
			CloseHandle(hLog);
			pp->m_pClient->reqContractDetails(  k+m*10000, ContractSamples::OptionForQuery(StockNameList[k], OptionDataList[m]));
			if((k+1)%36==0)
			  std::this_thread::sleep_for(std::chrono::seconds(5));
	}
	return true;
}


void TestCppClient::GetOptionStrikeList()
{
	DWORD ThreadID;
	int nDataCount = sizeof(OptionDataList) / 32;
	for (int m = 0; m < /*nDataCount*/2; m++)
	{
		StListInfo *pInfo = (StListInfo *)malloc(sizeof(StListInfo));
		pInfo->mIndex = m;
		pInfo->pp = this;
		CreateThread(NULL, 0, &GetOptionStrikeListThread, (LPVOID)pInfo, 0, &ThreadID);
		//GetOptionStrikeListThread((LPVOID)pInfo);
	}
	//CreateThread(NULL, 0, &GetOptionStrikeListThread, (LPVOID)this, 0, &ThreadID);
	//GetOptionStrikeListThread(this);
	
}

void TestCppClient::contractOperations()
{
	//if(currentTime.wHour>=21)
	//if(currentTime.wDayOfWeek>=6 && )
	/*const char* from = "20220712";
	const char* to = "20220821";*/
	//SYSTEMTIME currentTime = { 0 };
	//GetLocalTime(&currentTime);
	//char pszInitDate[32] = "";
	//sprintf_s(pszInitDate, 32, "%04d%02d%02d", currentTime.wYear, currentTime.wMonth, currentTime.wDay);
	//int days = get_days(pszInitDate, to);
	/*SYSTEMTIME currentTime = { 0 };
	GetLocalTime(&currentTime);*/
	//return;

	/*GetOptionStrikeList();
	m_state = ST_CONTRACTOPERATION_ACK;
	return;*/

	//! [reqcontractdetails]
	DWORD ThreadID;
	CreateThread(NULL, 0, &RepDataThread, (LPVOID)this, 0, &ThreadID);

	char pszFileName[256];
	/*sprintf_s(pszFileName, 256, "C:\\bighouse\\波动率探索器\\%s.txt", "期权利率");
	HANDLE hLog = gamelog::OpenLogFile(pszFileName, 0);
	CloseHandle(hLog);*/
	int nMktId = 1000;
	int nEachSelect=15;
	int nStockCount = sizeof(StockNameList) / 64;
	//m_pClient->reqMktData(nMktId, ContractSamples::StockForQuery(StockNameList[0]), "", false, false, TagValueListSPtr());
	/*for (int k = 0; k < nStockCount; k++)
	{
		m_pClient->reqMktData(nMktId, ContractSamples::StockForQuery(StockNameList[k]), "", false, false, TagValueListSPtr());
		if ((k + 1) % nEachSelect == 0)
		{
			std::this_thread::sleep_for(std::chrono::seconds(10));
			for (int j = nMktId; j >nMktId- nEachSelect; j--)
			{
				m_pClient->cancelMktData(j);
			}
		}
		nMktId++;
	}
	if (nStockCount%nEachSelect != 0)
	{
		std::this_thread::sleep_for(std::chrono::seconds(10));
		for (int k = nMktId - 1; k > nMktId - 1 - (nStockCount%nEachSelect); k--)
		{
			m_pClient->cancelMktData(k);
		}
		
	}*/
	m_state = ST_CONTRACTOPERATION_ACK;
	return;

	/*m_pClient->reqMktData(1014, ContractSamples::USOptionContract(), "", false, false, TagValueListSPtr());
	

	std::this_thread::sleep_for(std::chrono::seconds(8));

	m_pClient->cancelMktData(1014);
	return;*/

	//int nStockId = 0;
	//char pszFileName[256];
	//sprintf_s(pszFileName, 256, "C:\\bighouse\\波动率探索器\\%s.txt", StockNameList[nStockId]);
	//HANDLE hLog = gamelog::OpenLogFile(pszFileName, 2);
	//DWORD dwSize = GetFileSize(hLog, 0);
	//char *pBuf = (char *)malloc(dwSize);
	//DWORD dwRead;
	//ReadFile(hLog, pBuf, dwSize, &dwRead, 0);
	//int k = 0;
	//std::vector<double> priceList;
	//char pszTemp[512]="";
	//int j = 0;
	//for (;;)
	//{
	//	if (pBuf[k] == 0x00)
	//		break;
	//	if (pBuf[k] == 0x0a)
	//	{
	//		pszTemp[j] = 0x00;
	//		double price = atof(pszTemp);
	//		priceList.push_back(price);
	//		j = 0;
	//	}
	//	else if (pBuf[k] != 0x0a && pBuf[k] != 0x0d)
	//	{
	//		pszTemp[j] = pBuf[k];
	//		j++;
	//	}
	//	k++;
	//	
	//	
	//}
	//free(pBuf);
	//CloseHandle(hLog);

	//std::sort(priceList.begin(), priceList.end());

	//double price = 2403.5*0.8;
	//double target= price;
	////股价大于1000，起跳价50 
	////股价大于500,起跳价20
	////股价大于200到500,起跳价10
	////股价大于80小于200，起跳价5
	////股价大于50小于80，起跳价2.5
	////股价大于20小于50，起跳价1
	////股价大于0小于20，起跳价0.5
	//double fpa[7] = { 1000,500,200,80,50,20,0 };
	//double fbb[7] = { 50,20,10,5,2.5,1,0.5 };
	//for (int k = 0; k < 7; k++)
	//{
	//	if (price >= fpa[k])
	//	{
	//		double fVlaue=fmod(price, fbb[k]);
	//		double ftemp = price - fVlaue;
	//		if (price > ftemp + fbb[k] / 2)
	//		{
	//			ftemp += fbb[k];
	//		}
	//		target = ftemp;
	//		break;
	//		
	//	}
	//}
	//double fStrike=0;
	////从列表中得出估算出最优的行权价
	//for (int k = 0; k < priceList.size(); k++)
	//{
	//	if (target <= priceList[k] )
	//	{
	//		fStrike = priceList[k];
	//		break;
	//	}
	//}
	//m_pClient->reqMktData(9000+ nStockId, ContractSamples::USOptionContractEx(StockNameList[nStockId],"20220715",fStrike), "", false, false, TagValueListSPtr());


	//std::this_thread::sleep_for(std::chrono::seconds(10));

	//m_pClient->cancelMktData(9000 + nStockId);


	//m_state = ST_CONTRACTOPERATION_ACK;
	//return;
	//for (int k = 0; k <nStockCount; k++)
	//{
	//	//如果没有找到这股票，返回200错误代码,
	//	char pszFileName[256];
	//	sprintf_s(pszFileName,256, "C:\\bighouse\\波动率探索器\\%s.txt", StockNameList[k]);
	//	HANDLE hLog=gamelog::OpenLogFile(pszFileName, 0);
	//	CloseHandle(hLog);
	//	m_pClient->reqContractDetails(210+k, ContractSamples::OptionForQuery(StockNameList[k]));
	//	//std::this_thread::sleep_for(std::chrono::seconds(10));
	//}
	//m_pClient->reqContractDetails(212, ContractSamples::IBMBond());
	//m_pClient->reqContractDetails(213, ContractSamples::IBKRStk());
	//m_pClient->reqContractDetails(214, ContractSamples::Bond());
	//m_pClient->reqContractDetails(215, ContractSamples::FuturesOnOptions());
	//m_pClient->reqContractDetails(216, ContractSamples::SimpleFuture());
	//! [reqcontractdetails]

	//! [reqcontractdetailsnews]
	//m_pClient->reqContractDetails(211, ContractSamples::NewsFeedForQuery());
	//! [reqcontractdetailsnews]

	m_state = ST_CONTRACTOPERATION_ACK;
}

void TestCppClient::marketScanners()
{
	/*** Requesting all available parameters which can be used to build a scanner request ***/
	//! [reqscannerparameters]
	m_pClient->reqScannerParameters();
	//! [reqscannerparameters]
	std::this_thread::sleep_for(std::chrono::seconds(2));

	/*** Triggering a scanner subscription ***/
	//! [reqscannersubscription]
	m_pClient->reqScannerSubscription(7001, ScannerSubscriptionSamples::HotUSStkByVolume(), TagValueListSPtr(), TagValueListSPtr());
	
	TagValueSPtr t1(new TagValue("usdMarketCapAbove", "10000"));
	TagValueSPtr t2(new TagValue("optVolumeAbove", "1000"));
	TagValueSPtr t3(new TagValue("usdMarketCapAbove", "100000000"));

	TagValueListSPtr TagValues(new TagValueList());
	TagValues->push_back(t1);
	TagValues->push_back(t2);
	TagValues->push_back(t3);

	m_pClient->reqScannerSubscription(7002, ScannerSubscriptionSamples::HotUSStkByVolume(), TagValueListSPtr(), TagValues); // requires TWS v973+
	
	//! [reqscannersubscription]

	//! [reqcomplexscanner]

	TagValueSPtr t(new TagValue("underConID", "265598"));
	TagValueListSPtr AAPLConIDTag(new TagValueList());
	AAPLConIDTag->push_back(t);
	m_pClient->reqScannerSubscription(7003, ScannerSubscriptionSamples::ComplexOrdersAndTrades(), TagValueListSPtr(), AAPLConIDTag); // requires TWS v975+

	//! [reqcomplexscanner]

	std::this_thread::sleep_for(std::chrono::seconds(2));
	/*** Canceling the scanner subscription ***/
	//! [cancelscannersubscription]
	m_pClient->cancelScannerSubscription(7001);
	m_pClient->cancelScannerSubscription(7002);
	//! [cancelscannersubscription]

	m_state = ST_MARKETSCANNERS_ACK;
}

void TestCppClient::fundamentals()
{
	/*** Requesting Fundamentals ***/
	//! [reqfundamentaldata]
	m_pClient->reqFundamentalData(8001, ContractSamples::USStock(), "ReportsFinSummary", TagValueListSPtr());
	//! [reqfundamentaldata]
	std::this_thread::sleep_for(std::chrono::seconds(2));

	/*** Canceling fundamentals request ***/
	//! [cancelfundamentaldata]
	m_pClient->cancelFundamentalData(8001);
	//! [cancelfundamentaldata]

	m_state = ST_FUNDAMENTALS_ACK;
}

void TestCppClient::bulletins()
{
	/*** Requesting Interactive Broker's news bulletins */
	//! [reqnewsbulletins]
	m_pClient->reqNewsBulletins(true);
	//! [reqnewsbulletins]
	std::this_thread::sleep_for(std::chrono::seconds(2));
	/*** Canceling IB's news bulletins ***/
	//! [cancelnewsbulletins]
	m_pClient->cancelNewsBulletins();
	//! [cancelnewsbulletins]

	m_state = ST_BULLETINS_ACK;
}

void TestCppClient::accountOperations()
{
	/*** Requesting managed accounts***/
	//! [reqmanagedaccts]
	m_pClient->reqManagedAccts();
	//! [reqmanagedaccts]
	std::this_thread::sleep_for(std::chrono::seconds(2));
	/*** Requesting accounts' summary ***/
	//! [reqaaccountsummary]
	m_pClient->reqAccountSummary(9001, "All", AccountSummaryTags::getAllTags());
	//! [reqaaccountsummary]
	std::this_thread::sleep_for(std::chrono::seconds(2));
	//! [reqaaccountsummaryledger]
	m_pClient->reqAccountSummary(9002, "All", "$LEDGER");
	//! [reqaaccountsummaryledger]
	std::this_thread::sleep_for(std::chrono::seconds(2));
	//! [reqaaccountsummaryledgercurrency]
	m_pClient->reqAccountSummary(9003, "All", "$LEDGER:EUR");
	//! [reqaaccountsummaryledgercurrency]
	std::this_thread::sleep_for(std::chrono::seconds(2));
	//! [reqaaccountsummaryledgerall]
	m_pClient->reqAccountSummary(9004, "All", "$LEDGER:ALL");
	//! [reqaaccountsummaryledgerall]
	std::this_thread::sleep_for(std::chrono::seconds(2));
	//! [cancelaaccountsummary]
	m_pClient->cancelAccountSummary(9001);
	m_pClient->cancelAccountSummary(9002);
	m_pClient->cancelAccountSummary(9003);
	m_pClient->cancelAccountSummary(9004);
	//! [cancelaaccountsummary]
	std::this_thread::sleep_for(std::chrono::seconds(2));
	/*** Subscribing to an account's information. Only one at a time! ***/
	//! [reqaaccountupdates]
	m_pClient->reqAccountUpdates(true, "U150462");
	//! [reqaaccountupdates]
	std::this_thread::sleep_for(std::chrono::seconds(2));
	//! [cancelaaccountupdates]
	m_pClient->reqAccountUpdates(false, "U150462");
	//! [cancelaaccountupdates]
	std::this_thread::sleep_for(std::chrono::seconds(2));

	//! [reqaaccountupdatesmulti]
	m_pClient->reqAccountUpdatesMulti(9002, "U150462", "EUstocks", true);
	//! [reqaaccountupdatesmulti]
	std::this_thread::sleep_for(std::chrono::seconds(2));

	/*** Requesting all accounts' positions. ***/
	//! [reqpositions]
	m_pClient->reqPositions();
	//! [reqpositions]
	std::this_thread::sleep_for(std::chrono::seconds(2));
	//! [cancelpositions]
	m_pClient->cancelPositions();
	//! [cancelpositions]

	//! [reqpositionsmulti]
	m_pClient->reqPositionsMulti(9003, "U150462", "EUstocks");
	//! [reqpositionsmulti]

	m_state = ST_ACCOUNTOPERATIONS_ACK;
}

void TestCppClient::orderOperations()
{
	/*** Requesting the next valid id ***/
	//! [reqids]
	//The parameter is always ignored.
	m_pClient->reqIds(-1);
	//! [reqids]
	//! [reqallopenorders]
	m_pClient->reqAllOpenOrders();
	//! [reqallopenorders]
	//! [reqautoopenorders]
	m_pClient->reqAutoOpenOrders(true);
	//! [reqautoopenorders]
	//! [reqopenorders]
	m_pClient->reqOpenOrders();
	//! [reqopenorders]

	/*** Placing/modifying an order - remember to ALWAYS increment the nextValidId after placing an order so it can be used for the next one! ***/
    //! [order_submission]
	m_pClient->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::LimitOrder("SELL", 1, 50));
    //! [order_submission]

	//m_pClient->placeOrder(m_orderId++, ContractSamples::OptionAtBox(), OrderSamples::Block("BUY", 50, 20));
	//m_pClient->placeOrder(m_orderId++, ContractSamples::OptionAtBox(), OrderSamples::BoxTop("SELL", 10));
	//m_pClient->placeOrder(m_orderId++, ContractSamples::FutureComboContract(), OrderSamples::ComboLimitOrder("SELL", 1, 1, false));
	//m_pClient->placeOrder(m_orderId++, ContractSamples::StockComboContract(), OrderSamples::ComboMarketOrder("BUY", 1, false));
	//m_pClient->placeOrder(m_orderId++, ContractSamples::OptionComboContract(), OrderSamples::ComboMarketOrder("BUY", 1, true));
	//m_pClient->placeOrder(m_orderId++, ContractSamples::StockComboContract(), OrderSamples::LimitOrderForComboWithLegPrices("BUY", 1, std::vector<double>(10, 5), true));
	//m_pClient->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::Discretionary("SELL", 1, 45, 0.5));
	//m_pClient->placeOrder(m_orderId++, ContractSamples::OptionAtBox(), OrderSamples::LimitIfTouched("BUY", 1, 30, 34));
	//m_pClient->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::LimitOnClose("SELL", 1, 34));
	//m_pClient->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::LimitOnOpen("BUY", 1, 35));
	//m_pClient->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::MarketIfTouched("BUY", 1, 35));
	//m_pClient->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::MarketOnClose("SELL", 1));
	//m_pClient->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::MarketOnOpen("BUY", 1));
	//m_pClient->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::MarketOrder("SELL", 1));
	//m_pClient->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::MarketToLimit("BUY", 1));
	//m_pClient->placeOrder(m_orderId++, ContractSamples::OptionAtIse(), OrderSamples::MidpointMatch("BUY", 1));
	//m_pClient->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::Stop("SELL", 1, 34.4));
	//m_pClient->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::StopLimit("BUY", 1, 35, 33));
	//m_pClient->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::StopWithProtection("SELL", 1, 45));
	//m_pClient->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::SweepToFill("BUY", 1, 35));
	//m_pClient->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::TrailingStop("SELL", 1, 0.5, 30));
	//m_pClient->placeOrder(m_orderId++, ContractSamples::USStock(), OrderSamples::TrailingStopLimit("BUY", 1, 2, 5, 50));
	
	//! [place_midprice]
	m_pClient->placeOrder(m_orderId++, ContractSamples::USStockAtSmart(), OrderSamples::Midprice("BUY", 1, 150));
	//! [place_midprice]
	
	//! [place order with cashQty]
	m_pClient->placeOrder(m_orderId++, ContractSamples::USStockAtSmart(), OrderSamples::LimitOrderWithCashQty("BUY", 30, 5000));
	//! [place order with cashQty]

	std::this_thread::sleep_for(std::chrono::seconds(1));

	/*** Cancel one order ***/
	//! [cancelorder]
	m_pClient->cancelOrder(m_orderId-1);
	//! [cancelorder]
	
	/*** Cancel all orders for all accounts ***/
	//! [reqglobalcancel]
	m_pClient->reqGlobalCancel();
	//! [reqglobalcancel]

	/*** Request the day's executions ***/
	//! [reqexecutions]
	m_pClient->reqExecutions(10001, ExecutionFilter());
	//! [reqexecutions]

	//! [reqcompletedorders]
	m_pClient->reqCompletedOrders(false);
	//! [reqcompletedorders]

	m_state = ST_ORDEROPERATIONS_ACK;
}

void TestCppClient::ocaSamples()
{
	//OCA ORDER
	//! [ocasubmit]
	std::vector<Order> ocaOrders;
	ocaOrders.push_back(OrderSamples::LimitOrder("BUY", 1, 10));
	ocaOrders.push_back(OrderSamples::LimitOrder("BUY", 1, 11));
	ocaOrders.push_back(OrderSamples::LimitOrder("BUY", 1, 12));
	for(unsigned int i = 0; i < ocaOrders.size(); i++){
		OrderSamples::OneCancelsAll("TestOca", ocaOrders[i], 2);
		m_pClient->placeOrder(m_orderId++, ContractSamples::USStock(), ocaOrders[i]);
	}
	//! [ocasubmit]

	m_state = ST_OCASAMPLES_ACK;
}

void TestCppClient::conditionSamples()
{
	//! [order_conditioning_activate]
	Order lmt = OrderSamples::LimitOrder("BUY", 100, 10);
	//Order will become active if conditioning criteria is met
	PriceCondition* priceCondition = dynamic_cast<PriceCondition *>(OrderSamples::Price_Condition(208813720, "SMART", 600, false, false));
	ExecutionCondition* execCondition = dynamic_cast<ExecutionCondition *>(OrderSamples::Execution_Condition("EUR.USD", "CASH", "IDEALPRO", true));
	MarginCondition* marginCondition = dynamic_cast<MarginCondition *>(OrderSamples::Margin_Condition(30, true, false));
	PercentChangeCondition* pctChangeCondition = dynamic_cast<PercentChangeCondition *>(OrderSamples::Percent_Change_Condition(15.0, 208813720, "SMART", true, true));
	TimeCondition* timeCondition = dynamic_cast<TimeCondition *>(OrderSamples::Time_Condition("20160118 23:59:59", true, false));
	VolumeCondition* volumeCondition = dynamic_cast<VolumeCondition *>(OrderSamples::Volume_Condition(208813720, "SMART", false, 100, true));

	lmt.conditions.push_back(std::shared_ptr<PriceCondition>(priceCondition));
	lmt.conditions.push_back(std::shared_ptr<ExecutionCondition>(execCondition));
	lmt.conditions.push_back(std::shared_ptr<MarginCondition>(marginCondition));
	lmt.conditions.push_back(std::shared_ptr<PercentChangeCondition>(pctChangeCondition));
	lmt.conditions.push_back(std::shared_ptr<TimeCondition>(timeCondition));
	lmt.conditions.push_back(std::shared_ptr<VolumeCondition>(volumeCondition));
	m_pClient->placeOrder(m_orderId++, ContractSamples::USStock(),lmt);
	//! [order_conditioning_activate]

	//Conditions can make the order active or cancel it. Only LMT orders can be conditionally canceled.
	//! [order_conditioning_cancel]
	Order lmt2 = OrderSamples::LimitOrder("BUY", 100, 20);
	//The active order will be cancelled if conditioning criteria is met
	lmt2.conditionsCancelOrder = true;
	PriceCondition* priceCondition2 = dynamic_cast<PriceCondition *>(OrderSamples::Price_Condition(208813720, "SMART", 600, false, false));
	lmt2.conditions.push_back(std::shared_ptr<PriceCondition>(priceCondition2));
	m_pClient->placeOrder(m_orderId++, ContractSamples::EuropeanStock(), lmt2);
	//! [order_conditioning_cancel]

	m_state = ST_CONDITIONSAMPLES_ACK;
}

void TestCppClient::bracketSample(){
	Order parent;
	Order takeProfit;
	Order stopLoss;
	//! [bracketsubmit]
	OrderSamples::BracketOrder(m_orderId++, parent, takeProfit, stopLoss, "BUY", 100, 30, 40, 20);
	m_pClient->placeOrder(parent.orderId, ContractSamples::EuropeanStock(), parent);
	m_pClient->placeOrder(takeProfit.orderId, ContractSamples::EuropeanStock(), takeProfit);
	m_pClient->placeOrder(stopLoss.orderId, ContractSamples::EuropeanStock(), stopLoss);
	//! [bracketsubmit]

	m_state = ST_BRACKETSAMPLES_ACK;
}

void TestCppClient::hedgeSample(){
	//F Hedge order
	//! [hedgesubmit]
	//Parent order on a contract which currency differs from your base currency
	Order parent = OrderSamples::LimitOrder("BUY", 100, 10);
	parent.orderId = m_orderId++;
	parent.transmit = false;
	//Hedge on the currency conversion
	Order hedge = OrderSamples::MarketFHedge(parent.orderId, "BUY");
	//Place the parent first...
	m_pClient->placeOrder(parent.orderId, ContractSamples::EuropeanStock(), parent);
	//Then the hedge order
	m_pClient->placeOrder(m_orderId++, ContractSamples::EurGbpFx(), hedge);
	//! [hedgesubmit]

	m_state = ST_HEDGESAMPLES_ACK;
}

void TestCppClient::testAlgoSamples(){
	//! [algo_base_order]
	Order baseOrder = OrderSamples::LimitOrder("BUY", 1000, 1);
	//! [algo_base_order]

	//! [arrivalpx]
	AvailableAlgoParams::FillArrivalPriceParams(baseOrder, 0.1, "Aggressive", "09:00:00 CET", "16:00:00 CET", true, true, 100000);
	m_pClient->placeOrder(m_orderId++, ContractSamples::USStockAtSmart(), baseOrder);
	//! [arrivalpx]

	//! [darkice]
	AvailableAlgoParams::FillDarkIceParams(baseOrder, 10, "09:00:00 CET", "16:00:00 CET", true, 100000);
	m_pClient->placeOrder(m_orderId++, ContractSamples::USStockAtSmart(), baseOrder);
	//! [darkice]

	//! [ad]
	// The Time Zone in "startTime" and "endTime" attributes is ignored and always defaulted to GMT
	AvailableAlgoParams::FillAccumulateDistributeParams(baseOrder, 10, 60, true, true, 1, true, true, "20161010-12:00:00 GMT", "20161010-16:00:00 GMT");
	m_pClient->placeOrder(m_orderId++, ContractSamples::USStockAtSmart(), baseOrder);
	//! [ad]

	//! [twap]
	AvailableAlgoParams::FillTwapParams(baseOrder, "Marketable", "09:00:00 CET", "16:00:00 CET", true, 100000);
	m_pClient->placeOrder(m_orderId++, ContractSamples::USStockAtSmart(), baseOrder);
	//! [twap]

	//! [vwap]
	AvailableAlgoParams::FillBalanceImpactRiskParams(baseOrder, 0.1, "Aggressive", true);
	m_pClient->placeOrder(m_orderId++, ContractSamples::USStockAtSmart(), baseOrder);
	 //! [vwap]

	//! [balanceimpactrisk]
	AvailableAlgoParams::FillBalanceImpactRiskParams(baseOrder, 0.1, "Aggressive", true);
	m_pClient->placeOrder(m_orderId++, ContractSamples::USStockAtSmart(), baseOrder);
	//! [balanceimpactrisk]

	//! [minimpact]
	AvailableAlgoParams::FillMinImpactParams(baseOrder, 0.3);
	m_pClient->placeOrder(m_orderId++, ContractSamples::USStockAtSmart(), baseOrder);
	//! [minimpact]

	//! [adaptive]
	AvailableAlgoParams::FillAdaptiveParams(baseOrder, "Normal");
	m_pClient->placeOrder(m_orderId++, ContractSamples::USStockAtSmart(), baseOrder);
	//! [adaptive]

	//! [closepx]
    AvailableAlgoParams::FillClosePriceParams(baseOrder, 0.5, "Neutral", "12:00:00 EST", true, 100000);
    m_pClient->placeOrder(m_orderId++, ContractSamples::USStockAtSmart(), baseOrder);
    //! [closepx]
    
    //! [pctvol]
    AvailableAlgoParams::FillPctVolParams(baseOrder, 0.5, "12:00:00 EST", "14:00:00 EST", true, 100000);
    m_pClient->placeOrder(m_orderId++, ContractSamples::USStockAtSmart(), baseOrder);
    //! [pctvol]               
    
    //! [pctvolpx]
    AvailableAlgoParams::FillPriceVariantPctVolParams(baseOrder, 0.1, 0.05, 0.01, 0.2, "12:00:00 EST", "14:00:00 EST", true, 100000);
    m_pClient->placeOrder(m_orderId++, ContractSamples::USStockAtSmart(), baseOrder);
    //! [pctvolpx]
    
    //! [pctvolsz]
    AvailableAlgoParams::FillSizeVariantPctVolParams(baseOrder, 0.2, 0.4, "12:00:00 EST", "14:00:00 EST", true, 100000);
    m_pClient->placeOrder(m_orderId++, ContractSamples::USStockAtSmart(), baseOrder);
    //! [pctvolsz]
    
    //! [pctvoltm]
    AvailableAlgoParams::FillTimeVariantPctVolParams(baseOrder, 0.2, 0.4, "12:00:00 EST", "14:00:00 EST", true, 100000);
    m_pClient->placeOrder(m_orderId++, ContractSamples::USStockAtSmart(), baseOrder);
    //! [pctvoltm]

	//! [jeff_vwap_algo]
	AvailableAlgoParams::FillJefferiesVWAPParams(baseOrder, "10:00:00 EST", "16:00:00 EST", 10, 10, "Exclude_Both", 130, 135, 1, 10, "Patience", false, "Midpoint");
	m_pClient->placeOrder(m_orderId++, ContractSamples::JefferiesContract(), baseOrder);
	//! [jeff_vwap_algo]

	//! [csfb_inline_algo]
	AvailableAlgoParams::FillCSFBInlineParams(baseOrder, "10:00:00 EST", "16:00:00 EST", "Patient", 10, 20, 100, "Default", false, 40, 100, 100, 35);
	m_pClient->placeOrder(m_orderId++, ContractSamples::CSFBContract(), baseOrder);
	//! [csfb_inline_algo]
	
	m_state = ST_TESTALGOSAMPLES_ACK;
}

void TestCppClient::financialAdvisorOrderSamples()
{
	//! [faorderoneaccount]
	Order faOrderOneAccount = OrderSamples::MarketOrder("BUY", 100);
	// Specify the Account Number directly
	faOrderOneAccount.account = "DU119915";
	m_pClient->placeOrder(m_orderId++, ContractSamples::USStock(), faOrderOneAccount);
	//! [faorderoneaccount]
	std::this_thread::sleep_for(std::chrono::seconds(1));

	//! [faordergroupequalquantity]
	Order faOrderGroupEQ = OrderSamples::LimitOrder("SELL", 200, 2000);
	faOrderGroupEQ.faGroup = "Group_Equal_Quantity";
	faOrderGroupEQ.faMethod = "EqualQuantity";
	m_pClient->placeOrder(m_orderId++, ContractSamples::SimpleFuture(), faOrderGroupEQ);
	//! [faordergroupequalquantity]
	std::this_thread::sleep_for(std::chrono::seconds(1));

	//! [faordergrouppctchange]
	Order faOrderGroupPC;
	faOrderGroupPC.action = "BUY";
	faOrderGroupPC.orderType = "MKT";
	// You should not specify any order quantity for PctChange allocation method
	faOrderGroupPC.faGroup = "Pct_Change";
	faOrderGroupPC.faMethod = "PctChange";
	faOrderGroupPC.faPercentage = "100";
	m_pClient->placeOrder(m_orderId++, ContractSamples::EurGbpFx(), faOrderGroupPC);
	//! [faordergrouppctchange]
	std::this_thread::sleep_for(std::chrono::seconds(1));

	//! [faorderprofile]
	Order faOrderProfile = OrderSamples::LimitOrder("BUY", 200, 100);
	faOrderProfile.faProfile = "Percent_60_40";
	m_pClient->placeOrder(m_orderId++, ContractSamples::EuropeanStock(), faOrderProfile);
	//! [faorderprofile]

	//! [modelorder]
	Order modelOrder = OrderSamples::LimitOrder("BUY", 200, 100);
	modelOrder.account = "DF12345";
	modelOrder.modelCode = "Technology";
	m_pClient->placeOrder(m_orderId++, ContractSamples::USStock(), modelOrder);
	//! [modelorder]

	m_state = ST_FAORDERSAMPLES_ACK;
}

void TestCppClient::financialAdvisorOperations()
{
	/*** Requesting FA information ***/
	//! [requestfaaliases]
	m_pClient->requestFA(faDataType::ALIASES);
	//! [requestfaaliases]

	//! [requestfagroups]
	m_pClient->requestFA(faDataType::GROUPS);
	//! [requestfagroups]

	//! [requestfaprofiles]
	m_pClient->requestFA(faDataType::PROFILES);
	//! [requestfaprofiles]

	/*** Replacing FA information - Fill in with the appropriate XML string. ***/
	//! [replacefaonegroup]
	m_pClient->replaceFA(1000, faDataType::GROUPS, FAMethodSamples::FAOneGroup());
	//! [replacefaonegroup]

	//! [replacefatwogroups]
	m_pClient->replaceFA(1001, faDataType::GROUPS, FAMethodSamples::FATwoGroups());
	//! [replacefatwogroups]

	//! [replacefaoneprofile]
	m_pClient->replaceFA(1002, faDataType::PROFILES, FAMethodSamples::FAOneProfile());
	//! [replacefaoneprofile]

	//! [replacefatwoprofiles]
	m_pClient->replaceFA(1003, faDataType::PROFILES, FAMethodSamples::FATwoProfiles());
	//! [replacefatwoprofiles]

	//! [reqSoftDollarTiers]
	m_pClient->reqSoftDollarTiers(4001);
	//! [reqSoftDollarTiers]

	m_state = ST_FAOPERATIONS_ACK;
}

void TestCppClient::testDisplayGroups(){
	//! [querydisplaygroups]
	m_pClient->queryDisplayGroups(9001);
	//! [querydisplaygroups]

	std::this_thread::sleep_for(std::chrono::seconds(1));

	//! [subscribetogroupevents]
	m_pClient->subscribeToGroupEvents(9002, 1);
	//! [subscribetogroupevents]

	std::this_thread::sleep_for(std::chrono::seconds(1));

	//! [updatedisplaygroup]
	m_pClient->updateDisplayGroup(9002, "8314@SMART");
	//! [updatedisplaygroup]

	std::this_thread::sleep_for(std::chrono::seconds(1));

	//! [subscribefromgroupevents]
	m_pClient->unsubscribeFromGroupEvents(9002);
	//! [subscribefromgroupevents]

	m_state = ST_TICKDATAOPERATION_ACK;
}

void TestCppClient::miscelaneous()
{
	/*** Request TWS' current time ***/
	m_pClient->reqCurrentTime();
	/*** Setting TWS logging level  ***/
	m_pClient->setServerLogLevel(5);

	m_state = ST_MISCELANEOUS_ACK;
}

void TestCppClient::reqFamilyCodes()
{
	/*** Request TWS' family codes ***/
	//! [reqfamilycodes]
	m_pClient->reqFamilyCodes();
	//! [reqfamilycodes]

	m_state = ST_FAMILYCODES_ACK;
}

void TestCppClient::reqMatchingSymbols()
{
	/*** Request TWS' mathing symbols ***/
	//! [reqmatchingsymbols]
	m_pClient->reqMatchingSymbols(11001, "IBM");
	//! [reqmatchingsymbols]
	m_state = ST_SYMBOLSAMPLES_ACK;
}

void TestCppClient::reqMktDepthExchanges()
{
	/*** Request TWS' market depth exchanges ***/
	//! [reqMktDepthExchanges]
	m_pClient->reqMktDepthExchanges();
	//! [reqMktDepthExchanges]

	m_state = ST_REQMKTDEPTHEXCHANGES_ACK;
}

void TestCppClient::reqNewsTicks()
{
	//! [reqmktdata_ticknews]
	m_pClient->reqMktData(12001, ContractSamples::USStockAtSmart(), "mdoff,292", false, false, TagValueListSPtr());
	//! [reqmktdata_ticknews]

	std::this_thread::sleep_for(std::chrono::seconds(5));

	//! [cancelmktdata2]
	m_pClient->cancelMktData(12001);
	//! [cancelmktdata2]

	m_state = ST_REQNEWSTICKS_ACK;
}

void TestCppClient::reqSmartComponents()
{
	static bool bFirstRun = true;

	if (bFirstRun) {
		m_pClient->reqMktData(13001, ContractSamples::USStockAtSmart(), "", false, false, TagValueListSPtr());

		bFirstRun = false;
	}

	std::this_thread::sleep_for(std::chrono::seconds(5));

	if (m_bboExchange.size() > 0) {
		m_pClient->cancelMktData(13001);

		//! [reqsmartcomponents]
		m_pClient->reqSmartComponents(13002, m_bboExchange);
		//! [reqsmartcomponents]
		m_state = ST_REQSMARTCOMPONENTS_ACK;
	}
}

void TestCppClient::reqNewsProviders()
{
	/*** Request TWS' news providers ***/
	//! [reqNewsProviders]
	m_pClient->reqNewsProviders();
	//! [reqNewsProviders]

	m_state = ST_NEWSPROVIDERS_ACK;
}

void TestCppClient::reqNewsArticle()
{
	/*** Request TWS' news article ***/
	//! [reqNewsArticle]
	TagValueList* list = new TagValueList();
	// list->push_back((TagValueSPtr)new TagValue("manual", "1"));
	m_pClient->reqNewsArticle(12001, "MST", "MST$06f53098", TagValueListSPtr(list));
	//! [reqNewsArticle]

	m_state = ST_REQNEWSARTICLE_ACK;
}

void TestCppClient::reqHistoricalNews(){

	//! [reqHistoricalNews]
	TagValueList* list = new TagValueList();
	list->push_back((TagValueSPtr)new TagValue("manual", "1"));
	m_pClient->reqHistoricalNews(12001, 8314, "BZ+FLY", "", "", 5, TagValueListSPtr(list));
	//! [reqHistoricalNews]

	std::this_thread::sleep_for(std::chrono::seconds(1));

	m_state = ST_REQHISTORICALNEWS_ACK;
}


void TestCppClient::reqHeadTimestamp() {
	//! [reqHeadTimeStamp]
	m_pClient->reqHeadTimestamp(14001, ContractSamples::EurGbpFx(), "MIDPOINT", 1, 1);
	//! [reqHeadTimeStamp]	
	std::this_thread::sleep_for(std::chrono::seconds(1));
	//! [cancelHeadTimestamp]
	m_pClient->cancelHeadTimestamp(14001);
	//! [cancelHeadTimestamp]
	
	m_state = ST_REQHEADTIMESTAMP_ACK;
}

void TestCppClient::reqHistogramData() {
	//! [reqHistogramData]
	m_pClient->reqHistogramData(15001, ContractSamples::IBMUSStockAtSmart(), false, "1 weeks");
	//! [reqHistogramData]
	std::this_thread::sleep_for(std::chrono::seconds(2));
	//! [cancelHistogramData]
	m_pClient->cancelHistogramData(15001);
	//! [cancelHistogramData]
	m_state = ST_REQHISTOGRAMDATA_ACK;
}

void TestCppClient::rerouteCFDOperations()
{
    //! [reqmktdatacfd]
	m_pClient->reqMktData(16001, ContractSamples::USStockCFD(), "", false, false, TagValueListSPtr());
    std::this_thread::sleep_for(std::chrono::seconds(1));
	m_pClient->reqMktData(16002, ContractSamples::EuropeanStockCFD(), "", false, false, TagValueListSPtr());
    std::this_thread::sleep_for(std::chrono::seconds(1));
	m_pClient->reqMktData(16003, ContractSamples::CashCFD(), "", false, false, TagValueListSPtr());
	std::this_thread::sleep_for(std::chrono::seconds(1));
	//! [reqmktdatacfd]

    //! [reqmktdepthcfd]
	m_pClient->reqMktDepth(16004, ContractSamples::USStockCFD(), 10, false, TagValueListSPtr());
    std::this_thread::sleep_for(std::chrono::seconds(1));
	m_pClient->reqMktDepth(16005, ContractSamples::EuropeanStockCFD(), 10, false, TagValueListSPtr());
    std::this_thread::sleep_for(std::chrono::seconds(1));
	m_pClient->reqMktDepth(16006, ContractSamples::CashCFD(), 10, false, TagValueListSPtr());
	std::this_thread::sleep_for(std::chrono::seconds(1));
	//! [reqmktdepthcfd]

	m_state = ST_REROUTECFD_ACK;
}

void TestCppClient::marketRuleOperations()
{
	m_pClient->reqContractDetails(17001, ContractSamples::IBMBond());
	m_pClient->reqContractDetails(17002, ContractSamples::IBKRStk());

    std::this_thread::sleep_for(std::chrono::seconds(2));

	//! [reqmarketrule]
	m_pClient->reqMarketRule(26);
	m_pClient->reqMarketRule(635);
	m_pClient->reqMarketRule(1388);
	//! [reqmarketrule]

	m_state = ST_MARKETRULE_ACK;
}

void TestCppClient::continuousFuturesOperations()
{
	m_pClient->reqContractDetails(18001, ContractSamples::ContFut());

	//! [reqhistoricaldatacontfut]
	std::time_t rawtime;
    std::tm* timeinfo;
    char queryTime [80];

	std::time(&rawtime);
    timeinfo = std::localtime(&rawtime);
	std::strftime(queryTime, 80, "%Y%m%d %H:%M:%S", timeinfo);

	m_pClient->reqHistoricalData(18002, ContractSamples::ContFut(), queryTime, "1 Y", "1 month", "TRADES", 0, 1, false, TagValueListSPtr());

    std::this_thread::sleep_for(std::chrono::seconds(10));

	m_pClient->cancelHistoricalData(18002);
	//! [reqhistoricaldatacontfut]

	m_state = ST_CONTFUT_ACK;
}

void TestCppClient::reqHistoricalTicks() 
{
	//! [reqhistoricalticks]
    m_pClient->reqHistoricalTicks(19001, ContractSamples::IBMUSStockAtSmart(), "20170621 09:38:33", "", 10, "BID_ASK", 1, true, TagValueListSPtr());
    m_pClient->reqHistoricalTicks(19002, ContractSamples::IBMUSStockAtSmart(), "20170621 09:38:33", "", 10, "MIDPOINT", 1, true, TagValueListSPtr());
    m_pClient->reqHistoricalTicks(19003, ContractSamples::IBMUSStockAtSmart(), "20170621 09:38:33", "", 10, "TRADES", 1, true, TagValueListSPtr());
    //! [reqhistoricalticks]
    m_state = ST_REQHISTORICALTICKS_ACK;
}

void TestCppClient::reqTickByTickData() 
{
    /*** Requesting tick-by-tick data (only refresh) ***/
    
    m_pClient->reqTickByTickData(20001, ContractSamples::EuropeanStock(), "Last", 0, false);
    m_pClient->reqTickByTickData(20002, ContractSamples::EuropeanStock(), "AllLast", 0, false);
    m_pClient->reqTickByTickData(20003, ContractSamples::EuropeanStock(), "BidAsk", 0, true);
    m_pClient->reqTickByTickData(20004, ContractSamples::EurGbpFx(), "MidPoint", 0, false);

    std::this_thread::sleep_for(std::chrono::seconds(10));

	//! [canceltickbytick]
    m_pClient->cancelTickByTickData(20001);
    m_pClient->cancelTickByTickData(20002);
    m_pClient->cancelTickByTickData(20003);
    m_pClient->cancelTickByTickData(20004);
    //! [canceltickbytick]
	
    /*** Requesting tick-by-tick data (historical + refresh) ***/
    //! [reqtickbytick]
    m_pClient->reqTickByTickData(20005, ContractSamples::EuropeanStock(), "Last", 10, false);
    m_pClient->reqTickByTickData(20006, ContractSamples::EuropeanStock(), "AllLast", 10, false);
    m_pClient->reqTickByTickData(20007, ContractSamples::EuropeanStock(), "BidAsk", 10, false);
    m_pClient->reqTickByTickData(20008, ContractSamples::EurGbpFx(), "MidPoint", 10, true);
	//! [reqtickbytick]
	
    std::this_thread::sleep_for(std::chrono::seconds(10));

    m_pClient->cancelTickByTickData(20005);
    m_pClient->cancelTickByTickData(20006);
    m_pClient->cancelTickByTickData(20007);
    m_pClient->cancelTickByTickData(20008);

    m_state = ST_REQTICKBYTICKDATA_ACK;
}

void TestCppClient::whatIfSamples()
{
    /*** Placing waht-if order ***/
    //! [whatiforder]
    m_pClient->placeOrder(m_orderId++, ContractSamples::USStockAtSmart(), OrderSamples::WhatIfLimitOrder("BUY", 200, 120));
    //! [whatiforder]

    m_state = ST_WHATIFSAMPLES_ACK;
}



//! [nextvalidid]
void TestCppClient::nextValidId( OrderId orderId)
{
	printf("Next Valid Id: %ld\n", orderId);
	m_orderId = orderId;
	m_state = ST_CONTRACTOPERATION;
	//m_state = ST_DELAYEDTICKDATAOPERATION;
	//m_state = ST_OPTIONSOPERATIONS;
	//! [nextvalidid]

  //  m_state = ST_TICKOPTIONCOMPUTATIONOPERATION; 
    //m_state = ST_TICKDATAOPERATION; 
    //m_state = ST_OPTIONSOPERATIONS;
    //m_state = ST_REQTICKBYTICKDATA; 
    //m_state = ST_REQHISTORICALTICKS; 
    //m_state = ST_CONTFUT; 
   // m_state = ST_PNLSINGLE; 
  //  m_state = ST_PNL; 
	//m_state = ST_DELAYEDTICKDATAOPERATION; 
	//m_state = ST_MARKETDEPTHOPERATION;
	//m_state = ST_REALTIMEBARS;
	//m_state = ST_MARKETDATATYPE;
	//m_state = ST_HISTORICALDATAREQUESTS;
	//m_state = ST_CONTRACTOPERATION;
	//m_state = ST_MARKETSCANNERS;
	//m_state = ST_FUNDAMENTALS;
	//m_state = ST_BULLETINS;
	//m_state = ST_ACCOUNTOPERATIONS;
	//m_state = ST_ORDEROPERATIONS;
	//m_state = ST_OCASAMPLES;
	//m_state = ST_CONDITIONSAMPLES;
	//m_state = ST_BRACKETSAMPLES;
	//m_state = ST_HEDGESAMPLES;
	//m_state = ST_TESTALGOSAMPLES;
	//m_state = ST_FAORDERSAMPLES;
//	m_state = ST_FAOPERATIONS;
	//m_state = ST_DISPLAYGROUPS;
	//m_state = ST_MISCELANEOUS;
	//m_state = ST_FAMILYCODES;
	//m_state = ST_SYMBOLSAMPLES;
	//m_state = ST_REQMKTDEPTHEXCHANGES;
	//m_state = ST_REQNEWSTICKS;
	//m_state = ST_REQSMARTCOMPONENTS;
	//m_state = ST_NEWSPROVIDERS;
	//m_state = ST_REQNEWSARTICLE;
	//m_state = ST_REQHISTORICALNEWS;
	//m_state = ST_REQHEADTIMESTAMP;
	//m_state = ST_REQHISTOGRAMDATA;
	//m_state = ST_REROUTECFD;
	//m_state = ST_MARKETRULE;
	//m_state = ST_PING;
	//m_state = ST_WHATIFSAMPLES;
}


void TestCppClient::currentTime( long time)
{
	if ( m_state == ST_PING_ACK) {
		time_t t = ( time_t)time;
		struct tm * timeinfo = localtime ( &t);
		printf( "The current date/time is: %s", asctime( timeinfo));

		time_t now = ::time(NULL);
		m_sleepDeadline = now + SLEEP_BETWEEN_PINGS;

		m_state = ST_PING_ACK;
	}
}

//! [error]
void TestCppClient::error(int id, int errorCode, const std::string& errorString)
{
	printf( "Error. Id: %d, Code: %d, Msg: %s\n", id, errorCode, errorString.c_str());
	/*if (id >= 1000 && id < 9000 && (errorCode==200 || errorCode ==354))
	{
		m_pClient->cancelMktData(id);
		int nStockId = id - 1000;
		m_pClient->reqMktData(id + 1, ContractSamples::StockForQuery(StockNameList[nStockId + 1]), "", false, false, TagValueListSPtr());
	}*/
}
//! [error]

int TestCppClient::GetStrikeIndex(int nTickId)
{
	return nTickId / 10000;
}

//! [tickprice]
void TestCppClient::tickPrice( TickerId tickerId, TickType field, double price, const TickAttrib& attribs) {
	printf( "Tick Price. Ticker Id: %ld, Field: %d, Price: %g, CanAutoExecute: %d, PastLimit: %d, PreOpen: %d\n", tickerId, (int)field, price, attribs.canAutoExecute, attribs.pastLimit, attribs.preOpen);
	if (/*field == TickType::CLOSE || field== TickType::LAST ||*/ 1)
	{
		if (tickerId>=0  && tickerId < 200000 && field == TickType::LAST)
		{
			
			int nStockId = tickerId%10000;
			int nIndex = GetStrikeIndex(tickerId);
			NowPrice[nIndex][nStockId] = price;
			if (bFalg[nIndex][nStockId] == true)
				return;
			char pszFileName[256];
			printf("%s price\n", StockNameList[nStockId]);
			sprintf_s(pszFileName, 256, "C:\\bighouse\\波动率探索器\\%s\\%s.txt", OptionDataList[nIndex], StockNameList[nStockId]);
			HANDLE hLog = gamelog::OpenLogFile(pszFileName, 2);
			DWORD dwSize = GetFileSize(hLog, 0);
			
			char *pBuf = (char *)malloc(dwSize);
			DWORD dwRead;
			ReadFile(hLog, pBuf, dwSize, &dwRead, 0);
			int k = 0;
			std::vector<double> priceList;
			char pszTemp[512] = "";
			int j = 0;
			for (;;)
			{
				if (pBuf[k] == 0x00)
					break;
				if (pBuf[k] == 0x0a)
				{
					pszTemp[j] = 0x00;
					double price = atof(pszTemp);
					priceList.push_back(price);
					j = 0;
				}
				else if (pBuf[k] != 0x0a && pBuf[k] != 0x0d)
				{
					pszTemp[j] = pBuf[k];
					j++;
				}
				k++;


			}
			free(pBuf);
			CloseHandle(hLog);
			if (priceList.size() == 0)
				return;

			std::sort(priceList.begin(), priceList.end());

			double ppp = price *0.9;
			double target = price;
			//股价大于1000，起跳价50 
			//股价大于500,起跳价20
			//股价大于200到500,起跳价10
			//股价大于80小于200，起跳价5
			//股价大于50小于80，起跳价2.5
			//股价大于20小于50，起跳价1
			//股价大于0小于20，起跳价0.5
			double fpa[7] = { 1000,500,200,80,50,20,0 };
			double fbb[7] = { 50,20,10,5,2.5,1,0.5 };
			for (int k = 0; k < 7; k++)
			{
				if (ppp >= fpa[k])
				{
					double fVlaue = fmod(ppp, fbb[k]);
					double ftemp = ppp - fVlaue;
					if (ppp > ftemp + fbb[k] / 2)
					{
						ftemp += fbb[k];
					}
					target = ftemp;
					break;

				}
			}
			/*if (target>=price*0.9)
			{
				return;
			}*/
			double fStrike = 0;
			//从列表中得出估算出最优的行权价
			for (int k = 0; k < priceList.size(); k++)
			{
				if (target <= priceList[k])
				{
					fStrike = priceList[k];
					break;
				}
			}
			StrikeList[nIndex][nStockId] = fStrike;
			if (fStrike > price)
				return;
			m_pClient->reqMktData(200000 +nIndex*10000+ nStockId, ContractSamples::USOptionContractEx(StockNameList[nStockId], OptionDataList[nIndex], fStrike), "", false, false, TagValueListSPtr());
			bFalg[nIndex][nStockId] = true;
			llReqTick[nIndex][nStockId] = GetTickCount64();
			bReqSuc[nIndex][nStockId] = false;
			//std::this_thread::sleep_for(std::chrono::seconds(10));

			//m_pClient->cancelMktData(9000 + nStockId);
		}
		else if (tickerId >= 200000 && (field == TickType::BID || field == TickType::ASK))
		{

			int nStockId = (tickerId - 200000) % 10000;
			int nIndex = GetStrikeIndex(tickerId - 200000);
			if (field == TickType::BID)
			{
				if(price>=0)
				  bidPriceList[nIndex][nStockId] = price;
			}
			else if (field == TickType::ASK)
			{
				if (price >= 0)
				  askPriceList[nIndex][nStockId] = price;
			}
		}
		else if (tickerId >= 200000 && field == TickType::LAST)
		{

		  
			int nStockId = (tickerId - 200000)%10000;
			int nIndex = GetStrikeIndex(tickerId-200000);

			bReqSuc[nIndex][nStockId] = true;
			WriteRateToFile(nIndex, nStockId, price);
			
			//sprintf_s(pszDir, 256, "C:\\bighouse\\波动率探索器\\利率\\%s", OptionDataList[m]);
			//CreateDirectory(pszDir, NULL);

			m_pClient->cancelMktData(tickerId);
		
			
		}
		else if (tickerId >= 200000 && field == TickType::CLOSE)
		{
		   m_pClient->cancelMktData(tickerId);
		}
		/*int nStockId = tickerId - 1000;
		char pszFileName[256];
		sprintf_s(pszFileName, 256, "C:\\bighouse\\波动率探索器\\%s.txt", nStockId);
		HANDLE hLog = gamelog::OpenLogFile(pszFileName, 2);
		DWORD dwSize=GetFileSize(hLog, 0);
		char *pBuf = (char *)malloc(dwSize);*/

	}
	/*if (tickerId >= 200000 && field != TickType::LAST)
	{
		m_pClient->cancelMktData(tickerId);
	}*/
}
//! [tickprice]

//! [ticksize]
void TestCppClient::tickSize( TickerId tickerId, TickType field, int size) {
//	printf( "Tick Size. Ticker Id: %ld, Field: %d, Size: %d\n", tickerId, (int)field, size);
}
//! [ticksize]

//! [tickoptioncomputation]
void TestCppClient::tickOptionComputation( TickerId tickerId, TickType tickType, int tickAttrib, double impliedVol, double delta,
                                          double optPrice, double pvDividend,
                                          double gamma, double vega, double theta, double undPrice) {
	printf( "TickOptionComputation. Ticker Id: %ld, Type: %d, TickAttrib: %d, ImpliedVolatility: %g, Delta: %g, OptionPrice: %g, pvDividend: %g, Gamma: %g, Vega: %g, Theta: %g, Underlying Price: %g\n", tickerId, (int)tickType, tickAttrib, impliedVol, delta, optPrice, pvDividend, gamma, vega, theta, undPrice);
}
//! [tickoptioncomputation]

//! [tickgeneric]
void TestCppClient::tickGeneric(TickerId tickerId, TickType tickType, double value) {
	printf( "Tick Generic. Ticker Id: %ld, Type: %d, Value: %g\n", tickerId, (int)tickType, value);
}
//! [tickgeneric]

//! [tickstring]
void TestCppClient::tickString(TickerId tickerId, TickType tickType, const std::string& value) {
	printf( "Tick String. Ticker Id: %ld, Type: %d, Value: %s\n", tickerId, (int)tickType, value.c_str());
}
//! [tickstring]

void TestCppClient::tickEFP(TickerId tickerId, TickType tickType, double basisPoints, const std::string& formattedBasisPoints,
                            double totalDividends, int holdDays, const std::string& futureLastTradeDate, double dividendImpact, double dividendsToLastTradeDate) {
	printf( "TickEFP. %ld, Type: %d, BasisPoints: %g, FormattedBasisPoints: %s, Total Dividends: %g, HoldDays: %d, Future Last Trade Date: %s, Dividend Impact: %g, Dividends To Last Trade Date: %g\n", tickerId, (int)tickType, basisPoints, formattedBasisPoints.c_str(), totalDividends, holdDays, futureLastTradeDate.c_str(), dividendImpact, dividendsToLastTradeDate);
}

//! [orderstatus]
void TestCppClient::orderStatus(OrderId orderId, const std::string& status, double filled,
		double remaining, double avgFillPrice, int permId, int parentId,
		double lastFillPrice, int clientId, const std::string& whyHeld, double mktCapPrice){
	printf("OrderStatus. Id: %ld, Status: %s, Filled: %g, Remaining: %g, AvgFillPrice: %g, PermId: %d, LastFillPrice: %g, ClientId: %d, WhyHeld: %s, MktCapPrice: %g\n", orderId, status.c_str(), filled, remaining, avgFillPrice, permId, lastFillPrice, clientId, whyHeld.c_str(), mktCapPrice);
}
//! [orderstatus]

//! [openorder]
void TestCppClient::openOrder( OrderId orderId, const Contract& contract, const Order& order, const OrderState& orderState) {
	printf( "OpenOrder. PermId: %i, ClientId: %ld, OrderId: %ld, Account: %s, Symbol: %s, SecType: %s, Exchange: %s:, Action: %s, OrderType:%s, TotalQty: %g, CashQty: %g, "
	"LmtPrice: %g, AuxPrice: %g, Status: %s\n", 
		order.permId, order.clientId, orderId, order.account.c_str(), contract.symbol.c_str(), contract.secType.c_str(), contract.exchange.c_str(), 
		order.action.c_str(), order.orderType.c_str(), order.totalQuantity, order.cashQty == UNSET_DOUBLE ? 0 : order.cashQty, order.lmtPrice, order.auxPrice, orderState.status.c_str());
}
//! [openorder]

//! [openorderend]
void TestCppClient::openOrderEnd() {
	printf( "OpenOrderEnd\n");
}
//! [openorderend]

void TestCppClient::winError( const std::string& str, int lastError) {}
void TestCppClient::connectionClosed() {
	printf( "Connection Closed\n");
}

//! [updateaccountvalue]
void TestCppClient::updateAccountValue(const std::string& key, const std::string& val,
                                       const std::string& currency, const std::string& accountName) {
	printf("UpdateAccountValue. Key: %s, Value: %s, Currency: %s, Account Name: %s\n", key.c_str(), val.c_str(), currency.c_str(), accountName.c_str());
}
//! [updateaccountvalue]

//! [updateportfolio]
void TestCppClient::updatePortfolio(const Contract& contract, double position,
                                    double marketPrice, double marketValue, double averageCost,
                                    double unrealizedPNL, double realizedPNL, const std::string& accountName){
	printf("UpdatePortfolio. %s, %s @ %s: Position: %g, MarketPrice: %g, MarketValue: %g, AverageCost: %g, UnrealizedPNL: %g, RealizedPNL: %g, AccountName: %s\n", (contract.symbol).c_str(), (contract.secType).c_str(), (contract.primaryExchange).c_str(), position, marketPrice, marketValue, averageCost, unrealizedPNL, realizedPNL, accountName.c_str());
}
//! [updateportfolio]

//! [updateaccounttime]
void TestCppClient::updateAccountTime(const std::string& timeStamp) {
	printf( "UpdateAccountTime. Time: %s\n", timeStamp.c_str());
}
//! [updateaccounttime]

//! [accountdownloadend]
void TestCppClient::accountDownloadEnd(const std::string& accountName) {
	printf( "Account download finished: %s\n", accountName.c_str());
}
//! [accountdownloadend]

//! [contractdetails]
void TestCppClient::contractDetails( int reqId, const ContractDetails& contractDetails) {
	printf( "ContractDetails begin. ReqId: %d\n", reqId);
	printContractMsg(reqId,contractDetails.contract);
	printContractDetailsMsg(contractDetails);

	
	printf( "ContractDetails end. ReqId: %d\n", reqId);
}
//! [contractdetails]

//! [bondcontractdetails]
void TestCppClient::bondContractDetails( int reqId, const ContractDetails& contractDetails) {
	printf( "BondContractDetails begin. ReqId: %d\n", reqId);
	printBondContractDetailsMsg(contractDetails);
	printf( "BondContractDetails end. ReqId: %d\n", reqId);
}
//! [bondcontractdetails]

void TestCppClient::printContractMsg(int reqId,const Contract& contract) {
	char pszWrite[1024];
	char pszFileName[256];
	sprintf_s(pszFileName,256, "C:\\bighouse\\波动率探索器\\%s\\%s.txt", contract.lastTradeDateOrContractMonth.c_str(),contract.symbol.c_str());
	if (contract.right.at(0) == 'P')
	{
		sprintf_s(pszWrite, 1024, "%g\n",contract.strike);
		gamelog::WriteLog(pszFileName, pszWrite);

		sprintf_s(pszFileName, 256, "C:\\bighouse\\波动率探索器\\%s\\索引.txt", contract.lastTradeDateOrContractMonth.c_str());
		sprintf_s(pszWrite, 1024, "%d", reqId%10000+1);
		gamelog::WriteLog(pszFileName, pszWrite,2);
	}
	//gamelog::WriteLog()
	//gamelog::OpenLogFile(pszFileName, 0);
	printf("\tConId: %ld\n", contract.conId);
	printf("\tSymbol: %s\n", contract.symbol.c_str());
	printf("\tSecType: %s\n", contract.secType.c_str());
	printf("\tLastTradeDateOrContractMonth: %s\n", contract.lastTradeDateOrContractMonth.c_str());
	printf("\tStrike: %g\n", contract.strike);
	printf("\tRight: %s\n", contract.right.c_str());
	printf("\tMultiplier: %s\n", contract.multiplier.c_str());
	printf("\tExchange: %s\n", contract.exchange.c_str());
	printf("\tPrimaryExchange: %s\n", contract.primaryExchange.c_str());
	printf("\tCurrency: %s\n", contract.currency.c_str());
	printf("\tLocalSymbol: %s\n", contract.localSymbol.c_str());
	printf("\tTradingClass: %s\n", contract.tradingClass.c_str());
}

void TestCppClient::printContractDetailsMsg(const ContractDetails& contractDetails) {
	printf("\tMarketName: %s\n", contractDetails.marketName.c_str());
	printf("\tMinTick: %g\n", contractDetails.minTick);
	printf("\tPriceMagnifier: %ld\n", contractDetails.priceMagnifier);
	printf("\tOrderTypes: %s\n", contractDetails.orderTypes.c_str());
	printf("\tValidExchanges: %s\n", contractDetails.validExchanges.c_str());
	printf("\tUnderConId: %d\n", contractDetails.underConId);
	printf("\tLongName: %s\n", contractDetails.longName.c_str());
	printf("\tContractMonth: %s\n", contractDetails.contractMonth.c_str());
	printf("\tIndystry: %s\n", contractDetails.industry.c_str());
	printf("\tCategory: %s\n", contractDetails.category.c_str());
	printf("\tSubCategory: %s\n", contractDetails.subcategory.c_str());
	printf("\tTimeZoneId: %s\n", contractDetails.timeZoneId.c_str());
	printf("\tTradingHours: %s\n", contractDetails.tradingHours.c_str());
	printf("\tLiquidHours: %s\n", contractDetails.liquidHours.c_str());
	printf("\tEvRule: %s\n", contractDetails.evRule.c_str());
	printf("\tEvMultiplier: %g\n", contractDetails.evMultiplier);
	printf("\tMdSizeMultiplier: %d\n", contractDetails.mdSizeMultiplier);
	printf("\tAggGroup: %d\n", contractDetails.aggGroup);
	printf("\tUnderSymbol: %s\n", contractDetails.underSymbol.c_str());
	printf("\tUnderSecType: %s\n", contractDetails.underSecType.c_str());
	printf("\tMarketRuleIds: %s\n", contractDetails.marketRuleIds.c_str());
	printf("\tRealExpirationDate: %s\n", contractDetails.realExpirationDate.c_str());
	printf("\tLastTradeTime: %s\n", contractDetails.lastTradeTime.c_str());
	printf("\tStockType: %s\n", contractDetails.stockType.c_str());
	printContractDetailsSecIdList(contractDetails.secIdList);
}

void TestCppClient::printContractDetailsSecIdList(const TagValueListSPtr &secIdList) {
	const int secIdListCount = secIdList.get() ? secIdList->size() : 0;
	if (secIdListCount > 0) {
		printf("\tSecIdList: {");
		for (int i = 0; i < secIdListCount; ++i) {
			const TagValue* tagValue = ((*secIdList)[i]).get();
			printf("%s=%s;",tagValue->tag.c_str(), tagValue->value.c_str());
		}
		printf("}\n");
	}
}

void TestCppClient::printBondContractDetailsMsg(const ContractDetails& contractDetails) {
	printf("\tSymbol: %s\n", contractDetails.contract.symbol.c_str());
	printf("\tSecType: %s\n", contractDetails.contract.secType.c_str());
	printf("\tCusip: %s\n", contractDetails.cusip.c_str());
	printf("\tCoupon: %g\n", contractDetails.coupon);
	printf("\tMaturity: %s\n", contractDetails.maturity.c_str());
	printf("\tIssueDate: %s\n", contractDetails.issueDate.c_str());
	printf("\tRatings: %s\n", contractDetails.ratings.c_str());
	printf("\tBondType: %s\n", contractDetails.bondType.c_str());
	printf("\tCouponType: %s\n", contractDetails.couponType.c_str());
	printf("\tConvertible: %s\n", contractDetails.convertible ? "yes" : "no");
	printf("\tCallable: %s\n", contractDetails.callable ? "yes" : "no");
	printf("\tPutable: %s\n", contractDetails.putable ? "yes" : "no");
	printf("\tDescAppend: %s\n", contractDetails.descAppend.c_str());
	printf("\tExchange: %s\n", contractDetails.contract.exchange.c_str());
	printf("\tCurrency: %s\n", contractDetails.contract.currency.c_str());
	printf("\tMarketName: %s\n", contractDetails.marketName.c_str());
	printf("\tTradingClass: %s\n", contractDetails.contract.tradingClass.c_str());
	printf("\tConId: %ld\n", contractDetails.contract.conId);
	printf("\tMinTick: %g\n", contractDetails.minTick);
	printf("\tMdSizeMultiplier: %d\n", contractDetails.mdSizeMultiplier);
	printf("\tOrderTypes: %s\n", contractDetails.orderTypes.c_str());
	printf("\tValidExchanges: %s\n", contractDetails.validExchanges.c_str());
	printf("\tNextOptionDate: %s\n", contractDetails.nextOptionDate.c_str());
	printf("\tNextOptionType: %s\n", contractDetails.nextOptionType.c_str());
	printf("\tNextOptionPartial: %s\n", contractDetails.nextOptionPartial ? "yes" : "no");
	printf("\tNotes: %s\n", contractDetails.notes.c_str());
	printf("\tLong Name: %s\n", contractDetails.longName.c_str());
	printf("\tEvRule: %s\n", contractDetails.evRule.c_str());
	printf("\tEvMultiplier: %g\n", contractDetails.evMultiplier);
	printf("\tAggGroup: %d\n", contractDetails.aggGroup);
	printf("\tMarketRuleIds: %s\n", contractDetails.marketRuleIds.c_str());
	printf("\tTimeZoneId: %s\n", contractDetails.timeZoneId.c_str());
	printf("\tLastTradeTime: %s\n", contractDetails.lastTradeTime.c_str());
	printContractDetailsSecIdList(contractDetails.secIdList);
}

//! [contractdetailsend]
void TestCppClient::contractDetailsEnd( int reqId) {
	printf( "ContractDetailsEnd. %d\n", reqId);
}
//! [contractdetailsend]

//! [execdetails]
void TestCppClient::execDetails( int reqId, const Contract& contract, const Execution& execution) {
	printf( "ExecDetails. ReqId: %d - %s, %s, %s - %s, %ld, %g, %d\n", reqId, contract.symbol.c_str(), contract.secType.c_str(), contract.currency.c_str(), execution.execId.c_str(), execution.orderId, execution.shares, execution.lastLiquidity);
}
//! [execdetails]

//! [execdetailsend]
void TestCppClient::execDetailsEnd( int reqId) {
	printf( "ExecDetailsEnd. %d\n", reqId);
}
//! [execdetailsend]

//! [updatemktdepth]
void TestCppClient::updateMktDepth(TickerId id, int position, int operation, int side,
                                   double price, int size) {
	printf( "UpdateMarketDepth. %ld - Position: %d, Operation: %d, Side: %d, Price: %g, Size: %d\n", id, position, operation, side, price, size);
}
//! [updatemktdepth]

//! [updatemktdepthl2]
void TestCppClient::updateMktDepthL2(TickerId id, int position, const std::string& marketMaker, int operation,
                                     int side, double price, int size, bool isSmartDepth) {
	printf( "UpdateMarketDepthL2. %ld - Position: %d, Operation: %d, Side: %d, Price: %g, Size: %d, isSmartDepth: %d\n", id, position, operation, side, price, size, isSmartDepth);
}
//! [updatemktdepthl2]

//! [updatenewsbulletin]
void TestCppClient::updateNewsBulletin(int msgId, int msgType, const std::string& newsMessage, const std::string& originExch) {
	printf( "News Bulletins. %d - Type: %d, Message: %s, Exchange of Origin: %s\n", msgId, msgType, newsMessage.c_str(), originExch.c_str());
}
//! [updatenewsbulletin]

//! [managedaccounts]
void TestCppClient::managedAccounts( const std::string& accountsList) {
	printf( "Account List: %s\n", accountsList.c_str());
}
//! [managedaccounts]

//! [receivefa]
void TestCppClient::receiveFA(faDataType pFaDataType, const std::string& cxml) {
	std::cout << "Receiving FA: " << (int)pFaDataType << std::endl << cxml << std::endl;
}
//! [receivefa]

//! [historicaldata]
void TestCppClient::historicalData(TickerId reqId, const Bar& bar) {
	printf( "HistoricalData. ReqId: %ld - Date: %s, Open: %g, High: %g, Low: %g, Close: %g, Volume: %lld, Count: %d, WAP: %g\n", reqId, bar.time.c_str(), bar.open, bar.high, bar.low, bar.close, bar.volume, bar.count, bar.wap);
}
//! [historicaldata]

//! [historicaldataend]
void TestCppClient::historicalDataEnd(int reqId, const std::string& startDateStr, const std::string& endDateStr) {
	std::cout << "HistoricalDataEnd. ReqId: " << reqId << " - Start Date: " << startDateStr << ", End Date: " << endDateStr << std::endl;	
}
//! [historicaldataend]

//! [scannerparameters]
void TestCppClient::scannerParameters(const std::string& xml) {
	printf( "ScannerParameters. %s\n", xml.c_str());
}
//! [scannerparameters]

//! [scannerdata]
void TestCppClient::scannerData(int reqId, int rank, const ContractDetails& contractDetails,
                                const std::string& distance, const std::string& benchmark, const std::string& projection,
                                const std::string& legsStr) {
	printf( "ScannerData. %d - Rank: %d, Symbol: %s, SecType: %s, Currency: %s, Distance: %s, Benchmark: %s, Projection: %s, Legs String: %s\n", reqId, rank, contractDetails.contract.symbol.c_str(), contractDetails.contract.secType.c_str(), contractDetails.contract.currency.c_str(), distance.c_str(), benchmark.c_str(), projection.c_str(), legsStr.c_str());
}
//! [scannerdata]

//! [scannerdataend]
void TestCppClient::scannerDataEnd(int reqId) {
	printf( "ScannerDataEnd. %d\n", reqId);
}
//! [scannerdataend]

//! [realtimebar]
void TestCppClient::realtimeBar(TickerId reqId, long time, double open, double high, double low, double close,
                                long volume, double wap, int count) {
	printf( "RealTimeBars. %ld - Time: %ld, Open: %g, High: %g, Low: %g, Close: %g, Volume: %ld, Count: %d, WAP: %g\n", reqId, time, open, high, low, close, volume, count, wap);
}
//! [realtimebar]

//! [fundamentaldata]
void TestCppClient::fundamentalData(TickerId reqId, const std::string& data) {
	printf( "FundamentalData. ReqId: %ld, %s\n", reqId, data.c_str());
}
//! [fundamentaldata]

void TestCppClient::deltaNeutralValidation(int reqId, const DeltaNeutralContract& deltaNeutralContract) {
	printf( "DeltaNeutralValidation. %d, ConId: %ld, Delta: %g, Price: %g\n", reqId, deltaNeutralContract.conId, deltaNeutralContract.delta, deltaNeutralContract.price);
}

//! [ticksnapshotend]
void TestCppClient::tickSnapshotEnd(int reqId) {
	printf( "TickSnapshotEnd: %d\n", reqId);
}
//! [ticksnapshotend]

//! [marketdatatype]
void TestCppClient::marketDataType(TickerId reqId, int marketDataType) {
	printf( "MarketDataType. ReqId: %ld, Type: %d\n", reqId, marketDataType);
}
//! [marketdatatype]

//! [commissionreport]
void TestCppClient::commissionReport( const CommissionReport& commissionReport) {
	printf( "CommissionReport. %s - %g %s RPNL %g\n", commissionReport.execId.c_str(), commissionReport.commission, commissionReport.currency.c_str(), commissionReport.realizedPNL);
}
//! [commissionreport]

//! [position]
void TestCppClient::position( const std::string& account, const Contract& contract, double position, double avgCost) {
	printf( "Position. %s - Symbol: %s, SecType: %s, Currency: %s, Position: %g, Avg Cost: %g\n", account.c_str(), contract.symbol.c_str(), contract.secType.c_str(), contract.currency.c_str(), position, avgCost);
}
//! [position]

//! [positionend]
void TestCppClient::positionEnd() {
	printf( "PositionEnd\n");
}
//! [positionend]

//! [accountsummary]
void TestCppClient::accountSummary( int reqId, const std::string& account, const std::string& tag, const std::string& value, const std::string& currency) {
	printf( "Acct Summary. ReqId: %d, Account: %s, Tag: %s, Value: %s, Currency: %s\n", reqId, account.c_str(), tag.c_str(), value.c_str(), currency.c_str());
}
//! [accountsummary]

//! [accountsummaryend]
void TestCppClient::accountSummaryEnd( int reqId) {
	printf( "AccountSummaryEnd. Req Id: %d\n", reqId);
}
//! [accountsummaryend]

void TestCppClient::verifyMessageAPI( const std::string& apiData) {
	printf("verifyMessageAPI: %s\b", apiData.c_str());
}

void TestCppClient::verifyCompleted( bool isSuccessful, const std::string& errorText) {
	printf("verifyCompleted. IsSuccessfule: %d - Error: %s\n", isSuccessful, errorText.c_str());
}

void TestCppClient::verifyAndAuthMessageAPI( const std::string& apiDatai, const std::string& xyzChallenge) {
	printf("verifyAndAuthMessageAPI: %s %s\n", apiDatai.c_str(), xyzChallenge.c_str());
}

void TestCppClient::verifyAndAuthCompleted( bool isSuccessful, const std::string& errorText) {
	printf("verifyAndAuthCompleted. IsSuccessful: %d - Error: %s\n", isSuccessful, errorText.c_str());
    if (isSuccessful)
        m_pClient->startApi();
}

//! [displaygrouplist]
void TestCppClient::displayGroupList( int reqId, const std::string& groups) {
	printf("Display Group List. ReqId: %d, Groups: %s\n", reqId, groups.c_str());
}
//! [displaygrouplist]

//! [displaygroupupdated]
void TestCppClient::displayGroupUpdated( int reqId, const std::string& contractInfo) {
	std::cout << "Display Group Updated. ReqId: " << reqId << ", Contract Info: " << contractInfo << std::endl;
}
//! [displaygroupupdated]

//! [positionmulti]
void TestCppClient::positionMulti( int reqId, const std::string& account,const std::string& modelCode, const Contract& contract, double pos, double avgCost) {
	printf("Position Multi. Request: %d, Account: %s, ModelCode: %s, Symbol: %s, SecType: %s, Currency: %s, Position: %g, Avg Cost: %g\n", reqId, account.c_str(), modelCode.c_str(), contract.symbol.c_str(), contract.secType.c_str(), contract.currency.c_str(), pos, avgCost);
}
//! [positionmulti]

//! [positionmultiend]
void TestCppClient::positionMultiEnd( int reqId) {
	printf("Position Multi End. Request: %d\n", reqId);
}
//! [positionmultiend]

//! [accountupdatemulti]
void TestCppClient::accountUpdateMulti( int reqId, const std::string& account, const std::string& modelCode, const std::string& key, const std::string& value, const std::string& currency) {
	printf("AccountUpdate Multi. Request: %d, Account: %s, ModelCode: %s, Key, %s, Value: %s, Currency: %s\n", reqId, account.c_str(), modelCode.c_str(), key.c_str(), value.c_str(), currency.c_str());
}
//! [accountupdatemulti]

//! [accountupdatemultiend]
void TestCppClient::accountUpdateMultiEnd( int reqId) {
	printf("Account Update Multi End. Request: %d\n", reqId);
}
//! [accountupdatemultiend]

//! [securityDefinitionOptionParameter]
void TestCppClient::securityDefinitionOptionalParameter(int reqId, const std::string& exchange, int underlyingConId, const std::string& tradingClass,
                                                        const std::string& multiplier, const std::set<std::string>& expirations, const std::set<double>& strikes) {
	printf("Security Definition Optional Parameter. Request: %d, Trading Class: %s, Multiplier: %s\n", reqId, tradingClass.c_str(), multiplier.c_str());
}
//! [securityDefinitionOptionParameter]

//! [securityDefinitionOptionParameterEnd]
void TestCppClient::securityDefinitionOptionalParameterEnd(int reqId) {
	printf("Security Definition Optional Parameter End. Request: %d\n", reqId);
}
//! [securityDefinitionOptionParameterEnd]

//! [softDollarTiers]
void TestCppClient::softDollarTiers(int reqId, const std::vector<SoftDollarTier> &tiers) {
	printf("Soft dollar tiers (%lu):", tiers.size());

	for (unsigned int i = 0; i < tiers.size(); i++) {
		printf("%s\n", tiers[i].displayName().c_str());
	}
}
//! [softDollarTiers]

//! [familyCodes]
void TestCppClient::familyCodes(const std::vector<FamilyCode> &familyCodes) {
	printf("Family codes (%lu):\n", familyCodes.size());

	for (unsigned int i = 0; i < familyCodes.size(); i++) {
		printf("Family code [%d] - accountID: %s familyCodeStr: %s\n", i, familyCodes[i].accountID.c_str(), familyCodes[i].familyCodeStr.c_str());
	}
}
//! [familyCodes]

//! [symbolSamples]
void TestCppClient::symbolSamples(int reqId, const std::vector<ContractDescription> &contractDescriptions) {
	printf("Symbol Samples (total=%lu) reqId: %d\n", contractDescriptions.size(), reqId);

	for (unsigned int i = 0; i < contractDescriptions.size(); i++) {
		Contract contract = contractDescriptions[i].contract;
		std::vector<std::string> derivativeSecTypes = contractDescriptions[i].derivativeSecTypes;
		printf("Contract (%u): %ld %s %s %s %s, ", i, contract.conId, contract.symbol.c_str(), contract.secType.c_str(), contract.primaryExchange.c_str(), contract.currency.c_str());
		printf("Derivative Sec-types (%lu):", derivativeSecTypes.size());
		for (unsigned int j = 0; j < derivativeSecTypes.size(); j++) {
			printf(" %s", derivativeSecTypes[j].c_str());
		}
		printf("\n");
	}
}
//! [symbolSamples]

//! [mktDepthExchanges]
void TestCppClient::mktDepthExchanges(const std::vector<DepthMktDataDescription> &depthMktDataDescriptions) {
	printf("Mkt Depth Exchanges (%lu):\n", depthMktDataDescriptions.size());

	for (unsigned int i = 0; i < depthMktDataDescriptions.size(); i++) {
		printf("Depth Mkt Data Description [%d] - exchange: %s secType: %s listingExch: %s serviceDataType: %s aggGroup: %s\n", i, 
			depthMktDataDescriptions[i].exchange.c_str(), 
			depthMktDataDescriptions[i].secType.c_str(), 
			depthMktDataDescriptions[i].listingExch.c_str(), 
			depthMktDataDescriptions[i].serviceDataType.c_str(), 
			depthMktDataDescriptions[i].aggGroup != INT_MAX ? std::to_string(depthMktDataDescriptions[i].aggGroup).c_str() : "");
	}
}
//! [mktDepthExchanges]

//! [tickNews]
void TestCppClient::tickNews(int tickerId, time_t timeStamp, const std::string& providerCode, const std::string& articleId, const std::string& headline, const std::string& extraData) {
	printf("News Tick. TickerId: %d, TimeStamp: %s, ProviderCode: %s, ArticleId: %s, Headline: %s, ExtraData: %s\n", tickerId, ctime(&(timeStamp /= 1000)), providerCode.c_str(), articleId.c_str(), headline.c_str(), extraData.c_str());
}
//! [tickNews]

//! [smartcomponents]]
void TestCppClient::smartComponents(int reqId, const SmartComponentsMap& theMap) {
	printf("Smart components: (%lu):\n", theMap.size());

	for (SmartComponentsMap::const_iterator i = theMap.begin(); i != theMap.end(); i++) {
		printf(" bit number: %d exchange: %s exchange letter: %c\n", i->first, std::get<0>(i->second).c_str(), std::get<1>(i->second));
	}
}
//! [smartcomponents]

//! [tickReqParams]
void TestCppClient::tickReqParams(int tickerId, double minTick, const std::string& bboExchange, int snapshotPermissions) {
	printf("tickerId: %d, minTick: %g, bboExchange: %s, snapshotPermissions: %u\n", tickerId, minTick, bboExchange.c_str(), snapshotPermissions);

	m_bboExchange = bboExchange;
}
//! [tickReqParams]

//! [newsProviders]
void TestCppClient::newsProviders(const std::vector<NewsProvider> &newsProviders) {
	printf("News providers (%lu):\n", newsProviders.size());

	for (unsigned int i = 0; i < newsProviders.size(); i++) {
		printf("News provider [%d] - providerCode: %s providerName: %s\n", i, newsProviders[i].providerCode.c_str(), newsProviders[i].providerName.c_str());
	}
}
//! [newsProviders]

//! [newsArticle]
void TestCppClient::newsArticle(int requestId, int articleType, const std::string& articleText) {
	printf("News Article. Request Id: %d, Article Type: %d\n", requestId, articleType);
	if (articleType == 0) {
		printf("News Article Text (text or html): %s\n", articleText.c_str());
	} else if (articleType == 1) {
		std::string path;
		#if defined(IB_WIN32)
			TCHAR s[200];
			GetCurrentDirectory(200, s);
			path = s + std::string("\\MST$06f53098.pdf");
		#elif defined(IB_POSIX)
			char s[1024];
			if (getcwd(s, sizeof(s)) == NULL) {
				printf("getcwd() error\n");
				return;
			}
			path = s + std::string("/MST$06f53098.pdf");
		#endif
		std::vector<std::uint8_t> bytes = Utils::base64_decode(articleText);
		std::ofstream outfile(path, std::ios::out | std::ios::binary); 
		outfile.write((const char*)bytes.data(), bytes.size());
		printf("Binary/pdf article was saved to: %s\n", path.c_str());
	}
}
//! [newsArticle]

//! [historicalNews]
void TestCppClient::historicalNews(int requestId, const std::string& time, const std::string& providerCode, const std::string& articleId, const std::string& headline) {
	printf("Historical News. RequestId: %d, Time: %s, ProviderCode: %s, ArticleId: %s, Headline: %s\n", requestId, time.c_str(), providerCode.c_str(), articleId.c_str(), headline.c_str());
}
//! [historicalNews]

//! [historicalNewsEnd]
void TestCppClient::historicalNewsEnd(int requestId, bool hasMore) {
	printf("Historical News End. RequestId: %d, HasMore: %s\n", requestId, (hasMore ? "true" : " false"));
}
//! [historicalNewsEnd]

//! [headTimestamp]
void TestCppClient::headTimestamp(int reqId, const std::string& headTimestamp) {
	printf( "Head time stamp. ReqId: %d - Head time stamp: %s,\n", reqId, headTimestamp.c_str());

}
//! [headTimestamp]

//! [histogramData]
void TestCppClient::histogramData(int reqId, const HistogramDataVector& data) {
	printf("Histogram. ReqId: %d, data length: %lu\n", reqId, data.size());

	for (auto item : data) {
		printf("\t price: %f, size: %lld\n", item.price, item.size);
	}
}
//! [histogramData]

//! [historicalDataUpdate]
void TestCppClient::historicalDataUpdate(TickerId reqId, const Bar& bar) {
	printf( "HistoricalDataUpdate. ReqId: %ld - Date: %s, Open: %g, High: %g, Low: %g, Close: %g, Volume: %lld, Count: %d, WAP: %g\n", reqId, bar.time.c_str(), bar.open, bar.high, bar.low, bar.close, bar.volume, bar.count, bar.wap);
}
//! [historicalDataUpdate]

//! [rerouteMktDataReq]
void TestCppClient::rerouteMktDataReq(int reqId, int conid, const std::string& exchange) {
	printf( "Re-route market data request. ReqId: %d, ConId: %d, Exchange: %s\n", reqId, conid, exchange.c_str());
}
//! [rerouteMktDataReq]

//! [rerouteMktDepthReq]
void TestCppClient::rerouteMktDepthReq(int reqId, int conid, const std::string& exchange) {
	printf( "Re-route market depth request. ReqId: %d, ConId: %d, Exchange: %s\n", reqId, conid, exchange.c_str());
}
//! [rerouteMktDepthReq]

//! [marketRule]
void TestCppClient::marketRule(int marketRuleId, const std::vector<PriceIncrement> &priceIncrements) {
	printf("Market Rule Id: %d\n", marketRuleId);
	for (unsigned int i = 0; i < priceIncrements.size(); i++) {
		printf("Low Edge: %g, Increment: %g\n", priceIncrements[i].lowEdge, priceIncrements[i].increment);
	}
}
//! [marketRule]

//! [pnl]
void TestCppClient::pnl(int reqId, double dailyPnL, double unrealizedPnL, double realizedPnL) {
	printf("PnL. ReqId: %d, daily PnL: %g, unrealized PnL: %g, realized PnL: %g\n", reqId, dailyPnL, unrealizedPnL, realizedPnL);
}
//! [pnl]

//! [pnlsingle]
void TestCppClient::pnlSingle(int reqId, int pos, double dailyPnL, double unrealizedPnL, double realizedPnL, double value) {
	printf("PnL Single. ReqId: %d, pos: %d, daily PnL: %g, unrealized PnL: %g, realized PnL: %g, value: %g\n", reqId, pos, dailyPnL, unrealizedPnL, realizedPnL, value);
}
//! [pnlsingle]

//! [historicalticks]
void TestCppClient::historicalTicks(int reqId, const std::vector<HistoricalTick>& ticks, bool done) {
    for (HistoricalTick tick : ticks) {
	std::time_t t = tick.time;
        std::cout << "Historical tick. ReqId: " << reqId << ", time: " << ctime(&t) << ", price: "<< tick.price << ", size: " << tick.size << std::endl;
    }
}
//! [historicalticks]

//! [historicalticksbidask]
void TestCppClient::historicalTicksBidAsk(int reqId, const std::vector<HistoricalTickBidAsk>& ticks, bool done) {
    for (HistoricalTickBidAsk tick : ticks) {
	std::time_t t = tick.time;
        std::cout << "Historical tick bid/ask. ReqId: " << reqId << ", time: " << ctime(&t) << ", price bid: "<< tick.priceBid <<
            ", price ask: "<< tick.priceAsk << ", size bid: " << tick.sizeBid << ", size ask: " << tick.sizeAsk <<
            ", bidPastLow: " << tick.tickAttribBidAsk.bidPastLow << ", askPastHigh: " << tick.tickAttribBidAsk.askPastHigh << std::endl;
    }
}
//! [historicalticksbidask]

//! [historicaltickslast]
void TestCppClient::historicalTicksLast(int reqId, const std::vector<HistoricalTickLast>& ticks, bool done) {
    for (HistoricalTickLast tick : ticks) {
	std::time_t t = tick.time;
        std::cout << "Historical tick last. ReqId: " << reqId << ", time: " << ctime(&t) << ", price: "<< tick.price <<
            ", size: " << tick.size << ", exchange: " << tick.exchange << ", special conditions: " << tick.specialConditions <<
            ", unreported: " << tick.tickAttribLast.unreported << ", pastLimit: " << tick.tickAttribLast.pastLimit << std::endl;
    }
}
//! [historicaltickslast]

//! [tickbytickalllast]
void TestCppClient::tickByTickAllLast(int reqId, int tickType, time_t time, double price, int size, const TickAttribLast& tickAttribLast, const std::string& exchange, const std::string& specialConditions) {
    printf("Tick-By-Tick. ReqId: %d, TickType: %s, Time: %s, Price: %g, Size: %d, PastLimit: %d, Unreported: %d, Exchange: %s, SpecialConditions:%s\n", 
        reqId, (tickType == 1 ? "Last" : "AllLast"), ctime(&time), price, size, tickAttribLast.pastLimit, tickAttribLast.unreported, exchange.c_str(), specialConditions.c_str());
}
//! [tickbytickalllast]

//! [tickbytickbidask]
void TestCppClient::tickByTickBidAsk(int reqId, time_t time, double bidPrice, double askPrice, int bidSize, int askSize, const TickAttribBidAsk& tickAttribBidAsk) {
    printf("Tick-By-Tick. ReqId: %d, TickType: BidAsk, Time: %s, BidPrice: %g, AskPrice: %g, BidSize: %d, AskSize: %d, BidPastLow: %d, AskPastHigh: %d\n", 
        reqId, ctime(&time), bidPrice, askPrice, bidSize, askSize, tickAttribBidAsk.bidPastLow, tickAttribBidAsk.askPastHigh);
}
//! [tickbytickbidask]

//! [tickbytickmidpoint]
void TestCppClient::tickByTickMidPoint(int reqId, time_t time, double midPoint) {
    printf("Tick-By-Tick. ReqId: %d, TickType: MidPoint, Time: %s, MidPoint: %g\n", reqId, ctime(&time), midPoint);
}
//! [tickbytickmidpoint]

//! [orderbound]
void TestCppClient::orderBound(long long orderId, int apiClientId, int apiOrderId) {
    printf("Order bound. OrderId: %lld, ApiClientId: %d, ApiOrderId: %d\n", orderId, apiClientId, apiOrderId);
}
//! [orderbound]

//! [completedorder]
void TestCppClient::completedOrder(const Contract& contract, const Order& order, const OrderState& orderState) {
	printf( "CompletedOrder. PermId: %i, ParentPermId: %lld, Account: %s, Symbol: %s, SecType: %s, Exchange: %s:, Action: %s, OrderType: %s, TotalQty: %g, CashQty: %g, FilledQty: %g, "
		"LmtPrice: %g, AuxPrice: %g, Status: %s, CompletedTime: %s, CompletedStatus: %s\n", 
		order.permId, order.parentPermId == UNSET_LONG ? 0 : order.parentPermId, order.account.c_str(), contract.symbol.c_str(), contract.secType.c_str(), contract.exchange.c_str(), 
		order.action.c_str(), order.orderType.c_str(), order.totalQuantity, order.cashQty == UNSET_DOUBLE ? 0 : order.cashQty, order.filledQuantity, 
		order.lmtPrice, order.auxPrice, orderState.status.c_str(), orderState.completedTime.c_str(), orderState.completedStatus.c_str());
}
//! [completedorder]

//! [completedordersend]
void TestCppClient::completedOrdersEnd() {
	printf( "CompletedOrdersEnd\n");
}
//! [completedordersend]

//! [replacefaend]
void TestCppClient::replaceFAEnd(int reqId, const std::string& text) {
	printf("Replace FA End. Request: %d, Text:%s\n", reqId, text.c_str());
}
//! [replacefaend]
