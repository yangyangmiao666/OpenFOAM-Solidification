/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2018-2026 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

Class
    Foam::solidificationSystem

Description
    Two-phase incompressible mixture for binary-alloy solidification
    (Bennon-Incropera). OpenFOAM-14 port: does not inherit twoPhaseMixture.

SourceFiles
    solidificationSystem.C

\*---------------------------------------------------------------------------*/

#ifndef solidificationSystem_H
#define solidificationSystem_H

#include "IOdictionary.H"
#include "volFields.H"
#include "surfaceFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                     Class solidificationSystem Declaration
\*---------------------------------------------------------------------------*/

class solidificationSystem
:
    public IOdictionary
{
    // Private data

        word phase1Name_;
        word phase2Name_;

        volScalarField alpha1_;
        volScalarField alpha2_;

        dimensionedScalar rho1_;
        dimensionedScalar rho2_;

        dimensionedScalar Cp1_;
        dimensionedScalar Cp2_;

        dimensionedScalar kappa1_;
        dimensionedScalar kappa2_;

        dimensionedScalar mu1_;
        dimensionedScalar mu2_;

        dimensionedScalar D1_;
        dimensionedScalar D2_;

        dimensionedScalar DAS_;

        dimensionedScalar betaT_;
        dimensionedScalar betaC_;

        dimensionedScalar TRef_;
        dimensionedScalar CRef_;

        const volVectorField& U_;


public:

    TypeName("solidificationSystem");


    // Constructors

        solidificationSystem(const volVectorField& U);

        solidificationSystem(const solidificationSystem&) = delete;


    //- Destructor
    virtual ~solidificationSystem()
    {}


    // Member Functions

        const word& phase1Name() const { return phase1Name_; }
        const word& phase2Name() const { return phase2Name_; }

        const volScalarField& alpha1() const { return alpha1_; }
        volScalarField& alpha1() { return alpha1_; }

        const volScalarField& alpha2() const { return alpha2_; }
        volScalarField& alpha2() { return alpha2_; }

        const dimensionedScalar& rho1() const { return rho1_; }
        const dimensionedScalar& rho2() const { return rho2_; }

        const dimensionedScalar& Cp1() const { return Cp1_; }
        const dimensionedScalar& Cp2() const { return Cp2_; }

        const dimensionedScalar& kappa1() const { return kappa1_; }
        const dimensionedScalar& kappa2() const { return kappa2_; }

        const dimensionedScalar& mu1() const { return mu1_; }
        const dimensionedScalar& mu2() const { return mu2_; }

        const dimensionedScalar& D1() const { return D1_; }
        const dimensionedScalar& D2() const { return D2_; }

        const dimensionedScalar& DAS() const { return DAS_; }

        const dimensionedScalar& betaT() const { return betaT_; }
        const dimensionedScalar& betaC() const { return betaC_; }

        const dimensionedScalar& TRef() const { return TRef_; }
        const dimensionedScalar& CRef() const { return CRef_; }

        const volVectorField& U() const { return U_; }

        tmp<volScalarField> rho() const
        {
            return alpha1_*rho1_ + alpha2_*rho2_;
        }

        tmp<volScalarField> Cp() const
        {
            return alpha1_*Cp1_ + alpha2_*Cp2_;
        }

        tmp<volScalarField> kappa() const
        {
            return alpha1_*kappa1_ + alpha2_*kappa2_;
        }

        virtual bool read();


    // Member Operators

        void operator=(const solidificationSystem&) = delete;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
