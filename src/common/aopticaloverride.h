#ifndef OPTICALOVERRIDECLASS_H
#define OPTICALOVERRIDECLASS_H

#include <QVector>
#include <QString>

class APhoton;
class AMaterialParticleCollection;
class TRandom2;
class TH1I;
class QJsonObject;
class TH1D;

class AOpticalOverride
{
public:
  //the status for photon tracing:
  enum OpticalOverrideResultEnum {NotTriggered, Absorbed, Forward, Back, _Error_};
  //detailed status for statistics only:
  enum ScatterStatusEnum {SpikeReflection, LobeReflection, LambertianReflection, Absorption, Transmission, ErrorDetected};


  AOpticalOverride(AMaterialParticleCollection* MatCollection, int MatFrom, int MatTo)
    : MatCollection(MatCollection), MatFrom(MatFrom), MatTo(MatTo) {}
  virtual ~AOpticalOverride() {}

  virtual OpticalOverrideResultEnum calculate(TRandom2* RandGen, APhoton* Photon, const double* NormalVector) = 0; //unitary vectors! iWave = -1 if not wavelength-resolved

  virtual void printConfiguration(int iWave) = 0;
  virtual QString getType() const = 0;
  virtual QString getReportLine() = 0; // for GUI: reports override status "to material blabla (#id): properies"
  virtual void initializeWaveResolved(double /*waveFrom*/, double /*waveStep*/, int /*waveNodes*/) {}  //override if override has wavelength-resolved data

  // save/load config
  virtual void writeToJson(QJsonObject &json);
  virtual bool readFromJson(QJsonObject &json);

  void updateMatIndices(int iMatFrom, int iMatTo) {MatFrom = iMatFrom; MatTo = iMatTo;}

  // read-out variables for standalone checker only (not multithreaded)
  ScatterStatusEnum Status;               // type of interaction which happened - use in 1 thread only!

protected:  
  AMaterialParticleCollection* MatCollection;
  int MatFrom, MatTo;   // material index of material before(from) and after(to) the optical interface

  void RandomDir(TRandom2* RandGen, APhoton* Photon);

};

class BasicOpticalOverride : public AOpticalOverride
{
public:
  BasicOpticalOverride(AMaterialParticleCollection* MatCollection, int MatFrom, int MatTo, double probLoss, double probRef, double probDiff, int scatterModel);
  BasicOpticalOverride(AMaterialParticleCollection* MatCollection, int MatFrom, int MatTo);
  virtual ~BasicOpticalOverride() {}

  virtual OpticalOverrideResultEnum calculate(TRandom2* RandGen, APhoton* Photon, const double* NormalVector); //unitary vectors! iWave = -1 if not wavelength-resolved

  virtual void printConfiguration(int iWave);
  virtual QString getType() const {return "Simplistic_model";}
  virtual QString getReportLine();

  // save/load config is not used for this type!
  virtual void writeToJson(QJsonObject &json);
  virtual bool readFromJson(QJsonObject &json);

  //-- parameters --
  double probLoss; //probability of absorption
  double probRef;  //probability of specular reflection
  double probDiff; //probability of scattering
  int    scatterModel; //0 - 4Pi, 1 - 2Pi back, 2 - 2Pi forward
};

class FSNPOpticalOverride : public AOpticalOverride
{
public:
  FSNPOpticalOverride(AMaterialParticleCollection* MatCollection, int MatFrom, int MatTo, double albedo)
    : AOpticalOverride(MatCollection, MatFrom, MatTo), Albedo(albedo) {}
  FSNPOpticalOverride(AMaterialParticleCollection* MatCollection, int MatFrom, int MatTo)
    : AOpticalOverride(MatCollection, MatFrom, MatTo) {Albedo = 1;}
  virtual ~FSNPOpticalOverride() {}

  virtual OpticalOverrideResultEnum calculate(TRandom2* RandGen, APhoton* Photon, const double* NormalVector); //unitary vectors! iWave = -1 if not wavelength-resolved

  virtual void printConfiguration(int iWave);
  virtual QString getType() const {return "FS_NP";}
  virtual QString getReportLine();

  // save/load config is not used for this type!
  virtual void writeToJson(QJsonObject &json);
  virtual bool readFromJson(QJsonObject &json);

  //-- parameters --
  double Albedo;
};

class AWaveshifterOverride : public AOpticalOverride
{
public:
  AWaveshifterOverride(AMaterialParticleCollection* MatCollection, int MatFrom, int MatTo);
  virtual ~AWaveshifterOverride();

  void initializeWaveResolved(double waveFrom, double waveStep, int waveNodes);
  virtual OpticalOverrideResultEnum calculate(TRandom2* RandGen, APhoton* Photon, const double* NormalVector); //unitary vectors! iWave = -1 if not wavelength-resolved

  virtual void printConfiguration(int iWave);
  virtual QString getType() const {return "SurfaceWLS";}
  virtual QString getReportLine();

  // save/load config is not used for this type!
  virtual void writeToJson(QJsonObject &json);
  virtual bool readFromJson(QJsonObject &json);

  //-- parameters --
  QVector<double> ReemissionProbability_lambda;
  QVector<double> ReemissionProbability;
  QVector<double> ReemissionProbabilityBinned;  

  QVector<double> EmissionSpectrum_lambda;
  QVector<double> EmissionSpectrum;
  TH1D* Spectrum;

  //tmp parameters
  double WaveFrom;
  double WaveStep;
  int WaveNodes;
};

AOpticalOverride* OpticalOverrideFactory(QString model, AMaterialParticleCollection* MatCollection, int MatFrom, int MatTo);

#endif // OPTICALOVERRIDECLASS_H
