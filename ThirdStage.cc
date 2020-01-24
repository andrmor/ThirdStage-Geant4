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

    SM.RiseTime             = 10.0;
    SM.DecayTime            = 45.0;

    std::string WorkingDir  = "/home/andr/tmp/test";
    SM.bBinaryInput         = false;
    std::string InputFile   = "depo.txt";

    // --- end of user inits ---

    SM.FileName_Input  = WorkingDir + "/" + InputFile;

    //std::string tmp = InputFile;
    //tmp.resize(InputFile.size()-4);
    SM.FileName_Output = WorkingDir + "/" + "Optical__" + InputFile+ "__";
    SM.FileName_Output += ".txt";

    std::cout << "Output file name: " << SM.FileName_Output << std::endl;

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

    std::cout << "Output file name:" << std::endl << SM.FileName_Output << std::endl;
}
