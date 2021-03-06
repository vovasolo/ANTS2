#include "tmpobjhubclass.h"
#include "atrackrecords.h"

#include <QDebug>

#include "TH1D.h"

void TmpObjHubClass::ClearTracks()
{
    for (int i=0; i<TrackInfo.size(); i++) delete TrackInfo[i];
    TrackInfo.clear();
}

void TmpObjHubClass::ClearTmpHistsPeaks()
{
    for (TH1D* h : PeakHists) delete h;
    PeakHists.clear();
}

void TmpObjHubClass::ClearTmpHistsSigma2()
{
    for (TH1D* h : SigmaHists) delete h;
    SigmaHists.clear();
}

void TmpObjHubClass::Clear()
{
    //  qDebug() << ">>> TMPHub: Clear requested";

    //do not clear script data

    ClearTracks();
    ClearTmpHistsPeaks();
    ClearTmpHistsSigma2();
    FoundPeaks.clear();
    ChPerPhEl_Peaks.clear();
    ChPerPhEl_Sigma2.clear();

    //  qDebug() << ">>> TMPHub: Clear done!";
}

TmpObjHubClass::~TmpObjHubClass()
{
    Clear();

    Graphs.clear();
    Hists.clear();
    Trees.clear();
}
