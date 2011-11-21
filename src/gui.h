/*
 * gui.h
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

#ifndef GUI_H_
#define GUI_H_

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class Editor;

class GUI : public QMainWindow
{
    Q_OBJECT

public:
    explicit GUI(Editor *editor, QWidget *parent = 0);
    virtual ~GUI();

    void refreshAll();

protected:
#ifdef TODO
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
#endif

private:
    Editor *editor;
    Ui::MainWindow *mainWindow;
};

#endif /* GUI_H_ */
