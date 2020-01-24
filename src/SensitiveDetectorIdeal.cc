#include "SensitiveDetectorIdeal.hh"
#include "SessionManager.hh"

#include <sstream>

#include "G4Step.hh"
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"

SensitiveDetectorIdeal::SensitiveDetectorIdeal(const G4String & name)
    : G4VSensitiveDetector(name) {}

G4bool SensitiveDetectorIdeal::ProcessHits(G4Step* aStep, G4TouchableHistory*)
{  
    SessionManager & SM = SessionManager::getInstance();

    G4StepPoint * postStep = aStep->GetPostStepPoint();
    G4StepPoint * preStep  = aStep->GetPreStepPoint();
    if (postStep->GetStepStatus() == fGeomBoundary) return true; // triggered also by reflection on the interface!

    //detection of the photon - detector material has very short absorption length
    //++SM.NumPhotonsDetected;

    int iScintIndex = postStep->GetPhysicalVolume()->GetCopyNo();
    int iTime = preStep->GetGlobalTime()/ns;

    if (iTime < SM.WaveformLength)
        ++SM.Waveforms[iScintIndex][iTime];

    aStep->GetTrack()->SetTrackStatus(fStopAndKill);
    return true;
}
