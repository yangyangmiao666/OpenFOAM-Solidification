/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2018-2026 OpenFOAM Foundation
     \\/     M anipulation  |
\*---------------------------------------------------------------------------*/

#include "solidificationSystem.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(solidificationSystem, 0);
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::solidificationSystem::solidificationSystem
(
    const volVectorField& U
)
:
    IOdictionary
    (
        IOobject
        (
            "transportProperties",
            U.time().constant(),
            U.db(),
            IOobject::MUST_READ_IF_MODIFIED,
            IOobject::NO_WRITE
        )
    ),

    phase1Name_(wordList(lookup("phases"))[0]),
    phase2Name_(wordList(lookup("phases"))[1]),

    alpha1_
    (
        IOobject
        (
            IOobject::groupName("alpha", phase1Name_),
            U.time().name(),
            U.mesh(),
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        U.mesh()
    ),

    alpha2_
    (
        IOobject
        (
            IOobject::groupName("alpha", phase2Name_),
            U.time().name(),
            U.mesh()
        ),
        scalar(1) - alpha1_
    ),

    rho1_("rho", dimDensity, subDict(phase1Name_)),
    rho2_("rho", dimDensity, subDict(phase2Name_)),

    Cp1_("Cp", dimSpecificHeatCapacity, subDict(phase1Name_)),
    Cp2_("Cp", dimSpecificHeatCapacity, subDict(phase2Name_)),

    kappa1_("kappa", dimThermalConductivity, subDict(phase1Name_)),
    kappa2_("kappa", dimThermalConductivity, subDict(phase2Name_)),

    mu1_("mu", dimDynamicViscosity, subDict(phase1Name_)),
    mu2_("mu", dimDynamicViscosity, subDict(phase2Name_)),

    D1_("D", dimKinematicViscosity, subDict(phase1Name_)),
    D2_("D", dimKinematicViscosity, subDict(phase2Name_)),

    DAS_("DAS", dimLength, subDict(phase1Name_)),

    betaT_("betaT", dimless/dimTemperature, subDict(phase2Name_)),
    betaC_("betaC", dimless, subDict(phase2Name_)),

    TRef_("TRef", dimTemperature, subDict(phase2Name_)),
    CRef_("CRef", dimless, subDict(phase2Name_)),

    U_(U)
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

bool Foam::solidificationSystem::read()
{
    if (regIOobject::read())
    {
        const dictionary& phaseDict1(subDict(phase1Name_));
        const dictionary& phaseDict2(subDict(phase2Name_));

        rho1_ = dimensionedScalar("rho", dimDensity, phaseDict1);
        rho2_ = dimensionedScalar("rho", dimDensity, phaseDict2);

        Cp1_ = dimensionedScalar("Cp", dimSpecificHeatCapacity, phaseDict1);
        Cp2_ = dimensionedScalar("Cp", dimSpecificHeatCapacity, phaseDict2);

        kappa1_ = dimensionedScalar("kappa", dimThermalConductivity, phaseDict1);
        kappa2_ = dimensionedScalar("kappa", dimThermalConductivity, phaseDict2);

        mu1_ = dimensionedScalar("mu", dimDynamicViscosity, phaseDict1);
        mu2_ = dimensionedScalar("mu", dimDynamicViscosity, phaseDict2);

        D1_ = dimensionedScalar("D", dimKinematicViscosity, phaseDict1);
        D2_ = dimensionedScalar("D", dimKinematicViscosity, phaseDict2);

        DAS_ = dimensionedScalar("DAS", dimLength, phaseDict1);

        betaT_ = dimensionedScalar("betaT", dimless/dimTemperature, phaseDict2);
        betaC_ = dimensionedScalar("betaC", dimless, phaseDict2);

        TRef_ = dimensionedScalar("TRef", dimTemperature, phaseDict2);
        CRef_ = dimensionedScalar("CRef", dimless, phaseDict2);

        return true;
    }
    else
    {
        return false;
    }
}


// ************************************************************************* //
