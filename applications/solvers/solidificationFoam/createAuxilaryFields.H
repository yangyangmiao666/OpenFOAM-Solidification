// Liquidus temperature
volScalarField Tliq
(
    IOobject
    (
        "Tliq",
        runTime.name(),
        mesh,
        IOobject::NO_READ,
        IOobject::AUTO_WRITE
    ),
    mesh,
    dimensionedScalar(dimTemperature, 0)
);

// Solidus temperature
volScalarField Tsol
(
    IOobject
    (
        "Tsol",
        runTime.name(),
        mesh,
        IOobject::NO_READ,
        IOobject::AUTO_WRITE
    ),
    mesh,
    dimensionedScalar(dimTemperature, 0)
);

// Phase compositions
volScalarField C1
(
    IOobject
    (
        IOobject::groupName("C", "solid"),
        runTime.name(),
        mesh,
        IOobject::NO_READ,
        IOobject::AUTO_WRITE
    ),
    C
);

volScalarField C2
(
    IOobject
    (
        IOobject::groupName("C", "liquid"),
        runTime.name(),
        mesh,
        IOobject::NO_READ,
        IOobject::AUTO_WRITE
    ),
    C
);


// Special handling of eutectic reaction
volScalarField::Internal visitedEut
(
    IOobject
    (
        "visitedEut",
        runTime.name(),
        mesh,
        IOobject::NO_READ,
        IOobject::NO_WRITE
    ),
    mesh,
    dimensionedScalar(dimless, 0)
);

volScalarField::Internal alphaEut
(
    IOobject
    (
        "alphaEut",
        runTime.name(),
        mesh,
        IOobject::NO_READ,
        IOobject::NO_WRITE
    ),
    mesh,
    dimensionedScalar(dimless, 0.0)
);

volScalarField dFdT
(
    IOobject
    (
        "dFdT",
        runTime.name(),
        mesh,
        IOobject::NO_READ,
        IOobject::NO_WRITE
    ),
    mesh,
    dimensionedScalar(dimless/dimTemperature, 0.0)
);
