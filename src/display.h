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

#ifndef DISPLAY_H
#define DISPLAY_H

#include <QLineEdit>

enum dataType_t{
    Uint=0,
    Int,
    Float32,
    Float64
};

class SimuCalc;

class Display : public QLineEdit
{
    Q_OBJECT
        friend class SimuCalc;

    public:
        Display( QWidget* parent=0 );
        ~Display();

        void setResult( QString text );

        void setBits( int bits );
        void setBase( int base );
        void setType( dataType_t type );

        void setValues(QString ops, QList<uint64_t> values );
        void setCurrentValue( uint64_t bitVal );

        void rl( bool continuous );
        void rr( bool continuous );
        void sn();
        void c2();
        void c1();

    public slots:
        void newText( QString text );
        void cursorPosChanged( int oldPos, int cursorPos );

    protected:
      virtual void focusInEvent( QFocusEvent* e );

 static Display* m_Currentdisplay;

    private:
        void updtCurrentValue( uint64_t bitVal );
        QString getValStr( uint64_t val );
        void analizeText( QString text );
        void addVal();

        uint64_t trimToBits( uint64_t val );

        dataType_t m_type;

        int m_bits;
        int m_base;
        int m_valStart;
        int m_currVal;

        //bool m_result;
        bool m_blocked;
        bool m_dot;

        QString m_text;
        QString m_operations;
        QString m_valStr;

        QString m_validChars;
        QString m_validDec;
        QString m_validOct;
        QString m_validHex;
        QString m_validOps;

        QList<uint64_t> m_values;
        QList<int> m_valStarts;

        SimuCalc* m_simuCalc;
};

#endif
