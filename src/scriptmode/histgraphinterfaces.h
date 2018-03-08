#ifndef HISTGRAPHINTERFACES_H
#define HISTGRAPHINTERFACES_H

#include "ascriptinterface.h"

#include <QString>
#include <QVariant>

class TmpObjHubClass;
class TObject;

class AInterfaceToHist : public AScriptInterface
{
  Q_OBJECT

public:
  AInterfaceToHist(TmpObjHubClass *TmpHub);
  AInterfaceToHist(const AInterfaceToHist& other);
  ~AInterfaceToHist(){}

  //bool           InitOnRun() override {}
  bool           IsMultithreadCapable() const override {return true;}

public slots:
  void           NewHist(const QString& HistName, int bins, double start, double stop);
  void           NewHist2D(const QString &HistName, int binsX, double startX, double stopX, int binsY, double startY, double stopY);

  void           SetTitles(const QString& HistName, QString X_Title, QString Y_Title, QString Z_Title = "");
  void           SetLineProperties(const QString& HistName, int LineColor, int LineStyle, int LineWidth);

  void           Fill(const QString& HistName, double val, double weight);
  void           Fill2D(const QString& HistName, double x, double y, double weight);

  void           FillArr(const QString& HistName, const QVariant Array);
  void           Fill2DArr(const QString& HistName, const QVariant Array);

  void           Divide(const QString& HistName, const QString& HistToDivideWith);

  void           Draw(const QString& HistName, const QString options = "");

  void           Smooth(const QString& HistName, int times);
  const QVariant FitGauss(const QString& HistName, const QString options = "");
  const QVariant FitGaussWithInit(const QString& HistName, const QVariant InitialParValues, const QString options = "");

  bool           Delete(const QString& HistName);
  void           DeleteAllHist();

signals:
  void           RequestDraw(TObject* obj, QString options, bool fFocus);

private:
  TmpObjHubClass *TmpHub;

};

// ---- G R A P H S ---- (TGraph of ROOT)
class AInterfaceToGraph : public AScriptInterface
{
  Q_OBJECT

public:
  AInterfaceToGraph(TmpObjHubClass *TmpHub);
  AInterfaceToGraph(const AInterfaceToGraph& other);
  ~AInterfaceToGraph(){}

  //virtual bool InitOnRun() override {}
  virtual bool IsMultithreadCapable() const override {return true;}

public slots:
  void NewGraph(const QString& GraphName);
  void SetMarkerProperties(QString GraphName, int MarkerColor, int MarkerStyle, int MarkerSize);
  void SetLineProperties(QString GraphName, int LineColor, int LineStyle, int LineWidth);
  void SetTitles(QString GraphName, QString X_Title, QString Y_Title);

  void AddPoint(QString GraphName, double x, double y);
  void AddPoints(QString GraphName, QVariant xArray, QVariant yArray);
  void AddPoints(QString GraphName, QVariant xyArray);

  void Sort(const QString& GraphName);

  void Draw(QString GraphName, QString options = "APL");

  bool Delete(QString GraphName);
  void DeleteAllGraph();

signals:
  void RequestDraw(TObject* obj, QString options, bool fFocus);

private:
  TmpObjHubClass *TmpHub;

};

#endif // HISTGRAPHINTERFACES_H