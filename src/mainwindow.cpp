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

#include <QApplication>
#include <QStyleFactory>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>
//#include <QComboBox>
//#include <QSettings>

#include "mainwindow.h"

MainWindow::MainWindow()
          : m_simuCalc( this )
{
    setWindowIcon( QIcon(":/simucalc-128.png") );

    m_version = "Simucalc-"+QString( APP_VERSION );
    this->setWindowTitle( m_version );

    QApplication::setStyle( QStyleFactory::create("Fusion") );

    setCentralWidget( &m_simuCalc );

    QString settingsPath = QStandardPaths::writableLocation( QStandardPaths::DataLocation )+"/simucalc.ini";
    m_settings = new QSettings( settingsPath, QSettings::IniFormat, this );

    if( QFile::exists( settingsPath ) )
    {
        restoreGeometry( m_settings->value("geometry" ).toByteArray() );
        restoreState(    m_settings->value("windowState" ).toByteArray());

        m_simuCalc.loadSettings();
    }
    else this->resize( 350, 450 );
}
MainWindow::~MainWindow()
{
    m_settings->setValue("geometry", saveGeometry() );
    m_settings->setValue("windowState", saveState() );
}
