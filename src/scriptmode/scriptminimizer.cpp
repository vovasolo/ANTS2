#include "scriptminimizer.h"
#include "ajavascriptmanager.h"

#ifdef GUI
#include "mainwindow.h"
#endif

#include <QScriptEngine>
#include <QDebug>

#include "TMath.h"
#include "Math/Functor.h"
#include "Minuit2/Minuit2Minimizer.h"

double JavaScriptFunctor(const double *p) //last parameter contains the pointer to MainWindow object
{
  void *thisvalue;
  memcpy(&thisvalue, &p[0], sizeof(void *));
  AJavaScriptManager* ScriptManager = (AJavaScriptManager*)thisvalue;

  if (ScriptManager->isEvalAborted()) return 1e30;

  /*
    QString str;
    for (int i=0; i<ScriptManager->MiniNumVariables; i++)
        str += QString::number(p[i+1])+"  ";
    qDebug() << "Functor call with parameters:"<<str<<ScriptManager;
  */

  QScriptValueList input;
  for (int i=0; i<ScriptManager->MiniNumVariables; i++) input << p[i+1];

  QScriptValue sv = ScriptManager->getMinimalizationFunction();
  double result = sv.call(QScriptValue(), input).toNumber();

  //    qDebug() << "SM"<<ScriptManager<<"engine"<<ScriptManager->engine<<"sv reports engine:"<<sv.engine();
  //    qDebug() << "Minimization parameter value obtained:"<<result;

  return result;
}

AInterfaceToMinimizerScript::AInterfaceToMinimizerScript(AScriptManager *ScriptManager) :
  ScriptManager(ScriptManager)
{
    Description = "Access to CERN ROOT minimizer";
}

AInterfaceToMinimizerScript::~AInterfaceToMinimizerScript()
{
  Clear();
}

void AInterfaceToMinimizerScript::ForceStop()
{    
  //qDebug() << "Abort requested for minimization procedure";
    //qDebug() << "aborted:"<<ScriptManager->fAborted;
}

void AInterfaceToMinimizerScript::Clear()
{
  for (AVarRecordBase* r : Variables)
    delete r;
  Variables.clear();
}

void AInterfaceToMinimizerScript::SetFunctorName(QString name)
{
    ScriptManager->MiniFunctionName = name;
}

void AInterfaceToMinimizerScript::AddVariable(QString name, double start, double step, double min, double max)
{
  Variables << new AVarRecordLimited(name, start, step, min, max);
}

void AInterfaceToMinimizerScript::AddVariable(QString name, double start, double step)
{
  Variables << new AVarRecordNormal(name, start, step);
}

void AInterfaceToMinimizerScript::AddFixedVariable(QString name, double value)
{
  Variables << new AVarRecordFixed(name, value);
}

void AInterfaceToMinimizerScript::AddLowerLimitedVariable(QString name, double value, double step, double lowerLimit)
{
  Variables << new AVarRecordLowerLimited(name, value, step, lowerLimit);
}

void AInterfaceToMinimizerScript::AddUpperLimitedVariable(QString name, double value, double step, double upperLimit)
{
  Variables << new AVarRecordUpperLimited(name, value, step, upperLimit);
}

void AInterfaceToMinimizerScript::AddAllVariables(QVariant array)
{
  if (array.type() != QMetaType::QVariantList )
  {
      abort("DefineAllVariables(): has to be an array containing initializers of the variables");
      return;
  }

  Clear();

  QVariantList vl = array.toList();
  if (vl.isEmpty())
    {
      abort("DefineAllVariables(): array of initializers is empty");
      return;
    }

  for (int i=0; i<vl.size(); i++)
    {
      //  qDebug() << "Adding variable #"<<i;
      QVariantList var = vl.at(i).toList();
      switch (var.size())
        {
        case 1:  // fixed
          Variables << new AVarRecordFixed(QString::number(i), var.at(0).toDouble());
          break;
        case 2:  // normal
          Variables << new AVarRecordNormal(QString::number(i), var.at(0).toDouble(), var.at(1).toDouble());
          break;
        case 4:
          qDebug() << var << var.at(2).isNull();
          if (var.at(2).isNull()) //upper limited
            {
               Variables << new AVarRecordUpperLimited(QString::number(i), var.at(0).toDouble(), var.at(1).toDouble(), var.at(3).toDouble());
               break;
            }
          else if (var.at(3).isNull()) //lower limited
            {
               Variables << new AVarRecordLowerLimited(QString::number(i), var.at(0).toDouble(), var.at(1).toDouble(), var.at(2).toDouble());
               break;
            }
          else
            {
               Variables << new AVarRecordLimited(QString::number(i), var.at(0).toDouble(), var.at(1).toDouble(), var.at(2).toDouble(), var.at(3).toDouble());
               break;
            }
        default:
            abort("DefineAllVariables(): variable definition arrays have to be of length 1, 2 or 4");
            return;
        }
        //  Variables.last()->Debug();
    }
}

void AInterfaceToMinimizerScript::SetSimplex()
{
  Method = 1;
}

void AInterfaceToMinimizerScript::SetMigrad()
{
  Method = 0;
}

bool AInterfaceToMinimizerScript::Run()
{
  if (!ScriptManager)
    {
      abort("ScriptManager is not set!");
      return false;
    }

  //  qDebug() << "Minimizer Run() started";
  ScriptManager->MiniNumVariables = Variables.size();
  if (ScriptManager->MiniNumVariables == 0)
    {
      abort("Variables are not defined!");
      return false;
    }

  ROOT::Math::Functor *Funct = configureFunctor();
  if (!Funct)
    {
      abort("Minimization function is not defined!");
      return false;
    }

  ROOT::Minuit2::Minuit2Minimizer *RootMinimizer = new ROOT::Minuit2::Minuit2Minimizer( Method==0 ? ROOT::Minuit2::kMigrad : ROOT::Minuit2::kSimplex );
  RootMinimizer->SetMaxFunctionCalls(500);
  RootMinimizer->SetMaxIterations(1000);
  RootMinimizer->SetTolerance(0.001);
  RootMinimizer->SetPrintLevel(PrintVerbosity);  
  RootMinimizer->SetStrategy( bHighPrecision ? 2 : 1 ); // 1 -> standard,  2 -> try to improve minimum (slower)

  RootMinimizer->SetFunction(*Funct);

  //prepare to transfer pointer to ScriptManager - it will the the first variable
  double dPoint;
  void *thisvalue = ScriptManager;
  memcpy(&dPoint, &thisvalue, sizeof(void *));
  //We need to fix for the possibility that double isn't enough to store void*
  RootMinimizer->SetFixedVariable(0, "p", dPoint);

  //setting up variables   -  start step min max etc
  for (int i=0; i<ScriptManager->MiniNumVariables; i++)
      Variables[i]->AddToMinimizer(i+1, RootMinimizer);

  //  qDebug() << "Starting minimization";
  bool fOK = RootMinimizer->Minimize();
  fOK = fOK && !ScriptManager->isEvalAborted();
  //  qDebug()<<"Minimization success? "<<fOK;

  //results
  Results.clear();
  if (fOK)
  {
      const double *VarVals = RootMinimizer->X();
      for (int i=0; i<Variables.size(); i++)
      {
          //  qDebug() << i << "-->--"<<VarVals[i+1];
          Results << VarVals[i+1];
      }
  }

  delete Funct;
  delete RootMinimizer;

  return fOK;
}

AInterfaceToMinimizerScript::AVarRecordNormal::AVarRecordNormal(QString name, double start, double step)
{
  Name = name.toLatin1().data();
  Value = start;
  Step = step;
}

void AInterfaceToMinimizerScript::AVarRecordNormal::AddToMinimizer(int varIndex, ROOT::Minuit2::Minuit2Minimizer *minimizer)
{
  minimizer->SetVariable(varIndex, Name, Value, Step);
}

void AInterfaceToMinimizerScript::AVarRecordNormal::Debug() const
{
   qDebug() << "Normal"<<Value<<Step;
}

AInterfaceToMinimizerScript::AVarRecordFixed::AVarRecordFixed(QString name, double value)
{
  Name = name.toLatin1().data();
  Value = value;
}

void AInterfaceToMinimizerScript::AVarRecordFixed::AddToMinimizer(int varIndex, ROOT::Minuit2::Minuit2Minimizer *minimizer)
{
  minimizer->SetFixedVariable(varIndex, Name, Value);
}

void AInterfaceToMinimizerScript::AVarRecordFixed::Debug() const
{
  qDebug() << "Fixed"<<Value;
}

AInterfaceToMinimizerScript::AVarRecordLimited::AVarRecordLimited(QString name, double start, double step, double min, double max)
{
  Name = name.toLatin1().data();
  Value = start;
  Step = step;
  Min = min;
  Max = max;
}

void AInterfaceToMinimizerScript::AVarRecordLimited::AddToMinimizer(int varIndex, ROOT::Minuit2::Minuit2Minimizer *minimizer)
{
  minimizer->SetLimitedVariable(varIndex, Name, Value, Step, Min, Max);
}

void AInterfaceToMinimizerScript::AVarRecordLimited::Debug() const
{
  qDebug() << "Limited"<<Value<<Step<<Min<<Max;
}

AInterfaceToMinimizerScript::AVarRecordLowerLimited::AVarRecordLowerLimited(QString name, double start, double step, double min)
{
  Name = name.toLatin1().data();
  Value = start;
  Step = step;
  Min = min;
}

void AInterfaceToMinimizerScript::AVarRecordLowerLimited::AddToMinimizer(int varIndex, ROOT::Minuit2::Minuit2Minimizer *minimizer)
{
  minimizer->SetLowerLimitedVariable(varIndex, Name, Value, Step, Min);
}

void AInterfaceToMinimizerScript::AVarRecordLowerLimited::Debug() const
{
  qDebug() << "LowerLimited"<<Value<<Step<<Min;
}

AInterfaceToMinimizerScript::AVarRecordUpperLimited::AVarRecordUpperLimited(QString name, double start, double step, double max)
{
  Name = name.toLatin1().data();
  Value = start;
  Step = step;
  Max = max;
}

void AInterfaceToMinimizerScript::AVarRecordUpperLimited::AddToMinimizer(int varIndex, ROOT::Minuit2::Minuit2Minimizer *minimizer)
{
  minimizer->SetUpperLimitedVariable(varIndex, Name, Value, Step, Max);
}

void AInterfaceToMinimizerScript::AVarRecordUpperLimited::Debug() const
{
  qDebug() << "UpperLimited"<<Value<<Step<<Max;
}

AInterfaceToMinimizerJavaScript::AInterfaceToMinimizerJavaScript(AJavaScriptManager *ScriptManager) :
  AInterfaceToMinimizerScript( dynamic_cast<AScriptManager*>(ScriptManager) ) {}

AInterfaceToMinimizerJavaScript::AInterfaceToMinimizerJavaScript(const AInterfaceToMinimizerJavaScript & /*other*/) :
  AInterfaceToMinimizerScript(0) { }

void AInterfaceToMinimizerJavaScript::SetScriptManager(AJavaScriptManager *NewScriptManager)
{
  ScriptManager = dynamic_cast<AScriptManager*>(NewScriptManager);
}

ROOT::Math::Functor* AInterfaceToMinimizerJavaScript::configureFunctor()
{
  AJavaScriptManager* jsm = static_cast<AJavaScriptManager*>(ScriptManager);
  QScriptValue sv = jsm->getMinimalizationFunction();
  if (!sv.isFunction()) return 0;

  return new ROOT::Math::Functor(&JavaScriptFunctor, ScriptManager->MiniNumVariables + 1);
}

#ifdef __USE_ANTS_PYTHON__
#include "apythonscriptmanager.h"
#include "PythonQt.h"
#include "PythonQt_QtAll.h"
#include "PythonQtConversion.h"

double PythonScriptFunctor(const double *p) //last parameter contains the pointer to MainWindow object
{
  void *thisvalue;
  memcpy(&thisvalue, &p[0], sizeof(void *));
  APythonScriptManager* psm = (APythonScriptManager*)thisvalue;

  if (psm->isEvalAborted()) return 1e30;

  const int numArguments = psm->MiniNumVariables;

  /*
    //Diagnostics
    QString str;
    for (int i=0; i<numArguments; i++)
        str += QString::number(p[i+1])+"  ";
    qDebug() << "Functor call with parameters:"<<str<<psm;
  */

  /*
    PythonQtObjectPtr mainModule = PythonQt::self()->getMainModule();
    PyObject* dict = NULL;
    if (PyModule_Check(mainModule)) dict = PyModule_GetDict(mainModule);
    else if (PyDict_Check(mainModule))
        dict = mainModule;

    if (dict)
      {
        PyObject* expression = PyDict_GetItemString(dict, psm->MiniFunctionName.toLatin1().data());
        qDebug() << "expression:"<<expression;

            PyObject *pyth_val;

            //PyObject* arg1 = PyFloat_FromDouble(p[1]);
            //PyObject* arg2 = PyFloat_FromDouble(p[2]);
            //qDebug() << "args:"<< arg1 << arg2;
            //pyth_val = PyObject_CallFunctionObjArgs(expression, arg1, arg2, NULL);

            PyObject* tupleArgs = PyTuple_New(numArguments);
            for (int i=0; i<numArguments; i++)
                PyTuple_SetItem(tupleArgs, i, PyFloat_FromDouble(p[i+1]));
            pyth_val = PyObject_Call(expression, tupleArgs, NULL);

            qDebug() << pyth_val;

            double result = 1e30;
            bool bOK;
            if (pyth_val)
                result = PythonQtConv::PyObjGetDouble(pyth_val, false, bOK);

            qDebug() << "-->" << result;
            return result;

      }
    else return 1e30;
    */

    //PyObject* tupleArgs = PyTuple_New(numArguments);
    PythonQtObjectPtr tupleArgs;
    tupleArgs.setNewRef( PyTuple_New(numArguments) );
    for (int i=0; i<numArguments; i++)
        PyTuple_SetItem(tupleArgs, i, PyFloat_FromDouble(p[i+1]));
    //PythonQtObjectPtr pyth_val = PyObject_Call(psm->MinimizationFunctor, tupleArgs, NULL);
    PythonQtObjectPtr pyth_val;
    pyth_val.setNewRef( PyObject_Call(psm->MinimizationFunctor, tupleArgs, NULL) );

    double result = 1e30;
    bool bOK;
    if (pyth_val)
        result = PythonQtConv::PyObjGetDouble(pyth_val, false, bOK);

    //  qDebug() << "-->" << result;
    return result;
}

AInterfaceToMinimizerPythonScript::AInterfaceToMinimizerPythonScript(APythonScriptManager *ScriptManager) :
  AInterfaceToMinimizerScript( dynamic_cast<AScriptManager*>(ScriptManager) ) {}

ROOT::Math::Functor *AInterfaceToMinimizerPythonScript::configureFunctor()
{
   APythonScriptManager* psm = static_cast<APythonScriptManager*>(ScriptManager);

   /*
   PythonQtObjectPtr mainModule = PythonQt::self()->getMainModule();   
   PyObject* dict = NULL;
   if (PyModule_Check(mainModule)) dict = PyModule_GetDict(mainModule);
   else if (PyDict_Check(mainModule))
       dict = mainModule;
   if (dict)
   {
       psm->MinimizationFunctor = PyDict_GetItemString(dict, ScriptManager->MiniFunctionName.toLatin1().data());

       if (psm->MinimizationFunctor && PyCallable_Check(psm->MinimizationFunctor))
           return new ROOT::Math::Functor(&PythonScriptFunctor, psm->MiniNumVariables + 1);
   }
   */

   if (psm->GlobalDict.object())
   {
       psm->MinimizationFunctor.setNewRef( PyDict_GetItemString(psm->GlobalDict, ScriptManager->MiniFunctionName.toLatin1().data()) );

       if (psm->MinimizationFunctor && PyCallable_Check(psm->MinimizationFunctor))
           return new ROOT::Math::Functor(&PythonScriptFunctor, psm->MiniNumVariables + 1);
   }


   psm->MinimizationFunctor = 0;
   return 0;
}
#endif
