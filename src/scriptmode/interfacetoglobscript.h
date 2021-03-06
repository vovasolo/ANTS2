#ifndef INTERFACETOGLOBSCRIPT
#define INTERFACETOGLOBSCRIPT

#include "ascriptinterface.h"

#include <QVector>
#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QVariant>

#ifdef GUI
  class MainWindow;
  class QMainWindow;
#endif

#ifdef SIM
  class ASimulationManager;
#endif

class DetectorClass;
class EventsDataClass;
class GeometryWindowClass;
class GraphWindowClass;
class APmHub;
class TF2;
class AConfiguration;
class AReconstructionManager;
class SensorLRFs;
class TmpObjHubClass;
class APmGroupsManager;
class QJsonValue;

// ====== interfaces which do not require GUI ======

// ---- C O N F I G U R A T I O N ----
class AInterfaceToConfig : public AScriptInterface
{
  Q_OBJECT

public:
  AInterfaceToConfig(AConfiguration* config);
  AInterfaceToConfig(const AInterfaceToConfig &other);
  ~AInterfaceToConfig(){}

  virtual bool IsMultithreadCapable() const override {return true;}

  AConfiguration* Config;
  QString LastError;

public slots:
  bool Load(QString FileName);
  bool Save(QString FileName);

  bool Replace(QString Key, QVariant val);
  QVariant GetKeyValue(QString Key);

  QString GetLastError();

  void RebuildDetector();
  void UpdateGui();

signals:
  void requestReadRasterGeometry();

private:
  bool modifyJsonValue(QJsonObject& obj, const QString& path, const QJsonValue& newValue);
  void find(const QJsonObject &obj, QStringList Keys, QStringList& Found, QString Path = "");
  bool keyToNameAndIndex(QString Key, QString &Name, QVector<int> &Indexes);
  bool expandKey(QString &Key);

};

// ---- R E C O N S T R U C T I O N ----
class InterfaceToReconstructor : public AScriptInterface
{
  Q_OBJECT

public:
  InterfaceToReconstructor(AReconstructionManager* RManager, AConfiguration* Config, EventsDataClass* EventsDataHub, TmpObjHubClass* TmpHub, int RecNumThreads);
  ~InterfaceToReconstructor(){}

  virtual void ForceStop() override;

public slots:
  void ReconstructEvents(int NumThreads = -1, bool fShow = true);
  void UpdateFilters(int NumThreads = -1);

  void DoBlurUniform(double range, bool fUpdateFilters = true);
  void DoBlurGauss(double sigma, bool fUpdateFilters = true);

  //Sensor groups
  int countSensorGroups();
  void clearSensorGroups();
  void addSensorGroup(QString name);
  void setPMsOfGroup(int igroup, QVariant PMlist);
  QVariant getPMsOfGroup(int igroup);

  //passive PMs
  bool isStaticPassive(int ipm);
  void setStaticPassive(int ipm);
  void clearStaticPassive(int ipm);
  void selectSensorGroup(int igroup);
  void clearSensorGroupSelection();

  //reconstruction data save
  void SaveAsTree(QString fileName, bool IncludePMsignals=true, bool IncludeRho=true, bool IncludeTrue=true, int SensorGroup=0);
  void SaveAsText(QString fileName);

  // manifest item handling
  void ClearManifestItems();
  int  CountManifestItems();
  void AddRoundManisfetItem(double x, double y, double Diameter);
  void AddRectangularManisfetItem(double x, double y, double dX, double dY, double Angle);  
  void SetManifestItemLineProperties(int i, int color, int width, int style);

  //signal per ph el extraction  
  const QVariant Peaks_GetSignalPerPhE() const;
  void Peaks_PrepareData();
  void Peaks_Configure(int bins, double from, double to, double sigmaPeakfinder, double thresholdPeakfinder, int maxPeaks = 30);
  double Peaks_Extract_NoAbortOnFail(int ipm);
  void Peaks_ExtractAll();
  QVariant Peaks_GetPeakPositions(int ipm);

  const QVariant GetSignalPerPhE_stat() const;


private:
  AReconstructionManager* RManager;
  AConfiguration* Config;
  EventsDataClass* EventsDataHub;
  APmGroupsManager* PMgroups;
  TmpObjHubClass* TmpHub;

  int RecNumThreads;

signals:
  void RequestUpdateGuiForManifest();
  void RequestStopReconstruction();
};

// ---- E V E N T S ----
class AInterfaceToData : public AScriptInterface
{
  Q_OBJECT

public:
  AInterfaceToData(AConfiguration* Config, EventsDataClass* EventsDataHub);
  ~AInterfaceToData(){}

  bool IsMultithreadCapable() const override {return true;}

public slots:
  int GetNumPMs();
  int countPMs();
  int GetNumEvents();
  int countEvents();
  int countTimedEvents();
  int countTimeBins();
  double GetPMsignal(int ievent, int ipm);
  QVariant GetPMsignals(int ievent);
  void SetPMsignal(int ievent, int ipm, double value);
  double GetPMsignalTimed(int ievent, int ipm, int iTimeBin);
  QVariant GetPMsignalVsTime(int ievent, int ipm);

  // Reconstructed values
    //assuming there is only one group, and single point reconstruction
  double GetReconstructedX(int ievent);  
  double GetReconstructedY(int ievent); 
  double GetReconstructedZ(int ievent);  
  double GetRho(int ievent, int iPM);  
  double GetRho2(int ievent, int iPM); 
  double GetReconstructedEnergy(int ievent);  
  bool IsReconstructedGoodEvent(int ievent);  
    //general
  double GetReconstructedX(int igroup, int ievent, int ipoint);
  double GetReconstructedY(int igroup, int ievent, int ipoint);
  double GetReconstructedZ(int igroup, int ievent, int ipoint);
  double GetRho(int igroup, int ievent, int ipoint, int iPM);
  double GetRho2(int igroup, int ievent, int ipoint, int iPM);
  double GetReconstructedEnergy(int igroup, int ievent, int ipoint);
  bool IsReconstructedGoodEvent(int igroup, int ievent);
  bool IsReconstructed_ScriptFilterPassed(int igroup, int ievent);
    //counters - return -1 when invalid parameters
  int countReconstructedGroups();
  int countReconstructedEvents(int igroup);
  int countReconstructedPoints(int igroup, int ievent);

  // True values known in sim or from a calibration dataset
  double GetTrueX(int ievent);
  double GetTrueY(int ievent);
  double GetTrueZ(int ievent);
  double GetTrueEnergy(int ievent);
  int GetTruePoints(int ievent);
  bool IsTrueGoodEvent(int ievent);
  bool GetTrueNumberPoints(int ievent);
  void SetScanX(int ievent, double value);
  void SetScanY(int ievent, double value);
  void SetScanZ(int ievent, double value);
  void SetScanEnergy(int ievent, double value);

  //raw signal values
  QVariant GetPMsSortedBySignal(int ievent);
  int GetPMwithMaxSignal(int ievent);

  //for custom reconstrtuctions
    //assuming there is only one group, and single point reconstruction
  void SetReconstructed(int ievent, double x, double y, double z, double e);
  void SetReconstructed(int ievent, double x, double y, double z, double e, double chi2);
  void SetReconstructedX(int ievent, double x);
  void SetReconstructedY(int ievent, double y);
  void SetReconstructedZ(int ievent, double z);
  void SetReconstructedEnergy(int ievent, double e);
  void SetReconstructed_ScriptFilterPass(int ievent, bool flag);
  void SetReconstructedGoodEvent(int ievent, bool good);
  void SetReconstructedAllEventsGood(bool flag);
  void SetReconstructionOK(int ievent, bool OK);

    //general
  void SetReconstructed(int igroup, int ievent, int ipoint, double x, double y, double z, double e);
  void SetReconstructedFast(int igroup, int ievent, int ipoint, double x, double y, double z, double e); // no checks!!! unsafe
  void AddReconstructedPoint(int igroup, int ievent, double x, double y, double z, double e);
  void SetReconstructedX(int igroup, int ievent, int ipoint, double x);
  void SetReconstructedY(int igroup, int ievent, int ipoint, double y);
  void SetReconstructedZ(int igroup, int ievent, int ipoint, double z);
  void SetReconstructedEnergy(int igroup, int ievent, int ipoint, double e);
  void SetReconstructedGoodEvent(int igroup, int ievent, int ipoint, bool good);
  void SetReconstructed_ScriptFilterPass(int igroup, int ievent, bool flag);
  void SetReconstructionOK(int igroup, int ievent, int ipoint, bool OK);
    //set when reconstruction is ready for all events! - otherwise GUI will complain
  void SetReconstructionReady();
    //clear reconstruction and prepare containers
  void ResetReconstructionData(int numGroups);

  //load data
  void LoadEventsTree(QString fileName, bool Append = false, int MaxNumEvents = -1);
  void LoadEventsAscii(QString fileName, bool Append = false);

  //clear data
  void ClearEvents();

  //Purges
  void PurgeBad();
  void Purge(int LeaveOnePer);

  //Statistics
  QVariant GetStatistics(int igroup);

private:
  AConfiguration* Config;
  EventsDataClass* EventsDataHub;

  bool checkEventNumber(int ievent);
  bool checkEventNumber(int igroup, int ievent, int ipoint);
  bool checkPM(int ipm);
  bool checkTrueDataRequest(int ievent);
  bool checkSetReconstructionDataRequest(int ievent);

signals:
  void RequestEventsGuiUpdate();
};

// ---- P M S ----
class AInterfaceToPMs : public AScriptInterface
{
  Q_OBJECT

public:
  AInterfaceToPMs(AConfiguration* Config);
  ~AInterfaceToPMs() {}

  bool IsMultithreadCapable() const override {return true;}

public slots:  
  int      CountPM() const;

  double   GetPMx(int ipm);
  double   GetPMy(int ipm);
  double   GetPMz(int ipm);

  bool     IsPmCenterWithin(int ipm, double x, double y, double distance_in_square);
  bool     IsPmCenterWithinFast(int ipm, double x, double y, double distance_in_square) const;

  QVariant GetPMtypes();
  QVariant GetPMpositions() const;

  void     RemoveAllPMs();
  bool     AddPMToPlane(int UpperLower, int type, double X, double Y, double angle = 0);
  bool     AddPM(int UpperLower, int type, double X, double Y, double Z, double phi, double theta, double psi);
  void     SetAllArraysFullyCustom();

private:
  AConfiguration* Config;
  APmHub* PMs;

  bool checkValidPM(int ipm);
  bool checkAddPmCommon(int UpperLower, int type);
};

#ifdef SIM
// ---- S I M U L A T I O N S ----
class InterfaceToSim : public AScriptInterface
{
  Q_OBJECT

public:
  InterfaceToSim(ASimulationManager* SimulationManager, EventsDataClass* EventsDataHub, AConfiguration* Config, int RecNumThreads, bool fGuiPresent = true);
  ~InterfaceToSim(){}

  virtual void ForceStop();

public slots:
  bool RunPhotonSources(int NumThreads = -1);
  bool RunParticleSources(int NumThreads = -1);

  void SetSeed(long seed);
  long GetSeed();

  void ClearCustomNodes();
  void AddCustomNode(double x, double y, double z);
  QVariant GetCustomNodes();
  bool SetCustomNodes(QVariant ArrayOfArray3);

  bool SaveAsTree(QString fileName);
  bool SaveAsText(QString fileName);

  //monitors
  int countMonitors();
  //int getMonitorHits(int imonitor);
  int getMonitorHits(QString monitor);

  QVariant getMonitorTime(QString monitor);
  QVariant getMonitorAngular(QString monitor);
  QVariant getMonitorWave(QString monitor);
  QVariant getMonitorEnergy(QString monitor);
  QVariant getMonitorXY(QString monitor);

signals:
  void requestStopSimulation();

private:  
  ASimulationManager* SimulationManager;
  EventsDataClass* EventsDataHub;
  AConfiguration* Config;

  int RecNumThreads;
  bool fGuiPresent;

  QVariant getMonitorData1D(QString monitor, QString whichOne);
};
#endif

// ---- L R F ----
class AInterfaceToLRF : public AScriptInterface
{
  Q_OBJECT

public:
  AInterfaceToLRF(AConfiguration* Config, EventsDataClass* EventsDataHub);

  bool IsMultithreadCapable() const override {return true;}

public slots:
  QString Make();
  double GetLRF(int ipm, double x, double y, double z);
  double GetLRFerror(int ipm, double x, double y, double z);

  QVariant GetAllLRFs(double x, double y, double z);

  //iterations  
  int CountIterations();
  int GetCurrent();
  void SetCurrent(int iterIndex);
  void SetCurrentName(QString name);
  void DeleteCurrent();
  QString Save(QString fileName);
  int Load(QString fileName);

private:
  AConfiguration* Config;
  EventsDataClass* EventsDataHub;
  SensorLRFs* SensLRF; //alias

  bool getValidIteration(int &iterIndex);
};

// ---- New LRF Module ----
namespace LRF {
  class ARepository;
}
class ALrfScriptInterface : public AScriptInterface
{
  Q_OBJECT

public:
  ALrfScriptInterface(DetectorClass* Detector, EventsDataClass* EventsDataHub);

public slots:
  QString Make(QString name, QVariantList instructions, bool use_scan_data, bool fit_error, bool scale_by_energy);
  QString Make(int recipe_id, bool use_scan_data, bool fit_error, bool scale_by_energy);
  double GetLRF(int ipm, double x, double y, double z);

  QList<int> GetListOfRecipes();
  int GetCurrentRecipeId();
  int GetCurrentVersionId();
  bool SetCurrentRecipeId(int rid);
  bool SetCurrentVersionId(int vid, int rid = -1);
  void DeleteCurrentRecipe();
  void DeleteCurrentRecipeVersion();

  bool SaveRepository(QString fileName);
  bool SaveCurrentRecipe(QString fileName);
  bool SaveCurrentVersion(QString fileName);
  QList<int> Load(QString fileName); //Returns list of loaded recipes

private:
  DetectorClass* Detector;
  EventsDataClass* EventsDataHub;
  LRF::ARepository *repo; //alias
};

#ifdef GUI // =============== GUI mode only ===============

// -- GRAPH WINDOW --
class InterfaceToGraphWin : public AScriptInterface
{
  Q_OBJECT

public:
  InterfaceToGraphWin(MainWindow* MW);
  ~InterfaceToGraphWin(){}

public slots:
  void Show();
  void Hide();

  void PlotDensityXY();
  void PlotEnergyXY();
  void PlotChi2XY();
  void ConfigureXYplot(int binsX, double X0, double X1, int binsY, double Y0, double Y1);

  void SetLog(bool Xaxis, bool Yaxis);

  void AddLegend(double x1, double y1, double x2, double y2, QString title);
  void SetLegendBorder(int color, int style, int size);

  void AddText(QString text, bool Showframe, int Alignment_0Left1Center2Right);

  void AddLine(double x1, double y1, double x2, double y2, int color, int width, int style);

  //basket operation
  void AddToBasket(QString Title);
  void ClearBasket();

  void SaveImage(QString fileName);  
  void ExportTH2AsText(QString fileName);
  QVariant GetProjection();

  QVariant GetAxis();
private:
  MainWindow* MW;
};

// -- OUTPUT WINDOW --
class AInterfaceToOutputWin : public AScriptInterface
{
  Q_OBJECT

public:
  AInterfaceToOutputWin(MainWindow* MW);
  ~AInterfaceToOutputWin(){}

public slots:
  void ShowOutputWindow(bool flag = true, int tab = -1);
  void Show();
  void Hide();

private:
  MainWindow* MW;
};

// -- GEOMETRY WINDOW --
class InterfaceToGeoWin : public AScriptInterface
{
  Q_OBJECT

public:
  InterfaceToGeoWin(MainWindow* MW, TmpObjHubClass* TmpHub);
  ~InterfaceToGeoWin();

public slots:  
  void Show();
  void Hide();

  void BlockUpdates(bool on); //forbids updates

  //orientation of TView3D
  double GetPhi();
  double GetTheta();
  void SetPhi(double phi);
  void SetTheta(double theta);
  void Rotate(double Theta, double Phi, int Steps, int msPause = 50);

  //view manipulation
  void SetZoom(int level);
  void SetParallel(bool on);
  void UpdateView();

  //position and size
  int  GetX();
  int  GetY();
  int  GetW();
  int  GetH();

  //show things
  void ShowGeometry();
  void ShowPMnumbers();
  void ShowReconstructedPositions();
  void SetShowOnlyFirstEvents(bool fOn, int number = -1);
  void ShowTruePositions();
  void ShowTracks(int num, int OnlyColor = -1);
  void ShowSPS_position();
  void ShowTracksMovie(int num, int steps, int msDelay, double dTheta, double dPhi, double rotSteps, int color = -1);

  void ShowEnergyVector();

  //clear things
  void ClearTracks();
  void ClearMarkers();

  void SaveImage(QString fileName);

private:
  MainWindow* MW;
  TmpObjHubClass* TmpHub;
  DetectorClass* Detector;
};
#endif // GUI

#endif // INTERFACETOGLOBSCRIPT

