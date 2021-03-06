#include "scatteronmetal.h"
#include "amaterialparticlecolection.h"
#include "aphoton.h"
#include "asimulationstatistics.h"

#include <QDebug>
#include <QJsonObject>

#include "TComplex.h"
#include "TRandom2.h"

void ScatterOnMetal::printConfiguration(int /*iWave*/)
{
  qDebug() << "-------Configuration:-------";
  qDebug() << "Model:"<<getType();
  qDebug() << "Real N:"<<RealN;
  qDebug() << "Imaginary N:"<<ImaginaryN;
  qDebug() << "----------------------------";
}

QString ScatterOnMetal::getReportLine()
{
  QString s = "to " + (*MatCollection)[MatTo]->name;
  QString s1;
  s1.setNum(MatTo);
  s += " ("+s1+") --> Scatter on metal: ";
  s += " n="+QString::number(RealN);
  s += " k="+QString::number(ImaginaryN);
  return s;
}

void ScatterOnMetal::writeToJson(QJsonObject &json)
{
  AOpticalOverride::writeToJson(json);

  json["RealN"]  = RealN;
  json["ImaginaryN"]  = ImaginaryN;
}

bool ScatterOnMetal::readFromJson(QJsonObject &json)
{
  QString type = json["Model"].toString();
  if (type != getType()) return false; //file for wrong model!

  RealN = json["RealN"].toDouble();
  ImaginaryN = json["ImaginaryN"].toDouble();

  return true;
}

AOpticalOverride::OpticalOverrideResultEnum ScatterOnMetal::calculate(TRandom2 *RandGen, APhoton *Photon, const double *NormalVector)
{
  double CosTheta = Photon->v[0]*NormalVector[0] + Photon->v[1]*NormalVector[1] + Photon->v[2]*NormalVector[2];

//  double Rindex1, Rindex2;
//  if (iWave == -1)
//    {
//      Rindex1 = (*MatCollection)[MatFrom]->n;
//      //Rindex2 = (*MatCollection)[MatTo]->n;
//      Rindex2 = RealN;
//    }
//  else
//    {
//      Rindex1 = (*MatCollection)[MatFrom]->nWaveBinned.at(iWave);
//      //Rindex2 = (*MatCollection)[MatTo]->nWaveBinned.at(iWave);
//      Rindex2 = RealN;
//    }
//  double Refl = calculateReflectivity(CosTheta, RealN/Rindex1, ImaginaryN/Rindex1);

  double Refl = calculateReflectivity(CosTheta, RealN, ImaginaryN);
  //qDebug() << "Dielectric-metal override: Cos theta="<<CosTheta<<" Reflectivity:"<<Refl;

  if ( RandGen->Rndm() > Refl )
    {
      //Absorption
      //qDebug() << "Override: Loss on metal";
      Status = Absorption;
      Photon->SimStat->OverrideMetalAbs++;
      return Absorbed;
    }

  //else specular reflection
  //qDebug()<<"Override: Specular reflection from metal";
    //rotating the vector: K = K - 2*(NK)*N
  double NK = NormalVector[0]*Photon->v[0]; NK += NormalVector[1]*Photon->v[1];  NK += NormalVector[2]*Photon->v[2];
  Photon->v[0] -= 2.0*NK*NormalVector[0]; Photon->v[1] -= 2.0*NK*NormalVector[1]; Photon->v[2] -= 2.0*NK*NormalVector[2];
  Status = SpikeReflection;
  Photon->SimStat->OverrideMetalReflection++;
  return Back;
}

double ScatterOnMetal::calculateReflectivity(double CosTheta, double RealN, double ImaginaryN)
{
  //qDebug() << "cosTheta, n, k: "<< CosTheta << RealN << ImaginaryN;

  TComplex N(RealN, ImaginaryN);
  TComplex U(1,0);
  double SinTheta = (CosTheta < 0.9999999) ? sqrt(1.0 - CosTheta*CosTheta) : 0;
  TComplex CosPhi = TMath::Sqrt( U - SinTheta*SinTheta/(N*N));

  TComplex rs = (CosTheta - N*CosPhi) / (CosTheta + N*CosPhi);
  TComplex rp = ( -N*CosTheta + CosPhi) / (N*CosTheta + CosPhi);

  double RS = rs.Rho2();
  double RP = rp.Rho2();

  double R = 0.5 * (RS+RP);
  //qDebug() << "Refl coeff = "<< R;

  return R;
}

