/* 
 * Copyright (C) 2020 Christian Kröner, University of Warwick 
 *
 * This file is part of SeqFileProcessing2D.
 *
 * SeqFileProcessing2D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SeqFileProcessing2D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with SeqFileProcessing2D.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <QApplication>
#include <QCommandLineParser>
#include "MainWindow.h"
#include <iostream>
int main ( int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("SeqFileProcessing2D");
    app.setOrganizationDomain("http://www2.warwick.ac.uk/fac/sci/eng/");
    app.setApplicationName("SeqFileProcessing2D");
    
    QApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Segment Seq Files");
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption seqFileOption(QStringList() << "i" << "seq",
                                     QCoreApplication::translate("main", "Load file <iSeq.seq>."),
                                     QCoreApplication::translate("main", "seqFile"));
    parser.addOption(seqFileOption);
    QCommandLineOption posFileOption(QStringList() << "p" << "positions",
                                     QCoreApplication::translate("main", "Load positions file <iSeq.dat>."),
                                     QCoreApplication::translate("main", "posFile"));
    parser.addOption(posFileOption);
    QCommandLineOption trkFileOption(QStringList() << "t" << "tracks",
                                     QCoreApplication::translate("main", "Load tracks file <iSeq.dat>."),
                                     QCoreApplication::translate("main", "trkFile"));
    parser.addOption(trkFileOption);

    QCommandLineOption sFileOption(QStringList() << "s" << "settings",
                                     QCoreApplication::translate("main", "Load custom setting from file <settings.ini>."),
                                     QCoreApplication::translate("main", "sFile"));
    parser.addOption(sFileOption);

    QCommandLineOption debFileOption(QStringList() << "d" << "debug",
                                     QCoreApplication::translate("main", "Write Debug info to <debug.log>."),
                                     QCoreApplication::translate("main", "debFile"));
    parser.addOption(debFileOption);

    QCommandLineOption projFileOption(QStringList() << "x" << "project",
                                     QCoreApplication::translate("main", "Load Project <proj.yml>."),
                                     QCoreApplication::translate("main", "projFile"));
    parser.addOption(projFileOption);

    QCommandLineOption forceOption(QStringList() << "n" << "non-native",
            QCoreApplication::translate("main", "Select QT's own file dialog."));
    parser.addOption(forceOption);
    
    parser.process(app);
    QStringList seqFile = parser.values(seqFileOption);
    QStringList posFile = parser.values(posFileOption);
    QStringList trkFile = parser.values(trkFileOption);
    QStringList debFile = parser.values(debFileOption);
    QStringList sFile = parser.values(sFileOption);
    QStringList args = parser.positionalArguments();
    QStringList projFile = parser.values(projFileOption);

    MainWindow mw;
    
    if(debFile.size()>0)
      if(!debFile.at(0).isEmpty()) 
        mw.setDebug(debFile.at(0));
    
    if(seqFile.size()>0)
    {
    for(int i = 0; i <seqFile.size();i++)
    {
        if(!seqFile.at(i).isEmpty()) 
        mw.loadFile(seqFile.at(i));
        if(i<posFile.size())
         if(!posFile.at(i).isEmpty())        
          mw.loadPositionsFile(posFile.at(i));
        if(i<trkFile.size())
         if(!trkFile.at(i).isEmpty())
          mw.loadTracksFile(trkFile.at(i));
    }
    }

    if(args.size()>0)
        for(int i = 0; i <args.size();i++)
            if(!args.at(i).isEmpty()) 
                mw.loadFile(args.at(i));

    if(sFile.size()>0)
        for(int i = 0; i <sFile.size();i++)
           mw.customSettingsFromFile(sFile.at(i));
    
    mw.setNative(parser.isSet(forceOption));

    if(projFile.size()>0)
    {
     for(int i = 0; i <projFile.size();i++)
     {
         if(!projFile.at(i).isEmpty()) 
         mw.loadProject(projFile.at(i));
     }
    }

    mw.show();

    return app.exec();
}
