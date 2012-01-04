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
//! @file   loadFunctions.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains functions used when models are loaded from hmf
//!
//$Id$

#include "loadObjects.h"

#include "GUIObjects/GUIModelObject.h"
#include "GUIObjects/GUIContainerObject.h"
#include "GUIObjects/GUISystem.h"
#include "GUIConnector.h"
#include "GUIPort.h"
#include "Widgets/LibraryWidget.h"
#include "MainWindow.h"
#include "UndoStack.h"
#include "Widgets/MessageWidget.h"
#include "Utilities/GUIUtilities.h"

#include <QMap>


//! @brief Loads a Connector from the supplied load data
//! @param[in] rDomElement The DOM element to load from
//! @param[in] pContainer The Container Object to load into
//! @param[in] undoSettings Wheter or not to register undo for the operation
bool loadConnector(QDomElement &rDomElement, GUIContainerObject* pContainer, undoStatus undoSettings)
{
    // -----First read from DOM element-----
    QString startComponentName, endComponentName, startPortName, endPortName;
    bool isDashed;
    QVector<QPointF> pointVector;
    QStringList geometryList;

    // Read core specific stuff
    startComponentName  = rDomElement.attribute(HMF_CONNECTORSTARTCOMPONENTTAG);
    startPortName       = rDomElement.attribute(HMF_CONNECTORSTARTPORTTAG);
    endComponentName    = rDomElement.attribute(HMF_CONNECTORENDCOMPONENTTAG);
    endPortName         = rDomElement.attribute(HMF_CONNECTORENDPORTTAG);
    //qDebug() << "loadConnector: " << startComponentName << " " << startPortName << " " << endComponentName << " " << endPortName;

    // Read gui specific stuff
    qreal x,y;
    QDomElement guiData         = rDomElement.firstChildElement(HMF_HOPSANGUITAG);
    QDomElement guiCoordinates  = guiData.firstChildElement(HMF_COORDINATES);
    QDomElement coordTag        = guiCoordinates.firstChildElement(HMF_COORDINATETAG);
    while (!coordTag.isNull())
    {
        parseCoordinateTag(coordTag, x, y);
        pointVector.push_back(QPointF(x,y));
        coordTag = coordTag.nextSiblingElement(HMF_COORDINATETAG);
    }
    QDomElement guiGeometries   = guiData.firstChildElement(HMF_GEOMETRIES);
    QDomElement geometryTag     = guiGeometries.firstChildElement(HMF_GEOMETRYTAG);
    while (!geometryTag.isNull())
    {
        geometryList.append(geometryTag.text());
        geometryTag = geometryTag.nextSiblingElement(HMF_GEOMETRYTAG);
    }
    QDomElement styleTag = guiData.firstChildElement(HMF_STYLETAG);
    if(!styleTag.isNull())
    {
        isDashed = (styleTag.text() == "dashed");
    }
    else
    {
        isDashed = false;
    }

    // -----Now establish the connection-----
    bool success=false;
    GUIPort *startPort = pContainer->getGUIModelObjectPort(startComponentName, startPortName);
    GUIPort *endPort = pContainer->getGUIModelObjectPort(endComponentName, endPortName);
    if ((startPort != 0) && (endPort != 0))
    {
        Connector* pConn = pContainer->createConnector(startPort, endPort, NOUNDO);
        if (pConn != 0)
        {
            pConn->setPointsAndGeometries(pointVector, geometryList);
            pConn->setDashed(isDashed);
            pConn->refreshConnectorAppearance();

            if(undoSettings == UNDO)
            {
                pContainer->getUndoStackPtr()->registerAddedConnector(pConn);
            }
            success = true;
        }
    }

    if (!success)
    {
        QString str;
        str = QString("Failed to load connector between: ") + startComponentName + QString("->") + startPortName
            + QString(" and ") + endComponentName + QString("->") + endPortName;
        gpMainWindow->mpMessageWidget->printGUIWarningMessage(str, "FailedLoadConnector");
    }

    return success;
}



//! @brief xml version
//! @todo Make undo settings work or remove it
void loadParameterValue(QDomElement &rDomElement, GUIModelObject* pObject, undoStatus /*undoSettings*/)
{
    QString parameterName;
    QString parameterValue;
    QString parameterType;

    parameterName = rDomElement.attribute(HMF_NAMETAG);
    parameterValue = rDomElement.attribute(HMF_VALUETAG);
    parameterType = rDomElement.attribute(HMF_TYPETAG);

    //    bool isDbl;
    //    //Assumes that if it is convertible to a double it is a plain value otherwise it is assumed to be mapped to a System parameter
    //    double value = rData.parameterValue.toDouble(&isDbl);
    //    if(isDbl)
    //    {
    //        pObject->setParameterValue(rData.parameterName, value);
    //    }
    //    else
    {
        //Use the setParameter method that mapps to System parameter
        if(!parameterName.startsWith("noname_subport:") && !pObject->getParameterNames().contains(parameterName))
        {
            gpMainWindow->mpMessageWidget->printGUIWarningMessage("Parameter name " + parameterName + " in component " + pObject->getName() + " mismatch. Parameter ignored.", "parametermismatch");
            return;
        }
        pObject->setParameterValue(parameterName, parameterValue, true);
    }
}


//! @deprecated This StartValue load code is only kept for upconverting old files, we should keep it here until we have some other way of upconverting old formats
void loadStartValue(QDomElement &rDomElement, GUIModelObject* pObject, undoStatus /*undoSettings*/)
{
    QString portName = rDomElement.attribute("portname");
    QString variable = rDomElement.attribute("variable");
    QString startValue = rDomElement.attribute("value");

//    bool isDbl;
//    //Assumes that if it is convertible to a double it is a plain value otherwise it is assumed to be mapped to a System parameter
//    double value = rData.startValue.toDouble(&isDbl);
//    if(isDbl)
//    {
//        pObject->setStartValue(rData.portName, rData.variable, value);
//    }
//    else
    {
        //Use the setStartValue method that mapps to System parameter
        pObject->setStartValue(portName, variable, startValue);
    }
}


//! @brief Loads a ModelObject from the supplied load data
//! @param[in] rData The ModelObjectLoadData to load from
//! @param[in] pLibrary a pointer to the library widget which holds appearance data
//! @param[in] pContainer The Container Object to load into
//! @param[in] undoSettings Wheter or not to register undo for the operation
GUIModelObject* loadGUIModelObject(QDomElement &rDomElement, LibraryWidget* pLibrary, GUIContainerObject* pContainer, undoStatus undoSettings)
{
    //Read core specific data
    QString type = rDomElement.attribute(HMF_TYPETAG);
    QString name = rDomElement.attribute(HMF_NAMETAG);

    //Read gui specific data
    qreal posX, posY, target_rotation;
    bool isFlipped;

    QDomElement guiData = rDomElement.firstChildElement(HMF_HOPSANGUITAG);
    parsePoseTag(guiData.firstChildElement(HMF_POSETAG), posX, posY, target_rotation, isFlipped);
    target_rotation = normDeg360(target_rotation); //Make sure target rotation between 0 and 359.999

    int nameTextPos = guiData.firstChildElement(HMF_NAMETEXTTAG).attribute("position").toInt();
    int nameTextVisible = guiData.firstChildElement(HMF_NAMETEXTTAG).attribute("visible").toInt(); //should be bool, +0.5 to roound to int on truncation
    //bool portsHidden = guiData.firstChildElement(HMF_PORTSTAG).attribute("hidden").toInt();
    //bool namesHidden = guiData.firstChildElement(HMF_NAMESTAG).attribute("hidden").toInt();

    GUIModelObjectAppearance *pAppearanceData = pLibrary->getAppearanceData(type);
    if (pAppearanceData != 0)
    {
        GUIModelObjectAppearance appearanceData = *pAppearanceData; //Make a copy
        appearanceData.setName(name);

        nameVisibility nameStatus;
        if(nameTextVisible)
        {
            nameStatus = NAMEVISIBLE;
        }
        else
        {
            nameStatus = NAMENOTVISIBLE;
        }

        GUIModelObject* pObj = pContainer->addGUIModelObject(&appearanceData, QPointF(posX, posY), 0, DESELECTED, nameStatus, undoSettings);
        pObj->setNameTextPos(nameTextPos);

        //First set flip (before rotate, Important!)
        //! @todo For now If flipped than we need to rotate in wrong direction also, saving saves flipped rotation angle i think but changing save and load would couse old models to load incorrectly
        if (isFlipped)
        {
            pObj->flipHorizontal(undoSettings);
            pObj->rotate(-target_rotation, undoSettings); //This assumes object created with initial rotation 0 in addGuiModelObject above
        }
        else
        {
            pObj->rotate(target_rotation, undoSettings); //This assumes object created with initial rotation 0 in addGuiModelObject above
        }

        if (rDomElement.tagName() == HMF_SYSTEMTAG)
        {
            QString externalfilepath;
            QDomElement embededSystemDomElement;

            //Overwrite the typename with the gui specific one for systems, or set the type if it is missing (which is should be)
            //! @todo or maybe core should contain system typename for systems
            //type = HOPSANGUISYSTEMTYPENAME;

            //Read system specific corea and gui data
            externalfilepath = rDomElement.attribute(HMF_EXTERNALPATHTAG);

            //Save the domElement to read embeded system
            if (externalfilepath.isEmpty())
            {
                embededSystemDomElement = rDomElement;
            }

            //Check if we should load a embeded or external system
            if (externalfilepath.isEmpty())
            {
                //Load embeded system
                pObj->getAppearanceData()->setBasePath(pContainer->getAppearanceData()->getBasePath()); // Set the basepath for relative icon paths
                pObj->loadFromDomElement(embededSystemDomElement);
            }
            else
            {
                //Now read the external file to change appearance and populate the system
                //! @todo assumes that the supplied path is rellative, need to make sure that this does not crash if that is not the case
                QString path = pContainer->getModelFileInfo().absolutePath() + "/" + externalfilepath;
                QFile file(path);
                if (!(file.exists()))
                {
                    qDebug() << "file: " << path << " does not exist";
                }
                QDomDocument domDocument;
                QDomElement externalRoot = loadXMLDomDocument(file, domDocument, HMF_ROOTTAG);
                QDomElement externalSystemRoot = externalRoot.firstChildElement(HMF_SYSTEMTAG);
                //! @todo set the modefile info, maybe we should have built in helpfunction for loading directly from file in System
                pObj->setModelFileInfo(file);
                pObj->loadFromDomElement(externalSystemRoot);
                //! @todo this code is duplicated with the one in system->loadfromdomelement (external code) that code will never run, as this will take care of it. When we have embeded subsystems will will need to fix this

                //Overwrite any loaded external name with the one that was stored in the main file from which we are loading
                if (!name.isEmpty())
                {
                    pObj->setName(name);
                }

            }
        }

        return pObj;

    }
    else
    {
        qDebug() << "loadGUIObj Some error happend pAppearanceData == 0";
        //! @todo Some error message
        return 0;
    }
}




//! @brief Loads a containerport object from a xml dom element
GUIModelObject* loadContainerPortObject(QDomElement &rDomElement, LibraryWidget* pLibrary, GUIContainerObject* pContainer, undoStatus undoSettings)
{
    //! @todo this does not feel right should try to avoid it maybe
    rDomElement.setAttribute(HMF_TYPETAG, HOPSANGUICONTAINERPORTTYPENAME); //Set the typename for the gui, or overwrite if anything was actaully given in the HMF file (should not be)
    return loadGUIModelObject(rDomElement, pLibrary, pContainer, undoSettings); //We use the loadGUIModelObject function as it does what is needed
}

//! @brief Loads a SystemParameter from the supplied load data
//! @param[in] rDomElement The SystemParameter DOM element to load from
//! @param[in] pContainer The Container Object to load into
void loadSystemParameter(QDomElement &rDomElement, double hmfVersion, GUIContainerObject* pContainer)
{
    QString name = rDomElement.attribute(HMF_NAMETAG);
    QString value = rDomElement.attribute(HMF_VALUETAG);
    QString type = rDomElement.attribute(HMF_TYPETAG);

    if(hmfVersion <= 0.3 && type.isEmpty())     //Version check, types did not exist in 0.3 and bellow (everything was double)
    {
        type = "double";
    }

    pContainer->getCoreSystemAccessPtr()->setSystemParameter(name, value, "", "", type, true);

}

//! @brief Loads a FavouriteParameter from the supplied load data
//! @param[in] rDomElement The FavoriteVariableLoadData DOM element to load from
//! @param[in] pContainer The Container Object to load into (Must be a system)
void loadFavoriteVariable(QDomElement &rDomElement, GUIContainerObject* pContainer)
{
    QString componentName = rDomElement.attribute("componentname");
    QString portName = rDomElement.attribute("portname");
    QString dataName = rDomElement.attribute("dataname");
    QString dataUnit = rDomElement.attribute("dataunit");

    dynamic_cast<GUISystem *>(pContainer)->setFavoriteVariable(componentName, portName, dataName, dataUnit);
}


void loadPlotAlias(QDomElement &rDomElement, GUIContainerObject* pContainer)
{
    QString alias = rDomElement.attribute("alias");
    QString componentName = rDomElement.attribute("component");
    QString portName = rDomElement.attribute("port");
    QString dataName = rDomElement.attribute("data");

    pContainer->definePlotAlias(alias, componentName, portName, dataName);
}


GUITextBoxWidget *loadTextBoxWidget(QDomElement &rDomElement, GUIContainerObject *pContainer, undoStatus undoSettings)
{
    QString text;
    QFont font;
    QColor color;
    QString linestyle;
    bool lineVisible;
    QPointF point;
    qreal width, height, linewidth;

    //Read gui specific stuff
    QDomElement guiData = rDomElement.firstChildElement(HMF_HOPSANGUITAG);

    QDomElement textobjectTag = guiData.firstChildElement("textobject");
    text = textobjectTag.attribute("text");
    font.fromString(textobjectTag.attribute("font"));
    color.setNamedColor(textobjectTag.attribute("fontcolor"));

    QDomElement poseTag = guiData.firstChildElement(HMF_POSETAG);
    QPointF tempPoint;
    tempPoint.setX(poseTag.attribute("x").toDouble());
    tempPoint.setY(poseTag.attribute("y").toDouble());
    point = tempPoint.toPoint();

    QDomElement sizeTag = guiData.firstChildElement("size");
    width = sizeTag.attribute("width").toDouble();
    height = sizeTag.attribute("height").toDouble();

    QDomElement lineTag = guiData.firstChildElement("line");
    lineVisible = lineTag.attribute("visible").toInt();
    linewidth = lineTag.attribute("width").toDouble();
    linestyle = lineTag.attribute(HMF_STYLETAG);

    GUITextBoxWidget *pWidget = pContainer->addTextBoxWidget(point, NOUNDO);
    pWidget->setText(text);
    pWidget->setFont(font);
    pWidget->setColor(color);
    pWidget->setSize(width, height);
    pWidget->setLineWidth(linewidth);
    pWidget->setBoxVisible(lineVisible);
    if(linestyle == "solidline")
        pWidget->setLineStyle(Qt::SolidLine);
    if(linestyle == "dashline")
        pWidget->setLineStyle(Qt::DashLine);
    if(linestyle == "dotline")
        pWidget->setLineStyle(Qt::DotLine);
    if(linestyle == "dashdotline")
        pWidget->setLineStyle(Qt::DashDotLine);
    pWidget->setSelected(true);     //!< @todo Stupid!
    pWidget->setSelected(false);    //For some reason this is needed...
    if(undoSettings == UNDO)
    {
        pContainer->getUndoStackPtr()->registerAddedWidget(pWidget);
    }

    return pWidget;
}
