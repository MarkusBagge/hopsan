/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 The full license is available in the file GPLv3.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   ContainerPortPropertiesDialog.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2011-01-03
//!
//! @brief Contains a class for manipulation of Container properties
//!
//$Id$

#ifndef CONTAINERPORTPROPERTIESDIALOG_H
#define CONTAINERPORTPROPERTIESDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QLineEdit>

//Forward Declaration
class ContainerPort;

class ContainerPortPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    ContainerPortPropertiesDialog(ContainerPort *pContainerPort, QWidget *pParentWidget=0);

private:
    ContainerPort *mpContainerPort;
    QLineEdit *mpNameEdit;

private slots:
    void setValues();
};

#endif // CONTAINERPORTPROPERTIESDIALOG_H
