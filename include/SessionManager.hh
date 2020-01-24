#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <string>
#include <vector>

#include "G4ThreeVector.hh"

struct DepositionRecord
{
    int             ScintIndex = 0;
    G4double        Energy = 0;
    G4ThreeVector   Position  = {0, 0, 0};
    G4double        Time = 0;
};

class G4Material;
namespace CLHEP{class RanecuEngine;}

class SessionManager
{
    public:
        static SessionManager& getInstance();

    private:
        SessionManager();
        ~SessionManager();

    public:
        SessionManager(SessionManager const&) = delete;
        void operator=(SessionManager const&) = delete;

        void startSession();
        void endSession();

        void runSimulation();

        bool bGuiMode               = false;

        //int NumPhotonsDetected = 0;

        double RiseTime  = 0;
        double DecayTimeFast = 0;
        double FastDecayFraction = 1.0;
        double DecayTimeSlow = 0;

        bool bBinaryInput           = false;
        std::string FileName_Input;

        std::string FileName_Output;

        int NextEventId = 0;

        CLHEP::RanecuEngine * randGen = nullptr;

        int NumScint = 1;
        int WaveformLength = 10000;
        std::vector<std::vector<short>> Waveforms;

public:
        void terminateSession(const std::string & ErrorMessage);
        void prepareInputStream();
        void initWaveforms();
        void saveWaveforms();
        bool isEndOfInputFileReached() const;

        std::vector<DepositionRecord> & getNextEventDeposition();

private:
        std::ifstream * inStream = nullptr;
        std::vector<DepositionRecord> ThisEventDeposition;
        std::string EventIdString = "#0";

private:
        void readEventFromTextInput();
        void readEventFromBinaryInput();
};

#endif // SESSIONMANAGER_H
