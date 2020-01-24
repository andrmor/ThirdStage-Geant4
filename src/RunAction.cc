#include "RunAction.hh"
#include "SessionManager.hh"

#include "G4Run.hh"
#include "G4RunManager.hh"

#include <QDebug>

RunAction::RunAction()
    : G4UserRunAction()
{ 
    // set printing event number per each 10 events
    G4RunManager::GetRunManager()->SetPrintProgress(1);
}

RunAction::~RunAction() {}


void RunAction::BeginOfRunAction(const G4Run*)
{
    //qDebug() << "Starting new run...";

    //SessionManager & SM = SessionManager::getInstance();

    //G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
    //SM.proton = particleTable->FindParticle("proton");
    //SM.gamma = particleTable->FindParticle("gamma");
    //SM.neutron = particleTable->FindParticle("neutron");

    //qDebug() << "Run start";
    /*

    SM.resetPredictedTrackID();

    SM.sendLineToDepoOutput(SM.getEventId());

    if (SM.getNumEventsForTrackExport() > 0)
        SM.sendLineToTracksOutput(SM.getEventId());

    //inform the runManager to save random number seed             *** need?
    //G4RunManager::GetRunManager()->SetRandomNumberStore(false);
    */
}

#include <QString>
void RunAction::EndOfRunAction(const G4Run* )
{
    /*
    qDebug() << "Run finished";

    SessionManager & SM = SessionManager::getInstance();
    qDebug() << "Detected " << SM.NumPhotonsDetected << "photons from" << SM.NumPhotonsGenerated << "generated";

    for (int iS = 0; iS < SM.NumScint; iS++)
    {
        qDebug() << "Scint #" << iS;
        QString s;
        for (int iW = 0; iW < SM.WaveformLength; iW++)
            s += QString::number(SM.Waveforms[iS][iW]) + " ";
        qDebug() << "  " << s;
    }
    */
}
