//!
//! @file   ParameterDialog.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-03-01
//!
//! @brief Contains a class for interact with paramters
//!
//$Id$

#include <QtGui>
#include <cassert>

#include "ParameterDialog.h"
#include "HopsanCore.h"
#include "mainwindow.h"
#include "GUIObject.h"


//! @class ParameterDialog
//! @brief The ParameterDialog class is a Widget used to interact with component parameters.
//!
//! It read and write parameters to the core components.
//!


//! Constructor.
//! @param coreComponent is a ponter to the core component.
//! @param parent defines a parent to the new instanced object.
ParameterDialog::ParameterDialog(GUIComponent *pGUIComponent, QWidget *parent)
    : QDialog(parent)
{
    mpGUIObject = pGUIComponent;
    isGUISubsystem = false;

    createEditStuff();
}


ParameterDialog::ParameterDialog(GUISubsystem *pGUISubsystem, QWidget *parent)     : QDialog(parent)
{
    mpGUIObject = pGUISubsystem;
    isGUISubsystem = true;

    createEditStuff();
}

void ParameterDialog::createEditStuff()
{
    mpNameEdit = new QLineEdit(mpGUIObject->getName());

    QVector<QString> parnames = mpGUIObject->getParameterNames();
    QVector<QString>::iterator pit;
    for ( pit=parnames.begin(); pit!=parnames.end(); ++pit )
    {
        mVarVector.push_back(new QLabel(*pit));
        mVarVector.back()->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        mDescriptionVector.push_back(new QLabel(mpGUIObject->getParameterDescription(*pit).append(", ")));
        mUnitVector.push_back(new QLabel(mpGUIObject->getParameterUnit(*pit)));

        mValueVector.push_back(new QLineEdit());
        mValueVector.back()->setValidator(new QDoubleValidator(-999.0, 999.0, 6, mValueVector.back()));

        QString valueTxt;
        valueTxt.setNum(mpGUIObject->getParameterValue(*pit), 'g', 6 );
        mValueVector.back()->setText(valueTxt);

        mVarVector.back()->setBuddy(mValueVector.back());

    }

    okButton = new QPushButton(tr("&Ok"));
    okButton->setDefault(true);
    cancelButton = new QPushButton(tr("&Cancel"));
    cancelButton->setDefault(false);

    buttonBox = new QDialogButtonBox(Qt::Vertical);
    buttonBox->addButton(okButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(cancelButton, QDialogButtonBox::ActionRole);

    connect(okButton, SIGNAL(pressed()), SLOT(setParameters()));
    connect(cancelButton, SIGNAL(pressed()), SLOT(close()));

    QHBoxLayout *pNameLayout = new QHBoxLayout;
    QLabel *pNameLabel = new QLabel("Name: ");
    pNameLayout->addWidget(pNameLabel);
    pNameLayout->addWidget(mpNameEdit);

    QHBoxLayout *pCQSLayout;
    if (isGUISubsystem)
    {
        pCQSLayout = new QHBoxLayout;
        //This is very hopsan specific (or actually TLM specific)
        mpCQSEdit = new QLineEdit(mpGUIObject->getTypeCQS());
        QLabel *pCQSLabel = new QLabel("CQS: ");
        //! @todo We never set the cqs type

        pCQSLayout->addWidget(pCQSLabel);
        pCQSLayout->addWidget(mpCQSEdit);
    }

    QVBoxLayout *topLeftLayout1 = new QVBoxLayout;
    QVBoxLayout *topLeftLayout2 = new QVBoxLayout;
    QVBoxLayout *topLeftLayout3 = new QVBoxLayout;
    QVBoxLayout *topLeftLayout4 = new QVBoxLayout;
    for (size_t i=0 ; i <mVarVector.size(); ++i )
    {
        topLeftLayout1->addWidget(mDescriptionVector[i]);
        topLeftLayout2->addWidget(mVarVector[i]);
        topLeftLayout3->addWidget(mValueVector[i]);
        topLeftLayout4->addWidget(mUnitVector[i]);
    }

    QHBoxLayout *leftLayout = new QHBoxLayout;
    leftLayout->addLayout(topLeftLayout1);
    leftLayout->addLayout(topLeftLayout2);
    leftLayout->addLayout(topLeftLayout3);
    leftLayout->addLayout(topLeftLayout4);
    leftLayout->addStretch(1);

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    int lr = 0; //Layout row
    mainLayout->addLayout(pNameLayout, lr, 0);
    mainLayout->addWidget(buttonBox, lr, 1);
    ++lr;
    if (isGUISubsystem)
    {
        mainLayout->addLayout(pCQSLayout, lr, 0);
        ++lr;
    }
    mainLayout->addLayout(leftLayout, lr, 0);
    setLayout(mainLayout);

    setWindowTitle(tr("Parameters"));
}


//! Sets the parameters in the core component. Read the values from the dialog and write them into the core component.
void ParameterDialog::setParameters()
{
    mpGUIObject->setName(mpNameEdit->text());
    mpGUIObject->refreshName();

    qDebug() << mpNameEdit->text();

    for (size_t i=0 ; i < mValueVector.size(); ++i )
    {
        bool ok;
        double newValue = mValueVector[i]->text().toDouble(&ok);
        if (!ok)
        {
            MessageWidget *messageWidget = qobject_cast<MainWindow *>(this->parent()->parent()->parent()->parent()->parent()->parent())->mpMessageWidget;
            messageWidget->printGUIMessage(QString("ParameterDialog::setParameters(): You must give a correct value for '").append(mVarVector[i]->text()).append(QString("', putz. Try again!")));
            return;
        }
        mpGUIObject->setParameterValue(mVarVector[i]->text(), newValue);
    }
    std::cout << "Parameters updated." << std::endl;
    this->close();
}
