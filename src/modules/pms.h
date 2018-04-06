#ifndef PMS_H
#define PMS_H

#include "apm.h"

#include <QVector>
#include <QString>
#include <QPair>

class TRandom2;
//class TH1D;
class AMaterialParticleCollection;
class GeneralSimSettings;
class APmPosAngTypeRecord;
class PMtypeClass;
class AGammaRandomGenerator;
//class QTextStream;
class QJsonObject;
//class ACustomRandomSampling;

class pms
{   
public:
    explicit pms(AMaterialParticleCollection* materialCollection, TRandom2* randGen);
    ~pms();

    //json
    void writePMtypesToJson(QJsonObject &json);
    bool readPMtypesFromJson(QJsonObject &json);
    void writeInividualOverridesToJson(QJsonObject &json);
    bool readInividualOverridesFromJson(QJsonObject &json);
    void writeElectronicsToJson(QJsonObject &json);
    bool readElectronicsFromJson(QJsonObject &json);

    bool readReconstructionRelatedSettingsFromJson(QJsonObject &json);

    void configure(GeneralSimSettings* SimSet);
      //recalculate PDE for selected wave binning
    void RebinPDEs(); //triggered by changing between true and false for WavelenegthResolved on the MainWindow
    void RebinPDEsForType(int typ);
    void RebinPDEsForPM(int ipm);
      //recalculate angular for selected angle binning
    void RecalculateAngular();
    void RecalculateAngularForType(int typ);
    void RecalculateAngularForPM(int ipm);
      //prepare pulse height spectra hists
    void preparePHSs();
      // prepare MCcrosstalk
    void prepareMCcrosstalk();
    void prepareMCcrosstalkForPM(int ipm);
      //prepare ADC
    void updateADClevels();

      // Single PM signal using photoelectron spectrum
    double GenerateSignalFromOnePhotoelectron(int ipm);  //used only in GUI of mainwindow!
      // used during simulation:
    double getActualPDE(int ipm, int WaveIndex); //returns partial probability to be detected (wave-resolved PDE)
    double getActualAngularResponse(int ipm, double cosAngle); //returns partial probability to be detected (vs angular response - cos of refracted beam)
    double getActualAreaResponse(int ipm, double x, double y); //returns partial probability to be detected (vs area response - x, y in local coordinates of the PM)

    int count() const {return numPMs;} //returns number of PMs
    void clear();
    void resetOverrides();
    void add(int upperlower, double xx, double yy, double zz, double Psi, int typ);
    void add(int upperlower, APmPosAngTypeRecord* pat);
    void insert(int ipm, int upperlower, double xx, double yy, double zz, double Psi, int typ);
    void remove(int ipm); //removes PM number i

    //New memory layout stuff
    bool saveCoords(const QString &filename);
    bool saveAngles(const QString &filename);
    bool saveTypes(const QString &filename);
    bool saveUpperLower(const QString &filename);

    inline APm& at(int ipm) {return PMs[ipm];}
    inline const APm& at(int ipm) const {return PMs.at(ipm);}

    //accelerator
    double getMaxQE() const {return MaxQE;}
    double getMaxQEvsWave(int iWave) const {return MaxQEvsWave.at(iWave);}

    //individual PMs
    inline double X(int ipm) const {return PMs[ipm].x;}
    inline double Y(int ipm) const {return PMs[ipm].y;}
    inline double Z(int ipm) const {return PMs[ipm].z;}
    int getPixelsX(int ipm) const;
    int getPixelsY(int ipm) const;
    double SizeX(int ipm) const;
    double SizeY(int ipm) const;
    double SizeZ(int ipm) const;
    bool isSiPM(int ipm) const;


    //PDE
    bool        isPDEwaveOverriden(int i) const {return (!PDE[i].isEmpty());}
    bool        isPDEwaveOverriden() const;
    bool        isPDEeffectiveOverriden() const;
    void        writePDEeffectiveToJson(QJsonObject &json);
    bool        readPDEeffectiveFromJson(QJsonObject &json);
    void        writePDEwaveToJson(QJsonObject &json);
    bool        readPDEwaveFromJson(QJsonObject &json);
    void        setPDEeffective(int ipm, double val) {effectivePDE[ipm] = val;}
    double      getPDEeffective(int ipm) {return effectivePDE[ipm];}
    void        setAllEffectivePDE(QVector<double>* vec) {effectivePDE = *vec;} //will be gone sone
    void        setPDEwave(int ipm, QVector<double>* x, QVector<double>* y) {PDE_lambda[ipm] = *x; PDE[ipm] = *y;}
    void        setPDEbinned(int ipm, QVector<double>* vec) {PDEbinned[ipm] = *vec;}
    const QVector<double>* getPDE(int ipm) {return &PDE[ipm];}
    const QVector<double>* getPDE_lambda(int ipm) {return &PDE_lambda[ipm];}
    const QVector<double>* getPDEbinned(int ipm) {return &PDEbinned[ipm];}
    const QVector<double>* getAllEffectivePDE() {return &effectivePDE;}

    //Angular response
    bool        isAngularOverriden(int i) const {return !AngularSensitivity[i].isEmpty();}
    bool        isAngularOverriden() const;
    void        writeAngularToJson(QJsonObject &json);
    bool        readAngularFromJson(QJsonObject &json);
    void        setAngular(int ipm, QVector<double>* x, QVector<double>* y) {AngularSensitivity_lambda[ipm] = *x; AngularSensitivity[ipm] = *y;}
    void        setAngularN1(int ipm, double n1) {AngularN1[ipm] = n1;}
    double      getAngularN1(int ipm) {return AngularN1[ipm];}
    void        setAngularBinned(int ipm, QVector<double>* vec) {AngularSensitivityCosRefracted[ipm] = *vec;}
    const QVector<double>* getAngularSensitivity(int ipm) {return &AngularSensitivity[ipm];}
    const QVector<double>* getAngularSensitivity_lambda(int ipm) {return &AngularSensitivity_lambda[ipm];}
    const QVector<double>* getAngularSensitivityCosRefracted(int ipm) {return &AngularSensitivityCosRefracted[ipm];}

    //Area response
    bool        isAreaOverriden(int i) const {return !AreaSensitivity[i].isEmpty();}
    bool        isAreaOverriden() const;
    void        writeAreaToJson(QJsonObject &json);
    bool        readAreaFromJson(QJsonObject &json);
    void        setArea(int ipm, QVector<QVector <double> > *vec, double xStep, double yStep) {AreaSensitivity[ipm] = *vec; AreaStepX[ipm] = xStep; AreaStepY[ipm] = yStep;}
    double      getAreaStepX(int ipm) {return AreaStepX[ipm];}
    double      getAreaStepY(int ipm) {return AreaStepY[ipm];}
    const QVector<QVector<double> >* getAreaSensitivity(int ipm) {return &AreaSensitivity[ipm];}

    //type properties    
    PMtypeClass* getType(int i) {return PMtypes[i];}
    PMtypeClass* getTypeForPM(int ipm) {return PMtypes.at(PMs[ipm].type);}
    bool removePMtype(int itype);
    void appendNewPMtype(PMtypeClass* tp);
    int countPMtypes(){return PMtypes.size();}
    int findPMtype(QString typeName);
    void clearPMtypes();
    void replaceType(int itype, PMtypeClass *newType);
    void updateTypePDE(int typ, QVector<double> *x, QVector<double> *y);
    void scaleTypePDE(int typ, double factor);
    void updateTypeAngular(int typ, QVector<double> *x, QVector<double> *y);
    void updateTypeAngularN1(int typ, double val);
    void updateTypeArea(int typ, QVector<QVector <double> > *vec, double xStep, double yStep);
    void clearTypeArea(int typ);

    void setElChanSPePHS(int ipm, QVector<double> *x, QVector<double> *y);  // ***!!!
    void setADC(int ipm, double max, int bits); // ***!!!

    void CopySPePHSdata(int ipmFrom, int ipmTo);
    void CopyMCcrosstalkData(int ipmFrom, int ipmTo);
    void CopyElNoiseData(int ipmFrom, int ipmTo);
    void CopyADCdata(int ipmFrom, int ipmTo);

    void ScaleSPePHS(int ipm, double gain);
    void CalculateElChannelsStrength();

    //config
    void SetWave(bool wavelengthResolved, double waveFrom, double waveStep, int waveNodes);
    void setWavelengthResolved(bool flag) {WavelengthResolved = flag;}
    void setAngularResolved(bool flag) {AngularResolved = flag;}
    void setAreaResolved(bool flag){AreaResolved = flag;}
    void setWaveFrom(double from) {WaveFrom = from;}
    void setWaveStep(double step) {WaveStep = step;}
    void setWaveNodes(int nodes) {WaveNodes = nodes;}
    void setCosBins(int bins) {CosBins = bins;}
    void setDoPHS(bool flag) {fDoPHS = flag;}
    void setDoMCcrosstalk(bool flag) {fDoMCcrosstalk = flag;}
    void setDoElNoise(bool flag) {fDoElNoise = flag;}
    void setDoADC(bool flag) {fDoADC = flag;}
    void setMeasurementTime(double time) {MeasurementTime = time;}

    bool isDoPHS() const {return fDoPHS;}
    bool isDoMCcrosstalk() const {return fDoMCcrosstalk;}
    bool isDoElNoise() const {return fDoElNoise;}
    bool isDoADC() const {return fDoADC;}
    double getMeasurementTime() const {return MeasurementTime;}

    QVector<QPair<double, int> > getPMsSortedByR() const;

private:
    TRandom2* RandGen;
    AMaterialParticleCollection* MaterialCollection;
    AGammaRandomGenerator* GammaRandomGen;

    int numPMs; //number of PMs

    //  ----====-----
    //when NEW vector properties are added to PMs -> "clear" "insert" and "remove" methodes have to be updated!
    //

    //individual PM properties
    QVector<APm> PMs;
    QVector<double> tmpGains;

    double MaxQE; //accelerator, not wave-resolved value
    QVector<double> MaxQEvsWave; //vs wavelength
    void calculateMaxQEs(); // first vector has to be rebinned according to acxtual wave properties (see configure())

    //overrides over base PM class:
    QVector<QVector<double> > PDE; //Quantun efficiency & Collection efficiency for PMTs; Photon Detection Efficiency for SiPMs
    QVector<QVector<double> > PDE_lambda;
    QVector<QVector<double> > PDEbinned;
    QVector<double> effectivePDE;
    QVector<QVector<double> > AngularSensitivity;
    QVector<QVector<double> > AngularSensitivity_lambda;
    QVector<double> AngularN1; //refractive index of the medium where PM was positioned to measure the angular response
    QVector<QVector<double> > AngularSensitivityCosRefracted; //Response vs cos of refracted beam, binned from 0 to 1 in CosBins bins
    QVector<QVector< QVector <double> > > AreaSensitivity;
    QVector<double> AreaStepX;
    QVector<double> AreaStepY;

    double MeasurementTime;  // measurement time to calculate dark counts for SiPMs

    //flags for the current simulation mode
    bool WavelengthResolved;
    bool AngularResolved;
    bool AreaResolved;
    bool fDoPHS;
    bool fDoMCcrosstalk;
    bool fDoElNoise;
    bool fDoADC;
    //wave info
    double WaveFrom, WaveStep;
    int WaveNodes;
    //number of bins for angular response
    int CosBins;

    //defined base PM types
    QVector<PMtypeClass*> PMtypes;

    QString ErrorString;
    void writePHSsettingsToJson(int ipm, QJsonObject &json);    // ***!!! to PMs
    bool readPHSsettingsFromJson(int ipm, QJsonObject &json);   // ***!!! to PMs
    void writeRelQE_PDE(QJsonObject &json);
    void readRelQE_PDE(QJsonObject &json);
};

#endif // PMS_H
