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

#include <QScriptEngine>
#include <QSettings>
#include <QFile>
#include <QDebug>

#include <math.h>

#include "simucalc.h"
#include "mainwindow.h"

SimuCalc::SimuCalc( MainWindow* mainW )
        : QWidget()
{
    setupUi( this );

    m_mainWindow = mainW;
    m_nBits = 32;
    m_base = 10;
    m_type = Uint;
    m_bitVal = 0;
    m_blocked = false;
    m_cRot = false;

    m_bits.setExclusive( false );

    ///m_font.setFamily("Ubuntu Mono");
    //m_font.setBold( true );
    ///m_font.setPointSize( 11 );
    //m_font.setLetterSpacing( QFont::PercentageSpacing, 100 );

    int i=0;
    for( int byte=0; byte<8; ++byte )
    {
        QGroupBox* group = this->findChild<QGroupBox*>( "byte_"+QString::number(byte) );
        m_bytes.append( group );

        for( int bit=0; bit<8; ++bit )
        {
            QPushButton* bitBut = new QPushButton();
            ///bitBut->setMaximumSize( 15, 15 );
            bitBut->setCheckable( true );
            bitBut->setFlat( true );
            ///bitBut->setFont( m_font );
            bitBut->setText("0");
            m_bits.addButton( bitBut, i );
            i++;
            QHBoxLayout* l = (QHBoxLayout*)group->layout();
            l->insertWidget( 0, bitBut );
            //group->layout()->addWidget( bitBut );
        }
    }
    connect( &m_bits, SIGNAL( buttonToggled( QAbstractButton*, bool ) ),
                this, SLOT(   bitToggled( QAbstractButton*, bool ) ) );

    for( int i=0; i<16; ++i ) // Nunber keys
    {
        QPushButton* keyBut = this->findChild<QPushButton*>( "key_"+QString::number(i) );
        m_keys.addButton( keyBut, i );
    }
    connect( &m_keys, SIGNAL( buttonClicked( int ) ),
                this, SLOT(   keyPressed( int ) ) );

    for( int i=0; i<10; ++i ) // Operations
    {
        QPushButton* opBut = this->findChild<QPushButton*>( "op_"+QString::number(i) );
        m_ops.addButton( opBut, i );
    }
    QPushButton* dotBut = this->findChild<QPushButton*>( "key_dot" );
    m_ops.addButton( dotBut, 10 );
    connect( &m_ops, SIGNAL( buttonClicked( int ) ),
               this, SLOT(   opPressed( int ) ) );

    connect( displayDec, SIGNAL( returnPressed() ), this, SLOT( calculate() ) );
    connect( displayOct, SIGNAL( returnPressed() ), this, SLOT( calculate() ) );
    connect( displayHex, SIGNAL( returnPressed() ), this, SLOT( calculate() ) );

    connect( key_equ, SIGNAL( clicked() ), this, SLOT( calculate() ) );
    connect( key_del, SIGNAL( clicked() ), this, SLOT( del() ) );
    connect( key_bck, SIGNAL( clicked() ), this, SLOT( bck() ) );
    connect( key_fwd, SIGNAL( clicked() ), this, SLOT( fwd() ) );
    connect( key_bwd, SIGNAL( clicked() ), this, SLOT( bwd() ) );
    connect( key_brl, SIGNAL( clicked() ), this, SLOT( brl() ) );
    connect( key_brr, SIGNAL( clicked() ), this, SLOT( brr() ) );

    connect( key_rl, SIGNAL( clicked() ), this, SLOT( rl() ) );
    connect( key_rr, SIGNAL( clicked() ), this, SLOT( rr() ) );
    connect( key_sn, SIGNAL( clicked() ), this, SLOT( sn() ) );
    connect( key_c2, SIGNAL( clicked() ), this, SLOT( c2() ) );
    connect( key_c1, SIGNAL( clicked() ), this, SLOT( c1() ) );
    connect( key_c,  SIGNAL( clicked() ), this, SLOT( c() ) );

    connect( key_undo, SIGNAL( clicked() ), this, SLOT( undo() ) );
    connect( key_redo, SIGNAL( clicked() ), this, SLOT( redo() ) );

    displayDec->setPlaceholderText(" DEC");
    displayDec->m_simuCalc = this;
    displayDec->setBase( 10 );

    displayOct->setPlaceholderText(" OCT");
    displayOct->m_simuCalc = this;
    displayOct->setBase( 8 );
    displayOct->setVisible( false );

    displayHex->setPlaceholderText(" HEX");
    displayHex->m_simuCalc = this;
    displayHex->setBase( 16 );

    m_activeDisp = displayDec;

    bitsBox->setCurrentIndex( 1 ); // Default to 32 bits
    key_dot->setEnabled( false );  // Float not active
}
SimuCalc::~SimuCalc()
{
    QSettings* settings = m_mainWindow->settings();
    settings->setValue("showHex", hexCheck->isChecked() );
    settings->setValue("showOct", octCheck->isChecked() );
    settings->setValue("dataBits", bitsBox->currentIndex() );
    settings->setValue("dataType", typeBox->currentIndex() );
}

void SimuCalc::loadSettings()
{
    QSettings* settings = m_mainWindow->settings();
    octCheck->setChecked( settings->value("showOct" ).toBool() );
    hexCheck->setChecked( settings->value("showHex" ).toBool() );
    bitsBox->setCurrentIndex( settings->value("dataBits").toInt() );
    typeBox->setCurrentIndex( settings->value("dataType").toInt() );
}

void SimuCalc::calculate()
{
 static QScriptEngine engine;

    appendUndo();

    qsreal result = engine.evaluate( displayDec->text() ).toNumber();

    m_resultStr.clear();
    QString resultStr;
    if( m_type == Uint )
    {
        if     ( m_nBits ==  8 ) resultStr = QString::number( (uint8_t )result );
        else if( m_nBits == 16 ) resultStr = QString::number( (uint16_t)result );
        else if( m_nBits == 32 ) resultStr = QString::number( (uint32_t)result );
        else if( m_nBits == 64 ) resultStr = QString::number( (uint64_t)result );
    }
    else if( m_type == Int )
    {
        if     ( m_nBits ==  8 ) resultStr = QString::number( (int8_t )result );
        else if( m_nBits == 16 ) resultStr = QString::number( (int16_t)result );
        else if( m_nBits == 32 ) resultStr = QString::number( (int32_t)result );
        else if( m_nBits == 64 ) resultStr = QString::number( (int64_t)result );
    }
    else // Float
    {
        if     ( m_nBits == 32 ) resultStr = QString::number( (float)result );
        else if( m_nBits == 64 ) resultStr = QString::number( (double)result );
    }
    displayDec->setResult( resultStr );
    m_resultStr = resultStr;

    m_blocked = true;
    updateBits();
    m_blocked = false;
}

void SimuCalc::newBitVal( uint64_t val )
{
    if( m_blocked ) return;
    m_blocked = true;
    m_bitVal = val;
    updateBits();
    m_blocked = false;
    /// updateValues();
}

void SimuCalc::setValues( QString text, QList<uint64_t> values )
{
    if( m_blocked ) return;
    m_blocked = true;
    displayDec->setValues( text, values );
    displayOct->setValues( text, values );
    displayHex->setValues( text, values );
    m_activeDisp->setFocus();
    m_blocked = false;
}

void SimuCalc::setType( dataType_t type )
{
    displayDec->setType( type );
    displayOct->setType( type );
    displayHex->setType( type );
    displayDec->newText( displayDec->text() );
}

void SimuCalc::setBits( int bits )
{
    displayDec->setBits( bits );
    displayOct->setBits( bits );
    displayHex->setBits( bits );
    displayDec->newText( displayDec->text() );
}

void SimuCalc::updateBits()
{
    for( int i=0; i<64; ++i )
    {
        QPushButton* bitBut = (QPushButton*)m_bits.button( i );
        uint64_t bitMask = pow( 2, i );

        if( bitBut->isEnabled() ) bitBut->setChecked( (m_bitVal & bitMask)>0 );
    }
}

void SimuCalc::bitToggled( QAbstractButton* aBut, bool state )
{
    int bit = m_bits.id( aBut );
    uint64_t bitMask = pow( 2, bit );
    m_bitVal &= ~bitMask; // Clear bit
    if( state ) m_bitVal |= bitMask; // Set bit (if state)

    QPushButton* bitBut = (QPushButton*)aBut;
    bitBut->setText( state ? "1" : "0");

    if( !m_blocked ) m_activeDisp->setCurrentValue( m_bitVal );
}

void SimuCalc::del() { m_activeDisp->del(); }
void SimuCalc::bck() { m_activeDisp->backspace(); }
void SimuCalc::fwd() { m_activeDisp->cursorForward( false ); m_activeDisp->setFocus(); }
void SimuCalc::bwd() { m_activeDisp->cursorBackward( false ); m_activeDisp->setFocus(); }
void SimuCalc::brl() { m_activeDisp->insert( "(" ); }
void SimuCalc::brr() { m_activeDisp->insert( ")" ); }

void SimuCalc::rl() { appendUndo(); m_activeDisp->rl( m_cRot ); }
void SimuCalc::rr() { appendUndo(); m_activeDisp->rr( m_cRot ); }
void SimuCalc::sn() { appendUndo(); m_activeDisp->sn();}
void SimuCalc::c2() { appendUndo(); m_activeDisp->c2(); }
void SimuCalc::c1() { appendUndo(); m_activeDisp->c1(); }
void SimuCalc::c()
{
    m_cRot = key_c->isChecked();
    if( m_cRot ){
        key_rl->setStyleSheet("background-color: #A0A0C0");
        key_rr->setStyleSheet("background-color: #A0A0C0");
    }else{
        key_rl->setStyleSheet("background-color: #BCBCDE");
        key_rr->setStyleSheet("background-color: #BCBCDE");
    }
}

void SimuCalc::keyPressed( int index )
{ m_activeDisp->insert( QString::number( index, 16 ) ); }

void SimuCalc::opPressed( int index )
{
    QString op = "+-*/%&|^~ .";
    op = op.mid( index, 1 );
    m_activeDisp->insert( op );
}

void SimuCalc::dispChanged( Display* d )
{
    m_activeDisp = d;
    m_base = m_activeDisp->m_base;
    for( int i=0; i<16; ++i )
    {
        QPushButton* keyBut = (QPushButton*)m_keys.button( i );
        keyBut->setEnabled( i<m_base );
    }
}

void SimuCalc::on_typeBox_currentIndexChanged( int index ) // Uint, Int, Float
{
    m_blocked = true;
    m_type = (dataType_t)index;

    int oldBits = bitsBox->currentIndex();
    bitsBox->clear();
    bitsBox->addItems( {"64 bits","32 bits"} );
    if( index < Float32 )
    {
        bitsBox->addItems( {"16 bits","8 bits"} );
        m_blocked = false;
        setType( (dataType_t)index );
    }
    else if( oldBits > 0 ) oldBits = 1 ;// Float32

    m_blocked = false;
    bitsBox->setCurrentIndex( oldBits );

    key_dot->setEnabled( index == Float32 ); // Float
}

void SimuCalc::on_bitsBox_currentIndexChanged( int index ) // 8, 16, 32, 64
{
    if( m_blocked ) return;

    int nBits = 8;
    switch( index ) {
        case 0: nBits = 64; break;
        case 1: nBits = 32; break;
        case 2: nBits = 16; break;
        case 3: nBits = 8;  break;
    }
    if( m_nBits != nBits )
    {
        bool old64 = m_nBits == 64;
        bool new64 = nBits == 64;

        if( old64 != new64 ){
            const QSize size = this->parentWidget()->size();
            int delta = ( old64 && !new64 ) ? -35 : 35;
            this->parentWidget()->resize( size.width(), size.height()+delta );
        }
        m_nBits = nBits;
    }

    for( int i=0; i<64; ++i )
    {
        bool enable = i < m_nBits;
        QPushButton* bitBut = (QPushButton*)m_bits.button( i );
        if( !enable ) bitBut->setChecked( false );
        bitBut->setEnabled( enable );
    }
    int bytes = m_nBits/8;
    for( int i=0; i<8; ++i ) m_bytes.at( i )->setHidden( i>=bytes );

    /*bool enLogic =  m_nBits < 64;
    op_5->setEnabled( enLogic ); // AND
    op_6->setEnabled( enLogic ); // OR
    op_7->setEnabled( enLogic ); // XOR
    op_8->setEnabled( enLogic ); // NOT
    op_9->setEnabled( enLogic ); //*/

    if( m_type == Float32 ) // Float
    {
        if( index == 0 ) setType( Float64 );
        else             setType( Float32 );
    }
    setBits( m_nBits );
}

void SimuCalc::textChanged()
{
    if( m_resultStr.isEmpty() ) return;
    m_undoStack.append( m_resultStr );
    m_resultStr.clear();
}

void SimuCalc::appendUndo()
{
    QString exp = displayDec->text();

    m_undoStack.append( exp );
    m_redoStack.clear();
}

void SimuCalc::undo()
{
    if( m_undoStack.isEmpty() ) return;

    m_redoStack.prepend( displayDec->text() );
    displayDec->newText( m_undoStack.takeLast() );
}

void SimuCalc::redo()
{
    if( m_redoStack.isEmpty() ) return;

    m_undoStack.append( displayDec->text() );
    displayDec->newText( m_redoStack.takeFirst() );
}

void SimuCalc::on_hexCheck_stateChanged( int state )
{
    enableDisplay( displayHex, state );
}

void SimuCalc::on_octCheck_stateChanged( int state )
{
    enableDisplay( displayOct, state );
}

void SimuCalc::enableDisplay( Display* d, bool en )
{
    if( m_blocked ) return;
    const QSize size = this->parentWidget()->size();
    int delta = en ? 40 : -40;
    d->setVisible( en );
    this->parentWidget()->resize( size.width(), size.height()+delta );
}

