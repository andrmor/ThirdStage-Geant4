#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "SteppingAction.hh"
#include "RunAction.hh"

ActionInitialization::ActionInitialization()
    : G4VUserActionInitialization() {}

ActionInitialization::~ActionInitialization() {}

void ActionInitialization::Build() const
{
    SetUserAction(new PrimaryGeneratorAction);
    SetUserAction(new SteppingAction);
    SetUserAction(new RunAction);
}
