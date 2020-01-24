#include "SessionManager.hh"
#include "DetectorConstruction.hh"
#include "ActionInitialization.hh"

#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "QGSP_BIC_HP.hh"
#include "QGSP_BIC.hh"
#include "G4OpticalPhysics.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"

int main(int argc, char** argv)
{
    SessionManager& SM = SessionManager::getInstance();

    // --- user inits ---

    SM.bGuiMode = false;

    long Seed               = 111111;

    //https://doi.org/10.1016/0168-9002(95)01244-3
    SM.RiseTime             = 14.4;
    SM.DecayTime1           = 41.0;
    SM.FastDecayFraction    = 0.7;
    SM.DecayTime2           = 300.0;

    std::string WorkingDir  = "/home/andr/tmp/test";
    SM.bBinaryInput         = false;
    //std::string InputFile   = "depo.txt";
    std::string InputFile   = "DepoScint__test_seed111111_shift0_runs1000.bin__.txt";

    // --- end of user inits ---

    SM.FileName_Input  = WorkingDir + "/" + InputFile;

    SM.FileName_Output = WorkingDir + "/" + "Optical__" + InputFile+ "__";    
    std::cout << "Output file name template: " << SM.FileName_Output << std::endl;

    SM.randGen = new CLHEP::RanecuEngine();
    SM.randGen->setSeed(Seed);
    G4Random::setTheEngine(SM.randGen);

    G4UIExecutive* ui =  0;
    if (SM.bGuiMode) ui = new G4UIExecutive(argc, argv);

    G4RunManager* runManager = new G4RunManager;

    DetectorConstruction * theDetector = new DetectorConstruction();
    runManager->SetUserInitialization(theDetector);

    G4VModularPhysicsList* physicsList = new QGSP_BIC;
    physicsList->RegisterPhysics(new G4OpticalPhysics());
    runManager->SetUserInitialization(physicsList);

    runManager->SetUserInitialization(new ActionInitialization());

    G4UImanager* UImanager = G4UImanager::GetUIpointer();
    UImanager->ApplyCommand("/run/initialize");
    UImanager->ApplyCommand("/control/verbose 0");
    UImanager->ApplyCommand("/run/verbose 0");
    if (SM.bGuiMode)
    {
        UImanager->ApplyCommand("/hits/verbose 2");
        UImanager->ApplyCommand("/tracking/verbose 2");
        UImanager->ApplyCommand("/control/saveHistory");
    }
    UImanager->ApplyCommand("/run/initialize");

    SM.startSession();

    G4VisManager* visManager = 0;

    if (SM.bGuiMode)
    {
        visManager = new G4VisExecutive("Quiet");
        visManager->Initialize();
        UImanager->ApplyCommand("/control/execute vis.mac");
        ui->SessionStart();
    }
    else
    {
        SM.runSimulation();
    }

    delete visManager;
    delete runManager;
    delete ui;

    SM.endSession();
}
