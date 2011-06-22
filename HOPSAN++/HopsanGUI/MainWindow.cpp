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
//! @file   MainWindow.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the HopsanGUI MainWindow class
//!
//$Id$

#include <QDebug>
#include "MainWindow.h"
#include "version.h"
#include "common.h"

#include "Widgets/PlotWidget.h"
#include "Widgets/MessageWidget.h"
#include "Widgets/LibraryWidget.h"
#include "Widgets/ProjectTabWidget.h"
#include "Widgets/PyDockWidget.h"
#include "Widgets/SystemParametersWidget.h"
#include "Widgets/UndoWidget.h"

#include "Dialogs/OptionsDialog.h"
#include "Dialogs/AboutDialog.h"
#include "Dialogs/HelpDialog.h"
#include "Dialogs/WelcomeDialog.h"

#include "UndoStack.h"
#include "Configuration.h"
#include "CopyStack.h"

//! @todo maybe we can make sure that we dont need to include these here
#include "GraphicsView.h"
#include "GUIObjects/GUISystem.h"

//Declaration of global variables
Configuration gConfig;
CopyStack gCopyStack;

//! @brief Constructor for main window
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{    
    //! @todo Much of the stuff here has nothing todo with creating a main window. It should probably be moved to InitializeWorkspace, or somewhere else.

    //Create globals
    gpMainWindow = this;    //First we set the global mainwindow pointer to this, we can (should) only have ONE main window
    gConfig = Configuration();
    gCopyStack = CopyStack();
    mpConfig = &gConfig;        //! @todo Is this pointer variable needed?

    //Set the name and size of the main window
    this->setObjectName("MainWindow");
    int sh = qApp->desktop()->screenGeometry().height();
    int sw = qApp->desktop()->screenGeometry().width();
    this->resize(sw*0.8, sh*0.8);   //Resize window to 80% of screen height and width
    int w = this->size().width();
    int h = this->size().height();
    int x = (sw - w)/2;
    int y = (sh - h)/2;
    this->move(x, y);       //Move window to center of screen
    this->setWindowTitle("Hopsan");
    this->setWindowIcon(QIcon(QString(QString(ICONPATH) + "hopsan.png")));
    this->setDockOptions(QMainWindow::ForceTabbedDocks);
    this->setMouseTracking(true);

    //Create dialogs
    mpAboutDialog = new AboutDialog(this);
    mpHelpDialog = new HelpDialog(this);

    //Create the message widget and its dock
    mpMessageDock = new QDockWidget(tr("Messages"), this);
    mpMessageDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    mpMessageWidget = new MessageWidget(this);
    mpMessageDock->setWidget(mpMessageWidget);
    mpMessageDock->setFeatures(QDockWidget::DockWidgetVerticalTitleBar | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    addDockWidget(Qt::BottomDockWidgetArea, mpMessageDock);
    mpMessageWidget->checkMessages();
    mpMessageWidget->printGUIInfoMessage("HopsanGUI, Version: " + QString(HOPSANGUIVERSION));

    //Create the Python widget
    mpPyDockWidget = new PyDockWidget(this, this);
    mpPyDockWidget->setFeatures(QDockWidget::DockWidgetVerticalTitleBar | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    addDockWidget(Qt::BottomDockWidgetArea, mpPyDockWidget);

    //Load configuration from settings file
    gConfig.loadFromXml();

    //Update style sheet setting
    if(!gConfig.getUseNativeStyleSheet())
    {
        setStyleSheet(gConfig.getStyleSheet());
        setPalette(gConfig.getPalette());
        qApp->setFont(gConfig.getFont());
    }

    //Create the component library widget and its dock
    mpLibDock = new QDockWidget(tr("Component Library"), this);
    mpLibDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mpLibrary = new LibraryWidget(this);
    mpLibDock->setWidget(mpLibrary);
    addDockWidget(Qt::LeftDockWidgetArea, mpLibDock);

    //Set dock widget corner owner
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);

    //Create the central widget for the main window
    mpCentralWidget = new QWidget(this);
    mpCentralWidget->setObjectName("centralwidget");

    //Create the grid layout for the centralwidget
    mpCentralGridLayout = new QGridLayout(mpCentralWidget);
    mpCentralGridLayout->setContentsMargins(4,4,4,4);

    //Create the main tab container, need at least one tab
    mpProjectTabs = new ProjectTabWidget(this);
    mpProjectTabs->setObjectName("projectTabs");
    mpCentralGridLayout->addWidget(mpProjectTabs,0,0,4,4);

    //Initialize the help message popup
    mpHelpPopup = new QWidget(this);
    mpHelpPopupIcon = new QLabel();
    mpHelpPopupIcon->setPixmap(QPixmap(QString(ICONPATH) + "Hopsan-Info.png"));
    mpHelpPopupLabel = new QLabel();
    mpHelpPopupGroupBoxLayout = new QHBoxLayout(mpHelpPopup);
    mpHelpPopupGroupBoxLayout->addWidget(mpHelpPopupIcon);
    mpHelpPopupGroupBoxLayout->addWidget(mpHelpPopupLabel);
    mpHelpPopupGroupBoxLayout->setContentsMargins(3,3,3,3);
    mpHelpPopupGroupBox = new QGroupBox(mpHelpPopup);
    mpHelpPopupGroupBox->setLayout(mpHelpPopupGroupBoxLayout);
    mpHelpPopupLayout = new QHBoxLayout(mpHelpPopup);
    mpHelpPopupLayout->addWidget(mpHelpPopupGroupBox);
    mpHelpPopup->setLayout(mpHelpPopupLayout);
    mpHelpPopup->setBaseSize(100,30);
    mpHelpPopup->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    mpHelpPopup->setStyleSheet("QGroupBox { background-color : rgba(255,255,224,255); } QLabel { margin : 0px; } ");
    mpHelpPopup->hide();
    mpHelpPopupTimer = new QTimer(this);
    connect(mpHelpPopupTimer, SIGNAL(timeout()), mpHelpPopup, SLOT(hide()));

    //Set the correct position of the popup message in the central widget
    mpCentralGridLayout->addWidget(mpHelpPopup, 1,1,1,1);
    mpCentralGridLayout->setColumnMinimumWidth(0,5);
    mpCentralGridLayout->setColumnStretch(0,0);
    mpCentralGridLayout->setColumnStretch(1,0);
    mpCentralGridLayout->setColumnStretch(2,0);
    mpCentralGridLayout->setColumnStretch(3,1);
    mpCentralGridLayout->setRowMinimumHeight(0,25);
    mpCentralGridLayout->setRowStretch(0,0);
    mpCentralGridLayout->setRowStretch(1,0);
    mpCentralGridLayout->setRowStretch(2,1);

    //Create actions, toolbars and menus
    this->createActions();
    this->createToolbars();
    this->createMenus();

    //Set the central widget
    this->setCentralWidget(mpCentralWidget);

    //Create the Statusbar
    mpStatusBar = new QStatusBar();
    mpStatusBar->setObjectName("statusBar");
    this->setStatusBar(mpStatusBar);

    //Create the undo widget and the options dialog
    mpUndoWidget = new UndoWidget(this);
    mpOptionsDialog = new OptionsDialog(this);

    //Load default libraries
    QString componentPath = gExecPath + QString(COMPONENTPATH);
    componentPath.chop(1);
    mpLibrary->loadLibrary(componentPath);
    for(int i=0; i<gConfig.getUserLibs().size(); ++i)
    {
        mpLibrary->loadExternalLibrary(gConfig.getUserLibs().at(i));
    }

    //Create the plot dock widget and hide it
    mpPlotWidgetDock = new QDockWidget(tr("Plot Variables"), this);
    mpPlotWidgetDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mpPlotWidgetDock->hide();
    addDockWidget(Qt::RightDockWidgetArea, mpPlotWidgetDock);

    //Create the system parameters dock widget and hide it
    mpSystemParametersDock = new QDockWidget(tr("System Parameters"), this);
    mpSystemParametersDock->setAllowedAreas((Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea));
    addDockWidget(Qt::RightDockWidgetArea, mpSystemParametersDock);
    mpSystemParametersDock->hide();

    //Create the undo dock widget and hide it
    mpUndoWidgetDock = new QDockWidget(tr("Undo History"), this);
    mpUndoWidgetDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mpUndoWidgetDock->hide();
    addDockWidget(Qt::RightDockWidgetArea, mpUndoWidgetDock);

    //Make dock widgets that share same dock area tabified, instead of stacking them above each other
    tabifyDockWidget(mpPlotWidgetDock, mpSystemParametersDock);
    tabifyDockWidget(mpSystemParametersDock, mpUndoWidgetDock);
    tabifyDockWidget(mpUndoWidgetDock, mpPlotWidgetDock);
    tabifyDockWidget(mpPyDockWidget, mpMessageDock);

    //Create the system parameter widget and hide it
    mpSystemParametersWidget = new SystemParametersWidget(this);
    mpSystemParametersWidget->setVisible(false);

    //Connect tab change slot with toolbars and undo widget
    //! @todo Can't this be done in the creator for ProjectTabWidget?
    connect(mpProjectTabs, SIGNAL(currentChanged(int)), this, SLOT(updateToolBarsToNewTab()));
    connect(mpProjectTabs, SIGNAL(currentChanged(int)), this, SLOT(refreshUndoWidgetList()));
}


//! @brief Destructor
MainWindow::~MainWindow()
{
    delete mpProjectTabs;
    delete menubar;
    delete mpStatusBar;
}


//! @brief Initializes the workspace by either opening specified model, loading last session or showing the Welcome dialog.
void MainWindow::initializeWorkspace()
{
    //Create the plot widget, only once! :)
    mpPlotWidget = new PlotWidget(this);

    //File association - ignore everything else and open the specified file if there is a hmf file in the argument list
    for(int i=0; i<qApp->arguments().size(); ++i)
    {
        if(qApp->arguments().at(i).endsWith(".hmf"))
        {
            mpProjectTabs->closeAllProjectTabs();
            mpProjectTabs->loadModel(qApp->arguments().at(i));
            return;
        }
    }

    if(gConfig.getShowWelcomeDialog())
    {
        mpWelcomeDialog = new WelcomeDialog(this);
        mpWelcomeDialog->show();
    }
    else
    {
        if(!gConfig.getLastSessionModels().empty())
        {
            for(int i=0; i<gConfig.getLastSessionModels().size(); ++i)
            {
                mpProjectTabs->loadModel(gConfig.getLastSessionModels().at(i));
            }
            if(mpProjectTabs->count() != 0) // Failed loading last session models
            {
                mpProjectTabs->getCurrentTab()->mpGraphicsView->centerView();
            }
        }
        else
        {
            updateToolBarsToNewTab();       //This will disable the buttons if last session did not contain any models
        }
    }
}


//! @brief Overloaded function for showing the mainwindow. This is to make sure the view is centered when the program starts.
//! @todo This function is supposed to do something, but doesn't do anything?!
void MainWindow::show()
{
    QMainWindow::show();
    //! @todo this should not be done here should happen when a new tab is created, OK! MainWindow must be shown before center works, maybe we can go through projecttabwidget instead, leaveing it for now
}


//! @brief Opens the plot widget.
void MainWindow::openPlotWidget()
{
    if(mpProjectTabs->count() != 0)
    {
        if(!mpPlotWidgetDock->isVisible())
        {
            mpPlotWidgetDock->setWidget(mpPlotWidget);

            mpPlotWidgetDock->show();
            mpPlotWidgetDock->raise();
            plotAction->setChecked(true);
            connect(mpPlotWidgetDock, SIGNAL(visibilityChanged(bool)), this, SLOT(updatePlotActionButton(bool)));
        }
        else
        {
            mpPlotWidgetDock->hide();
            plotAction->setChecked(false);
        }
    }
}


//! @brief Event triggered re-implemented method that closes the main window.
//! First all tabs (models) are closed, if the user do not push Cancel
//! (closeAllProjectTabs then returns 'false') the event is accepted and
//! the main window is closed.
//! @param event contains information of the closing operation.
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (mpProjectTabs->closeAllProjectTabs())
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }

    //this->saveSettings();
    gConfig.saveToXml();
}


//! @brief Shows the help popup message for 5 seconds with specified message.
//! Any message already being shown will be replaced. Messages can be hidden in advance by calling mpHelpPopup->hide().
//! @param message String with text so show in message
void MainWindow::showHelpPopupMessage(QString message)
{
    if(gConfig.getShowPopupHelp())
    {
        mpHelpPopupLabel->setText(message);
        mpHelpPopup->show();
        mpHelpPopupTimer->stop();
        mpHelpPopupTimer->start(5000);
    }
}


//! @brief Hides the help popup message
void MainWindow::hideHelpPopupMessage()
{
    mpHelpPopup->hide();
}


//! @brief Returns a pointer to the python scripting dock widget.
PyDockWidget *MainWindow::getPythonDock()
{
    return mpPyDockWidget;
}



//! @brief Defines the actions used by the toolbars
void MainWindow::createActions()
{
    newAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-New.png"), tr("&New"), this);
    newAction->setShortcut(tr("New"));
    newAction->setToolTip(tr("Create New Project"));
    connect(newAction, SIGNAL(triggered()), mpProjectTabs, SLOT(addNewProjectTab()));

    openAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Open.png"), tr("&Open"), this);
    openAction->setShortcut(QKeySequence("Ctrl+o"));
    openAction->setToolTip(tr("Load Model File (Ctrl+O)"));
    connect(openAction, SIGNAL(triggered()), mpProjectTabs, SLOT(loadModel()));

    saveAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Save.png"), tr("&Save"), this);
    saveAction->setShortcut(QKeySequence("Ctrl+s"));
    saveAction->setToolTip(tr("Save Model File (Ctrl+S)"));

    saveAsAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-SaveAs.png"), tr("&Save As"), this);
    saveAsAction->setShortcut(QKeySequence("Ctrl+Alt+s"));
    saveAsAction->setToolTip(tr("Save Model File As (Ctrl+Alt+S)"));

    closeAction = new QAction(this);
    closeAction->setText("Close");
    closeAction->setShortcut(QKeySequence("Ctrl+q"));
    connect(closeAction,SIGNAL(triggered()),this,SLOT(close()));

    undoAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Undo.png"), tr("&Undo"), this);
    undoAction->setText("Undo");
    undoAction->setShortcut(QKeySequence(tr("Ctrl+z")));
    undoAction->setToolTip(tr("Undo One Step (Ctrl+Z)"));

    redoAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Redo.png"), tr("&Redo"), this);
    redoAction->setText("Redo");
    redoAction->setShortcut(QKeySequence(tr("Ctrl+y")));
    redoAction->setToolTip(tr("Redo One Step (Ctrl+Y)"));

    openUndoAction = new QAction(tr("&Undo History"), this);
    openUndoAction->setToolTip("Undo History (Ctrl+Shift+U)");
    connect(openUndoAction,SIGNAL(triggered()),this,SLOT(openUndoWidget()));
    openUndoAction->setShortcut(QKeySequence("Ctrl+Shift+u"));

    disableUndoAction = new QAction(tr("&Disable Undo"), this);
    disableUndoAction->setText("Disable Undo");
    disableUndoAction->setCheckable(true);
    disableUndoAction->setChecked(false);

    openSystemParametersAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-SystemParameter.png"), tr("&System Parameters"), this);
    openSystemParametersAction->setToolTip("System Parameters (Ctrl+Shift+Y)");
    openSystemParametersAction->setShortcut(tr("Ctrl+Shift+y"));
    openSystemParametersAction->setCheckable(true);
    connect(openSystemParametersAction,SIGNAL(triggered()),this,SLOT(openSystemParametersWidget()));
    connect(openSystemParametersAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));


    cutAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Cut.png"), tr("&Cut"), this);
    cutAction->setShortcut(tr("Ctrl+x"));
    cutAction->setToolTip(tr("Cut (Ctrl+X)"));

    copyAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Copy.png"), tr("&Copy"), this);
    copyAction->setShortcut(tr("Ctrl+c"));
    copyAction->setToolTip("Copy (Ctrl+C)");

    pasteAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Paste.png"), tr("&Paste"), this);
    pasteAction->setShortcut(tr("Ctrl+v"));
    pasteAction->setToolTip(tr("Paste (Ctrl+V)"));

    simulateAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Simulate.png"), tr("&Simulate"), this);
    simulateAction->setToolTip(tr("Simulate Current Project (Ctrl+Shift+S)"));
    simulateAction->setShortcut(QKeySequence("Ctrl+Shift+s"));
    connect(simulateAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    plotAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Plot.png"), tr("&Plot Variables"), this);
    plotAction->setToolTip(tr("Plot Variables (Ctrl+Shift+P)"));
    plotAction->setCheckable(true);
    plotAction->setShortcut(QKeySequence("Ctrl+Shift+p"));
    connect(plotAction, SIGNAL(triggered()),this,SLOT(openPlotWidget()));
    connect(plotAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    loadLibsAction = new QAction(this);
    loadLibsAction->setText("Load Libraries");
    connect(loadLibsAction,SIGNAL(triggered()),mpLibrary,SLOT(addExternalLibrary()));

    propertiesAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Configure.png"), tr("&Model Properties"), this);
    propertiesAction->setToolTip("Model Properties (Ctrl+Shift+M)");
    propertiesAction->setShortcut(QKeySequence("Ctrl+Shift+m"));
    connect(propertiesAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    optionsAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Options.png"), tr("&Options"), this);
    optionsAction->setToolTip("Options (Ctrl+Shift+O)");
    optionsAction->setShortcut(QKeySequence("Ctrl+Shift+o"));

    alignXAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-AlignX.png"), tr("&Align Vertical (by last selected)"), this);
    alignXAction->setText("Align Vertical");

    alignYAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-AlignY.png"), tr("&Align Horizontal (by last selected)"), this);
    alignYAction->setText("Align Horizontal");

    rotateLeftAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-RotateLeft.png"), tr("&Rotate Left (Ctrl+E)"), this);
    rotateLeftAction->setText("Rotate Left (Ctrl+E)");
    rotateLeftAction->setShortcut(QKeySequence("Ctrl+E"));

    rotateRightAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-RotateRight.png"), tr("&Rotate Right (Ctrl+R)"), this);
    rotateRightAction->setText("Rotate Right (Ctrl+R)");
    rotateRightAction->setShortcut(QKeySequence("Ctrl+R"));

    flipHorizontalAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-FlipHorizontal.png"), tr("&Flip Horizontal"), this);
    flipHorizontalAction->setText("Flip Horizontal (Ctrl+F)");
    flipHorizontalAction->setShortcut(QKeySequence("Ctrl+F"));

    flipVerticalAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-FlipVertical.png"), tr("&Flip Vertical"), this);
    flipVerticalAction->setText("Flip Vertical (Ctrl+D");
    flipVerticalAction->setShortcut(QKeySequence("Ctrl+D"));

    resetZoomAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Zoom100.png"), tr("&Reset Zoom (Ctrl+0)"), this);
    resetZoomAction->setText("Reset Zoom (Ctrl+0)");
    resetZoomAction->setShortcut(QKeySequence("Ctrl+0"));

    zoomInAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-ZoomIn.png"), tr("&Zoom In (Ctrl+Plus)"), this);
    zoomInAction->setText("Zoom In (Ctrl+Plus)");
    zoomInAction->setShortcut(QKeySequence("Ctrl++"));

    zoomOutAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-ZoomOut.png"), tr("&Zoom Out (Ctrl+Minus)"), this);
    zoomOutAction->setText("Zoom Out (Ctrl+Minus)");
    zoomOutAction->setShortcut(QKeySequence("Ctrl+-"));

    centerViewAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-CenterView.png"), tr("&Center View (Ctrl+Space)"), this);
    centerViewAction->setText("Center View (Ctrl+Space)");
    centerViewAction->setShortcut(QKeySequence("Ctrl+Space"));

    QIcon toggleNamesIcon;
    toggleNamesIcon.addFile(QString(ICONPATH) + "Hopsan-ToggleNames.png", QSize(), QIcon::Normal, QIcon::On);
    toggleNamesAction = new QAction(toggleNamesIcon, tr("&Show Component Names (Ctrl+N)"), this);
    toggleNamesAction->setText("Show Component Names (Ctrl+N)");
    toggleNamesAction->setCheckable(true);
    toggleNamesAction->setChecked(gConfig.getToggleNamesButtonCheckedLastSession());
    toggleNamesAction->setShortcut(QKeySequence("Ctrl+n"));

    exportPDFAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-SaveToPDF.png"), tr("&Export To PDF"), this);
    exportPDFAction->setText("Export Model to PDF");

    aboutAction = new QAction(this);
    aboutAction->setText("About");
    connect(aboutAction, SIGNAL(triggered()), mpAboutDialog, SLOT(open()));
    connect(mpAboutDialog->timer, SIGNAL(timeout()), mpAboutDialog, SLOT(update()));

    helpAction = new QAction(this);
    helpAction->setText("User Guide");
    connect(helpAction, SIGNAL(triggered()), mpHelpDialog, SLOT(open()));

    newVersionsAction = new QAction(this);
    newVersionsAction->setText("Check For New Versions");
    connect(newVersionsAction, SIGNAL(triggered()), this, SLOT(openArchiveURL()));

    QIcon togglePortsIcon;
    togglePortsIcon.addFile(QString(ICONPATH) + "Hopsan-TogglePorts.png", QSize(), QIcon::Normal, QIcon::On);
    togglePortsAction = new QAction(togglePortsIcon, tr("&Show Unconnected Ports (Ctrl+T)"), this);
    togglePortsAction->setText("Show Unconnected Ports (Ctrl+T)");
    togglePortsAction->setCheckable(true);
    togglePortsAction->setChecked(gConfig.getTogglePortsButtonCheckedLastSession());
    togglePortsAction->setShortcut(QKeySequence("Ctrl+t"));

    QIcon toggleSignalsIcon;
    toggleSignalsIcon.addFile(QString(ICONPATH) + "Hopsan-ToggleSignal.png", QSize(), QIcon::Normal, QIcon::On);
    toggleSignalsAction = new QAction(toggleSignalsIcon, tr("&Show Signal Components"), this);
    toggleSignalsAction->setText("Show Signal Components");
    toggleSignalsAction->setCheckable(true);
    toggleSignalsAction->setChecked(true);      //! @todo Shall depend on gConfig setting

    saveToWrappedCodeAction = new QAction(this);
    saveToWrappedCodeAction->setShortcut(QKeySequence("Ctrl+Shift+Alt+W"));
    this->addAction(saveToWrappedCodeAction);
    connect(saveToWrappedCodeAction, SIGNAL(triggered()), mpProjectTabs, SLOT(saveCurrentModelToWrappedCode()));

    createSimulinkWrapperAction = new QAction(this);
    createSimulinkWrapperAction->setShortcut(QKeySequence("Ctrl+Shift+Alt+S"));
    this->addAction(createSimulinkWrapperAction);
    connect(createSimulinkWrapperAction, SIGNAL(triggered()), mpProjectTabs, SLOT(createSimulinkWrapperFromCurrentModel()));

    mpStartTimeLineEdit = new QLineEdit("0.0");
    mpStartTimeLineEdit->setMaximumWidth(70);
    mpStartTimeLineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    mpStartTimeLineEdit->setValidator(new QDoubleValidator(-999.0, 999.0, 6, mpStartTimeLineEdit));
    mpTimeStepLineEdit = new QLineEdit("0.001");
    mpTimeStepLineEdit->setMaximumWidth(70);
    mpTimeStepLineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    mpTimeStepLineEdit->setValidator(new QDoubleValidator(0.0, 999.0, 6, mpStartTimeLineEdit));
    mpFinishTimeLineEdit = new QLineEdit("10.0");
    mpFinishTimeLineEdit->setValidator(new QDoubleValidator(-999.0, 999.0, 6, mpFinishTimeLineEdit));
    mpFinishTimeLineEdit->setMaximumWidth(70);
    mpFinishTimeLineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    mpTimeLabelDeliminator1 = new QLabel(tr(" :: "));
    mpTimeLabelDeliminator2 = new QLabel(tr(" :: "));

    connect(mpStartTimeLineEdit, SIGNAL(editingFinished()), SLOT(fixSimulationParameterValues()));
    connect(mpTimeStepLineEdit, SIGNAL(editingFinished()), SLOT(fixSimulationParameterValues()));
    connect(mpFinishTimeLineEdit, SIGNAL(editingFinished()), SLOT(fixSimulationParameterValues()));
}


//! @brief Creates the menus
void MainWindow::createMenus()
{
    //Create the menubar
    menubar = new QMenuBar();
    menubar->setGeometry(QRect(0,0,800,25));
    menubar->setObjectName("menubar");

    //Create the menues
    menuFile = new QMenu(menubar);
    menuFile->setObjectName("menuFile");
    menuFile->setTitle("&File");

    recentMenu = new QMenu(this);
    recentMenu->setTitle("Recent Models");

    menuNew = new QMenu(menubar);
    menuNew->setObjectName("menuNew");
    menuNew->setTitle("New");

    menuSimulation = new QMenu(menubar);
    menuSimulation->setObjectName("menuSimulation");
    menuSimulation->setTitle("&Simulation");

    menuEdit = new QMenu(menubar);
    menuEdit->setTitle("&Edit");

    menuView = new QMenu(menubar);
    menuView->setTitle("&View");

    menuTools = new QMenu(menubar);
    menuTools->setTitle("&Tools");

    menuHelp = new QMenu(menubar);
    menuHelp->setTitle("&Help");

    this->setMenuBar(menubar);

    //Add the actionbuttons to the menues
    newAction->setText("Project");
    menuNew->addAction(newAction);

    menuFile->addAction(menuNew->menuAction());
    menuFile->addAction(openAction);
    menuFile->addAction(saveAction);
    menuFile->addAction(saveAsAction);
    menuFile->addMenu(recentMenu);
    menuFile->addSeparator();
    menuFile->addMenu(menuExport);
    menuFile->addSeparator();
    menuFile->addAction(loadLibsAction);
    menuFile->addSeparator();
    menuFile->addAction(propertiesAction);
    menuFile->addAction(openSystemParametersAction);
    menuFile->addSeparator();
    menuFile->addAction(closeAction);

    this->updateRecentList();

    menuSimulation->addAction(simulateAction);

    menuEdit->addAction(undoAction);
    menuEdit->addAction(redoAction);
    menuEdit->addAction(openUndoAction);
    menuEdit->addAction(disableUndoAction);
    menuEdit->addSeparator();
    menuEdit->addAction(copyAction);
    menuEdit->addAction(cutAction);
    menuEdit->addAction(pasteAction);

    //The View menu shall be alphabetically sorted!
    menuView->addAction(toggleNamesAction);
    menuView->addAction(togglePortsAction);
    menuView->addAction(toggleSignalsAction);
    menuView->addSeparator();
    menuView->addAction(mpLibDock->toggleViewAction());
    menuView->addAction(mpEditToolBar->toggleViewAction());
    menuView->addAction(mpFileToolBar->toggleViewAction());
    menuView->addAction(mpMessageDock->toggleViewAction());
    menuView->addAction(mpPyDockWidget->toggleViewAction());
    menuView->addAction(mpSimToolBar->toggleViewAction());

    menuTools->addAction(optionsAction);
    menuTools->addAction(openSystemParametersAction);

    menuSimulation->addAction(plotAction);

    menuHelp->addAction(helpAction);
    menuHelp->addAction(newVersionsAction);
    menuHelp->addAction(aboutAction);

    menubar->addAction(menuFile->menuAction());
    menubar->addAction(menuEdit->menuAction());
    menubar->addAction(menuTools->menuAction());
    menubar->addAction(menuSimulation->menuAction());
    menubar->addAction(menuView->menuAction());
    menubar->addAction(menuHelp->menuAction());
}

//! @brief Creates the toolbars
void MainWindow::createToolbars()
{
    //File toolbar, contains all file handling stuff (open, save etc)
    mpFileToolBar = addToolBar(tr("File Toolbar"));
    mpFileToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::LeftToolBarArea | Qt::RightToolBarArea);
    mpFileToolBar->addAction(newAction);
    mpFileToolBar->addAction(openAction);
    mpFileToolBar->addAction(saveAction);
    mpFileToolBar->addAction(saveAsAction);
    mpFileToolBar->addAction(exportPDFAction);
    //! @note Action and menu shouldn't be here, but it doesn't work otherwise because the menus are created after the toolbars
    exportToSimulinkAction = new QAction(tr("Export to Simulink S-function Source Files"), this);
    menuExport = new QMenu("Export Model");
    menuExport->addAction(exportToSimulinkAction);
    mpExportButton = new QToolButton(mpFileToolBar);
    mpExportButton->setToolTip("Export");
    mpExportButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Export.png"));
    mpExportButton->setMenu(menuExport);
    mpExportButton->setPopupMode(QToolButton::InstantPopup);
    mpFileToolBar->addWidget(mpExportButton);

    //Simulation toolbar, contains tools for simulationg, plotting and model preferences
    mpSimToolBar = addToolBar(tr("Simulation Toolbar"));
    mpSimToolBar->setAllowedAreas(Qt::TopToolBarArea);
    mpSimToolBar->setAttribute(Qt::WA_MouseTracking);
    mpSimToolBar->addWidget(mpStartTimeLineEdit);
    mpSimToolBar->addWidget(mpTimeLabelDeliminator1);
    mpSimToolBar->addWidget(mpTimeStepLineEdit);
    mpSimToolBar->addWidget(mpTimeLabelDeliminator2);
    mpSimToolBar->addWidget(mpFinishTimeLineEdit);
    mpSimToolBar->addAction(simulateAction);
    mpSimToolBar->addAction(plotAction);
    mpSimToolBar->addAction(propertiesAction);
    mpSimToolBar->addAction(openSystemParametersAction);

    //addToolBarBreak(Qt::TopToolBarArea);

    //Edit toolbar, contains clipboard operations, undo/redo and global options
    mpEditToolBar = new QToolBar(tr("Edit Toolbar"));
    addToolBar(Qt::LeftToolBarArea, mpEditToolBar);
    mpEditToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::LeftToolBarArea | Qt::RightToolBarArea);
    mpEditToolBar->addAction(cutAction);
    mpEditToolBar->addAction(copyAction);
    mpEditToolBar->addAction(pasteAction);
    mpEditToolBar->addAction(undoAction);
    mpEditToolBar->addAction(redoAction);
    mpEditToolBar->addAction(optionsAction);

    //View toolbar, contains all cosmetic and zooming tools
    mpViewToolBar = new QToolBar(tr("View Toolbar"));
    addToolBar(Qt::LeftToolBarArea, mpViewToolBar);
    mpViewToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::LeftToolBarArea | Qt::RightToolBarArea);
    mpViewToolBar->addAction(centerViewAction);
    mpViewToolBar->addAction(resetZoomAction);
    mpViewToolBar->addAction(zoomInAction);
    mpViewToolBar->addAction(zoomOutAction);
    mpViewToolBar->addAction(toggleNamesAction);
    mpViewToolBar->addAction(togglePortsAction);
    mpViewToolBar->addAction(toggleSignalsAction);

    //Tools toolbar, contains all tools used to modify the model
    mpToolsToolBar = new QToolBar(tr("Tools Toolbar"));
    addToolBar(Qt::LeftToolBarArea, mpToolsToolBar);
    mpToolsToolBar->addAction(alignXAction);
    mpToolsToolBar->addAction(alignYAction);
    mpToolsToolBar->addAction(rotateRightAction);
    mpToolsToolBar->addAction(rotateLeftAction);
    mpToolsToolBar->addAction(flipHorizontalAction);
    mpToolsToolBar->addAction(flipVerticalAction);

    connect(exportToSimulinkAction, SIGNAL(triggered()), mpProjectTabs, SLOT(createSimulinkWrapperFromCurrentModel()));
}


//! @brief Opens the undo widget.
void MainWindow::openUndoWidget()
{
    if(!mpUndoWidgetDock->isVisible())
    {
        mpUndoWidgetDock->setWidget(mpUndoWidget);
        mpUndoWidgetDock->show();
        mpUndoWidgetDock->raise();
        mpUndoWidget->refreshList();
    }
}


//! @brief Opens the SystemParametersWidget widget.
void MainWindow::openSystemParametersWidget()
{
    if(!mpSystemParametersDock->isVisible())
    {
        mpSystemParametersDock->setWidget(mpSystemParametersWidget);
        mpSystemParametersDock->show();
        mpSystemParametersDock->raise();
        connect(mpSystemParametersDock, SIGNAL(visibilityChanged(bool)), this, SLOT(updateSystemParametersActionButton(bool)));
    }
    else
    {
        mpSystemParametersDock->hide();
    }
}


//! @brief Opens a recent model
void MainWindow::openRecentModel()
{
    QAction *action = qobject_cast<QAction *>(sender());
    qDebug() << "Trying to open " << action->text();
    if (action)
    {
        mpProjectTabs->loadModel(action->text());
    }
}


void MainWindow::openArchiveURL()
{
    QDesktopServices::openUrl(QUrl("http://www.iei.liu.se/flumes/system-simulation/hopsanng/archive?l=en"));
}


//! @brief Changes the checked setting of plot widget button when plot widget is opened or closed
void MainWindow::updatePlotActionButton(bool)
{
    plotAction->setChecked(mpPlotWidgetDock->isVisible() || !tabifiedDockWidgets(mpPlotWidgetDock).isEmpty());
}


//! @brief Changes the checked setting of system parameters button when the system parameter widget is opened or closed
void MainWindow::updateSystemParametersActionButton(bool)
{
    openSystemParametersAction->setChecked(mpSystemParametersDock->isVisible() || !tabifiedDockWidgets(mpSystemParametersDock).isEmpty());
}


void MainWindow::showToolBarHelpPopup()
{
    QCursor cursor;
    QAction *pHoveredAction = mpSimToolBar->actionAt(mpSimToolBar->mapFromGlobal(cursor.pos()));
    if(pHoveredAction == simulateAction)
    {
        showHelpPopupMessage("Starts a new simlation of current model.");
    }
    else if(pHoveredAction == plotAction)
    {
        showHelpPopupMessage("Opens the list with all available plot variables from current model.");
    }
    else if(pHoveredAction == openSystemParametersAction)
    {
        showHelpPopupMessage("Opens the list of system parameters.");
    }
    else if(pHoveredAction == propertiesAction)
    {
        showHelpPopupMessage("Opens a dialog with settings for the current model.");
    }
}


//! @brief Updates the toolbar values that are tab specific when a new tab is activated
void MainWindow::updateToolBarsToNewTab()
{
    if(mpProjectTabs->count() > 0)
    {
        togglePortsAction->setChecked(!mpProjectTabs->getCurrentTab()->mpSystem->mPortsHidden);
    }

    bool noTabs = !(mpProjectTabs->count() > 0);
    saveAction->setEnabled(!noTabs);
    saveAsAction->setEnabled(!noTabs);
    cutAction->setEnabled(!noTabs);
    copyAction->setEnabled(!noTabs);
    pasteAction->setEnabled(!noTabs);
    undoAction->setEnabled(!noTabs);
    redoAction->setEnabled(!noTabs);
    centerViewAction->setEnabled(!noTabs);
    resetZoomAction->setEnabled(!noTabs);
    zoomInAction->setEnabled(!noTabs);
    zoomOutAction->setEnabled(!noTabs);
    toggleNamesAction->setEnabled(!noTabs);
    togglePortsAction->setEnabled(!noTabs);
    exportPDFAction->setEnabled(!noTabs);
    alignXAction->setEnabled(!noTabs);
    alignYAction->setEnabled(!noTabs);
    rotateLeftAction->setEnabled(!noTabs);
    rotateRightAction->setEnabled(!noTabs);
    flipHorizontalAction->setEnabled(!noTabs);
    flipVerticalAction->setEnabled(!noTabs);
    mpStartTimeLineEdit->setEnabled(!noTabs);
    mpTimeStepLineEdit->setEnabled(!noTabs);
    mpFinishTimeLineEdit->setEnabled(!noTabs);
    simulateAction->setEnabled(!noTabs);
    plotAction->setEnabled(!noTabs);
    propertiesAction->setEnabled(!noTabs);
    openSystemParametersAction->setEnabled(!noTabs);
}


//! @brief Slot that calls refresh list function in undo widget. Used because undo widget cannot have slots.
void MainWindow::refreshUndoWidgetList()
{
    mpUndoWidget->refreshList();
}


//! @brief Registers a recently opened model file in the "Recent Models" list
void MainWindow::registerRecentModel(QFileInfo model)
{
    if(model.fileName() == "")
        return;

    gConfig.addRecentModel(model.filePath());
    updateRecentList();
}


//! @brief Updates the "Recent Models" list
void MainWindow::updateRecentList()
{
    recentMenu->clear();

    recentMenu->setEnabled(!gConfig.getRecentModels().empty());
    if(!gConfig.getRecentModels().empty())
    {
        for(int i=0; i<gConfig.getRecentModels().size(); ++i)
        {
            if(gConfig.getRecentModels().at(i) != "")
            {
                QAction *tempAction;
                tempAction = recentMenu->addAction(gConfig.getRecentModels().at(i));
                disconnect(recentMenu, SIGNAL(triggered(QAction *)), mpProjectTabs, SLOT(loadModel(QAction *)));    //Ugly hack to make sure connecetions are not made twice (then program would try to open model more than once...)
                connect(tempAction, SIGNAL(triggered()), this, SLOT(openRecentModel()));
            }
        }
    }
}


void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    hideHelpPopupMessage();

    QMainWindow::mouseMoveEvent(event);
}

//! @brief Sets a new startvalue.
//! @param startTime is the new value
void MainWindow::setStartTimeInToolBar(double startTime)
{
    QString valueTxt;
    valueTxt.setNum(startTime, 'g', 6 );
    mpStartTimeLineEdit->setText(valueTxt);
    fixTimeStep();
    fixFinishTime();
}


//! @brief Sets a new timestep.
//! @param timeStep is the new value
void MainWindow::setTimeStepInToolBar(double timeStep)
{
    QString valueTxt;
    valueTxt.setNum(timeStep, 'g', 6 );
    mpTimeStepLineEdit->setText(valueTxt);
    fixTimeStep();
    fixFinishTime();
}


//! @brief Sets a new finish value.
//! @param finishTime is the new value
void MainWindow::setFinishTimeInToolBar(double finishTime)
{
    QString valueTxt;
    valueTxt.setNum(finishTime, 'g', 6 );
    mpFinishTimeLineEdit->setText(valueTxt);
    fixTimeStep();
    fixFinishTime();
}


//! @brief Access function to the starttimelabel value.
//! @returns the starttime value
double MainWindow::getStartTimeFromToolBar()
{
    return mpStartTimeLineEdit->text().toDouble();
}


//! @brief Access function to the timesteplabel value.
//! @returns the timestep value
double MainWindow::getTimeStepFromToolBar()
{
    return mpTimeStepLineEdit->text().toDouble();
}


//! @brief Access function to the finishlabel value.
//! @returns the finish value
double MainWindow::getFinishTimeFromToolBar()
{
    return mpFinishTimeLineEdit->text().toDouble();
}


//! @brief Make sure the values make sens.
//! @see fixTimeStep()
void MainWindow::fixSimulationParameterValues()
{
    fixFinishTime();
    fixTimeStep();
}


//! @brief Make sure that the finishs time of the simulation is not smaller than start time.
//! @see fixTimeStep()
//! @see fixLabelValues()
void MainWindow::fixFinishTime()
{
    if (getFinishTimeFromToolBar() < getStartTimeFromToolBar())
        setFinishTimeInToolBar(getStartTimeFromToolBar());
}


//! @brief Make sure that the timestep is in the right range i.e. not larger than the simulation time.
//! @see fixFinishTime()
//! @see fixLabelValues()
void MainWindow::fixTimeStep()
{
    //! @todo Maybe more checks, i.e. the time step should be even divided into the simulation time.
    if (getTimeStepFromToolBar() > (getFinishTimeFromToolBar() - getStartTimeFromToolBar()))
        setTimeStepInToolBar(getFinishTimeFromToolBar() - getStartTimeFromToolBar());

    if (mpProjectTabs->count() > 0)
    {
        mpProjectTabs->getCurrentTab()->mpSystem->getCoreSystemAccessPtr()->setDesiredTimeStep(getTimeStepFromToolBar());
    }
}
