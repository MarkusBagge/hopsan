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
//! @file   CoreAccess.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the HopsanCore Qt API classes for communication with the HopsanCore
//!
//$Id$

#ifndef GUIROOTSYSTEM_H
#define GUIROOTSYSTEM_H

#include <QString>
#include <QVector>
#include <QPair>
#include <QStringList>

//Forward declaration of hopsan core classes
namespace hopsan {
class ComponentSystem;
class Port;
class SimulationHandler;
}


class CoreLibraryAccess
{
public:
    bool hasComponent(QString componentName);
    bool loadComponentLib(QString fileName);
    bool unLoadComponentLib(QString fileName);
    bool reserveComponentTypeName(const QString typeName);
    void getLoadedLibNames(QVector<QString> &rLibNames);
};

class CoreMessagesAccess
{
public:
    size_t getNumberOfMessages();
    void getMessage(QString &rMessage, QString &rType, QString &rTag);
};

class CoreParameterData
{
public:
    CoreParameterData() : mIsDynamic(false), mIsEnabled(true) {}
    CoreParameterData(const QString name, const QString value, const QString type, const QString unit="", const QString desc="")
        : mName(name), mValue(value), mType(type), mUnit(unit), mDescription(desc), mIsDynamic(false), mIsEnabled(true) {}

    QString mName;
    QString mValue;
    QString mType;
    QString mUnit;
    QString mDescription;
    bool    mIsDynamic;
    bool    mIsEnabled;
};

//Forward declaration
class CoreSimulationHandler;

class CoreSystemAccess
{
    friend class CoreSimulationHandler;
public:
    enum PortTypeIndicatorT {INTERNALPORTTYPE, ACTUALPORTTYPE, EXTERNALPORTTYPE};

    CoreSystemAccess(QString name=QString(), CoreSystemAccess* pParentCoreSystemAccess=0);
    ~CoreSystemAccess();
    void deleteRootSystemPtr(); //!< @todo This is very strange, needed becouse core systems are deleted from parent if they are subsystems (not if root systems), this is the only way to safely delete the core object

    QString getHopsanCoreVersion();

    // Name and type functions
    //! @todo maybe we should use name="" (empty) to indicate root system instead, to cut down on the number of functions
    QString getSystemTypeCQS();
    QString getSubComponentTypeCQS(QString componentName);

    QString setSystemName(QString name);
    QString getSystemName();
    QString renameSubComponent(QString componentName, QString name);

    // Parameters and start values
    QStringList getParameterNames(QString componentName);
    void getParameters(QString componentName, QVector<CoreParameterData> &rParameterDataVec);
    void getParameter(QString componentName, QString parameterName, CoreParameterData &rData);
    QString getParameterValue(QString componentName, QString parameterName);
    //void getStartValueDataNamesValuesAndUnits(QString componentName, QString portName, QVector<QString> &rNames, QVector<QString> &rStartDataValuesTxt, QVector<QString> &rUnits);
    bool setParameterValue(QString componentName, QString parameterName, QString value, bool force=0);

    // Port Functions
    QString getPortType(const QString componentName, const QString portName, const PortTypeIndicatorT portTypeIndicator=ACTUALPORTTYPE);
    QString getNodeType(QString componentName, QString portName);
    bool isPortConnected(QString componentName, QString portName);

    // Component creation and removal
    QString createComponent(QString type, QString name="");
    QString createSubSystem(QString name="");
    void removeSubComponent(QString componentName, bool doDelete);
    QString reserveUniqueName(QString desiredName);
    void unReserveUniqueName(QString name);

    // Component connection and disconnection
    bool connect(QString compname1, QString portname1, QString compname2, QString portname2);
    bool disconnect(QString compname1, QString portname1, QString compname2, QString portname2);

    // Simualtion functions
    bool isSimulationOk();
    bool initialize(double mStartTime, double mFinishTime, int nSamples=2048);
    void simulate(double mStartTime, double mFinishTime, int nThreads=-1, bool modelHasNotChanged=false);
    void finalize();
    double getCurrentTime();
    void stop();
    bool writeNodeData(const QString compname, const QString portname, const QString dataname, double data);

    // System settings
    bool doesKeepStartValues();
    void setLoadStartValues(bool load);

    void setDesiredTimeStep(double timestep);
    void setInheritTimeStep(bool inherit);
    bool doesInheritTimeStep();
    double getDesiredTimeStep();

    // System Port Functions
    void deleteSystemPort(QString portname);
    QString addSystemPort(QString portname);
    QString renameSystemPort(QString oldname, QString newname);

    // System Parameter Functions
    QStringList getSystemParameterNames();
    void getSystemParameters(QVector<CoreParameterData> &rParameterDataVec);
    void getSystemParameter(const QString name, CoreParameterData &rParameterData);
    QString getSystemParameterValue(const QString name);
    bool setSystemParameter(const CoreParameterData &rParameter, bool force=false);
    bool setSystemParameterValue(QString name, QString value, bool force=false);
    bool hasSystemParameter(const QString name);
    bool renameSystemParameter(const QString oldName, const QString newName);
    void removeSystemParameter(const QString name);

    // Simulation results data retrieval
    void getPlotDataNamesAndUnits(const QString compname, const QString portname, QVector<QString> &rNames, QVector<QString> &rUnits);
    std::vector<double> getTimeVector(QString componentName, QString portName);
    void getPlotData(const QString compname, const QString portname, const QString dataname, QPair<QVector<double>, QVector<double> > &rData);
    bool havePlotData(const QString compname, const QString portname, const QString dataname);
    bool getLastNodeData(const QString compname, const QString portname, const QString dataname, double& rData);
    double *getNodeDataPtr(const QString compname, const QString portname, const QString dataname);

private:
    hopsan::ComponentSystem *getCoreSystemPtr();
    hopsan::ComponentSystem *getCoreSubSystemPtr(QString name);
    hopsan::Port* getCorePortPtr(QString componentName, QString portName);

    hopsan::ComponentSystem *mpCoreComponentSystem;
};

class CoreSimulationHandler
{
public:
    //! @todo a doitall function
    bool initialize(const double startTime, const double stopTime, const int nLogSamples, CoreSystemAccess* pCoreSystemAccess);
    bool initialize(const double startTime, const double stopTime, const int nLogSamples, QVector<CoreSystemAccess*> &rvCoreSystemAccess);

    void simulate(const double startTime, const double stopTime, const int nThreads, CoreSystemAccess* pCoreSystemAccess, bool modelHasNotChanged=false);
    void simulate(const double startTime, const double stopTime, const int nThreads, QVector<CoreSystemAccess*> &rvCoreSystemAccess, bool modelHasNotChanged=false);

    void finalize(CoreSystemAccess* pCoreSystemAccess);
    void finalize(QVector<CoreSystemAccess*> &rvCoreSystemAccess);

private:


};

#endif // GUIROOTSYSTEM_H
