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
    Foam::robinTemperatureFvPatchScalarField

Description
    Robin (third-kind) temperature wall BC: h*(T - Tinf) = -kappa*snGrad(T)

SourceFiles
    robinTemperatureFvPatchScalarField.C

\*---------------------------------------------------------------------------*/

#ifndef robinTemperatureFvPatchScalarField_H
#define robinTemperatureFvPatchScalarField_H

#include "mixedFvPatchFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
            Class robinTemperatureFvPatchScalarField Declaration
\*---------------------------------------------------------------------------*/

class robinTemperatureFvPatchScalarField
:
    public mixedFvPatchScalarField
{
    // Private data

        //- Far-field / wall ambient temperature
        scalarField Tinf_;

        //- Heat transfer coefficient
        scalarField h_;


public:

    //- Runtime type information
    TypeName("robinTemperature");


    // Constructors

        //- Construct from patch, internal field and dictionary
        robinTemperatureFvPatchScalarField
        (
            const fvPatch&,
            const DimensionedField<scalar, fvMesh>&,
            const dictionary&
        );

        //- Construct by mapping
        robinTemperatureFvPatchScalarField
        (
            const robinTemperatureFvPatchScalarField&,
            const fvPatch&,
            const DimensionedField<scalar, fvMesh>&,
            const fieldMapper&
        );

        //- Disallow copy without setting internal field reference
        robinTemperatureFvPatchScalarField
        (
            const robinTemperatureFvPatchScalarField&
        ) = delete;

        //- Copy constructor setting internal field reference
        robinTemperatureFvPatchScalarField
        (
            const robinTemperatureFvPatchScalarField&,
            const DimensionedField<scalar, fvMesh>&
        );

        //- Construct and return a clone setting internal field reference
        virtual tmp<fvPatchScalarField> clone
        (
            const DimensionedField<scalar, fvMesh>& iF
        ) const
        {
            return tmp<fvPatchScalarField>
            (
                new robinTemperatureFvPatchScalarField(*this, iF)
            );
        }


    // Member Functions

        // Access

            const scalarField& Tinf() const { return Tinf_; }
            scalarField& Tinf() { return Tinf_; }

            const scalarField& h() const { return h_; }
            scalarField& h() { return h_; }


        // Mapping functions

            virtual void map(const fvPatchScalarField&, const fieldMapper&);
            virtual void reset(const fvPatchScalarField&);


        // Evaluation functions

            virtual void updateCoeffs();


        //- Write
        virtual void write(Ostream&) const;


    // Member Operators

        using mixedFvPatchScalarField::operator=;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
