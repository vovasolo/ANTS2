#include "aneutroninfodialog.h"
#include "ui_aneutroninfodialog.h"
#include "amaterial.h"
#include "acommonfunctions.h"
#include "graphwindowclass.h"

#include <QString>
#include <QDoubleValidator>
#include <QDebug>

#include "TString.h"
#include "TGraph.h"

ANeutronInfoDialog::ANeutronInfoDialog(const AMaterial *mat, int ipart, bool bLogLog, bool bShowAbs, bool bShowScat, GraphWindowClass* GraphWindow, QWidget *parent) :
    QDialog(parent), ui(new Ui::ANeutronInfoDialog), mat(mat), ipart(ipart), bLogLog(bLogLog), bShowAbs(bShowAbs), bShowScat(bShowScat), GraphWindow(GraphWindow)
{
    ui->setupUi(this);

    QDoubleValidator* val = new QDoubleValidator(this);
    val->setBottom(0);
    ui->ledEnergy->setValidator(val);

    update();

    ui->ledEnergy->setFocus();
    ui->pbClose->setAutoDefault(false);
    ui->pbAbs->setAutoDefault(false);
    ui->pbScatter->setAutoDefault(false);
    ui->pbTotal->setAutoDefault(false);

    QObject::connect(ui->tabwIso->horizontalHeader(), &QHeaderView::sectionClicked, this, &ANeutronInfoDialog::onIsotopeTable_ColumnSelected);
}

ANeutronInfoDialog::~ANeutronInfoDialog()
{
    delete ui;
}

void ANeutronInfoDialog::on_ledEnergy_textChanged(const QString&)
{
    update();
}

void ANeutronInfoDialog::update()
{
    QString energyStr = ui->ledEnergy->text();
    bool bOK;
    double energy = energyStr.toDouble(&bOK);
    if (!bOK) return; //because triggeres on_change

    energy *= 1.0e-6; // meV -> keV
    //      qDebug() << "Energy:" << energy;

    if (!mat) return;
    if (mat->MatParticle.isEmpty()) return;
    if (ipart<0 || ipart>=mat->MatParticle.size()) return;
    if (mat->MatParticle.at(ipart).Terminators.size() != 2) return;

    const NeutralTerminatorStructure& termAb = mat->MatParticle.at(ipart).Terminators.at(0);
    const NeutralTerminatorStructure& termSc = mat->MatParticle.at(ipart).Terminators.at(1);

    ui->leName->setText(mat->name);

    const double& Density = mat->density;
    ui->ledDensity->setText( QString::number(Density, 'g', 4) );
    double MeanAtMass = mat->ChemicalComposition.getMeanAtomMass();
    ui->leMAM->setText( QString::number(MeanAtMass, 'g', 4) );
    double AtDens = Density / MeanAtMass / 1.66054e-24;
    ui->ad->setText( QString::number(AtDens, 'g', 4) );

    ui->tabwMain->clearContents();
    double cs_abs = 0;
    QString s_cs = "-off-";
    QString s_mfp = "-off-";
    if (bShowAbs && !termAb.PartialCrossSectionEnergy.isEmpty())
    {
        cs_abs = GetInterpolatedValue(energy, &termAb.PartialCrossSectionEnergy, &termAb.PartialCrossSection, bLogLog);
        s_cs = QString::number(cs_abs*1.0e24, 'g', 4);  //in barns
        double mfp = 10.0/cs_abs/AtDens;
        s_mfp = QString::number(mfp, 'g', 4);
    }
    QTableWidgetItem* twi = new QTableWidgetItem(s_cs);
    twi->setTextAlignment(Qt::AlignCenter);
    ui->tabwMain->setItem(0, 1, twi);
    twi = new QTableWidgetItem(s_mfp);
    twi->setTextAlignment(Qt::AlignCenter);
    ui->tabwMain->setItem(1, 1, twi);

    double cs_scat = 0;
    s_cs = "-off-";
    s_mfp = "-off-";
    if (bShowScat && !termSc.PartialCrossSectionEnergy.isEmpty())
    {
        cs_scat = GetInterpolatedValue(energy, &termSc.PartialCrossSectionEnergy, &termSc.PartialCrossSection, bLogLog);
        s_cs = QString::number(cs_scat*1.0e24, 'g', 4);  //in barns
        double mfp = 10.0/cs_scat/AtDens;
        s_mfp = QString::number(mfp, 'g', 4);
    }
    twi = new QTableWidgetItem(s_cs);
    twi->setTextAlignment(Qt::AlignCenter);
    ui->tabwMain->setItem(0, 2, twi);
    twi = new QTableWidgetItem(s_mfp);
    twi->setTextAlignment(Qt::AlignCenter);
    ui->tabwMain->setItem(1, 2, twi);

    double cs = cs_abs + cs_scat;
    twi = new QTableWidgetItem(QString::number(cs*1.0e24, 'g', 4));
    twi->setTextAlignment(Qt::AlignCenter);
    ui->tabwMain->setItem(0, 0, twi);
    double mfp = 10.0/cs/AtDens;
    twi = new QTableWidgetItem(QString::number(mfp, 'g', 4));
    twi->setTextAlignment(Qt::AlignCenter);
    ui->tabwMain->setItem(1, 0, twi);

    updateIsotopeTable();
}

void ANeutronInfoDialog::updateIsotopeTable()
{
    //      qDebug() << "Updating isotope table";

    ui->tabwIso->clearContents();

    const NeutralTerminatorStructure& termAb = mat->MatParticle.at(ipart).Terminators.at(0);
    const NeutralTerminatorStructure& termSc = mat->MatParticle.at(ipart).Terminators.at(1);

    ui->tabwIso->setRowCount(mat->ChemicalComposition.countIsotopes());

    double energy = ui->ledEnergy->text().toDouble() * 1.0e-6;
    double totCS_abs  = 0;
    if (bShowAbs)
      if (!termAb.PartialCrossSectionEnergy.isEmpty())
        totCS_abs  = GetInterpolatedValue(energy, &termAb.PartialCrossSectionEnergy, &termAb.PartialCrossSection, bLogLog);
    double totCS_scat = 0;
    if (bShowScat)
      if (!termSc.PartialCrossSectionEnergy.isEmpty())
        totCS_scat = GetInterpolatedValue(energy, &termSc.PartialCrossSectionEnergy, &termSc.PartialCrossSection, bLogLog);
    //      qDebug() << totCS_abs << totCS_scat;

    int row = 0;
    for (int iElement=0; iElement<mat->ChemicalComposition.countElements(); iElement++)
    {
        const AChemicalElement* el = mat->ChemicalComposition.getElement(iElement);
        for (int iIso=0; iIso<el->countIsotopes(); iIso++)
        {
            QString name = el->Symbol + "-" + QString::number(el->Isotopes.at(iIso).Mass);
            //      qDebug() << "-----"<<name;
            QTableWidgetItem* twi = new QTableWidgetItem(name);
            twi->setTextAlignment(Qt::AlignCenter);
            ui->tabwIso->setItem(row, 0, twi);

            QString s = "-off-";
            if (bShowAbs)
            {
                if (!termAb.IsotopeRecords.at(row).Energy.isEmpty())
                {
                    double cs_abs = GetInterpolatedValue(energy, &termAb.IsotopeRecords.at(row).Energy, &termAb.IsotopeRecords.at(row).CrossSection, bLogLog);
                    double fraction = cs_abs * termAb.IsotopeRecords.at(row).MolarFraction / totCS_abs * 100.0;
                    s = QString::number(fraction, 'g', 4);
                }
                else s = "0";
            }
            ATableWidgetDoubleItem* twdi = new ATableWidgetDoubleItem(s);
            ui->tabwIso->setItem(row, 1, twdi);

            s = "-off-";
            if (bShowScat)
            {
                if (!termSc.IsotopeRecords.at(row).Energy.isEmpty())
                {
                    double cs_scat = GetInterpolatedValue(energy, &termSc.IsotopeRecords.at(row).Energy, &termSc.IsotopeRecords.at(row).CrossSection, bLogLog);
                    double fraction = cs_scat * termSc.IsotopeRecords.at(row).MolarFraction/ totCS_scat * 100.0;
                    s = QString::number(fraction, 'g', 4);
                }
                else s = "0";
            }
            twdi = new ATableWidgetDoubleItem(s);
            ui->tabwIso->setItem(row, 2, twdi);

            row++;
        }
    }
    ui->tabwIso->resizeRowsToContents();
}

void ANeutronInfoDialog::on_pbClose_clicked()
{
    accept();
    hide();
}

void ANeutronInfoDialog::on_pbTotal_clicked()
{
    if (!bShowAbs && !bShowScat) return;

    if (!mat) return;
    if (mat->MatParticle.isEmpty()) return;
    if (ipart<0 || ipart>=mat->MatParticle.size()) return;
    if (mat->MatParticle.at(ipart).Terminators.size() != 2) return;

    const NeutralTerminatorStructure& termAb = mat->MatParticle.at(ipart).Terminators.at(0);
    const NeutralTerminatorStructure& termSc = mat->MatParticle.at(ipart).Terminators.at(1);

    TString xTitle("Total cross-section, barns");

    if (!bShowAbs)
        drawCrossSection(termSc.PartialCrossSectionEnergy, termSc.PartialCrossSection, xTitle);
    else if (!bShowScat)
        drawCrossSection(termAb.PartialCrossSectionEnergy, termAb.PartialCrossSection, xTitle);
    else
    {
        if (termAb.PartialCrossSectionEnergy.isEmpty())
            drawCrossSection(termSc.PartialCrossSectionEnergy, termSc.PartialCrossSection, xTitle);
        else if (termSc.PartialCrossSectionEnergy.isEmpty())
            drawCrossSection(termAb.PartialCrossSectionEnergy, termAb.PartialCrossSection, xTitle);
        else
        {
            QVector<double> energy = termAb.PartialCrossSectionEnergy;
            QVector<double> cs = termAb.PartialCrossSection;
            for (int i=0; i<energy.size(); i++)
            {
                double val = GetInterpolatedValue(energy.at(i), &termSc.PartialCrossSectionEnergy, &termSc.PartialCrossSection, bLogLog);
                cs[i] += val;
            }
            drawCrossSection(energy, cs, xTitle);
        }
    }
}

void ANeutronInfoDialog::on_pbAbs_clicked()
{
    if (!bShowAbs) return;

    if (!mat) return;
    if (mat->MatParticle.isEmpty()) return;
    if (ipart<0 || ipart>=mat->MatParticle.size()) return;
    if (mat->MatParticle.at(ipart).Terminators.size() != 2) return;

    const NeutralTerminatorStructure& termAb = mat->MatParticle.at(ipart).Terminators.at(0);

    drawCrossSection(termAb.PartialCrossSectionEnergy, termAb.PartialCrossSection, TString("Absorption cross-section, barns"));
}

void ANeutronInfoDialog::on_pbScatter_clicked()
{
    if (!bShowScat) return;

    if (!mat) return;
    if (mat->MatParticle.isEmpty()) return;
    if (ipart<0 || ipart>=mat->MatParticle.size()) return;
    if (mat->MatParticle.at(ipart).Terminators.size() != 2) return;

    const NeutralTerminatorStructure& termSc = mat->MatParticle.at(ipart).Terminators.at(1);

    drawCrossSection(termSc.PartialCrossSectionEnergy, termSc.PartialCrossSection, TString("Ellastic scattering cross-section, barns"));
}

void ANeutronInfoDialog::drawCrossSection(const QVector<double>& energy, const QVector<double>& cs, const TString &xTitle)
{
    if (energy.isEmpty()) return;

    QVector<double> x,y;
    for (int i=0; i<energy.size(); i++)
      {
        x << 1.0e6 * energy.at(i);  // keV -> meV
        y << 1.0e24 * cs.at(i);     // cm2 to barns
      }

    GraphWindow->ShowAndFocus();
    TGraph* gr = GraphWindow->ConstructTGraph(x, y, mat->name.toLocal8Bit(),
                                              "Energy, meV", xTitle,
                                              kRed, 2, 1, kRed, 1, 2);
    GraphWindow->Draw(gr, "AL");

//    TGraph* graphOver = constructInterpolationGraph(x, y);
//    graphOver->SetLineColor(kRed);
//    graphOver->SetLineWidth(1);
//    MW->GraphWindow->Draw(graphOver, "L same");
}

void ANeutronInfoDialog::onIsotopeTable_ColumnSelected(int column)
{
    ui->tabwIso->sortByColumn(column);
}

ATableWidgetDoubleItem::ATableWidgetDoubleItem(QString text) :
    QTableWidgetItem(text)
{
    setTextAlignment(Qt::AlignCenter);
}

bool ATableWidgetDoubleItem::operator< (const QTableWidgetItem &other) const
{
    bool bOK;
    double val = text().toDouble(&bOK);
    if (!bOK) return true;

    double otherVal = other.text().toDouble(&bOK);
    if (!bOK) return true;

    return val > otherVal;
}
