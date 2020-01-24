#ifndef DetectorConstruction_H
#define DetectorConstruction_H

#include "G4VUserDetectorConstruction.hh"

class G4VPhysicalVolume;
class G4Material;

class DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
     G4VPhysicalVolume* Construct();

private:
     void setTeflonOpticalProperties(G4Material * mat);
     void setScintillatorOpticalProperties(G4Material * mat);
     void setGlassOpticalProperties(G4Material * mat);

     //void setOptical_TeflonWrap(G4VPhysicalVolume *from, G4VPhysicalVolume *to);
};

#endif //DetectorConstruction_H
