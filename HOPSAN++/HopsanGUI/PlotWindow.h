//!
//! @file   PlotWindow.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the PlotWindow class
//!
//$Id$

#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include <QtGui>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>

class MainWindow;
class VariablePlot;
class VariablePlotZoomer;
class PlotParameterTree;
class PlotWidget;
class GUISystem;

class PlotWindow : public QMainWindow
{
    Q_OBJECT
    friend class PlotWidget;
public:
    PlotWindow(PlotParameterTree *PlotParameterTree, MainWindow *parent);
    void addPlotCurve(QVector<double> xarray, QVector<double> yarray, QString componentName, QString portName, QString dataName, QString dataUnit, int axisY);
    void addPlotCurve(QString componentName, QString portName, QString dataName);
    void changeXVector(QVector<double> xarray, QString componentName, QString portName, QString dataName, QString dataUnit);
    void setGeneration(int gen);

    //MainWindow *mpParentMainWindow;
    GUISystem *mpCurrentGUISystem;

protected:
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragLeaveEvent(QDragLeaveEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);
    virtual void contextMenuEvent(QContextMenuEvent *event);
    virtual void closeEvent(QCloseEvent *);

public slots:
    void discardGeneration();
    void discardOldestGeneration();
    void setUnit(int yAxis, QString physicalQuantity, QString selectedUnit);
    void enableZoom(bool);
    void enablePan(bool);
    void exportSVG();
    void exportGNUPLOT();
    void importGNUPLOT();
    void enableGrid(bool);
    void setLineWidth(int);
    void setLineColor();
    void setBackgroundColor();
    void checkNewValues();
    void setAutoUpdate(bool value);
    void stepBack();
    void stepForward();
    void saveToXml();
    bool saveToHmpf(QString fileName);
    void close();

private:

    bool mHasLeftCurve;
    bool mHasRightCurve;

    QVector<QwtPlotCurve *> mpCurves;
    QList<QStringList> mCurveParameters;
    QStringList mSpecialXParameter;
    PlotParameterTree *mpPlotParameterTree;
    QMap<QString, QString> mCurrentUnitsLeft;
    QMap<QString, QString> mCurrentUnitsRight;
    QwtPlot *mpVariablePlot;
    QwtPlotGrid *mpGrid;
    QwtSymbol *mpMarkerSymbol;
    QwtPlotMarker *mpActiveMarker;

    QVector <QwtPlotMarker *> mpMarkers;
    QHash <QwtPlotCurve *, QwtPlotMarker *> mCurveToMarkerMap;
    QHash <QwtPlotMarker *, QwtPlotCurve *> mMarkerToCurveMap;
    QwtPlotCurve *tempCurve;
    QwtPlotZoomer *mpZoomer;
    QwtPlotZoomer *mpZoomerRight;
    QwtPlotMagnifier *mpMagnifier;
    QwtPlotPanner *mpPanner;
    int mCurrentGeneration;

    QToolBar *mpToolBar;
    QToolButton *mpZoomButton;
    QToolButton *mpPanButton;
    QToolButton *mpSaveButton;
    QToolButton *mpSVGButton;
    QToolButton *mpExportGNUPLOTButton;
    QToolButton *mpImportGNUPLOTButton;
    QToolButton *mpGridButton;
    QToolButton *mpPreviousButton;
    QToolButton *mpNextButton;
    QToolButton *mpDiscardGenerationButton;
    QToolBar *mpSizeButton;
    QSpinBox *mpSizeSpinBox;
    QToolButton *mpColorButton;
    QToolButton *mpBackgroundColorButton;
    QCheckBox *mpAutoUpdateCheckBox;
    QLabel *mpSizeLabel;
    QLabel *mpGenerationLabel;

    QList< QList< QVector<double> > > mVectorX;
    QList< QList< QVector<double> > > mVectorY;

    QRubberBand *mpHoverRect;

    QString mUnitLeft;
    QString mUnitRight;

    int nCurves;
    QStringList mCurveColors;
    bool mHasSpecialXAxis;
    bool mRightAxisLogarithmic;
    bool mLeftAxisLogarithmic;
    bool mAutoUpdate;

    void insertMarker(QwtPlotCurve *curve);
    void setActiveMarker(QwtPlotMarker *marker);
};


//! Stuff bellow are for new plot window


class PlotCurve;
class PlotInfoBox;

//! @brief Tab widget for plots in plot window
//! @todo Not sure if this is needed
class PlotTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    PlotTabWidget(QWidget *parent);
};


//! @brief Plot window tab containing a plot area with plot curves
class PlotTab : public QWidget
{
    Q_OBJECT
public:
    PlotTab(PlotTabWidget *parent);
private:
    QList<PlotCurve> mPlotCurves;
    QVector<double> mSpecialXAxis;
    QColor mBackgroundColor;
    bool mShowGrid();
};


//! @brief Class describing a plot curve in plot window
class PlotCurve
{
public:
    PlotCurve(int generation, QString componentName, QString portName, QString dataName, PlotTab *parent);
    int getGeneration();
    QString getComponentName();
    QString getPortName();
    QString getDataName();
    QString getDataUnit();
    QVector<double> getDataVector();
    QVector<double> getTimeVector();
    void setGeneration(int generation);
    void setDataUnit();
    void setLineWidth(int);
    void setLineColor(QString colorName);
private:
    int mGeneration;
    QString mComponentName;
    QString mPortName;
    QString mDataName;
    QString mDataUnit;
    QVector<double> mDataVector;
    QVector<double> mTimeVector;
    QString mLineColorName;
    int mLineWidth;
    int mAxisY;
    PlotInfoBox *mpPlotInfoBox;
};


//! @todo Use the already created one
class PlotInfoBox : public QWidget
{
    Q_OBJECT
public:
    PlotInfoBox();
};

#endif // PLOTWINDOW_H
