#include "DetectorConstruction.hh"
#include "SessionManager.hh"
#include "SensitiveDetectorIdeal.hh"

#include "G4SystemOfUnits.hh"
#include "G4Element.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"
#include "G4Transform3D.hh"
#include "G4PVPlacement.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4SDManager.hh"

#include "G4OpticalSurface.hh"
#include "G4LogicalBorderSurface.hh"

G4VPhysicalVolume* DetectorConstruction::Construct()
{
    G4NistManager * man = G4NistManager::Instance();
    SessionManager & SM = SessionManager::getInstance();

    // -- Materials --
    std::vector<G4int> natoms;
    std::vector<G4String> elements;
    elements.push_back("Gd");     natoms.push_back(2);
    elements.push_back("Si");     natoms.push_back(1);
    elements.push_back("O");      natoms.push_back(5);
    G4double density = 6.7*g/cm3;
    G4Material * matScint = man->ConstructNewMaterial("GSO", elements, natoms, density);
    setScintillatorOpticalProperties(matScint);

    G4Material * matTeflon = man->FindOrBuildMaterial("G4_TEFLON");
    setTeflonOpticalProperties(matTeflon);

    G4Material * matGlass = man->FindOrBuildMaterial("G4_Galactic"); // does not matter what composition it has - only optical photons are simulated
    setGlassOpticalProperties(matGlass);

    G4Material * matDummy = man->FindOrBuildMaterial("G4_Galactic"); // does not have optical properties - will kill photons

    // -- optical wrapper --
    G4OpticalSurface* ScintWrapper = new G4OpticalSurface("wrapper");
    ScintWrapper->SetModel(DAVIS);
    ScintWrapper->SetType(dielectric_LUTDAVIS);
    ScintWrapper->SetFinish(PolishedTeflon_LUT);
    //const int NUM = 2;
    //double pp[NUM] = {2.0*eV, 8.0*eV};
    //double reflectivity[NUM] = {1.0, 1.0};
    //double efficiency[NUM] = {0, 0};
    G4MaterialPropertiesTable* wrapperProperty = new G4MaterialPropertiesTable();
    //wrapperProperty->AddProperty("REFLECTIVITY", pp, reflectivity, NUM);
    //wrapperProperty->AddProperty("EFFICIENCY", pp, efficiency, NUM);
    ScintWrapper->SetMaterialPropertiesTable(wrapperProperty);

    // -- SD --
    SensitiveDetectorIdeal * pSD_Ideal = new SensitiveDetectorIdeal("Ideal");
    G4SDManager::GetSDMpointer()->AddNewDetector(pSD_Ideal);

    // -- Geometry --

    G4double BeamToCollSurface = 250.0*mm;

    G4double colSizeX  = 200.0*mm; // along the beam
    G4double colSizeY  = 500.0*mm; // width
    G4double colSizeZ  = 200.0*mm; // height

    G4double septa     = 2.4*mm;
    G4double aperture  = 5.1*mm;
    G4double pitch     = septa + aperture;

    G4double sciSizeX  = 2.0*mm; // should be < 0.5*aperture !
    G4double sciSizeZ  = 30.0*mm;
    G4double tapeX     = (aperture - 2.0 * sciSizeX) * 0.25;

    G4double teflonTop = 2.0*mm;

    G4int    numEl     = 1;//colSizeX / pitch;
    SM.NumScint        = numEl * 2;
    std::cout << "Number of scintillators: " << SM.NumScint << std::endl;
    G4int HalfNumWalls = ceil(0.5*colSizeY / sciSizeZ);
    G4double slabSize = 0.5*colSizeY / HalfNumWalls;
    std::cout << "Number or scintillator slabs per plane: " << HalfNumWalls*2 << "      Slab size: " << slabSize << std::endl;

    G4double IdealDetectorHeight = 2.0*mm;

    // WORLD
    G4Box             * solidWorld = new G4Box("World", 450.0*mm, 300.0*mm, colSizeZ + 2.0*BeamToCollSurface + 10.0*mm);
    G4LogicalVolume   * logicWorld = new G4LogicalVolume(solidWorld, matTeflon, "World");
    G4VPhysicalVolume * physWorld = new G4PVPlacement(nullptr, G4ThreeVector(0.0, 0.0, 0.0), logicWorld, "World", 0, false, 0);
    logicWorld->SetVisAttributes(G4VisAttributes(G4Colour(0.0, 1.0, 0.0)));
    logicWorld->SetVisAttributes(G4VisAttributes::Invisible);

    // Solids
    G4VSolid * solidColBlade = new G4Box("ColBlade", 0.5*septa, 0.5*colSizeY, 0.5*colSizeZ);
    G4VSolid * solidTeflon   = new G4Box("Teflon", 0.5*aperture, 0.5*colSizeY, 0.5*sciSizeZ + 0.5*teflonTop);
    G4VSolid * solidScint    = new G4Box("Scint", 0.5*sciSizeX, 0.5*colSizeY, 0.5*sciSizeZ);
    G4VSolid * solidWall     = new G4Box("Wall", 0.5*sciSizeX, 0.1*mm, 0.5*sciSizeZ);
    G4VSolid * solidIdeal    = new G4Box("Sensor", 0.5*sciSizeX, 0.5*colSizeY, 0.5*IdealDetectorHeight);

    // Resuable logical volumes
    G4LogicalVolume * logicWall = new G4LogicalVolume(solidWall, matTeflon, "Wall");
    logicWall->SetVisAttributes(G4VisAttributes(G4Colour(1.0, 0, 0)));
    G4LogicalVolume * logicIdeal = new G4LogicalVolume(solidIdeal, matGlass, "Sensor");
    logicIdeal->SetSensitiveDetector(pSD_Ideal);
    logicIdeal->SetVisAttributes(G4VisAttributes(G4Colour(1.0, 1.0, 0)));

    // Building geometry:
    std::vector<double> scintPos;
    int iWallCounter = 0;
    int iSensorCounter = 0;
    for (int iCol = 0; iCol <= numEl; iCol++)
    {
        G4double offset = iCol*pitch;

        //collimator blade
        G4LogicalVolume * logicColBlade = new G4LogicalVolume(solidColBlade, matDummy, "ColBlade");
        logicColBlade->SetVisAttributes(G4VisAttributes(G4Colour(0, 0.0, 1.0)));
        G4double bladeZ = -BeamToCollSurface - 0.5*colSizeZ;
        new G4PVPlacement(nullptr, G4ThreeVector(offset, 0, bladeZ), logicColBlade, "Blade", logicWorld, true, iCol);

        if (iCol == numEl) break; // last blade

        //teflon wrapper box
        G4LogicalVolume * logicTeflon = new G4LogicalVolume(solidTeflon, matTeflon, "Teflon");
        logicTeflon->SetVisAttributes(G4VisAttributes(G4Colour(1.0, 0.0, 0.0)));
        G4double teflonBoxZ = -BeamToCollSurface - colSizeZ + 0.5*sciSizeZ    + 0.5*teflonTop;
        G4VPhysicalVolume * physTeflonBox = new G4PVPlacement(nullptr, G4ThreeVector(offset + 0.5*pitch, 0, teflonBoxZ), logicTeflon, "Tf", logicWorld, true, iCol);

        //scintillator: Left
        G4LogicalVolume * logicScintL = new G4LogicalVolume(solidScint, matScint, "ScintL");
        //logicScintL->SetSensitiveDetector(pSD_Scint);
        logicScintL->SetVisAttributes(G4VisAttributes(G4Colour(1.0, 1.0, 1.0)));
        G4double posLeft  = -tapeX - 0.5*sciSizeX;
        scintPos.push_back(offset + 0.5*pitch + posLeft);
        G4VPhysicalVolume * physScintLeft = new G4PVPlacement(nullptr, G4ThreeVector(posLeft , 0, -0.5*teflonTop), logicScintL, "ScL", logicTeflon, true, iCol*2);
        new G4LogicalBorderSurface("wrapper", physScintLeft, physTeflonBox, ScintWrapper);

        //scintillator: Right
        G4LogicalVolume * logicScintR = new G4LogicalVolume(solidScint, matScint, "ScintR");
        //logicScintR->SetSensitiveDetector(pSD_Scint);
        logicScintR->SetVisAttributes(G4VisAttributes(G4Colour(1.0, 1.0, 1.0)));
        G4double posRight =  tapeX + 0.5*sciSizeX;
        scintPos.push_back(offset + 0.5*pitch + posRight);
        G4VPhysicalVolume * physScintRight = new G4PVPlacement(nullptr, G4ThreeVector(posRight, 0, -0.5*teflonTop), logicScintR, "ScR", logicTeflon, true, iCol*2+1);
        new G4LogicalBorderSurface("wrapper", physScintRight, physTeflonBox, ScintWrapper);

        //teflon wall splitters
        for (int iW = 0; iW< HalfNumWalls; iW++)
        {
            G4VPhysicalVolume * physWall = new G4PVPlacement(nullptr, G4ThreeVector(0, slabSize*iW, 0), logicWall, "Wall", logicScintL, true, iWallCounter++);
            new G4LogicalBorderSurface("wrapper", physScintLeft, physWall, ScintWrapper);
            if (iW != 0)
            {
                physWall = new G4PVPlacement(nullptr, G4ThreeVector(0, -slabSize*iW, 0), logicWall, "Wall", logicScintL, true, iWallCounter++);
                new G4LogicalBorderSurface("wrapper", physScintLeft, physWall, ScintWrapper);
            }

            physWall = new G4PVPlacement(nullptr, G4ThreeVector(0, slabSize*iW, 0), logicWall, "Wall", logicScintR, true, iWallCounter++);
            new G4LogicalBorderSurface("wrapper", physScintRight, physWall, ScintWrapper);
            if (iW != 0)
            {
                physWall = new G4PVPlacement(nullptr, G4ThreeVector(0, -slabSize*iW, 0), logicWall, "Wall", logicScintR, true, iWallCounter++);
                new G4LogicalBorderSurface("wrapper", physScintRight, physWall, ScintWrapper);
            }
        }

        //photosensors
        G4double Z = -BeamToCollSurface - colSizeZ -0.5*IdealDetectorHeight;
        posLeft   += offset + 0.5*pitch;//  - tapeX - 0.5*sciSizeX;
        new G4PVPlacement(nullptr, G4ThreeVector(posLeft, 0, Z), logicIdeal, "IdealL", logicWorld, true, iSensorCounter++);
        posRight  += offset + 0.5*pitch;//  + tapeX + 0.5*sciSizeX;
        new G4PVPlacement(nullptr, G4ThreeVector(posRight, 0, Z), logicIdeal, "IdealR", logicWorld, true, iSensorCounter++);
    }

    return physWorld;
}

void DetectorConstruction::setTeflonOpticalProperties(G4Material * mat)
{
    double energyArr[]    = { 2.0*eV, 8.0*eV };
    double refIndexArr[]  = { 1.35, 1.35 };
    double absLengthArr[] = { 1.0e6*m, 1.0e6*mm }; // intentionally very large absorption!

    G4MaterialPropertiesTable * mpt = new G4MaterialPropertiesTable();

    mpt->AddProperty("RINDEX",        energyArr, refIndexArr, 2);
    mpt->AddProperty("ABSLENGTH",     energyArr, absLengthArr, 2);

    mat->SetMaterialPropertiesTable(mpt);
}

void DetectorConstruction::setScintillatorOpticalProperties(G4Material *mat)
{
    const G4int nE = 2;
    G4double energyArr[nE] = { 2.0*eV, 8.0*eV };
    G4double refIndexArr[nE]  = { 1.9, 1.9 };
    G4double absLengthArr[nE]  = { 20.8*cm, 20.8*cm }; //doi: 10.1134/S154747711403011X
    //https://www.semanticscholar.org/paper/Investigation-of-LYSO-and-GSO-crystals-and-of-the-Kalinnikov-Velicheva/7f725600453abe5a559334ee0f6b1299476cb397

    G4MaterialPropertiesTable * mpt = new G4MaterialPropertiesTable();

    mpt->AddProperty("RINDEX",        energyArr, refIndexArr, nE);
    mpt->AddProperty("ABSLENGTH",     energyArr, absLengthArr, nE);

    mat->SetMaterialPropertiesTable(mpt);
}

void DetectorConstruction::setGlassOpticalProperties(G4Material *mat)
{
    const G4int nE = 2;
    G4double energyArr[nE] = { 2.0*eV, 8.0*eV };
    G4double refIndexArr[nE]  = { 1.5, 1.5 };
    G4double absLengthArr[nE]  = { 0.01*mm, 0.01*mm }; //intentionally very strong absorption!

    G4MaterialPropertiesTable * mpt = new G4MaterialPropertiesTable();

    mpt->AddProperty("RINDEX",        energyArr, refIndexArr, nE);
    mpt->AddProperty("ABSLENGTH",     energyArr, absLengthArr, nE);

    mat->SetMaterialPropertiesTable(mpt);
}
