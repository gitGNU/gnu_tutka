/*
 * main.cpp
 *
 * Copyright 2002-2016 Vesa Halttunen
 *
 * This file is part of Tutka.
 *
 * Tutka is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Tutka is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Tutka; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <QApplication>
#include <QTranslator>
#include <QLocale>
#ifdef __APPLE__
#include "coremidi.h"
#define HostMIDI CoreMIDI
#elif __linux
#include "alsamidi.h"
#define HostMIDI AlsaMIDI
#else
#include "midi.h"
#define HostMIDI MIDI
#endif
#include "midiinterface.h"
#include "player.h"
#include "scheduler.h"
#include "mainwindow.h"
#include <signal.h>
#include <dlfcn.h>

#ifndef sighandler_t
#define sighandler_t sig_t
#endif

sighandler_t originalSigIntHandler = NULL;
sighandler_t originalSigTermHandler = NULL;

void quitSignalHandler(int)
{
    if (qApp != NULL) {
        qApp->quit();
    }
}

void installSignalHandlers()
{
    originalSigIntHandler = signal(SIGINT, quitSignalHandler);
    originalSigTermHandler = signal(SIGTERM, quitSignalHandler);
}

void restoreSignalHandlers()
{
    signal(SIGINT, originalSigIntHandler);
    signal(SIGTERM, originalSigTermHandler);
}

int runWithGUI(int argc, char **argv)
{
    QApplication app(argc, argv);
    QTranslator translator;
    translator.load(QString("/usr/share/tutka/translations/tutka_") + QLocale::system().name());
    app.installTranslator(&translator);

    MIDI *midi = new HostMIDI;
    Player *player = new Player(midi, argc > 1 ? argv[1] : QString());
    MainWindow *mainWindow = new MainWindow(player);
    mainWindow->show();
    mainWindow->raise();

    int returnCode = app.exec();

    delete mainWindow;
    delete player;
    delete midi;

    return returnCode;
}

int runWithoutGUI(int argc, char **argv)
{
    // Remove the -p argument
    for (int i = 2; i < argc; i++) {
        argv[i - 1] = argv[i];
    }
    argc--;

    installSignalHandlers();

    QCoreApplication app(argc, argv);
    MIDI *midi = new HostMIDI;
    Player *player = new Player(midi, argv[1]);
    player->setScheduler(Scheduler::schedulers().last());
    for (int output = 0; output < midi->outputs(); output++) {
        midi->output(output)->setEnabled(true);
    }

    app.processEvents();

    // Let the potentially sent SysEx messages take effect before playing
    usleep(1000000);

    player->playSong();

    int returnCode = app.exec();

    player->stopAllNotes();
    app.processEvents();

    delete player;
    delete midi;

    restoreSignalHandlers();

    return returnCode;
}

int main(int argc, char **argv)
{
    if (argc > 2 && strcmp(argv[1], "-p") == 0) {
        return runWithoutGUI(argc, argv);
    } else {
        return runWithGUI(argc, argv);
    }
}
