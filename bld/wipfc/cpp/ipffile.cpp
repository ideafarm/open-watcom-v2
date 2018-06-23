/****************************************************************************
*
*                            Open Watcom Project
*
*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  IPF Input file reader
*
****************************************************************************/
#include "ipffile.hpp"
#include "errors.hpp"
#include "util.hpp"
#include <cstdlib>
#if !defined( __UNIX__ ) && !defined( __APPLE__ )
    #include <mbctype.h>
#endif

IpfFile::IpfFile( const std::wstring* fname ) : IpfData(), fileName ( fname ),
    ungottenChar( WEOF ), ungotten( false )
{
    std::string buffer;
    wtomb_string( *fname, buffer );
    if( (stream = std::fopen( buffer.c_str(), "rb" )) == 0 ) {
        throw FatalIOError( ERR_OPEN, *fileName );
    }
}
/*****************************************************************************/
//Read a character
//Returns EOB if end-of-file reached
std::wint_t IpfFile::get()
{
    std::wint_t     ch;

    if( ungotten ) {
        ch = ungottenChar;
        ungotten = false;
    } else {
        ch = read_wchar();
    }
    if( ch == L'\r' ) {
        ch = read_wchar();
    }
    incCol();
    if( ch == L'\n' ) {
        incLine();
        resetCol();
    } else if( ch == WEOF ) {
        ch = EOB;
        if( !std::feof( stream ) ) {
            throw FatalIOError( ERR_READ, *fileName );
        }
    }
    return( ch );
}
/*****************************************************************************/
void IpfFile::unget( wchar_t ch )
{
    //std::ungetwc( ch, stream );
    ungottenChar = ch;
    ungotten = true;
    decCol();
    if( ch == L'\n' ) {
        decLine();
    }
}
/*****************************************************************************/
std::wint_t IpfFile::read_wchar()
{
    wchar_t ch;

#if defined( __UNIX__ ) || defined( __APPLE__ )
    // TODO! read MBCS character and convert it to UNICODE by mbtow_char
    ch = std::fgetwc( stream );
#else
    char    mbc[ MB_LEN_MAX ];
    if( std::fread( &mbc[0], sizeof( char ), 1, stream ) != 1 )
        return( WEOF );
    if( _ismbblead( mbc[0] ) ) {
        if( std::fread( &mbc[1], sizeof( char ), 1, stream ) != 1 ) {
            return( WEOF );
        }
    }
    if( mbtow_char( &ch, mbc, MB_CUR_MAX ) < 0 ) {
        throw FatalError( ERR_T_CONV );
    }
#endif
    return( ch );
}
