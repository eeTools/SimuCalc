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

#ifndef OSCWIDGET_H
#define OSCWIDGET_H


#include <QWidget>
#include <QButtonGroup>

#include "ui_simucalc.h"

class QSettings;
class MainWindow;

class SimuCalc : public QWidget, private Ui::SimuCalc
{
        Q_OBJECT

    public:
        SimuCalc( MainWindow* mainW );
        ~SimuCalc();

        void loadSettings();

        void newBitVal( uint64_t val );

        void dispChanged( Display* d );

        void textChanged();

        void setValues( QString text, QList<uint64_t> values );

    private slots:
        void bitToggled( QAbstractButton* aBut, bool state );
        void keyPressed( int );
        void opPressed( int index );
        void calculate();

        void undo();
        void redo();

        void del();
        void bck();
        void fwd();
        void bwd();
        void brl();
        void brr();

        void rl();
        void rr();
        void sn();
        void c2();
        void c1();
        void c();

    public slots:
        void on_typeBox_currentIndexChanged( int index );
        void on_bitsBox_currentIndexChanged( int index );
        void on_hexCheck_stateChanged( int state );
        void on_octCheck_stateChanged( int state );

    private:
        void appendUndo();
        void updateBits();
        void setType( dataType_t type );
        void setBits( int bits );

        void enableDisplay( Display* d, bool en );

        bool m_blocked;
        bool m_cRot;

        double   m_doubVal;
        float    m_floatVal;
        int64_t  m_intVal;
        uint64_t m_bitVal;

        QString m_decVal;
        QString m_hexVal;
        QString m_octVal;

        int m_base;
        int m_nBits;
        dataType_t m_type;

        QFont m_font;

        QButtonGroup m_bits;
        QButtonGroup m_keys;
        QButtonGroup m_ops;

        QList<QGroupBox*> m_bytes;
        Display* m_activeDisp;

        QString m_resultStr;
        QStringList m_undoStack;
        QStringList m_redoStack;

        MainWindow* m_mainWindow;
};
#endif
