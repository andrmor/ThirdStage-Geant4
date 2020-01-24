#include "SessionManager.hh"

#include <iostream>
#include <sstream>
#include <fstream>

#include "G4UImanager.hh"

SessionManager &SessionManager::getInstance()
{
    static SessionManager instance; // Guaranteed to be destroyed, instantiated on first use.
    return instance;
}

SessionManager::SessionManager(){}

SessionManager::~SessionManager()
{
    endSession();
}

void SessionManager::startSession()
{
    initWaveforms();

    prepareInputStream();
}

void SessionManager::endSession()
{
    saveWaveforms();

    if (inStream)   inStream->close();
    delete inStream; inStream = nullptr;
}

void SessionManager::runSimulation()
{
    G4UImanager* UImanager = G4UImanager::GetUIpointer();
    while (!isEndOfInputFileReached())
    {
        if (NextEventId % 1000 == 0)
            std::cout << NextEventId << std::endl;

        UImanager->ApplyCommand("/run/beamOn");
    }
}

bool SessionManager::isEndOfInputFileReached() const
{
    if (!inStream) return true;
    return inStream->eof();
}

void SessionManager::saveWaveforms()
{
    std::cout << "Output file name template:" << std::endl << FileName_Output << std::endl;

    for (int iS = 0; iS < NumScint; iS++)
    {
        std::ofstream outStream;// = new std::ofstream();
        std::string name = FileName_Output + std::to_string(iS) + ".txt";
        outStream.open(name);
        if (!outStream.is_open())
            terminateSession("Cannot open output file: " + FileName_Output);

        std::stringstream text;
        for (int iW = 0; iW < WaveformLength; iW++)
            text << Waveforms[iS][iW] << '\n';

        outStream << text.rdbuf() << std::endl;
        outStream.close();
    }
}

void SessionManager::prepareInputStream()
{
    if (bBinaryInput) inStream = new std::ifstream(FileName_Input, std::ios::in | std::ios::binary);
    else              inStream = new std::ifstream(FileName_Input);

    if (!inStream->is_open())
        terminateSession("Cannot open input file: " + FileName_Input);

    //first line should start with the event tag and contain the event number
    if (bBinaryInput)
    {
        char header = 0;
        *inStream >> header;
        if (header != char(0xee))
            terminateSession("Unexpected format of the input file (Event tag not found)");
        inStream->read((char*)&NextEventId, sizeof(int));
        if (inStream->fail())
            terminateSession("Unexpected format of the input file (Event tag not found)");
    }
    else
    {
        std::getline( *inStream, EventIdString );
        if (EventIdString.size() < 2 || EventIdString[0] != '#')
            terminateSession("Unexpected format of the input file (Event tag not found)");
        try
        {
            NextEventId = std::stoi(EventIdString.substr(1, EventIdString.size()-1));
        }
        catch (...)
        {
            terminateSession("Unexpected format of the input file (Event tag '#' not found)");
        }
    }
    std::cout << '#' << NextEventId << std::endl;
}

void SessionManager::initWaveforms()
{
    Waveforms.resize(NumScint);
    for (auto & w : Waveforms)
        w.assign(WaveformLength, 0);
}

std::vector<DepositionRecord> & SessionManager::getNextEventDeposition()
{
    ThisEventDeposition.clear();

    if (bBinaryInput)  readEventFromBinaryInput();
    else               readEventFromTextInput();

    return ThisEventDeposition;
}

void SessionManager::readEventFromTextInput()
{
    for( std::string line; std::getline( *inStream, line ); )
    {
        //std::cout << line << std::endl;
        if (line.size() < 1) continue; //allow empty lines

        if (line[0] == '#')
        {
            try
            {
                NextEventId = std::stoi( line.substr(1, line.size()-1) );
            }
            catch (...)
            {
                terminateSession("Unexpected format of the input file: event number format error");
            }
            break; //event finished
        }

        DepositionRecord r;

        std::string particle;
        std::stringstream ss(line);  // #scint particleName depoEnergy[keV] time[ns] x[mm] y[mm] z[mm]
        ss >> r.ScintIndex >> particle >> r.Energy >> r.Time
           >> r.Position[0]  >> r.Position[1] >>  r.Position[2];

        if (ss.fail())
            terminateSession("Unexpected format of a line in the input deposition file");

        //r.Particle = makeGeant4Particle(particle);
        //std::cout << str << ' ' << r.Energy << ' ' << r.Time << ' ' << r.Position[0] << ' ' << r.Position[1] << ' ' << r.Position[2] << ' ' << r.Direction[0] << ' ' << r.Direction[1] << ' ' << r.Direction[2] << std::endl;
        ThisEventDeposition.push_back(r);
    }
}

void SessionManager::readEventFromBinaryInput()
{
    char header = 0;

    while (*inStream >> header)
    {
        if (header == char(0xee))
        {
            inStream->read((char*)&NextEventId, sizeof(int));
            //std::cout << '#' << NextEventId << std::endl;
            break; //event finished
        }
        else if (header == char(0xff))
        {
            DepositionRecord r;
            inStream->read((char*)&r.Energy,       sizeof(double));
            inStream->read((char*)&r.Time,         sizeof(double));
            inStream->read((char*)&r.Position[0],  sizeof(double));
            inStream->read((char*)&r.Position[1],  sizeof(double));
            inStream->read((char*)&r.Position[2],  sizeof(double));
            char ch;
            std::string str;
            while (*inStream >> ch)
            {
                if (ch == 0x00) break;
                str += ch;
            }
            if (inStream->fail())
                terminateSession("Unexpected format of a line in the binary file with the input particles");

            //r.Particle = makeGeant4Particle(str);
            //std::cout << str << ' ' << r.Energy << ' ' << r.Time << ' ' << r.Position[0] << ' ' << r.Position[1] << ' ' << r.Position[2] << ' ' << r.Direction[0] << ' ' << r.Direction[1] << ' ' << r.Direction[2] << std::endl;
            ThisEventDeposition.push_back(r);
        }
        else
        {
            terminateSession("Unexpected format of binary input");
            break;
        }
    }
}

void SessionManager::terminateSession(const std::string & ErrorMessage)
{
    std::cerr << ErrorMessage << std::endl;
    endSession();
    exit(1);
}
