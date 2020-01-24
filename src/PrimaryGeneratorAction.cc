#include "PrimaryGeneratorAction.hh"
#include "SessionManager.hh"

#include "G4ParticleGun.hh"
#include "G4OpticalPhoton.hh"
#include "G4SystemOfUnits.hh"

PrimaryGeneratorAction::PrimaryGeneratorAction()
    : G4VUserPrimaryGeneratorAction()
{
    fParticleGun = new G4ParticleGun(1);

    fParticleGun->SetParticleDefinition(G4OpticalPhoton::OpticalPhotonDefinition());
    fParticleGun->SetParticlePosition(G4ThreeVector(2.2, 15.0, -435.0)); //position in millimeters - no need units
    fParticleGun->SetParticleMomentumDirection(G4ThreeVector(sqrt(0.5), sqrt(0.5), 0));
    fParticleGun->SetParticlePolarization(G4ThreeVector(1.0, 1.0, 0));
    fParticleGun->SetParticleEnergy(7.07*eV);
    fParticleGun->SetParticleTime(0*ns);
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fParticleGun;
}

double single_exp(double t, double tau2)
{
    return std::exp(-1.0*t/tau2)/tau2;
}

double bi_exp(double t, double tau1,double tau2)
{
    return std::exp(-1.0*t/tau2)*(1-std::exp(-1.0*t/tau1))/tau2/tau2*(tau1+tau2);
}

#include "G4RunManager.hh"
void PrimaryGeneratorAction::GeneratePrimaries(G4Event * anEvent)
{
    SessionManager & SM = SessionManager::getInstance();

    //SM.NumPhotonsDetected = 0;

    const std::vector<DepositionRecord> & Depositions = SM.getNextEventDeposition();
    for (auto & r : Depositions)
    {
        std::cout <<"  Index:" << r.ScintIndex //r.Particle->GetParticleName()
                  <<"  E:"<< r.Energy <<"  T:"<< r.Time
                  <<"  Pos:"<< r.Position[0]  << ' ' << r.Position[1]  << ' ' << r.Position[2] << std::endl;

        //Photon emission position - common for all photons of this record
        fParticleGun->SetParticlePosition(r.Position); //position in millimeters - no need units

        const int NumPhotons = r.Energy * 10.0;
        for (int i = 0; i < NumPhotons; i++)
        {
            //Photon direction    <- using sphere function of Root
            double a=0, b=0, r2=1.0;
            while (r2 > 0.25)
            {
                a  = SM.randGen->flat() - 0.5;
                b  = SM.randGen->flat() - 0.5;
                r2 = a*a + b*b;
            }
            const double scale = 8.0 * sqrt(0.25 - r2);
            G4ThreeVector dir(a*scale, b*scale, -1.0 + 8.0 * r2);
            fParticleGun->SetParticleMomentumDirection(dir);

            //Photon time  <- using approach from Geant4 (G4Scintillation)
            double time;
            double DecayTime = (SM.randGen->flat() < SM.FastDecayFraction ? SM.DecayTimeFast : SM.DecayTimeSlow);
            double d = (SM.RiseTime + DecayTime) / DecayTime;
            while (true)
            {
                double ran1 = SM.randGen->flat();
                double ran2 = SM.randGen->flat();
                time = -1.0 * DecayTime * std::log(1.0 - ran1);
                double gg = d * single_exp(time, DecayTime);
                if (ran2 <= bi_exp(time, SM.RiseTime, DecayTime)/gg)
                    break;
            }
            time += r.Time;
            fParticleGun->SetParticleTime(time*ns);

            fParticleGun->GeneratePrimaryVertex(anEvent);
        }
    }
}
