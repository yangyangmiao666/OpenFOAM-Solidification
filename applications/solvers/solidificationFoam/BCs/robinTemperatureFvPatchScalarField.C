/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2018-2026 OpenFOAM Foundation
     \\/     M anipulation  |
\*---------------------------------------------------------------------------*/

#include "robinTemperatureFvPatchScalarField.H"
#include "addToRunTimeSelectionTable.H"
#include "fieldMapper.H"
#include "volFields.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::robinTemperatureFvPatchScalarField::robinTemperatureFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, fvMesh>& iF,
    const dictionary& dict
)
:
    mixedFvPatchScalarField(p, iF),
    Tinf_("Tinf", dimTemperature, dict, p.size()),
    h_("h", dimThermalConductivity/dimLength, dict, p.size())
{
    refValue() = Tinf_;
    refGrad() = 0.0;
    valueFraction() = 0.0;

    if (dict.found("value"))
    {
        fvPatchField<scalar>::operator=
        (
            scalarField("value", iF.dimensions(), dict, p.size())
        );
    }
    else
    {
        evaluate();
    }
}


Foam::robinTemperatureFvPatchScalarField::robinTemperatureFvPatchScalarField
(
    const robinTemperatureFvPatchScalarField& ptf,
    const fvPatch& p,
    const DimensionedField<scalar, fvMesh>& iF,
    const fieldMapper& mapper
)
:
    mixedFvPatchScalarField(ptf, p, iF, mapper),
    Tinf_(mapper(ptf.Tinf_)),
    h_(mapper(ptf.h_))
{}


Foam::robinTemperatureFvPatchScalarField::robinTemperatureFvPatchScalarField
(
    const robinTemperatureFvPatchScalarField& tppsf,
    const DimensionedField<scalar, fvMesh>& iF
)
:
    mixedFvPatchScalarField(tppsf, iF),
    Tinf_(tppsf.Tinf_),
    h_(tppsf.h_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::robinTemperatureFvPatchScalarField::map
(
    const fvPatchScalarField& ptf,
    const fieldMapper& mapper
)
{
    mixedFvPatchScalarField::map(ptf, mapper);

    const robinTemperatureFvPatchScalarField& tiptf =
        refCast<const robinTemperatureFvPatchScalarField>(ptf);

    mapper(Tinf_, tiptf.Tinf_);
    mapper(h_, tiptf.h_);
}


void Foam::robinTemperatureFvPatchScalarField::reset
(
    const fvPatchScalarField& ptf
)
{
    mixedFvPatchScalarField::reset(ptf);

    const robinTemperatureFvPatchScalarField& tiptf =
        refCast<const robinTemperatureFvPatchScalarField>(ptf);

    Tinf_.reset(tiptf.Tinf_);
    h_.reset(tiptf.h_);
}


void Foam::robinTemperatureFvPatchScalarField::updateCoeffs()
{
    if (updated())
    {
        return;
    }

    const scalarField& k =
        patch().lookupPatchField<volScalarField, scalar>("kappa");

    valueFraction() =
        1.0
       /(
            1.0
          + k*patch().deltaCoeffs()/max(h_, small)
        );

    refValue() = Tinf_;

    mixedFvPatchScalarField::updateCoeffs();
}


void Foam::robinTemperatureFvPatchScalarField::write(Ostream& os) const
{
    fvPatchScalarField::write(os);
    writeEntry(os, "Tinf", Tinf_);
    writeEntry(os, "h", h_);
    writeEntry(os, "value", *this);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
    makePatchTypeField
    (
        fvPatchScalarField,
        robinTemperatureFvPatchScalarField
    );
}

// ************************************************************************* //
