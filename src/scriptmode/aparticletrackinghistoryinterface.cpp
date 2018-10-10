#include "aparticletrackinghistoryinterface.h"
#include "ahistoryrecords.h"

AParticleTrackingHistoryInterface::AParticleTrackingHistoryInterface(QVector<EventHistoryStructure *> &EventHistory) :
    EventHistory(EventHistory) {}

QVariantList AParticleTrackingHistoryInterface::getAllDefinedTerminatorTypes()
{
    const QStringList defined = EventHistoryStructure::getAllDefinedTerminationTypes();
    QVariantList l;
    for (int i=0; i<defined.size(); ++i)
        l << QString::number(i)+ " = " + defined.at(i);
    return l;
}

int AParticleTrackingHistoryInterface::getTermination(int iParticle)
{
    if (!checkParticle(iParticle)) return -1;
    return EventHistory.at(iParticle)->Termination;
}

//QVariantList AParticleTrackingHistoryInterface::getDirection(int i)
//{
//    QVariantList vl;
//    if (checkParticle(i))
//       vl << EventHistory.at(i)->dx << EventHistory.at(i)->dy << EventHistory.at(i)->dz;
//    return vl;
//}

int AParticleTrackingHistoryInterface::getParticleId(int iParticle)
{
    if (!checkParticle(iParticle)) return -1;
    return EventHistory.at(iParticle)->ParticleId;
}

//int AParticleTrackingHistoryInterface::sernum(int i)
//{
//  if (!checkParticle(i)) return -1;
//  return EventHistory.at(i)->index;
//}

bool AParticleTrackingHistoryInterface::isSecondary(int iParticle)
{
    if (!checkParticle(iParticle)) return false;
    return EventHistory.at(iParticle)->isSecondary();
}

int AParticleTrackingHistoryInterface::getParent(int iParticle)
{
    if (!checkParticle(iParticle)) return -1;
    return EventHistory.at(iParticle)->SecondaryOf;
}

double AParticleTrackingHistoryInterface::getInitialEnergy(int iParticle)
{
    if (!checkParticle(iParticle)) return -1;
    return EventHistory.at(iParticle)->initialEnergy;
}

int AParticleTrackingHistoryInterface::countRecords(int iParticle)
{
    if (!checkParticle(iParticle)) return 0;
    return EventHistory.at(iParticle)->Deposition.size();
}

int AParticleTrackingHistoryInterface::getRecordMaterial(int iParticle, int iRecord)
{
    if (!checkParticleAndMaterial(iParticle, iRecord)) return -1;
    return EventHistory.at(iParticle)->Deposition.at(iRecord).MaterialId;
}

double AParticleTrackingHistoryInterface::getRecordDepositedEnergy(int iParticle, int iRecord)
{
    if (!checkParticleAndMaterial(iParticle, iRecord)) return -1;
    return EventHistory.at(iParticle)->Deposition.at(iRecord).DepositedEnergy;
}

double AParticleTrackingHistoryInterface::getRecordDistance(int iParticle, int iRecord)
{
    if (!checkParticleAndMaterial(iParticle, iRecord)) return -1;
    return EventHistory.at(iParticle)->Deposition.at(iRecord).Distance;
}

bool AParticleTrackingHistoryInterface::checkParticle(int i)
{
    if (i<0 || i >= EventHistory.size())
    {
        abort("Attempt to address non-existent particle number in history");
        return false;
    }
    return true;
}

bool AParticleTrackingHistoryInterface::checkParticleAndMaterial(int i, int m)
{
    if (i<0 || i >= EventHistory.size())
    {
        abort("Attempt to address non-existent particle number in history");
        return false;
    }
    if (m<0 || m >= EventHistory.at(i)->Deposition.size())
    {
        abort("Attempt to address non-existent material record in history");
        return false;
    }
    return true;
}


#include "TTree.h"
#include "TFile.h"
#include <vector>
bool AParticleTrackingHistoryInterface::saveHistoryToTree(QString fileName)
{
    TFile f(fileName.toLatin1().data(),"RECREATE");

    TTree *t = new TTree("","Particle tracking history");

    int     index;
    int     particleId;
    int     secondaryOf;
    std::vector<double> dirVector; dirVector.resize(3);
    float   initialEnergy;
    int     termination;

    std::vector<int>    Vol_MaterialId;
    std::vector<double> Vol_DepositedEnergy;
    std::vector<double> Vol_TravelledDistance;

    t->Branch("index", &index, "index/I");
    t->Branch("partId", &particleId, "partId/I");
    t->Branch("secondaryOf", &secondaryOf, "secondaryOf/I");
    t->Branch("dirVector", &dirVector);
    t->Branch("energyOnEntrance", &initialEnergy, "energyOnEntrance/F");
    t->Branch("termination", &termination, "termination/I");
    t->Branch("vol_materialId", &Vol_MaterialId);
    t->Branch("vol_depositedEnergy", &Vol_DepositedEnergy);
    t->Branch("vol_distance", &Vol_TravelledDistance);

    for (const EventHistoryStructure* h : EventHistory)
    {
        index = h->index;
        particleId = h->ParticleId;
        secondaryOf = h->SecondaryOf;
        dirVector[0] = h->dx; dirVector[1] = h->dy; dirVector[2] = h->dz;
        initialEnergy = h->initialEnergy;
        termination = h->Termination;

        Vol_MaterialId.clear(); Vol_DepositedEnergy.clear(); Vol_TravelledDistance.clear();
        for (const MaterialHistoryStructure& d : h->Deposition)
        {
            Vol_MaterialId.push_back(d.MaterialId);
            Vol_DepositedEnergy.push_back(d.DepositedEnergy);
            Vol_TravelledDistance.push_back(d.Distance);
        }
        t->Fill();
    }
    f.Close();
}

