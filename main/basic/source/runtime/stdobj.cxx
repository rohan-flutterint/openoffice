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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basic.hxx"

#include "runtime.hxx"
#include "stdobj.hxx"
#include <basic/sbstdobj.hxx>
#include "rtlproto.hxx"
#include "sbintern.hxx"

// Das nArgs-Feld eines Tabelleneintrags ist wie folgt verschluesselt:
// Zur Zeit wird davon ausgegangen, dass Properties keine Parameter
// benoetigen!

#define _ARGSMASK   0x007F  // Bis zu 127 Argumente
#define _COMPTMASK  0x0080  // Only valid in compatibility mode
#define _RWMASK     0x0F00  // Maske fuer R/W-Bits
#define _TYPEMASK   0xF000  // Maske fuer den Typ des Eintrags

#define _READ       0x0100  // kann gelesen werden
#define _BWRITE     0x0200  // kann as Lvalue verwendet werden
#define _LVALUE     _BWRITE // kann as Lvalue verwendet werden
#define _READWRITE  0x0300  // beides
#define _OPT        0x0400  // Parameter ist optional
#define	_CONST		0x0800	// Property ist const
#define _METHOD     0x3000  // Masken-Bits fuer eine Methode
#define _PROPERTY   0x4000  // Masken-Bit fuer eine Property
#define _OBJECT     0x8000  // Masken-Bit fuer ein Objekt
							// Kombination von oberen Bits:
#define _FUNCTION   0x1100  // Maske fuer Function
#define _LFUNCTION  0x1300  // Maske fuer Function, die auch als Lvalue geht
#define _SUB        0x2100  // Maske fuer Sub
#define _ROPROP     0x4100  // Maske Read Only-Property
#define _WOPROP     0x4200  // Maske Write Only-Property
#define _RWPROP     0x4300  // Maske Read/Write-Property
#define	_CPROP		0x4900	// Maske fuer Konstante

struct Methods {
	const char* pName;      // Name des Eintrags
	SbxDataType eType;      // Datentyp
	short       nArgs;      // Argumente und Flags
	RtlCall     pFunc;      // Function Pointer
	sal_uInt16		nHash;		// Hashcode
};

static Methods aMethods[] = {

{ "AboutStarBasic", SbxNULL,      1 | _FUNCTION, RTLNAME(AboutStarBasic),0  },
  { "Name",			SbxSTRING, 0,NULL,0 },
{ "Abs",            SbxDOUBLE,    1 | _FUNCTION, RTLNAME(Abs),0             },
  { "number",       SbxDOUBLE, 0,NULL,0 },
{ "Array",			SbxOBJECT,        _FUNCTION, RTLNAME(Array),0	 	    },
{ "Asc",            SbxLONG,      1 | _FUNCTION, RTLNAME(Asc),0             },
  { "string",       SbxSTRING, 0,NULL,0 },
{ "AscW",           SbxLONG,      1 | _FUNCTION | _COMPTMASK, RTLNAME(Asc),0},
  { "string",       SbxSTRING, 0,NULL,0 },
{ "Atn",            SbxDOUBLE,    1 | _FUNCTION, RTLNAME(Atn),0             },
  { "number",       SbxDOUBLE, 0,NULL,0 },
{ "ATTR_ARCHIVE",   SbxINTEGER,       _CPROP,    RTLNAME(ATTR_ARCHIVE),0    },
{ "ATTR_DIRECTORY", SbxINTEGER,       _CPROP,    RTLNAME(ATTR_DIRECTORY),0  },
{ "ATTR_HIDDEN",    SbxINTEGER,       _CPROP,    RTLNAME(ATTR_HIDDEN),0     },
{ "ATTR_NORMAL",    SbxINTEGER,       _CPROP,    RTLNAME(ATTR_NORMAL),0     },
{ "ATTR_READONLY",  SbxINTEGER,       _CPROP,    RTLNAME(ATTR_READONLY),0   },
{ "ATTR_SYSTEM",    SbxINTEGER,       _CPROP,    RTLNAME(ATTR_SYSTEM),0     },
{ "ATTR_VOLUME",    SbxINTEGER,       _CPROP,    RTLNAME(ATTR_VOLUME),0     },

{ "Beep",           SbxNULL,          _FUNCTION, RTLNAME(Beep),0            },
{ "Blue",        SbxINTEGER,   1 | _FUNCTION, RTLNAME(Blue),0               },
  { "RGB-Value",     SbxLONG, 0,NULL,0 },

{ "CallByName",     SbxVARIANT,	  3 | _FUNCTION, RTLNAME(CallByName),0 },
  { "Object",		SbxOBJECT,  0,NULL,0 },
  { "ProcedureName",SbxSTRING,  0,NULL,0 },
  { "CallType",     SbxINTEGER, 0,NULL,0 },
{ "CBool",          SbxBOOL,      1 | _FUNCTION, RTLNAME(CBool),0           },
  { "expression",   SbxVARIANT, 0,NULL,0 },
{ "CByte",          SbxBYTE,      1 | _FUNCTION, RTLNAME(CByte),0           },
  { "expression",   SbxVARIANT, 0,NULL,0 },
{ "CCur",           SbxCURRENCY,  1 | _FUNCTION, RTLNAME(CCur),0            },
  { "expression",   SbxVARIANT, 0,NULL,0 },
{ "CDate",          SbxDATE,      1 | _FUNCTION, RTLNAME(CDate),0           },
  { "expression",   SbxVARIANT, 0,NULL,0 },
{ "CDateFromIso",   SbxDATE,      1 | _FUNCTION, RTLNAME(CDateFromIso),0    },
  { "IsoDate",      SbxSTRING, 0,NULL,0 },
{ "CDateToIso",     SbxSTRING,    1 | _FUNCTION, RTLNAME(CDateToIso),0      },
  { "Date",         SbxDATE, 0,NULL,0 },
{ "CDec",           SbxDECIMAL,   1 | _FUNCTION, RTLNAME(CDec),0            },
  { "expression",   SbxVARIANT, 0,NULL,0 },
{ "CDbl",           SbxDOUBLE,    1 | _FUNCTION, RTLNAME(CDbl),0            },
  { "expression",   SbxVARIANT, 0,NULL,0 },
{ "CF_BITMAP",      SbxINTEGER,       _CPROP,    RTLNAME(CF_BITMAP),0       },
{ "CF_METAFILEPICT",SbxINTEGER,       _CPROP,    RTLNAME(CF_METAFILEPICT),0 },
{ "CF_TEXT",		SbxINTEGER,		  _CPROP,    RTLNAME(CF_TEXT),0         },
{ "ChDir",          SbxNULL,      1 | _FUNCTION, RTLNAME(ChDir),0           },
  { "string",       SbxSTRING, 0,NULL,0 },
{ "ChDrive",        SbxNULL,      1 | _FUNCTION, RTLNAME(ChDrive),0         },
  { "string",       SbxSTRING, 0,NULL,0 },

{ "Choose",			SbxVARIANT,   2 | _FUNCTION, RTLNAME(Choose),0			},
  { "Index",   		SbxINTEGER, 0,NULL,0 },
  { "Expression",   SbxVARIANT, 0,NULL,0 },

{ "Chr",            SbxSTRING,    1 | _FUNCTION, RTLNAME(Chr),0             },
  { "string",       SbxINTEGER, 0,NULL,0 },
{ "ChrW",           SbxSTRING,    1 | _FUNCTION | _COMPTMASK, RTLNAME(ChrW),0},
  { "string",       SbxINTEGER, 0,NULL,0 },

{ "CInt",           SbxINTEGER,   1 | _FUNCTION, RTLNAME(CInt),0            },
  { "expression",   SbxVARIANT, 0,NULL,0 },
{ "CLEAR_ALLTABS",  SbxINTEGER,       _CPROP,    RTLNAME(CLEAR_ALLTABS),0   },
{ "CLEAR_TAB",      SbxINTEGER,       _CPROP,    RTLNAME(CLEAR_TAB),0      	},
{ "CLng",           SbxLONG,      1 | _FUNCTION, RTLNAME(CLng),0            },
  { "expression",   SbxVARIANT, 0,NULL,0 },
{ "CompatibilityMode",	SbxBOOL,  1 | _FUNCTION, RTLNAME(CompatibilityMode),0},
  { "bEnable",		SbxBOOL, 0,NULL,0 },
{ "ConvertFromUrl", SbxSTRING,    1 | _FUNCTION, RTLNAME(ConvertFromUrl),0  },
  { "Url",          SbxSTRING, 0,NULL,0 },
{ "ConvertToUrl",   SbxSTRING,    1 | _FUNCTION, RTLNAME(ConvertToUrl),0    },
  { "SystemPath",   SbxSTRING, 0,NULL,0 },
{ "Cos",            SbxDOUBLE,    1 | _FUNCTION, RTLNAME(Cos),0             },
  { "number",       SbxDOUBLE, 0,NULL,0 },
{ "CreateObject",   SbxOBJECT,    1 | _FUNCTION, RTLNAME( CreateObject ),0  },
  { "class",        SbxSTRING, 0,NULL,0 },
{ "CaptureAssertions",  SbxNULL, 1 | _FUNCTION, RTLNAME(CaptureAssertions), 0 },
  { "methodName",   SbxSTRING, 0, NULL, 0 },
{ "CreateUnoListener",SbxOBJECT,   1 | _FUNCTION, RTLNAME( CreateUnoListener ),0 },
  { "prefix",  SbxSTRING, 0,NULL,0 },
  { "typename",  SbxSTRING, 0,NULL,0 },
{ "CreateUnoDialog",SbxOBJECT,    2 | _FUNCTION, RTLNAME( CreateUnoDialog ),0 },
  { "dialoglibrary",SbxOBJECT, 0,NULL,0 },
  { "dialogname",   SbxSTRING, 0,NULL,0 },
{ "CreateUnoService",SbxOBJECT,   1 | _FUNCTION, RTLNAME( CreateUnoService ),0 },
  { "servicename",  SbxSTRING, 0,NULL,0 },
{ "CreateUnoServiceWithArguments",SbxOBJECT,   2 | _FUNCTION, RTLNAME( CreateUnoServiceWithArguments ),0 },
  { "servicename",  SbxSTRING, 0,NULL,0 },
  { "arguments",  SbxARRAY, 0,NULL,0 },
{ "CreateUnoStruct",SbxOBJECT,    1 | _FUNCTION, RTLNAME( CreateUnoStruct ),0 },
  { "classname",    SbxSTRING, 0,NULL,0 },
{ "CreateUnoValue", SbxOBJECT,    2 | _FUNCTION, RTLNAME( CreateUnoValue ),0 },
  { "type",         SbxSTRING, 0,NULL,0 },
  { "value",        SbxVARIANT, 0,NULL,0 },
{ "CreatePropertySet",SbxOBJECT, 1 | _FUNCTION, RTLNAME( CreatePropertySet ),0 },
  { "values",  SbxARRAY, 0,NULL,0 },
{ "CSng",           SbxSINGLE,    1 | _FUNCTION, RTLNAME(CSng),0            },
  { "expression",   SbxVARIANT, 0,NULL,0 },
{ "CStr",           SbxSTRING,    1 | _FUNCTION, RTLNAME(CStr),0            },
  { "expression",   SbxVARIANT, 0,NULL,0 },
{ "CurDir",         SbxSTRING,    1 | _FUNCTION, RTLNAME(CurDir),0          },
  { "string",       SbxSTRING, 0,NULL,0 },
{ "CVar",           SbxVARIANT,   1 | _FUNCTION, RTLNAME(CVar),0            },
  { "expression",   SbxVARIANT, 0,NULL,0 },
{ "CVErr",          SbxVARIANT,   1 | _FUNCTION, RTLNAME(CVErr),0           },
  { "expression",   SbxVARIANT, 0,NULL,0 },

{ "Date",           SbxDATE,          _LFUNCTION,RTLNAME(Date),0            },
{ "DateAdd",        SbxDATE,	  3 | _FUNCTION, RTLNAME(DateAdd),0         },
  { "Interval",     SbxSTRING, 0,NULL,0 },
  { "Number",     	SbxLONG, 0,NULL,0 },
  { "Date",			SbxDATE, 0,NULL,0 },
{ "DateDiff",       SbxDOUBLE,    5 | _FUNCTION, RTLNAME(DateDiff),0        },
  { "Interval",     SbxSTRING, 0,NULL,0 },
  { "Date1",     	SbxDATE, 0,NULL,0 },
  { "Date2",		SbxDATE, 0,NULL,0 },
  { "Firstdayofweek" , SbxINTEGER, _OPT,NULL,0 },
  { "Firstweekofyear", SbxINTEGER, _OPT,NULL,0 },
{ "DatePart",       SbxLONG,      4 | _FUNCTION, RTLNAME(DatePart),0        },
  { "Interval",     SbxSTRING, 0,NULL,0 },
  { "Date",     	SbxDATE, 0,NULL,0 },
  { "Firstdayofweek" , SbxINTEGER, _OPT, NULL,0 },
  { "Firstweekofyear", SbxINTEGER, _OPT, NULL,0 },
{ "DateSerial",     SbxDATE,      3 | _FUNCTION, RTLNAME(DateSerial),0      },
  { "Year",         SbxINTEGER, 0,NULL,0 },
  { "Month",        SbxINTEGER, 0,NULL,0 },
  { "Day",          SbxINTEGER, 0,NULL,0 },
{ "DateValue",      SbxDATE,      1 | _FUNCTION, RTLNAME(DateValue),0       },
  { "String",       SbxSTRING, 0,NULL,0 },
{ "Day",            SbxINTEGER,   1 | _FUNCTION, RTLNAME(Day),0             },
  { "Date",         SbxDATE, 0,NULL,0 },
{ "Ddeexecute",     SbxNULL,   2 | _FUNCTION, RTLNAME(DDEExecute),0         },
  { "Channel",         SbxLONG, 0,NULL,0 },
  { "Command",         SbxSTRING, 0,NULL,0 },
{ "Ddeinitiate",    SbxINTEGER,   2 | _FUNCTION, RTLNAME(DDEInitiate),0     },
  { "Application",     SbxSTRING, 0,NULL,0 },
  { "Topic",           SbxSTRING, 0,NULL,0 },
{ "Ddepoke",    	SbxNULL,   3 | _FUNCTION, RTLNAME(DDEPoke),0            },
  { "Channel",         SbxLONG, 0,NULL,0 },
  { "Item",     	   SbxSTRING, 0,NULL,0 },
  { "Data",            SbxSTRING, 0,NULL,0 },
{ "Dderequest",     SbxSTRING,   2 | _FUNCTION, RTLNAME(DDERequest),0       },
  { "Channel",         SbxLONG, 0,NULL,0 },
  { "Item",            SbxSTRING, 0,NULL,0 },
{ "Ddeterminate",   SbxNULL,   1 | _FUNCTION, RTLNAME(DDETerminate),0       },
  { "Channel",         SbxLONG, 0,NULL,0 },
{ "Ddeterminateall",   SbxNULL,   _FUNCTION, RTLNAME(DDETerminateAll),0     },
{ "DimArray",		SbxOBJECT,    _FUNCTION, RTLNAME(DimArray),0			},
{ "Dir",            SbxSTRING,    2 | _FUNCTION, RTLNAME(Dir),0             },
  { "FileSpec",     SbxSTRING,        _OPT, NULL,0 },
  { "attrmask",     SbxINTEGER,       _OPT, NULL,0 },
{ "DoEvents",		SbxEMPTY,     _FUNCTION, RTLNAME(DoEvents),0			},
{ "DumpAllObjects", SbxEMPTY,     2 | _SUB, RTLNAME(DumpAllObjects),0       },
  { "FileSpec",     SbxSTRING, 0,NULL,0 },
  { "DumpAll",      SbxINTEGER,		  _OPT, NULL,0 },

{ "Empty",        SbxVARIANT,        _CPROP,    RTLNAME(Empty),0         },
{ "EqualUnoObjects",SbxBOOL,      2 | _FUNCTION, RTLNAME(EqualUnoObjects),0 },
  { "Variant",      SbxVARIANT, 0,NULL,0 },
  { "Variant",      SbxVARIANT, 0,NULL,0 },
{ "EnableReschedule",	SbxNULL,  1 | _FUNCTION, RTLNAME(EnableReschedule),0},
  { "bEnable",		SbxBOOL, 0,NULL,0 },
{ "Environ",            SbxSTRING,   1 | _FUNCTION, RTLNAME(Environ),0      },
  { "Environmentstring",SbxSTRING, 0,NULL,0 },
{ "EOF",            SbxBOOL,      1 | _FUNCTION, RTLNAME(EOF),0             },
  { "Channel",      SbxINTEGER, 0,NULL,0 },
{ "Erl",            SbxLONG,          _ROPROP,   RTLNAME( Erl ),0           },
{ "Err",            SbxVARIANT,       _RWPROP,   RTLNAME( Err ),0           },
{ "Error",          SbxSTRING,    1 | _FUNCTION, RTLNAME( Error ),0         },
  { "code",         SbxLONG, 0,NULL,0 },
{ "Exp",            SbxDOUBLE,    1 | _FUNCTION, RTLNAME(Exp),0             },
  { "number",       SbxDOUBLE, 0,NULL,0 },

{ "False",          SbxBOOL,          _CPROP,    RTLNAME(False),0           },
{ "FileAttr",       SbxINTEGER,   2 | _FUNCTION, RTLNAME(FileAttr),0        },
  { "Channel",      SbxINTEGER, 0,NULL,0 },
  { "Attributes",   SbxINTEGER, 0,NULL,0 },
{ "FileCopy",       SbxNULL,      2 | _FUNCTION, RTLNAME(FileCopy),0        },
  { "Source",       SbxSTRING, 0,NULL,0 },
  { "Destination",  SbxSTRING, 0,NULL,0 },
{ "FileDateTime",   SbxSTRING,    1 | _FUNCTION, RTLNAME(FileDateTime),0    },
  { "filename",     SbxSTRING, 0,NULL,0 },
{ "FileExists",     SbxBOOL,   1 | _FUNCTION, RTLNAME(FileExists),0         },
  { "filename",     SbxSTRING, 0,NULL,0 },
{ "FileLen",        SbxLONG,      1 | _FUNCTION, RTLNAME(FileLen),0         },
  { "filename",     SbxSTRING, 0,NULL,0 },
{ "FindObject",		SbxOBJECT,	  1 | _FUNCTION, RTLNAME(FindObject),0      },
  { "Name",  SbxSTRING, 0,NULL,0 },
{ "FindPropertyObject",   SbxOBJECT,	  2 | _FUNCTION, RTLNAME(FindPropertyObject),0 },
  { "Object",		SbxOBJECT, 0,NULL,0 },
  { "Name",  SbxSTRING, 0,NULL,0 },
{ "Fix",            SbxDOUBLE,    1 | _FUNCTION, RTLNAME(Fix),0             },
  { "number",       SbxDOUBLE, 0,NULL,0 },
{ "Format",         SbxSTRING,    2 | _FUNCTION, RTLNAME(Format),0          },
  { "expression",   SbxVARIANT, 0,NULL,0 },
  { "format",       SbxSTRING,        _OPT, NULL,0 },
{ "FormatDateTime", SbxSTRING,    2 | _FUNCTION | _COMPTMASK, RTLNAME(FormatDateTime),0 },
  { "Date",         SbxDATE, 0,NULL,0 },
  { "NamedFormat",  SbxINTEGER,        _OPT, NULL,0 },
{ "FRAMEANCHORCHAR",   		SbxINTEGER,       _CPROP,    RTLNAME(FRAMEANCHORCHAR),0 },
{ "FRAMEANCHORPAGE",   		SbxINTEGER,       _CPROP,    RTLNAME(FRAMEANCHORPAGE),0 },
{ "FRAMEANCHORPARA",   		SbxINTEGER,       _CPROP,    RTLNAME(FRAMEANCHORPARA),0 },
{ "FreeFile",       SbxINTEGER,    _FUNCTION, RTLNAME(FreeFile),0           },
{ "FreeLibrary",    SbxNULL,      1 | _FUNCTION, RTLNAME(FreeLibrary),0     },
  { "Modulename",   SbxSTRING, 0,NULL,0 },

{ "Get",            SbxNULL,   3 | _FUNCTION, RTLNAME(Get),0                },
  { "filenumber",   SbxINTEGER, 0,NULL,0 },
  { "recordnumber", SbxLONG, 0,NULL,0 },
  { "variablename",	SbxVARIANT, 0,NULL,0 },
{ "GetAttr",        SbxINTEGER,   1 | _FUNCTION, RTLNAME(GetAttr),0         },
  { "filename",     SbxSTRING, 0,NULL,0 },
{ "GetDefaultContext", SbxOBJECT, 0 | _FUNCTION, RTLNAME(GetDefaultContext),0 },
{ "GetDialogZoomFactorX",  SbxDOUBLE,     _FUNCTION,RTLNAME(GetDialogZoomFactorX),0 },
{ "GetDialogZoomFactorY",  SbxDOUBLE,     _FUNCTION,RTLNAME(GetDialogZoomFactorY),0 },
{ "GetGUIType",  SbxINTEGER,     _FUNCTION,RTLNAME(GetGUIType),0            },
{ "GetGUIVersion",  SbxLONG,     _FUNCTION,RTLNAME(GetGUIVersion),0         },
{ "GetPathSeparator",  SbxSTRING,     _FUNCTION,RTLNAME(GetPathSeparator),0 },
{ "GetProcessServiceManager", SbxOBJECT, 0 | _FUNCTION, RTLNAME(GetProcessServiceManager),0 },
{ "GetSolarVersion",  SbxLONG,     _FUNCTION,RTLNAME(GetSolarVersion),0     },
{ "GetSystemTicks",  SbxLONG,      _FUNCTION,RTLNAME(GetSystemTicks),0      },
{ "GetSystemType",  SbxINTEGER,    _FUNCTION,RTLNAME(GetSystemType),0       },
{ "GlobalScope",    SbxOBJECT,     _FUNCTION,RTLNAME(GlobalScope),0         },
{ "Green",          SbxINTEGER,   1 | _FUNCTION, RTLNAME(Green),0           },
  { "RGB-Value",     SbxLONG, 0,NULL,0 },

{ "HasUnoInterfaces",	SbxBOOL,  1 | _FUNCTION, RTLNAME(HasUnoInterfaces),0},
  { "InterfaceName",SbxSTRING, 0,NULL,0 },
{ "Hex",            SbxSTRING,    1 | _FUNCTION, RTLNAME(Hex),0             },
  { "number",       SbxLONG, 0,NULL,0 },
{ "Hour",           SbxINTEGER,   1 | _FUNCTION, RTLNAME(Hour),0            },
  { "Date",         SbxDATE, 0,NULL,0 },

{ "IDABORT",   		SbxINTEGER,       _CPROP,    RTLNAME(IDABORT),0      	},
{ "IDCANCEL",   	SbxINTEGER,       _CPROP,    RTLNAME(IDCANCEL),0      	},
{ "IDNO",   		SbxINTEGER,       _CPROP,    RTLNAME(IDNO),0      		},
{ "IDOK",   		SbxINTEGER,       _CPROP,    RTLNAME(IDOK),0      		},
{ "IDRETRY",   		SbxINTEGER,       _CPROP,    RTLNAME(IDRETRY),0      	},
{ "IDYES",   		SbxINTEGER,       _CPROP,    RTLNAME(IDYES),0      		},

{ "Iif",            SbxVARIANT,   3 | _FUNCTION, RTLNAME(Iif),0             },
  { "Bool",      	SbxBOOL, 0,NULL,0 },
  { "Variant1",     SbxVARIANT, 0,NULL,0 },
  { "Variant2",     SbxVARIANT, 0,NULL,0 },

{ "Input",			SbxSTRING,    2 | _FUNCTION | _COMPTMASK, RTLNAME(Input),0},
  { "Number",       SbxLONG, 0,NULL,0 },
  { "FileNumber",   SbxLONG, 0,NULL,0 },
{ "InputBox",       SbxSTRING,    5 | _FUNCTION, RTLNAME(InputBox),0        },
  { "Prompt",       SbxSTRING, 0,NULL,0 },
  { "Title",        SbxSTRING,        _OPT, NULL,0 },
  { "Default",      SbxSTRING,        _OPT, NULL,0 },
  { "XPosTwips",    SbxLONG,          _OPT, NULL,0 },
  { "YPosTwips",    SbxLONG,          _OPT, NULL,0 },
{ "InStr",          SbxLONG,      4 | _FUNCTION, RTLNAME(InStr),0           },
  { "Start",        SbxSTRING,        _OPT, NULL,0 },
  { "String1",      SbxSTRING, 0,NULL,0 },
  { "String2",      SbxSTRING, 0,NULL,0 },
  { "Compare",      SbxINTEGER,       _OPT, NULL,0 },
{ "InStrRev",       SbxLONG,      4 | _FUNCTION | _COMPTMASK, RTLNAME(InStrRev),0},
  { "String1",      SbxSTRING, 0,NULL,0 },
  { "String2",      SbxSTRING, 0,NULL,0 },
  { "Start",        SbxSTRING,        _OPT, NULL,0 },
  { "Compare",      SbxINTEGER,       _OPT, NULL,0 },
{ "Int",            SbxDOUBLE,    1 | _FUNCTION, RTLNAME(Int),0             },
  { "number",       SbxDOUBLE, 0,NULL,0 },
{ "IsArray",        SbxBOOL,      1 | _FUNCTION, RTLNAME(IsArray),0         },
  { "Variant",      SbxVARIANT, 0,NULL,0 },
{ "IsDate",         SbxBOOL,      1 | _FUNCTION, RTLNAME(IsDate),0          },
  { "Variant",      SbxVARIANT, 0,NULL,0 },
{ "IsEmpty",        SbxBOOL,      1 | _FUNCTION, RTLNAME(IsEmpty),0         },
  { "Variant",      SbxVARIANT, 0,NULL,0 },
{ "IsError",        SbxBOOL,      1 | _FUNCTION, RTLNAME(IsError),0         },
  { "Variant",      SbxVARIANT, 0,NULL,0 },
{ "IsMissing",        SbxBOOL,      1 | _FUNCTION, RTLNAME(IsMissing),0     },
  { "Variant",      SbxVARIANT, 0,NULL,0 },
{ "IsNull",         SbxBOOL,      1 | _FUNCTION, RTLNAME(IsNull),0          },
  { "Variant",      SbxVARIANT, 0,NULL,0 },
{ "IsNumeric",      SbxBOOL,      1 | _FUNCTION, RTLNAME(IsNumeric),0       },
  { "Variant",      SbxVARIANT, 0,NULL,0 },
{ "IsObject",       SbxBOOL,      1 | _FUNCTION, RTLNAME(IsObject),0        },
  { "Variant",      SbxVARIANT, 0,NULL,0 },
{ "IsUnoStruct",    SbxBOOL,      1 | _FUNCTION, RTLNAME(IsUnoStruct),0     },
  { "Variant",      SbxVARIANT, 0,NULL,0 },
{ "Join",         	SbxSTRING,      2 | _FUNCTION, RTLNAME(Join),0          },
  { "list",			SbxOBJECT, 0,NULL,0 },
  { "delimiter",    SbxSTRING, 0,NULL,0 },
{ "Kill",           SbxNULL,      1 | _FUNCTION, RTLNAME(Kill),0            },
  { "filespec",     SbxSTRING, 0,NULL,0 },
{ "LBound",       	SbxLONG,      1 | _FUNCTION, RTLNAME(LBound),0          },
  { "Variant",      SbxVARIANT, 0,NULL,0 },
{ "LCase",          SbxSTRING,    1 | _FUNCTION, RTLNAME(LCase),0           },
  { "string",       SbxSTRING, 0,NULL,0 },
{ "Left",           SbxSTRING,    2 | _FUNCTION, RTLNAME(Left),0            },
  { "String",       SbxSTRING, 0,NULL,0 },
  { "Count",        SbxLONG, 0,NULL,0 },
{ "Len",            SbxLONG,      1 | _FUNCTION, RTLNAME(Len),0             },
  { "StringOrVariant", SbxVARIANT, 0,NULL,0 },
{ "LenB",           SbxLONG,      1 | _FUNCTION, RTLNAME(Len),0             },
  { "StringOrVariant", SbxVARIANT, 0,NULL,0 },
{ "Load",           SbxNULL,      1 | _FUNCTION, RTLNAME(Load),0            },
  { "object",       SbxOBJECT, 0,NULL,0 },
{ "LoadPicture",    SbxOBJECT,    1 | _FUNCTION, RTLNAME(LoadPicture),0     },
  { "string",       SbxSTRING, 0,NULL,0 },
{ "Loc",            SbxLONG,      1 | _FUNCTION, RTLNAME(Loc),0             },
  { "Channel",      SbxINTEGER, 0,NULL,0 },
{ "Lof",            SbxLONG,      1 | _FUNCTION, RTLNAME(Lof),0             },
  { "Channel",      SbxINTEGER, 0,NULL,0 },
{ "Log",            SbxDOUBLE,    1 | _FUNCTION, RTLNAME(Log),0             },
  { "number",       SbxDOUBLE, 0,NULL,0 },
{ "LTrim",          SbxSTRING,    1 | _FUNCTION, RTLNAME(LTrim),0           },
  { "string",       SbxSTRING, 0,NULL,0 },

{ "MB_ABORTRETRYIGNORE", SbxINTEGER,  _CPROP,    RTLNAME(MB_ABORTRETRYIGNORE),0},
{ "MB_APPLMODAL",   SbxINTEGER,       _CPROP,    RTLNAME(MB_APPLMODAL),0    },
{ "MB_DEFBUTTON1",  SbxINTEGER,       _CPROP,    RTLNAME(MB_DEFBUTTON1),0   },
{ "MB_DEFBUTTON2",  SbxINTEGER,       _CPROP,    RTLNAME(MB_DEFBUTTON2),0   },
{ "MB_DEFBUTTON3",  SbxINTEGER,       _CPROP,    RTLNAME(MB_DEFBUTTON3),0   },
{ "MB_ICONEXCLAMATION", SbxINTEGER,   _CPROP,    RTLNAME(MB_ICONEXCLAMATION),0},
{ "MB_ICONINFORMATION", SbxINTEGER,   _CPROP,    RTLNAME(MB_ICONINFORMATION),0},
{ "MB_ICONQUESTION",SbxINTEGER,       _CPROP,    RTLNAME(MB_ICONQUESTION),0 },
{ "MB_ICONSTOP",   	SbxINTEGER,       _CPROP,    RTLNAME(MB_ICONSTOP),0     },
{ "MB_OK",   		SbxINTEGER,       _CPROP,    RTLNAME(MB_OK),0      		},
{ "MB_OKCANCEL",   	SbxINTEGER,       _CPROP,    RTLNAME(MB_OKCANCEL),0     },
{ "MB_RETRYCANCEL", SbxINTEGER,       _CPROP,    RTLNAME(MB_RETRYCANCEL),0  },
{ "MB_SYSTEMMODAL", SbxINTEGER,       _CPROP,    RTLNAME(MB_SYSTEMMODAL),0  },
{ "MB_YESNO",   	SbxINTEGER,       _CPROP,    RTLNAME(MB_YESNO),0      	},
{ "MB_YESNOCANCEL", SbxINTEGER,       _CPROP,    RTLNAME(MB_YESNOCANCEL),0  },

{ "Me",		        SbxOBJECT,    0 | _FUNCTION | _COMPTMASK, RTLNAME(Me),0 },
{ "Mid",            SbxSTRING,    3 | _LFUNCTION,RTLNAME(Mid),0             },
  { "String",       SbxSTRING, 0,NULL,0 },
  { "StartPos",     SbxLONG, 0,NULL,0 },
  { "Length",       SbxLONG,          _OPT, NULL,0 },
{ "Minute",         SbxINTEGER,   1 | _FUNCTION, RTLNAME(Minute),0          },
  { "Date",         SbxDATE, 0,NULL,0 },
{ "MkDir",          SbxNULL,      1 | _FUNCTION, RTLNAME(MkDir),0           },
  { "pathname",     SbxSTRING, 0,NULL,0 },
{ "Month",          SbxINTEGER,   1 | _FUNCTION, RTLNAME(Month),0           },
  { "Date",         SbxDATE, 0,NULL,0 },
{ "MonthName",      SbxSTRING,    2 | _FUNCTION | _COMPTMASK, RTLNAME(MonthName),0 },
  { "Month",        SbxINTEGER, 0,NULL,0 },
  { "Abbreviate",   SbxBOOL,          _OPT, NULL,0 },
{ "MsgBox",         SbxINTEGER,    5 | _FUNCTION, RTLNAME(MsgBox),0         },
  { "Prompt",       SbxSTRING, 0,NULL,0 },
  { "Buttons",      SbxINTEGER,       _OPT, NULL,0 },
  { "Title",        SbxSTRING,        _OPT, NULL,0 },
  { "Helpfile",     SbxSTRING,        _OPT, NULL,0 },
  { "Context",      SbxINTEGER,       _OPT, NULL,0 },

{ "Nothing",        SbxOBJECT,        _CPROP,    RTLNAME(Nothing),0         },
{ "Now",            SbxDATE,          _FUNCTION, RTLNAME(Now),0             },
{ "Null",			SbxNULL,          _CPROP,    RTLNAME(Null),0			},

{ "Oct",            SbxSTRING,    1 | _FUNCTION, RTLNAME(Oct),0             },
  { "number",       SbxLONG, 0,NULL,0 },

{ "Partition",      SbxSTRING,    4 | _FUNCTION, RTLNAME(Partition),0       },
  { "number",   	SbxLONG, 	0,NULL,0 },
  { "start",     	SbxLONG, 	0,NULL,0 },
  { "stop",  	    SbxLONG, 	0,NULL,0 },
  { "interval",     SbxLONG, 	0,NULL,0 },
{ "Pi",				SbxDOUBLE,		  _CPROP,    RTLNAME(PI),0		      	},
{ "Put",            SbxNULL,   3 | _FUNCTION, RTLNAME(Put),0                },
  { "filenumber",   SbxINTEGER, 0,NULL,0 },
  { "recordnumber", SbxLONG, 0,NULL,0 },
  { "variablename",	SbxVARIANT, 0,NULL,0 },

{ "QBColor",		SbxLONG,	  1 | _FUNCTION, RTLNAME(QBColor),0			},
  { "number",		SbxINTEGER, 0,NULL,0 },

{ "Randomize",      SbxNULL,      1 | _FUNCTION, RTLNAME(Randomize),0       },
  { "Number",       SbxDOUBLE,        _OPT, NULL,0 },
{ "Red",        SbxINTEGER,   1 | _FUNCTION, RTLNAME(Red),0                 },
  { "RGB-Value",     SbxLONG, 0,NULL,0 },
{ "Reset",          SbxNULL,      0 | _FUNCTION, RTLNAME(Reset),0           },
{ "ResolvePath",    SbxSTRING,    1 | _FUNCTION, RTLNAME(ResolvePath),0     },
  { "Path",         SbxSTRING, 0,NULL,0 },
{ "RGB",            SbxLONG,      3 | _FUNCTION, RTLNAME(RGB),0             },
  { "Red",       	SbxINTEGER, 0,NULL,0 },
  { "Green",       	SbxINTEGER, 0,NULL,0 },
  { "Blue",       	SbxINTEGER, 0,NULL,0 },
{ "Replace",        SbxSTRING,    6 | _FUNCTION, RTLNAME(Replace),0         },
  { "Expression",   SbxSTRING, 0,NULL,0 },
  { "Find",     	SbxSTRING, 0,NULL,0 },
  { "Replace",      SbxSTRING, 0,NULL,0 },
  { "Start",        SbxINTEGER, 	_OPT, NULL,0 },
  { "Count",        SbxINTEGER,		_OPT, NULL,0 },
  { "Compare",      SbxINTEGER,		_OPT, NULL,0 },
{ "Right",          SbxSTRING,    2 | _FUNCTION, RTLNAME(Right),0           },
  { "String",       SbxSTRING, 0,NULL,0 },
  { "Count",        SbxLONG, 0,NULL,0 },
{ "RmDir",          SbxNULL,      1 | _FUNCTION, RTLNAME(RmDir),0           },
  { "pathname",     SbxSTRING, 0,NULL,0 },
{ "Round",          SbxDOUBLE,    2 | _FUNCTION | _COMPTMASK, RTLNAME(Round),0},
  { "Expression",   SbxDOUBLE, 0,NULL,0 },
  { "Numdecimalplaces", SbxINTEGER,   _OPT, NULL,0 },
{ "Rnd",            SbxDOUBLE,    1 | _FUNCTION, RTLNAME(Rnd),0             },
  { "Number",       SbxDOUBLE,        _OPT, NULL,0 },
{ "RTL",			SbxOBJECT,    0 | _FUNCTION | _COMPTMASK, RTLNAME(RTL),0},
{ "RTrim",          SbxSTRING,    1 | _FUNCTION, RTLNAME(RTrim),0           },
  { "string",       SbxSTRING, 0,NULL,0 },

{ "SavePicture",    SbxNULL,      2 | _FUNCTION, RTLNAME(SavePicture),0     },
  { "object",       SbxOBJECT, 0,NULL,0 },
  { "string",       SbxSTRING, 0,NULL,0 },
{ "Second",         SbxINTEGER,   1 | _FUNCTION, RTLNAME(Second),0          },
  { "Date",         SbxDATE, 0,NULL,0 },
{ "Seek",           SbxLONG,      1 | _FUNCTION, RTLNAME(Seek),0            },
  { "Channel",      SbxINTEGER, 0,NULL,0 },
{ "SendKeys",       SbxNULL,      2 | _FUNCTION, RTLNAME(SendKeys),0        },
  { "String",       SbxSTRING, 0,NULL,0 },
  { "Wait",         SbxBOOL,          _OPT, NULL,0 },
{ "SetAttr",        SbxNULL,      2 | _FUNCTION, RTLNAME(SetAttr),0         },
  { "File"  ,       SbxSTRING, 0,NULL,0 },
  { "Attributes",   SbxINTEGER, 0,NULL,0 },
{ "SET_OFF",   		SbxINTEGER,       _CPROP,    RTLNAME(SET_OFF),0      	},
{ "SET_ON",   		SbxINTEGER,       _CPROP,    RTLNAME(SET_ON),0      	},
{ "SET_TAB",   		SbxINTEGER,       _CPROP,    RTLNAME(SET_TAB),0      	},
{ "Sgn",            SbxINTEGER,   1 | _FUNCTION, RTLNAME(Sgn),0             },
  { "number",       SbxDOUBLE, 0,NULL,0 },
{ "Shell",          SbxLONG,      2 | _FUNCTION, RTLNAME(Shell),0           },
  { "Commandstring",SbxSTRING, 0,NULL,0 },
  { "WindowStyle",  SbxINTEGER,       _OPT, NULL,0 },
{ "Sin",            SbxDOUBLE,    1 | _FUNCTION, RTLNAME(Sin),0             },
  { "number",       SbxDOUBLE, 0,NULL,0 },
{ "Space",          SbxSTRING,      1 | _FUNCTION, RTLNAME(Space),0         },
  { "string",       SbxLONG, 0,NULL,0 },
{ "Spc",          	SbxSTRING,      1 | _FUNCTION, RTLNAME(Spc),0           },
  { "Count",        SbxLONG, 0,NULL,0 },
{ "Split",         	SbxOBJECT,      3 | _FUNCTION, RTLNAME(Split),0         },
  { "expression",	SbxSTRING, 0,NULL,0 },
  { "delimiter",    SbxSTRING, 0,NULL,0 },
  { "count",	    SbxLONG, 0,NULL,0 },
{ "Sqr",            SbxDOUBLE,    1 | _FUNCTION, RTLNAME(Sqr),0             },
  { "number",       SbxDOUBLE, 0,NULL,0 },
{ "Str",            SbxSTRING,    1 | _FUNCTION, RTLNAME(Str),0             },
  { "number",       SbxDOUBLE, 0,NULL,0 },
{ "StrComp",        SbxINTEGER,   3 | _FUNCTION, RTLNAME(StrComp),0         },
  { "String1",      SbxSTRING, 0,NULL,0 },
  { "String2",      SbxSTRING, 0,NULL,0 },
  { "Compare",      SbxINTEGER,       _OPT, NULL,0 },
{ "StrConv",        SbxOBJECT,   3 | _FUNCTION, RTLNAME(StrConv),0          },
  { "String",       SbxSTRING, 0,NULL,0 },
  { "Conversion",   SbxSTRING, 0,NULL,0 },
  { "LCID",			SbxINTEGER, _OPT,NULL,0 },
{ "String",         SbxSTRING,    2 | _FUNCTION, RTLNAME(String),0          },
  { "Count",        SbxLONG, 0,NULL,0 },
  { "Filler",       SbxVARIANT, 0,NULL,0 },
{ "StrReverse",     SbxSTRING,    1 | _FUNCTION | _COMPTMASK, RTLNAME(StrReverse),0 },
  { "String1",      SbxSTRING, 0,NULL,0 },
{ "Switch",			SbxVARIANT,   2 | _FUNCTION, RTLNAME(Switch),0			},
  { "Expression",   SbxVARIANT, 0,NULL,0 },
  { "Value",        SbxVARIANT, 0,NULL,0 },

{ "Tan",            SbxDOUBLE,    1 | _FUNCTION, RTLNAME(Tan),0             },
  { "number",       SbxDOUBLE, 0,NULL,0 },
{ "Time",           SbxVARIANT,       _LFUNCTION,RTLNAME(Time),0            },
{ "Timer",          SbxDATE,          _FUNCTION, RTLNAME(Timer),0           },
{ "TimeSerial",     SbxDATE,      3 | _FUNCTION, RTLNAME(TimeSerial),0      },
  { "Hour",         SbxLONG, 0,NULL,0 },
  { "Minute",       SbxLONG, 0,NULL,0 },
  { "Second",       SbxLONG, 0,NULL,0 },
{ "TimeValue",      SbxDATE,      1 | _FUNCTION, RTLNAME(TimeValue),0       },
  { "String",       SbxSTRING, 0,NULL,0 },
{ "TOGGLE",   		SbxINTEGER,       _CPROP,    RTLNAME(TOGGLE),0      	},
#ifdef DBG_TRACE_BASIC
{ "TraceCommand",   SbxNULL,	  1 | _FUNCTION, RTLNAME(TraceCommand),0 },
  { "Command",		SbxSTRING,  0,NULL,0 },
#endif
{ "Trim",           SbxSTRING,    1 | _FUNCTION, RTLNAME(Trim),0            },
  { "String",       SbxSTRING, 0,NULL,0 },
{ "True",           SbxBOOL,          _CPROP,    RTLNAME(True),0            },
{ "TwipsPerPixelX", SbxLONG,          _FUNCTION, RTLNAME(TwipsPerPixelX),0  },
{ "TwipsPerPixelY", SbxLONG,          _FUNCTION, RTLNAME(TwipsPerPixelY),0  },

{ "TYP_AUTHORFLD",   		SbxINTEGER,       _CPROP,    RTLNAME(TYP_AUTHORFLD),0      	},
{ "TYP_CHAPTERFLD",   		SbxINTEGER,       _CPROP,    RTLNAME(TYP_CHAPTERFLD),0      },
{ "TYP_CONDTXTFLD",   		SbxINTEGER,       _CPROP,    RTLNAME(TYP_CONDTXTFLD),0      },
{ "TYP_DATEFLD",   			SbxINTEGER,       _CPROP,    RTLNAME(TYP_DATEFLD),0      	},
{ "TYP_DBFLD",   			SbxINTEGER,       _CPROP,    RTLNAME(TYP_DBFLD),0      		},
{ "TYP_DBNAMEFLD",   		SbxINTEGER,       _CPROP,    RTLNAME(TYP_DBNAMEFLD),0      	},
{ "TYP_DBNEXTSETFLD",		SbxINTEGER,       _CPROP,    RTLNAME(TYP_DBNEXTSETFLD),0	},
{ "TYP_DBNUMSETFLD",   		SbxINTEGER,       _CPROP,    RTLNAME(TYP_DBNUMSETFLD),0     },
{ "TYP_DBSETNUMBERFLD",   	SbxINTEGER,       _CPROP,    RTLNAME(TYP_DBSETNUMBERFLD),0  },
{ "TYP_DDEFLD",   			SbxINTEGER,       _CPROP,    RTLNAME(TYP_DDEFLD),0      	},
{ "TYP_DOCINFOFLD",   		SbxINTEGER,       _CPROP,    RTLNAME(TYP_DOCINFOFLD),0      },
{ "TYP_DOCSTATFLD",   		SbxINTEGER,       _CPROP,    RTLNAME(TYP_DOCSTATFLD),0      },
{ "TYP_EXTUSERFLD",   		SbxINTEGER,       _CPROP,    RTLNAME(TYP_EXTUSERFLD),0      },
{ "TYP_FILENAMEFLD",   		SbxINTEGER,       _CPROP,    RTLNAME(TYP_FILENAMEFLD),0     },
{ "TYP_FIXDATEFLD",   		SbxINTEGER,       _CPROP,    RTLNAME(TYP_FIXDATEFLD),0      },
{ "TYP_FIXTIMEFLD",   		SbxINTEGER,       _CPROP,    RTLNAME(TYP_FIXTIMEFLD),0      },
{ "TYP_FORMELFLD",   		SbxINTEGER,       _CPROP,    RTLNAME(TYP_FORMELFLD),0      	},
{ "TYP_GETFLD",   			SbxINTEGER,       _CPROP,    RTLNAME(TYP_GETFLD),0      	},
{ "TYP_GETREFFLD",   		SbxINTEGER,       _CPROP,    RTLNAME(TYP_GETREFFLD),0      	},
{ "TYP_GETREFPAGEFLD",   	SbxINTEGER,       _CPROP,    RTLNAME(TYP_GETREFPAGEFLD),0   },
{ "TYP_HIDDENPARAFLD",   	SbxINTEGER,       _CPROP,    RTLNAME(TYP_HIDDENPARAFLD),0   },
{ "TYP_HIDDENTXTFLD",   	SbxINTEGER,       _CPROP,    RTLNAME(TYP_HIDDENTXTFLD),0    },
{ "TYP_INPUTFLD",   		SbxINTEGER,       _CPROP,    RTLNAME(TYP_INPUTFLD),0      	},
{ "TYP_INTERNETFLD",   		SbxINTEGER,       _CPROP,    RTLNAME(TYP_INTERNETFLD),0     },
{ "TYP_JUMPEDITFLD",   		SbxINTEGER,       _CPROP,    RTLNAME(TYP_JUMPEDITFLD),0     },
{ "TYP_MACROFLD",   		SbxINTEGER,       _CPROP,    RTLNAME(TYP_MACROFLD),0      	},
{ "TYP_NEXTPAGEFLD",   		SbxINTEGER,       _CPROP,    RTLNAME(TYP_NEXTPAGEFLD),0     },
{ "TYP_PAGENUMBERFLD",   	SbxINTEGER,       _CPROP,    RTLNAME(TYP_PAGENUMBERFLD),0   },
{ "TYP_POSTITFLD",   		SbxINTEGER,       _CPROP,    RTLNAME(TYP_POSTITFLD),0      	},
{ "TYP_PREVPAGEFLD",   		SbxINTEGER,       _CPROP,    RTLNAME(TYP_PREVPAGEFLD),0     },
{ "TYP_SEQFLD",   			SbxINTEGER,       _CPROP,    RTLNAME(TYP_SEQFLD),0      	},
{ "TYP_SETFLD",   			SbxINTEGER,       _CPROP,    RTLNAME(TYP_SETFLD),0      	},
{ "TYP_SETINPFLD",   		SbxINTEGER,       _CPROP,    RTLNAME(TYP_SETINPFLD),0      	},
{ "TYP_SETREFFLD",   		SbxINTEGER,       _CPROP,    RTLNAME(TYP_SETREFFLD),0      	},
{ "TYP_SETREFPAGEFLD", 		SbxINTEGER,       _CPROP,    RTLNAME(TYP_SETREFPAGEFLD),0   },
{ "TYP_TEMPLNAMEFLD",   		SbxINTEGER,       _CPROP,    RTLNAME(TYP_TEMPLNAMEFLD),0},
{ "TYP_TIMEFLD",   		SbxINTEGER,       _CPROP,    RTLNAME(TYP_TIMEFLD),0    		  	},
{ "TYP_USERFLD",   		SbxINTEGER,       _CPROP,    RTLNAME(TYP_USERFLD),0      		},
{ "TYP_USRINPFLD",   		SbxINTEGER,       _CPROP,    RTLNAME(TYP_USRINPFLD),0      	},

{ "TypeLen",       	SbxINTEGER,    1 | _FUNCTION, RTLNAME(TypeLen),0        },
  { "Var",          SbxVARIANT, 0,NULL,0 },
{ "TypeName",       SbxSTRING,    1 | _FUNCTION, RTLNAME(TypeName),0        },
  { "Var",          SbxVARIANT, 0,NULL,0 },

{ "UBound",         SbxLONG,      1 | _FUNCTION, RTLNAME(UBound),0          },
  { "Var",          SbxVARIANT, 0,NULL,0 },
{ "UCase",          SbxSTRING,    1 | _FUNCTION, RTLNAME(UCase),0           },
  { "String",       SbxSTRING, 0,NULL,0 },
{ "Unload",         SbxNULL,      1 | _FUNCTION, RTLNAME(Unload),0          },
  { "Dialog",       SbxOBJECT, 0,NULL,0 },

{ "Val",            SbxDOUBLE,    1 | _FUNCTION, RTLNAME(Val),0             },
  { "String",       SbxSTRING, 0,NULL,0 },
{ "VarType",        SbxINTEGER,   1 | _FUNCTION, RTLNAME(VarType),0         },
  { "Var",          SbxVARIANT, 0,NULL,0 },
{ "V_EMPTY",        SbxINTEGER,       _CPROP,    RTLNAME(V_EMPTY),0         },
{ "V_NULL",         SbxINTEGER,       _CPROP,    RTLNAME(V_NULL),0          },
{ "V_INTEGER",      SbxINTEGER,       _CPROP,    RTLNAME(V_INTEGER),0       },
{ "V_LONG",         SbxINTEGER,       _CPROP,    RTLNAME(V_LONG),0          },
{ "V_SINGLE",       SbxINTEGER,       _CPROP,    RTLNAME(V_SINGLE),0        },
{ "V_DOUBLE",       SbxINTEGER,       _CPROP,    RTLNAME(V_DOUBLE),0        },
{ "V_CURRENCY",     SbxINTEGER,       _CPROP,    RTLNAME(V_CURRENCY),0      },
{ "V_DATE",         SbxINTEGER,       _CPROP,    RTLNAME(V_DATE),0          },
{ "V_STRING",       SbxINTEGER,       _CPROP,    RTLNAME(V_STRING),0        },

{ "Wait",        	SbxNULL,      1 | _FUNCTION, RTLNAME(Wait),0            },
  { "Milliseconds", SbxLONG, 0,NULL,0 },
//#i64882#
{ "WaitUntil",          SbxNULL,      1 | _FUNCTION, RTLNAME(WaitUntil),0      },
  { "Date", SbxDOUBLE, 0,NULL,0 },
{ "Weekday",        SbxINTEGER,   2 | _FUNCTION, RTLNAME(Weekday),0         },
  { "Date",         SbxDATE, 0,NULL,0 },
  { "Firstdayofweek", SbxINTEGER, _OPT, NULL,0 },
{ "WeekdayName",    SbxSTRING,    3 | _FUNCTION | _COMPTMASK, RTLNAME(WeekdayName),0 },
  { "Weekday",      SbxINTEGER, 0,NULL,0 },
  { "Abbreviate",   SbxBOOL,      _OPT, NULL,0 },
  { "Firstdayofweek", SbxINTEGER, _OPT, NULL,0 },
{ "Year",           SbxINTEGER,   1 | _FUNCTION, RTLNAME(Year),0            },
  { "Date",         SbxDATE, 0,NULL,0 },

{ NULL,             SbxNULL,     -1,NULL,0 }};  // Tabellenende

SbiStdObject::SbiStdObject( const String& r, StarBASIC* pb ) : SbxObject( r )
{
	// Muessen wir die Hashcodes initialisieren?
	Methods* p = aMethods;
	if( !p->nHash )
	  while( p->nArgs != -1 )
	{
		String aName_ = String::CreateFromAscii( p->pName );
		p->nHash = SbxVariable::MakeHashCode( aName_ );
		p += ( p->nArgs & _ARGSMASK ) + 1;
	}

	// #i92642: Remove default properties
	Remove( XubString( RTL_CONSTASCII_USTRINGPARAM("Name") ), SbxCLASS_DONTCARE );
	Remove( XubString( RTL_CONSTASCII_USTRINGPARAM("Parent") ), SbxCLASS_DONTCARE );

	SetParent( pb );

	pStdFactory = new SbStdFactory;
	SbxBase::AddFactory( pStdFactory );

	Insert( new SbStdClipboard );
}

SbiStdObject::~SbiStdObject()
{
	SbxBase::RemoveFactory( pStdFactory );
	delete pStdFactory;
}

// Suche nach einem Element:
// Hier wird linear durch die Methodentabelle gegangen, bis eine
// passende Methode gefunden wurde. Auf Grund der Bits im nArgs-Feld
// wird dann die passende Instanz eines SbxObjElement generiert.
// Wenn die Methode/Property nicht gefunden wurde, nur NULL ohne
// Fehlercode zurueckliefern, da so auch eine ganze Chain von
// Objekten nach der Methode/Property befragt werden kann.

SbxVariable* SbiStdObject::Find( const String& rName, SbxClassType t )
{
	// Bereits eingetragen?
	SbxVariable* pVar = SbxObject::Find( rName, t );
	if( !pVar )
	{
		// sonst suchen
		sal_uInt16 nHash_ = SbxVariable::MakeHashCode( rName );
		Methods* p = aMethods;
		sal_Bool bFound = sal_False;
		short nIndex = 0;
		sal_uInt16 nSrchMask = _TYPEMASK;
		switch( t )
		{
			case SbxCLASS_METHOD:	nSrchMask = _METHOD; break;
			case SbxCLASS_PROPERTY:	nSrchMask = _PROPERTY; break;
			case SbxCLASS_OBJECT:	nSrchMask = _OBJECT; break;
			default: break;
		}
		while( p->nArgs != -1 )
		{
			if( ( p->nArgs & nSrchMask )
			 && ( p->nHash == nHash_ )
			 && ( rName.EqualsIgnoreCaseAscii( p->pName ) ) )
			{
				bFound = sal_True;
				if( p->nArgs & _COMPTMASK )
				{
					SbiInstance* pInst = pINST;
					if( !pInst || !pInst->IsCompatibility() )
						bFound = sal_False;
				}
				break;
			}
			nIndex += ( p->nArgs & _ARGSMASK ) + 1;
			p = aMethods + nIndex;
		}

		if( bFound )
		{
			// Args-Felder isolieren:
			short nAccess = ( p->nArgs & _RWMASK ) >> 8;
			short nType   = ( p->nArgs & _TYPEMASK );
			if( p->nArgs & _CONST )
				nAccess |= SBX_CONST;
			String aName_ = String::CreateFromAscii( p->pName );
			SbxClassType eCT = SbxCLASS_OBJECT;
			if( nType & _PROPERTY )
				eCT = SbxCLASS_PROPERTY;
			else if( nType & _METHOD )
				eCT = SbxCLASS_METHOD;
			pVar = Make( aName_, eCT, p->eType );
			pVar->SetUserData( nIndex + 1 );
			pVar->SetFlags( nAccess );
		}
	}
	return pVar;
}

// SetModified mu� bei der RTL abgklemmt werden
void SbiStdObject::SetModified( sal_Bool )
{
}

// Aufruf einer Property oder Methode.

void SbiStdObject::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
							 const SfxHint& rHint, const TypeId& rHintType )

{
	const SbxHint* pHint = PTR_CAST(SbxHint,&rHint);
	if( pHint )
	{
		SbxVariable* pVar = pHint->GetVar();
		SbxArray* pPar_ = pVar->GetParameters();
		sal_uIntPtr t = pHint->GetId();
		sal_uInt16 nCallId = (sal_uInt16) pVar->GetUserData();
		if( nCallId )
		{
			if( t == SBX_HINT_INFOWANTED )
				pVar->SetInfo( GetInfo( (short) pVar->GetUserData() ) );
			else
			{
				sal_Bool bWrite = sal_False;
				if( t == SBX_HINT_DATACHANGED )
					bWrite = sal_True;
				if( t == SBX_HINT_DATAWANTED || bWrite )
				{
					RtlCall p = (RtlCall) aMethods[ nCallId-1 ].pFunc;
					SbxArrayRef rPar( pPar_ );
					if( !pPar_ )
					{
						rPar = pPar_ = new SbxArray;
						pPar_->Put( pVar, 0 );
					}
					p( (StarBASIC*) GetParent(), *pPar_, bWrite );
					return;
				}
			}
		}
		SbxObject::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
	}
}

// Zusammenbau der Infostruktur fuer einzelne Elemente
// Falls nIdx = 0, nix erzeugen (sind Std-Props!)

SbxInfo* SbiStdObject::GetInfo( short nIdx )
{
	if( !nIdx )
		return NULL;
	Methods* p = &aMethods[ --nIdx ];
	// Wenn mal eine Hilfedatei zur Verfuegung steht:
	// SbxInfo* pInfo_ = new SbxInfo( Hilfedateiname, p->nHelpId );
	SbxInfo* pInfo_ = new SbxInfo;
	short nPar = p->nArgs & _ARGSMASK;
	for( short i = 0; i < nPar; i++ )
	{
		p++;
		String aName_ = String::CreateFromAscii( p->pName );
		sal_uInt16 nFlags_ = ( p->nArgs >> 8 ) & 0x03;
		if( p->nArgs & _OPT )
			nFlags_ |= SBX_OPTIONAL;
		pInfo_->AddParam( aName_, p->eType, nFlags_ );
	}
	return pInfo_;
}

