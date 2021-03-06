#ifndef AJAVASCRIPTMANAGER_H
#define AJAVASCRIPTMANAGER_H

#include "ascriptmanager.h"
#include "ascriptmessengerdialog.h"
#include <QObject>
#include <QVector>
#include <QString>
#include <QScriptValue>

class QScriptEngine;
class TRandom2;
class QDialog;
class AInterfaceToMessageWindow;

class AJavaScriptManager : public AScriptManager
{
    Q_OBJECT

public:
    AJavaScriptManager(TRandom2 *RandGen);
    ~AJavaScriptManager();

    //configuration
    virtual void    SetInterfaceObject(QObject* interfaceObject, QString name = "") override;

    //run
    virtual int     FindSyntaxError(const QString &script) override; //returns line number of the first syntax error; -1 if no errors found
    virtual QString Evaluate(const QString &Script) override;
    virtual QVariant EvaluateScriptInScript(const QString& script) override;

    virtual void    abortEvaluation() override;
    virtual void    collectGarbage() override;

    virtual bool    isUncaughtException() const override;
    virtual int     getUncaughtExceptionLineNumber() const override;
    virtual const QString getUncaughtExceptionString() const override;

    virtual void    hideMsgDialogs() override;
    virtual void    restoreMsgDialogs() override;

    QScriptValue    getMinimalizationFunction();

    //for multithread-in-scripting
    AJavaScriptManager* createNewScriptManager(int threadNumber); // *** !!!
    QScriptValue    getProperty(const QString& properyName) const;
    QScriptValue    registerNewVariant(const QVariant &Variant);
    QScriptValue    EvaluationResult;
    AInterfaceToCore* coreObj = 0;  //core interface - to forward evaluate-script-in-script

public slots:
    void            hideAllMessengerWidgets();
    void            showAllMessengerWidgets();
    void            clearUnusedMsgDialogs();
    void            closeAllMsgDialogs();

private:
    QScriptEngine*  engine;    
    QVector<AScriptMessengerDialog*> ThreadMessangerDialogs;

};

#endif // AJAVASCRIPTMANAGER_H
