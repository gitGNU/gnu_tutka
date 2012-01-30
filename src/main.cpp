/*
 * main.cpp
 *
 * Copyright 2002-2011 vesuri
 *
 * This file is part of Tutka2.
 *
 * Tutka2 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Tutka2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Tutka2; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <QApplication>
#include "song.h"
#include "midi.h"
#include "player.h"
#include "gui.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    MIDI *midi = new MIDI;
    Player *player = new Player(midi);
    GUI *gui = new GUI(player);
    Song *song = new Song(argc > 1 ? argv[1] : QString());
    player->setSong(song);
    gui->show();
    gui->raise();

    int returnCode = app.exec();

    delete gui;
    delete player;
    delete midi;
    delete song;

    return returnCode;
}
