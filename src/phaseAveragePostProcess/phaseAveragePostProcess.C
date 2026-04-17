// Phase-averaging post-processing utility for OpenFOAM 12.
// Reads fields from selected time directories, computes the
// arithmetic ensemble mean, and writes the result. This is the
// OpenFOAM 12 replacement for "pimpleFoam_WK_OF4_postProcess
// -postProcess" with fieldAverage.
//
// Unlike fieldAverage (which depends on the time-loop deltaT for
// its beta coefficient), this utility computes a straight 1/N
// arithmetic mean of exactly the time directories present in the
// case, with no dependence on deltaT or time spacing.

#include "argList.H"
#include "timeSelector.H"
#include "fvMesh.H"
#include "volFields.H"
#include "IOobjectList.H"

using namespace Foam;

int main(int argc, char *argv[])
{
    timeSelector::addOptions();

    #include "setRootCase.H"
    #include "createTime.H"

    instantList timeDirs = timeSelector::select0(runTime, args);

    #include "createMesh.H"

    if (timeDirs.size() < 2)
    {
        FatalErrorIn("phaseAveragePostProcess")
            << "Need at least 2 time directories to average."
            << exit(FatalError);
    }

    Info<< "\nPhase-averaging " << timeDirs.size()
        << " time directories\n" << endl;

    // --- Pass 1: read the first time dir to identify fields and dimensions
    runTime.setTime(timeDirs[0], 0);
    IOobjectList objects0(mesh, runTime.name());

    wordList scalarNames(objects0.names(volScalarField::typeName));
    wordList vectorNames(objects0.names(volVectorField::typeName));

    // Filter out mesh-utility fields (cellLevel, pointLevel, etc.)
    {
        wordList filteredS, filteredV;
        forAll(scalarNames, i)
        {
            if (scalarNames[i] != "cellLevel" && scalarNames[i] != "pointLevel")
            {
                filteredS.append(scalarNames[i]);
            }
        }
        forAll(vectorNames, i)
        {
            if (vectorNames[i].find("Mean") == std::string::npos)
            {
                filteredV.append(vectorNames[i]);
            }
        }
        scalarNames = filteredS;
        vectorNames = filteredV;
    }

    Info<< "Scalar fields: " << scalarNames << nl
        << "Vector fields: " << vectorNames << nl << endl;

    // Read the first time step to get dimensions and boundary types
    PtrList<volScalarField> scalarRef(scalarNames.size());
    forAll(scalarNames, i)
    {
        scalarRef.set
        (
            i,
            new volScalarField
            (
                IOobject
                (
                    scalarNames[i],
                    runTime.name(),
                    mesh,
                    IOobject::MUST_READ,
                    IOobject::NO_WRITE
                ),
                mesh
            )
        );
    }
    PtrList<volVectorField> vectorRef(vectorNames.size());
    forAll(vectorNames, i)
    {
        vectorRef.set
        (
            i,
            new volVectorField
            (
                IOobject
                (
                    vectorNames[i],
                    runTime.name(),
                    mesh,
                    IOobject::MUST_READ,
                    IOobject::NO_WRITE
                ),
                mesh
            )
        );
    }

    // Initialise accumulators with correct dimensions, zero values
    PtrList<volScalarField> scalarSum(scalarNames.size());
    forAll(scalarNames, i)
    {
        scalarSum.set(i, new volScalarField(scalarRef[i] * 0.0));
        scalarSum[i].rename(scalarNames[i] + "Sum");
    }
    PtrList<volVectorField> vectorSum(vectorNames.size());
    forAll(vectorNames, i)
    {
        vectorSum.set(i, new volVectorField(vectorRef[i] * 0.0));
        vectorSum[i].rename(vectorNames[i] + "Sum");
    }

    // Release reference fields
    scalarRef.clear();
    vectorRef.clear();

    // --- Accumulate sum over all selected time directories
    label N = 0;
    forAll(timeDirs, timeI)
    {
        runTime.setTime(timeDirs[timeI], timeI);
        Info<< "  Reading time = " << runTime.name() << endl;

        mesh.readUpdate();

        forAll(scalarNames, i)
        {
            volScalarField f
            (
                IOobject
                (
                    scalarNames[i],
                    runTime.name(),
                    mesh,
                    IOobject::MUST_READ,
                    IOobject::NO_WRITE
                ),
                mesh
            );
            scalarSum[i] += f;
        }

        forAll(vectorNames, i)
        {
            volVectorField f
            (
                IOobject
                (
                    vectorNames[i],
                    runTime.name(),
                    mesh,
                    IOobject::MUST_READ,
                    IOobject::NO_WRITE
                ),
                mesh
            );
            vectorSum[i] += f;
        }

        N++;
    }

    Info<< "\n  Averaging over " << N << " samples" << endl;

    forAll(scalarSum, i)
    {
        scalarSum[i] /= scalar(N);
        scalarSum[i].rename(scalarNames[i] + "Mean");
    }
    forAll(vectorSum, i)
    {
        vectorSum[i] /= scalar(N);
        vectorSum[i].rename(vectorNames[i] + "Mean");
    }

    // --- Write to the last time directory
    runTime.setTime(timeDirs.last(), timeDirs.size() - 1);
    Info<< "  Writing to time = " << runTime.name() << nl << endl;

    forAll(scalarSum, i)
    {
        scalarSum[i].write();
    }
    forAll(vectorSum, i)
    {
        vectorSum[i].write();
    }

    Info<< "End\n" << endl;
    return 0;
}
