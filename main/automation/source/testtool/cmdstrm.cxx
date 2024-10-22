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
#include "precompiled_automation.hxx"
#ifndef _KEYCOD_HXX //autogen
#include <vcl/keycod.hxx>
#endif
#include <basic/sbx.hxx>
#include <tools/stream.hxx>

#include "cmdstrm.hxx"
#include "rcontrol.hxx"
#include "objtest.hxx"
#include "sttresid.hxx"
#include "svcommstream.hxx"
#include <basic/testtool.hrc>

ControlDefLoad __READONLY_DATA CmdStream::arKeyCodes [] =
#include <keycodes.hxx>
CNames *CmdStream::pKeyCodes = NULL;

ResMgr* SttResId::getSttResMgr()
{
    static ResMgr* pMgr = NULL;
    if( ! pMgr )
        pMgr = CREATEVERSIONRESMGR( stt );
    return pMgr;
}

CmdStream::CmdStream()
{
	pSammel = new SvMemoryStream();
	pCommStream = new SvCommStream( pSammel );
}

CmdStream::~CmdStream()
{
	delete pCommStream;
	delete pSammel;
}


#define PUT_CHAR(ch) \
		sal_uInt16 nMCode = nModify + ch;						\
		sal_uInt16 _Code = 1;									\
		if ( (nMCode & 0xFF) == 0 )							\
			_Code |= 2;										\
		if ( (nMCode >> 8) == 0 )							\
			_Code |= 4;										\
		Result += (char) _Code;								\
		Result += (char) ( nMCode & 0xFF );					\
		Result += (char) ( nMCode >> 8 )


String CmdStream::WandleKeyEventString( String aKeys )
{
	if ( !pKeyCodes )
		ReadFlatArray(arKeyCodes,pKeyCodes);
	if ( !pKeyCodes )
		return aKeys;

	xub_StrLen nPos1 = 0;
	while ( (nPos1 = aKeys.Search('<',nPos1)) != STRING_NOTFOUND)
	{
		xub_StrLen nPos2 = aKeys.Search('>',nPos1);
		if ( nPos2 != STRING_NOTFOUND )
		{
			String Work = aKeys.Copy(nPos1+1,nPos2-nPos1+1-2);
			aKeys.Erase(nPos1,nPos2-nPos1+1);	// Inclusive Spitze Klammern weg
			String Result, Token;
			sal_uInt16 nModify = 0;
			while ( Work.Len() > 0 )
			{
				Token = Work.GetToken(0,' ');
				Work.Erase(0,Token.Len()+1);
				ControlDef WhatName(Token,rtl::OString());
				sal_uInt16 nElement;
				if (pKeyCodes->Seek_Entry(&WhatName,&nElement))
				{
				    // FIXME: HELPID
					sal_uInt16 nCode = 0;//(sal_uInt16) pKeyCodes->GetObject(nElement)->pData->aUId.GetNum();
					if ( nCode >= KEY_SHIFT )
						nModify ^= nCode;
					else
					{
						if ( ( nModify == 0 ) &&
							  (((nCode & 0xFF00) == KEYGROUP_NUM)   ||
							   ((nCode & 0xFF00) == KEYGROUP_ALPHA) ||
							   ( nCode == KEY_SPACE) ||
							   ( nCode == KEY_ADD) ||
							   ( nCode == KEY_SUBTRACT) ||
							   ( nCode == KEY_MULTIPLY) ||
							   ( nCode == KEY_DIVIDE) ||
							   ( nCode == KEY_POINT) ||
							   ( nCode == KEY_COMMA) ||
							   ( nCode == KEY_LESS) ||
							   ( nCode == KEY_GREATER) ||
							   ( nCode == KEY_EQUAL) ) )
						{
							switch ( nCode )
							{
								case KEY_SPACE:		Token = ' '; break;
								case KEY_ADD:		Token = '+'; break;
								case KEY_SUBTRACT:	Token = '-'; break;
								case KEY_MULTIPLY:	Token = '*'; break;
								case KEY_DIVIDE:	Token = '/'; break;
								case KEY_POINT:		Token = '.'; break;
								case KEY_COMMA:		Token = ','; break;
								case KEY_LESS:		Token = '<'; break;
								case KEY_GREATER:	Token = '>'; break;
								case KEY_EQUAL:		Token = '='; break;
								default:
//									if ( nModify == 0 )
//										Token.ToLower();
//									else
//										Token.ToUpper();
									;
							}
							Result += Token;
						}
						else
						{
							PUT_CHAR (nCode);
						}
					}
				}
				else
				{
					Result += '<';
					Result += Token;
					Result += ' ';
					Result += String( SttResId( S_INVALID_KEYCODE ) );
					Result += '>';
				}
			}
			aKeys.Insert(Result,nPos1);
			nPos1 = nPos1 + Result.Len();
		}
		else
			nPos1 = aKeys.Len() + 1;
	}

	return aKeys;
}


void CmdStream::WriteSortedParams( SbxArray* rPar, sal_Bool IsKeyString )
{
	sal_uInt16 nParams = PARAM_NONE;
	sal_uInt16 nNr1=0,nNr2=0,nNr3=0,nNr4=0;
	comm_ULONG nLNr1=0;
	String aString1,aString2;
	sal_Bool bBool1=sal_False,bBool2=sal_False;

	if ( rPar )
	{
		for ( sal_uInt16 i = 1; i < rPar->Count() ; i++)
		{
			switch (rPar->Get( i )->GetType())
			{
				case SbxLONG:		// alles immer als Short �bertragen
				case SbxULONG:
				case SbxLONG64:
				case SbxULONG64:
				case SbxDOUBLE:
				case SbxINTEGER:
				case SbxBYTE:
				case SbxUSHORT:
				case SbxINT:
				case SbxUINT:
				case SbxSINGLE:
					if ( (nParams & PARAM_USHORT_1) == 0 )
					{
						nParams |= PARAM_USHORT_1;
						nNr1 = rPar->Get( i )->GetUShort();
					}
					else if ( (nParams & PARAM_USHORT_2) == 0 )
					{
						nParams |= PARAM_USHORT_2;
						nNr2 = rPar->Get( i )->GetUShort();
					}
					else if ( (nParams & PARAM_USHORT_3) == 0 )
					{
						nParams |= PARAM_USHORT_3;
						nNr3 = rPar->Get( i )->GetUShort();
					}
					else if ( (nParams & PARAM_USHORT_4) == 0 )
					{
						nParams |= PARAM_USHORT_4;
						nNr4 = rPar->Get( i )->GetUShort();
					}
					else
						SbxBase::SetError( SbxERR_WRONG_ARGS );
					break;
				case SbxOBJECT:		// whenever a control is passed. TabPage, MenuBar
                    {
    		            SbxProperty *pMember = NULL;
    		            if ( rPar->Get( i )->ISA( SbxObject ) )
                            pMember = ((SbxObject*)rPar->Get( i ))->GetDfltProperty();
                        if ( pMember != NULL )
                        {
                            if ( pMember->GetType() == SbxSTRING )
                            {
					            if ( nParams & PARAM_STR_1 )
						            if ( nParams & PARAM_STR_2 )
							            SbxBase::SetError( SbxERR_WRONG_ARGS );
						            else
						            {
							            nParams |= PARAM_STR_2;
							            aString2 = pMember->GetString();
						            }
					            else
					            {
						            nParams |= PARAM_STR_1;
						            aString1 = pMember->GetString();
					            }
					            break;
                            }
                            else if ( pMember->GetType() == SbxULONG )
                            {
					            if ( nParams & PARAM_ULONG_1 )
						            SbxBase::SetError( SbxERR_WRONG_ARGS );
					            else
					            {
						            nParams |= PARAM_ULONG_1;
						            nLNr1 = pMember->GetULong();
					            }
                            }
                            else
                                SbxBase::SetError( SbxERR_NAMED_NOT_FOUND );
                        }
                        else
    		            {
    			            SbxBase::SetError( SbxERR_NAMED_NOT_FOUND );
    		            }
                    }
					break;
				case SbxSTRING:
				case SbxCHAR:
					if ( nParams & PARAM_STR_1 )
						if ( nParams & PARAM_STR_2 )
							SbxBase::SetError( SbxERR_WRONG_ARGS );
						else
						{
							nParams |= PARAM_STR_2;
							aString2 = rPar->Get( i )->GetString();
						}
					else
					{
						nParams |= PARAM_STR_1;
						aString1 = rPar->Get( i )->GetString();
					}
					break;
				case SbxBOOL:
					if ( nParams & PARAM_BOOL_1 )
						if ( nParams & PARAM_BOOL_2 )
							SbxBase::SetError( SbxERR_WRONG_ARGS );
						else
						{
							nParams |= PARAM_BOOL_2;
							bBool2 = rPar->Get( i )->GetBool();
						}
					else
					{
						nParams |= PARAM_BOOL_1;
						bBool1 = rPar->Get( i )->GetBool();
					}
					break;
				default:
					SbxBase::SetError( SbxERR_WRONG_ARGS );
					break;
			}
		}
	}
	Write (nParams);
	if( nParams & PARAM_USHORT_1 )	Write( nNr1 );
	if( nParams & PARAM_USHORT_2 )	Write( nNr2 );
	if( nParams & PARAM_USHORT_3 )	Write( nNr3 );
	if( nParams & PARAM_USHORT_4 )	Write( nNr4 );
	if( nParams & PARAM_ULONG_1 )	Write( nLNr1 );
	if( nParams & PARAM_STR_1 )		Write( aString1, IsKeyString );
	if( nParams & PARAM_STR_2 )		Write( aString2, IsKeyString );
	if( nParams & PARAM_BOOL_1 )	Write( bBool1 );
	if( nParams & PARAM_BOOL_2 )	Write( bBool2 );
}

void CmdStream::GenCmdCommand( sal_uInt16 nNr, SbxArray* rPar )
{
	Write(sal_uInt16(SICommand));
	Write(nNr);
	WriteSortedParams(rPar, (nNr & M_KEY_STRING) != 0 );
}

void CmdStream::GenCmdSlot( sal_uInt16 nNr, SbxArray* rPar )
{
	Write(sal_uInt16(SISlot));
	Write(nNr);
	if (rPar)
	{
		sal_uInt16 nAnz = (rPar->Count()-1) >> 1;	// Geteilt durch 2
		Write(nAnz);
        sal_Bool bWriteUnoSlot = rPar->Get( 1 )->GetType() == SbxSTRING;

		for (sal_uInt16 n = 1 ; n <= nAnz ; n++)
		{
			/// #59513# nicht mehr ben�tigt
//			sal_uLong nUserData = rPar->Get( 2*n-1 )->GetUserData();
//			rPar->Get( 2*n-1 )->SetUserData(ID_DoNothing);	// Verhindert Ausf�hrung der Slots, die als Parameter �bergeben sind.

            if ( bWriteUnoSlot )
    			Write(rPar->Get( 2*n-1 )->GetString());
            else
	    		Write(rPar->Get( 2*n-1 )->GetUShort());
			switch (rPar->Get( 2*n )->GetType())
			{
				case SbxINTEGER:
				case SbxBYTE:
				case SbxUSHORT:
				case SbxINT:
				case SbxUINT:
				case SbxSINGLE:
                    if ( !bWriteUnoSlot )
                        Write( (sal_uInt16)BinUSHORT );
					Write(rPar->Get( 2*n )->GetUShort());
					break;
				case SbxLONG:
				case SbxULONG:
				case SbxLONG64:
				case SbxULONG64:
				case SbxDOUBLE:
                    if ( !bWriteUnoSlot )
                        Write( (sal_uInt16)BinULONG );
					Write(rPar->Get( 2*n )->GetULong());
					break;
				case SbxSTRING:
				case SbxCHAR:
                    if ( !bWriteUnoSlot )
                        Write( (sal_uInt16)BinString);
					Write((String)rPar->Get( 2*n )->GetString());    // Cast f�r OS/2
					break;
				case SbxBOOL:
                    if ( !bWriteUnoSlot )
                        Write( (sal_uInt16)BinBool);
					Write(rPar->Get( 2*n )->GetBool());
					break;
				default:
					SbxBase::SetError( SbxERR_WRONG_ARGS );
					break;
			}

			/// #59513# nicht mehr ben�tigt ( siehe oben )
//			rPar->Get( 2*n-1 )->SetUserData(nUserData);	// Und wieder zur�cksetzen, so da� auch alles sauber ist.
		}
	}
	else
		Write(sal_uInt16(0));
}

void CmdStream::GenCmdUNOSlot( const String &aURL )
{
	Write( sal_uInt16(SIUnoSlot) );
/*	Write( sal_uInt16(0) );		// Hier wird im Office die SID_OPENURL Eingetragen.
							// Dies mu� nat�rlich im Office hart verdratet werden und nicht hier,
							// da sich die ID ja mal �ndern kann.

	// Da auch die ID f�r das PoolItem im Office entnommen werden mu� hier also kein PoolItem
	// gesendet werden.

	Write( sal_uInt16(0) );		// Anzahl PoolItems

	// Stattdessen wird noch eine extra String gesendet, der dann Officeseitig in ein
	// SfxStringItem mit entsprechender ID gewandelt wird.
	Write( aURL );			// String f�r das PoolItem*/

    Write( aURL );			// Die UNO URL eben
}

void CmdStream::GenCmdControl( comm_ULONG nUId, sal_uInt16 nMethodId, SbxArray* rPar )
{
	Write(sal_uInt16(SIControl));
	Write(nUId);
	Write(nMethodId);
	WriteSortedParams(rPar, (nMethodId & M_KEY_STRING) != 0 );
}

void CmdStream::GenCmdControl( String aUId, sal_uInt16 nMethodId, SbxArray* rPar )
{
	Write(sal_uInt16(SIStringControl));
	Write(aUId);
	Write(nMethodId);
	WriteSortedParams(rPar, (nMethodId & M_KEY_STRING) != 0 );
}

void CmdStream::GenCmdFlow( sal_uInt16 nArt )
{
	Write(sal_uInt16(SIFlow));
	Write(nArt);
	Write(sal_uInt16(PARAM_NONE));				// Typ der folgenden Parameter
}

void CmdStream::GenCmdFlow( sal_uInt16 nArt, sal_uInt16 nNr1 )
{
	Write(sal_uInt16(SIFlow));
	Write(nArt);
	Write(sal_uInt16(PARAM_USHORT_1));			// Typ der folgenden Parameter
	Write(nNr1);
}

void CmdStream::GenCmdFlow( sal_uInt16 nArt, comm_ULONG nNr1 )
{
	Write(sal_uInt16(SIFlow));
	Write(nArt);
	Write(sal_uInt16(PARAM_ULONG_1));			// Typ der folgenden Parameter
	Write(nNr1);
}

void CmdStream::GenCmdFlow( sal_uInt16 nArt, String aString1 )
{
	Write(sal_uInt16(SIFlow));
	Write(nArt);
	Write(sal_uInt16(PARAM_STR_1));				// Typ der folgenden Parameter
	Write(aString1);
}

void CmdStream::Write( String aString, sal_Bool IsKeyString )
{
	if ( IsKeyString )
		Write( WandleKeyEventString( aString ), sal_False );
	else
		Write( aString.GetBuffer(), aString.Len() );
}

SvMemoryStream* CmdStream::GetStream()
{
	return pSammel;
}

void CmdStream::Reset( comm_ULONG nSequence )
{
	delete pCommStream;
	delete pSammel;
	pSammel = new SvMemoryStream();
	pCommStream = new SvCommStream( pSammel );
	GenCmdFlow (F_Sequence,nSequence);
}

