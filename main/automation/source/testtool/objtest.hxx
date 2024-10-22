/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



#ifndef _OBJTEST_HXX
#define _OBJTEST_HXX

#include <tools/fsys.hxx>


#include <testapp.hxx>
#include <testtool.hxx>
#include "cmdstrm.hxx"
#include <basic/basicrt.hxx>
#include "httprequest.hxx"

#include <map>

//	#define ID_BeginBlock	1
//	#define ID_EndBlock		2
#define ID_Kontext		3
#define ID_GetError		4
#define ID_ErrorDummy	5            // Intern zum Behandlen von Fehlenden Controls	und Methoden
#define ID_RecordError	6
#define ID_Start		7
//#define ID_Kill			8
//#define ID_Reset		9
//	#define ID_AppAbort		10
#define ID_Dispatch		11
#define ID_UNODispatch	12
#define ID_Control		13
#define ID_StartUse		14
#define ID_Use			15
#define ID_FinishUse	16
#define ID_CaseLog		17
#define ID_ExceptLog	18
#define ID_PrintLog		19
#define ID_WarnLog		20
#define ID_ErrorLog		21
#define ID_EnableQaErrors 22
#define ID_QAErrorLog	23
#define ID_MaybeAddErr	24
#define ID_ClearError	25
#define ID_GetNextCloseWindow	26
#define ID_RemoteCommand 27
#define ID_SaveIDs		28
#define ID_AutoExecute	29
#define ID_Execute		30
#define ID_DialogHandler 31
#define ID_GetUnoApp	32
#define ID_GetIServer	33
#define ID_RemoteCommandDelay 34
#define ID_GetApplicationPath 35
#define ID_GetCommonApplicationPath 36
#define ID_MakeIniFileName 37
#define ID_StringControl   38
#define ID_Wait            39
#define ID_GetErrorCount   40
#define ID_GetWarningCount 41
#define ID_GetQAErrorCount 42
#define ID_GetUseFileWarningCount 43
#define ID_GetErrorList    44
#define ID_GetWarningList  45
#define ID_GetQAErrorList  46
#define ID_GetUseFileWarningList 47
#define ID_GetTestCaseName 48
#define ID_GetTestCaseFileName 49
#define ID_GetTestCaseLineNr 50
#define ID_StopOnSyntaxError 51
#define ID_SetChildEnv     52
#define ID_GetChildEnv     53
#define ID_GetLinkDestination 54
#define ID_GetRegistryValue 55
#define ID_KillApp         56
#define ID_HTTPSend        57
#define ID_HTTPSetProxy    58
#define ID_DoNothing       99

void ReadFlatArray( const ControlDefLoad arWas [], CNames *&pNames );

class ControlData
{
public:
	String Kurzname;
	rtl::OString aUId;
};

DBG_NAMEEX( ControlItem )
DBG_NAMEEX( ControlDef )
class ControlItem
{
private:
	void InitData() { pData = new ControlData; }
public:
	ControlData *pData;

	ControlItem( const char *Name, rtl::OString aUIdP );
	ControlItem( const String &Name, rtl::OString aUIdP );
//	ControlItem( const String &Name, const String &URL, const URLType aType );
//    ControlItem( const String &Name, const String &URL, const sal_uLong nUId );
//    ControlItem( const char *Name, const String &URL, const sal_uLong nUId );
	ControlItem( ControlData *pDataP );
	virtual ~ControlItem() {
DBG_DTOR(ControlItem,0);
		delete pData;
	}
	virtual sal_Bool operator < (const ControlItem &rPar)=0;
	virtual sal_Bool operator == (const ControlItem &rPar)=0;
//	void Write( SvStream &aStream );
};

SV_DECL_PTRARR_SORT_DEL(CNames, ControlItem*, 50, 10)

#define MK_SON_ACCESS( ClassName )\
	sal_Bool SonInsert( const ClassName *pNewEntry ) { const ControlItem *pItem = pNewEntry; return pSons->Insert( pItem ); }\
	sal_Bool SonSeek_Entry( const ClassName *pSearchEntry, sal_uInt16 *nRes = NULL) { return pSons->Seek_Entry( pSearchEntry, nRes ); }\
	ClassName* SonGetObject( sal_uInt16 nNr ) { return (ClassName*)pSons->GetObject( nNr ); }

class ControlSon
{
protected:
	CNames *pSons;		// um sicherzustelle, da� nur S�hne des richtien Type reinkommen

public:
	ControlSon() : pSons( NULL ) {};
	~ControlSon();
//	void Write( SvStream &aStream );

	sal_uInt16 Son_Count() { return pSons->Count(); }
	void Sons( CNames *pNewSons ) { pSons = pNewSons; }
	CNames*& GetSons() { return pSons; }
};

class ControlItemSon : public ControlItem, public ControlSon
{
public:
	ControlItemSon(const char *Name, rtl::OString aUIdP )
		: ControlItem( Name, aUIdP ) {}
	ControlItemSon(const String &Name, rtl::OString aUIdP );
//	ControlItemSon(const String &Name, const String &URL, const URLType aType );
//    ControlItemSon(const String &Name, const String &URL, const sal_uLong nUId );
//    ControlItemSon(const char *Name, const String &URL, const sal_uLong nUId );
//	void Write( SvStream &aStream );
};

class ControlDef : public ControlItemSon
{
public:
	ControlDef(const char *Name, rtl::OString aUIdP )
		: ControlItemSon( Name, aUIdP ) {DBG_CTOR(ControlDef,0);}
	ControlDef(const String &Name, rtl::OString aUIdP );
//	ControlDef(const String &Name, const String &URL, const URLType aType );
	ControlDef(const String &aOldName, const String &aNewName, ControlDef *pOriginal, sal_Bool bWithSons = sal_False );
	~ControlDef() {DBG_DTOR(ControlDef,0);}
	virtual sal_Bool operator < (const ControlItem &rPar);
	virtual sal_Bool operator == (const ControlItem &rPar);
	void Write( SvStream &aStream );
	MK_SON_ACCESS( ControlDef )
};

class ControlItemUId : public ControlItem
{
public:
	ControlItemUId(String Name, rtl::OString aUIdP)
        : ControlItem( Name, aUIdP){}
	virtual sal_Bool operator < (const ControlItem &rPar);
	virtual sal_Bool operator == (const ControlItem &rPar);
};

class ControlItemUIdSon : public ControlItemUId, public ControlSon
{
public:
	ControlItemUIdSon(String Name, rtl::OString aUIdP) : ControlItemUId( Name, aUIdP) {}
	MK_SON_ACCESS( ControlItemUId )
};

class ReverseName : public ControlItemUId
{
public:
	sal_uLong LastSequence;

	ReverseName(String Name, rtl::OString aUIdP, sal_uLong nSeq) : ControlItemUId( Name, aUIdP), LastSequence(nSeq) {}
};

class CRevNames: public CNames
{
public:
	void Insert( String aName, rtl::OString aUId, sal_uLong nSeq );
	String GetName( rtl::OString aUId );
	void Invalidate ( sal_uLong nSeq );
};


class SbxTransportMethod: public SbxMethod
{
public:
	SbxTransportMethod( SbxDataType );
	sal_uLong nValue;
	String aUnoSlot;
};
SV_DECL_IMPL_REF(SbxTransportMethod);

class Controls: public SbxObject
{
public:
	Controls( String aCName );
	~Controls();
	void ChangeListener( SbxObject* pParent );

	void SFX_NOTIFY( SfxBroadcaster&, const TypeId&, const SfxHint& rHint, const TypeId& );
	virtual SbxVariable* Find( const String&, SbxClassType );
	SbxTransportMethodRef pMethodVar;	  // zum Transport von Find nach Notify
	static CNames  *pClasses;
	static ControlDefLoad __READONLY_DATA arClasses [];
};
SV_DECL_IMPL_REF(Controls);

typedef std::map< String, String > Environment;

class ImplTestToolObj
{
public:
//	ImplTestToolObj()
//	~ImplTestToolObj()

	String ProgParam;			// Parameter der zu Testenden APP; Gesetzt �ber Start

	DirEntry aFileBase;			// Grundpfad f�r die *.sid und *.win Dateien (Aus Configdatei)
	DirEntry aLogFileBase;		// Grundpfad f�r die *.res Dateien (Aus Configdatei)
	DirEntry aHIDDir;			// Verzeichnis, in dem die hid.lst gesucht wird

	SbxTransportMethodRef pNextReturn;	// Verweis auf die Var, die den Returnwert aufnimmt.

	ControlsRef pControlsObj;			// Jeweiliges Objekt, an dem Methoden aufgerufen weden.

#define VAR_POOL_SIZE 8
	SbxTransportMethodRef pMyVars[VAR_POOL_SIZE];  // Falls in Ausdr�cken mehrere verwendet werden

	Time aServerTimeout;


// Profiling Datenfelder
	sal_uLong LocalStarttime;
	sal_uInt16 nNumBorders;
	sal_uLong naValBorders[4];
	sal_uLong naNumEntries[5];
	sal_uLong naRemoteTime[5];
	sal_uLong naLocalTime[5];

	sal_uLong nMinRemoteCommandDelay;
	sal_uLong nMaxRemoteCommandDelay;
	sal_Bool bDoRemoteCommandDelay;

	sal_Bool bLnaguageExtensionLoaded;		// Wurde �ber 'use' was geladen? F�r syntax highlighting
	SfxBroadcaster *pTTSfxBroadcaster;

	sal_uLong nErrorCount;
	sal_uLong nWarningCount;
	sal_uLong nQAErrorCount;
	sal_uLong nIncludeFileWarningCount;

    SbxDimArrayRef xErrorList;
    SbxDimArrayRef xWarningList;
    SbxDimArrayRef xQAErrorList;
    SbxDimArrayRef xIncludeFileWarningList;

    sal_Bool bIsStart;                      // set tu sal_True while command Start is initiating the communication

    MyBasic* pMyBasic;

    String aTestCaseName;       // holds name of current TestCase
    String aTestCaseFileName;   // holds FileName of current TestCase
    sal_uInt16 nTestCaseLineNr;     // holds Line of current TestCase

    sal_Bool bEnableQaErrors;       // include QA errors in report
    sal_Bool bDebugFindNoErrors;    // suppress generating errors when find of variables is called for variable viewing purposes

    sal_Bool bStopOnSyntaxError;    // catch syntax errors in testcases or not

    Environment *pChildEnv;     // Environment Variables for child Process

    HttpRequest *pHttpRequest;  // used for sending HTTP requests
};


class TTFormat
{
public:
	static String ms2s( sal_uLong nMilliSeconds );
};


#define ADD_TO_LOG( aLogTypep, aMsgp, aFilenamep, nLinep, nCol1p, nCol2p, aRevisionp, pLogList )\
{																		\
	TTLogMsg *pLogMsg = new TTLogMsg();									\
	pLogMsg->aDebugData.aMsg = aMsgp;									\
	pLogMsg->aLogFileName = (pImpl->aLogFileBase + DirEntry(aLogFileName)).GetFull();	\
	pLogMsg->aDebugData.aFilename = aFilenamep;							\
	pLogMsg->aDebugData.nLine = nLinep;									\
	pLogMsg->aDebugData.nCol1 = nCol1p;									\
	pLogMsg->aDebugData.nCol2 = nCol2p;									\
	pLogMsg->aDebugData.aLogType = aLogTypep;							\
	aLogHdl.Call( pLogMsg );											\
    void* pDummyForWarningAboutNULL = pLogList;                         \
    if( pDummyForWarningAboutNULL )														\
    {																	\
        SbxDimArray* pLogArray = (SbxDimArray*)pLogList;				\
        SbxVariable* pLogLine = new SbxVariable( SbxSTRING );				\
        String aCollect;												\
        aCollect.Append( pLogMsg->aDebugData.aFilename );				\
        aCollect.AppendAscii( ";" );									\
        aCollect.Append( String::CreateFromInt32( nLinep ) );			\
        aCollect.AppendAscii( ";" );									\
        aCollect.Append( aRevisionp );				\
        aCollect.AppendAscii( ";" );									\
        aCollect.Append( pLogMsg->aDebugData.aMsg );										\
        pLogLine->PutString( aCollect );									\
        pLogArray->Insert( pLogLine, pLogArray->Count() );	\
    }																	\
    delete pLogMsg;														\
}																		\


#define ADD_RUN_LOG()													\
	ADD_TO_LOG(LOG_RUN, String(), String(), 0, 0, 0, String(), NULL)					\

#define ADD_ERROR_LOG(aMsg, aFilename, nLine, nCol1, nCol2, aRevision)				\
{																		\
	ADD_TO_LOG(LOG_ERROR, aMsg, aFilename, nLine, nCol1, nCol2, aRevision, &pImpl->xErrorList) \
	pImpl->nErrorCount++;												\
}

#define ADD_CALL_STACK_LOG(aMsg, aFilename, nLine, nCol1, nCol2)		\
	ADD_TO_LOG(LOG_CALL_STACK, aMsg, aFilename, nLine, nCol1, nCol2, String(), NULL)	\


#define ADD_AUTO_LOG(aLogType, aMsg, pLogList)							\
{																		\
	if ( BasicRuntimeAccess::HasRuntime() )								\
	{																	\
		BasicRuntime aRun = BasicRuntimeAccess::GetRuntime();			\
		ADD_TO_LOG(aLogType, aMsg, aRun.GetModuleName(SbxNAME_SHORT_TYPES),	\
			aRun.GetLine(), aRun.GetCol1(), aRun.GetCol2(), aRun.GetSourceRevision(), pLogList)	\
	}																	\
	else																\
	{																	\
		ADD_TO_LOG(aLogType, aMsg, UniString(), 0, 0, STRING_LEN, String(), pLogList)	\
	}																	\
}																		\

#define ADD_CASE_LOG(aMsg)												\
	ADD_AUTO_LOG(LOG_TEST_CASE, aMsg, NULL)								\

#define ADD_MESSAGE_LOG(aMsg)											\
	ADD_AUTO_LOG(LOG_MESSAGE, aMsg, NULL)								\

#define ADD_WARNING_LOG(aMsg)											\
{																		\
	ADD_AUTO_LOG(LOG_WARNING, aMsg, &pImpl->xWarningList)				\
	pImpl->nWarningCount++;												\
}

#define ADD_WARNING_LOG2(aMsg, aFilename, nLine )						\
{																		\
	ADD_TO_LOG(LOG_WARNING, aMsg, aFilename, nLine, 0, STRING_LEN, String(), &pImpl->xWarningList) \
	pImpl->nWarningCount++;												\
}

#define ADD_ASSERTION_LOG(aMsg)											\
	ADD_AUTO_LOG(LOG_ASSERTION, aMsg, NULL)								\

#define ADD_QA_ERROR_LOG(aMsg)											\
    if ( pImpl->bEnableQaErrors )                                       \
    {                                                                   \
	    ADD_AUTO_LOG(LOG_QA_ERROR, aMsg, &pImpl->xQAErrorList)			\
    	pImpl->nQAErrorCount++;											\
    }

#endif
