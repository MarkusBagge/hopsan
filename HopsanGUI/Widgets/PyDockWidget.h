/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   PyDockWidget.h
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-09-21
//!
//! @brief Contains a derived QDockWidget class that contain a Python console
//!
//$Id$

#ifndef PYDOCKWIDGET_H
#define PYDOCKWIDGET_H

#include <QtGui>
#include <QtXml>

// Forward declarations
class MainWindow;
class PythonQtScriptingConsole;

#ifdef USEPYTHONQT
class PyDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    PyDockWidget(MainWindow *pMainWindow, QWidget * parent = 0);
    QString getScriptFileName();
    void saveSettingsToDomElement(QDomElement &rDomElement);
    QString getLastOutput();
    QString runCommand(QString command);

public slots:
    void runPyScript();
    void runPyScript(QString command);
    void runMultipleCommands(QString command, int n);
    void optimize();

private slots:
    void loadPyScript();
    void openInitScriptDialog();
    void setInitScriptFromDialog();

private:
    PythonQtScriptingConsole *mpPyConsole;
    QLineEdit *mpScriptFileLineEdit;
    QToolButton *mpInitScriptButton;
    QToolButton *mpLoadScriptButton;
    QString mInitScript;
    QDialog *mpDialog;
    QTextEdit *mpTextEdit;
};


class PyWidget : public QWidget
{
    Q_OBJECT

public:
    PyWidget(QWidget * parent = 0);
    QSize sizeHint() const;
};
#endif

#endif // PYDOCKWIDGET_H