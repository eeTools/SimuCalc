/***************************************************************************
 *   Copyright (C) 2022 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

//#include <QDebug>

#include "display.h"
#include "simucalc.h"
#include <math.h>

Display* Display::m_Currentdisplay = nullptr;

Display::Display( QWidget* parent )
       : QLineEdit( parent )
{
    //m_result = false;
    m_blocked = false;
    m_dot     = false;
    m_valStr = "";
    m_valStart = 0;
    m_currVal = 0;
    m_type = Uint;
    m_bits = 32;
    m_base = 10;

    m_validDec = "0123456789";
    m_validOct = "01234567";
    m_validHex = "0123456789ABCDEF";
    m_validOps = "+-*/%&|~^()";

    connect( this, SIGNAL( textChanged( QString ) ),
                this, SLOT( newText( QString ) ) );

    connect( this, SIGNAL( cursorPositionChanged( int, int ) ),
             this, SLOT( cursorPosChanged( int, int ) ) );
}
Display::~Display(){}

void Display::setResult( QString text )
{
    //m_result = true;

    newText( text );
}

void Display::newText( QString text )
{
    if( m_blocked ) return;
    m_blocked = true;

    if( text != m_text ) m_simuCalc->textChanged();

    m_text = "";
    m_Currentdisplay = this;
    analizeText( text );
    setText( m_text );

    m_simuCalc->setValues( m_operations, m_values );
    m_Currentdisplay = nullptr;

    uint64_t bitVal = 0;
    if( !m_values.isEmpty() ) bitVal = m_values.at( m_currVal );
    m_simuCalc->newBitVal( bitVal );

    m_blocked = false;
}

void Display::analizeText( QString text ) // Create a list of operations and values
{
    m_operations = "";
    m_valStr   = "";
    m_valStart = 0;
    m_currVal = 0;
    text = text.toUpper();

    m_values.clear();
    m_valStarts.clear();

    for( int i=0; i<text.size(); ++i )
    {
        QString key = text.at( i );

        if( m_validChars.contains( key ) )
        {
            if( m_type <= Int )                // Trim to m_bits
            {
                QString val = m_valStr + key;
                bool ok = false;
                if( m_type == Uint || m_base == 16 )
                {
                    uint64_t v64 = val.toULongLong( &ok, m_base );
                    if( !ok ) break;
                    if( m_bits < 64 && v64 >= m_bits*m_bits ) break;
                }else{
                    int64_t v64 = val.toLongLong( &ok, m_base );
                    if( !ok ) break;
                    int64_t max = m_bits*m_bits/2;
                    if( m_bits < 64 && (v64 >= max || v64 < -max) ) break;
                }
            }
            m_valStr += key;
            m_text   += key;
        }
        else if( m_validOps.contains( key ) ) // Adding operation
        {
            if( !m_valStr.isEmpty() )
            {
                addVal();
                m_valStart = i;               // Save last Value
                m_operations += key;
                m_text += key;
            }
            else if( (m_base == 10 && key == "-") || key == "~" )
            {
                m_valStr += key;
                m_text += key;
            }
        }
    }
    m_dot = (m_base == 10) && text.endsWith(".");

    if( !m_valStr.isEmpty() ) addVal();
}

void Display::addVal()
{
    bool ok = false;
    uint64_t uintVal = 0;

    if( m_base == 10 && m_type >= Float32 )
    {
        if( m_type == Float32 )
        {
            float fVal = m_valStr.toFloat( &ok );
            memcpy( &uintVal, &fVal, 4 );
        }
        else if( m_type == Float64 )
        {
            double dVal = m_valStr.toDouble( &ok );
            memcpy( &uintVal, &dVal, 8 );
        }
    }else                   // Int or Uint any base
    {
        if( m_valStr.startsWith("-") ) uintVal = m_valStr.toLongLong( &ok, m_base );
        else                           uintVal = m_valStr.toULongLong( &ok, m_base );
        switch( m_bits ){
            case 8:  uintVal &= 0xFF;       break;
            case 16: uintVal &= 0xFFFF;     break;
            case 32: uintVal &= 0xFFFFFFFF; break;
            default: break;
        }
    }
    if( ok ){
        m_values.append( uintVal );
        m_valStarts.append( m_valStart );

        if( this->cursorPosition() >= m_valStart ) // cursor is here
            m_currVal = m_values.size()-1;

        m_operations += "#"+QString::number( m_values.size()-1 );
        m_valStr   = "";
    }
    //else qDebug() << "Display::newText Error";
}

void Display::setValues( QString ops, QList<uint64_t> values )
{
    if( m_Currentdisplay == this ) return;
    m_operations = ops;
    m_values = values;

    for( int i=0; i<values.size(); ++i ){
        QString valStr = getValStr( values.at( i ) );
        ops.replace( "#"+QString::number(i), valStr );
    }
    if( m_dot ) ops.append(".");
    int cursorPos = this->cursorPosition();
    setText( ops );
    setCursorPosition( cursorPos );
}

void Display::setCurrentValue( uint64_t bitVal )
{
    m_blocked = true;
    int cursorPos = this->cursorPosition();

    QString mText = text();
    if( mText.isEmpty() ) mText += getValStr( bitVal ); //QString::number( bitVal, m_base );
    analizeText( mText );

    updtCurrentValue( bitVal );

    setCursorPosition( cursorPos );
    m_blocked = false;
}

void Display::updtCurrentValue( uint64_t bitVal )
{
    bitVal = trimToBits( bitVal );
    m_values.replace( m_currVal, bitVal );
    m_simuCalc->setValues( m_operations, m_values );
    m_simuCalc->newBitVal( bitVal );
}

void Display::cursorPosChanged( int, int cursorPos ) // Cursor moving: update bit values to cursor position
{
    if( m_blocked ) return;

    if( m_values.isEmpty() ) return;

    int i=0;
    for( ; i<m_valStarts.size(); i++ )
        if( cursorPos < m_valStarts.at(i) ) break;

    m_currVal = i-1;
    m_simuCalc->newBitVal( m_values.at( m_currVal ) );
}

QString Display::getValStr( uint64_t val ) // Convert uint64 value to String shown in display with base and type
{
    QString valStr;
    if( m_type == Uint )
    {
        valStr = QString::number( val, m_base );
    }
    else if( m_type == Int )
    {
        int64_t sval = val;
        switch( m_bits ){
            case 8:  if( sval & 1<<7  ) sval |= 0xFFFFFFFFFFFFFF00; break;
            case 16: if( sval & 1<<15 ) sval |= 0xFFFFFFFFFFFF0000; break;
            case 32: if( sval & 1<<31 ) sval |= 0xFFFFFFFF00000000; break;
            default: break;
        }
        valStr = QString::number( sval, m_base );
        if( m_base == 16 )
        {
            switch( m_bits ){
                case 8:  valStr = valStr.right( 2 ); break;
                case 16: valStr = valStr.right( 4 ); break;
                case 32: valStr = valStr.right( 8 ); break;
                default: break;
            }
        }
    }else{
        if( m_base == 10 )                // Decimal
        {
            if( m_type == Float32 )
            {
                float dval;
                memcpy( &dval, &val, 4 );
                valStr = QString::number( dval );
            }
            else if( m_type == Float64 )
            {
                double dval;
                memcpy( &dval, &val, 8 );
                valStr = QString::number( dval );
            }
        }
        else if( m_base == 8 )            // Octal
        {
            valStr = QString::number( val, 8 );
        }
        else if( m_base == 16 )            // Hexadecimal
        {
            valStr = QString::number( val, 16 );
        }
    }
    if( m_base == 16 ) valStr = valStr.toUpper();

    return valStr;
}

void Display::setBits( int bits )
{
    m_bits = bits;
}

void Display::setBase( int base )
{
    m_base = base;
    if     ( m_base == 8  ) m_validChars = m_validOct;
    else if( m_base == 10 ) m_validChars = m_validDec;
    else if( m_base == 16 ) m_validChars = m_validHex;
}

void Display::setType( dataType_t type )
{
    m_type = type;

    if( m_base == 10 && type >= Float32 ) m_validChars.append(".");
    else                                  m_validChars.remove(".");
}

void Display::focusInEvent( QFocusEvent* e )
{
    QLineEdit::focusInEvent( e );
    m_simuCalc->dispChanged( this );
}

void Display::rl( bool continuous )
{
    if( m_values.isEmpty() ) return;

    uint64_t val = trimToBits( m_values.at( m_currVal ) );
    bool c = (val & (uint64_t)1<<(m_bits-1)) > 0;
    val = trimToBits( val << 1 );
    if( continuous && c ) val |= 1;
    updtCurrentValue( val );
}

void Display::rr( bool continuous )
{
    if( m_values.isEmpty() ) return;

    uint64_t val = trimToBits( m_values.at( m_currVal ) );
    bool c = (val & 1) > 0;
    val = trimToBits( val >> 1 );
    if( continuous && c ) val |= (uint64_t)1<<(m_bits-1);
    updtCurrentValue( val );
}

void Display::sn() // Swap nibbles
{
    if( m_values.isEmpty() ) return;

    uint64_t val = trimToBits( m_values.at( m_currVal ) );
    uint64_t result = 0;

    for( int i=0; i<8; ++i )
    {
        uint64_t byte = val>>(8*i) & 0xFF;
        byte = (byte & 0x0F) << 4 | (byte & 0xF0) >> 4;
        result |= byte<<(8*i);
    }

    updtCurrentValue( result );
}

void Display::c2() // 2'2 complement
{
    if( m_values.isEmpty() ) return;
    uint64_t val = trimToBits( m_values.at( m_currVal ) );
    val = ~val+1;
    updtCurrentValue( val );
    //updtCurrentValue( ~m_values.at( m_currVal )+1 );
}

void Display::c1() // 1's complement
{
    if( m_values.isEmpty() ) return;
    uint64_t val = trimToBits( m_values.at( m_currVal ) );
    val = ~val;
    updtCurrentValue( val );
}

/*void Display::focusOutEvent(QFocusEvent *e)
{
  QLineEdit::focusOutEvent(e);
  emit(focussed(false));
}*/

uint64_t Display::trimToBits( uint64_t val )
{
    switch( m_bits ){
        case 8:  val &= 0x00000000000000FF; break;
        case 16: val &= 0x000000000000FFFF; break;
        case 32: val &= 0x00000000FFFFFFFF; break;
        default: break;
    }
    return val;
}

