#include "SteppingAction.hh"
#include "SessionManager.hh"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4StepPoint.hh"
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"

#include <QDebug>

void SteppingAction::UserSteppingAction(const G4Step *step)
{
    SessionManager & SM = SessionManager::getInstance();

    //qDebug() << step->GetTrack()->GetParticleDefinition()->GetParticleName();
    //const G4StepPoint * postP  = step->GetPostStepPoint();

    /*
        const double time = postP->GetGlobalTime()/ns;
        if (time > SM.TimeLimit) return;

        double buf[6];
        const G4ThreeVector & pos = postP->GetPosition();
        buf[0] = pos[0]/mm;
        buf[1] = pos[1]/mm;
        buf[2] = pos[2]/mm;
        const G4ThreeVector & dir = postP->GetMomentumDirection();
        buf[3] = dir[0];
        buf[4] = dir[1];
        buf[5] = dir[2];

        SM.saveParticle(step->GetTrack()->GetParticleDefinition()->GetParticleName(),
                        postP->GetKineticEnergy()/keV,
                        time,
                        buf);
 */
}
