#include "WKBCFvPatchScalarField.H"
#include "addToRunTimeSelectionTable.H"
#include "fvPatchFieldMapper.H"
#include "volFields.H"
#include "surfaceFields.H"

namespace Foam
{

// * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * //

WKBCFvPatchScalarField::WKBCFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedValueFvPatchScalarField(p, iF),
    index_(0)
{}

WKBCFvPatchScalarField::WKBCFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchScalarField(p, iF, dict),
    index_(dict.lookupOrDefault<scalar>("index", 0))
{}

WKBCFvPatchScalarField::WKBCFvPatchScalarField
(
    const WKBCFvPatchScalarField& ptf,
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedValueFvPatchScalarField(ptf, p, iF, mapper),
    index_(ptf.index_)
{}

WKBCFvPatchScalarField::WKBCFvPatchScalarField
(
    const WKBCFvPatchScalarField& ptf,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedValueFvPatchScalarField(ptf, iF),
    index_(ptf.index_)
{}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void WKBCFvPatchScalarField::updateCoeffs()
{
    if (updated())
    {
        return;
    }

    // Post-processing mode: just hold whatever value is in the field
    fixedValueFvPatchScalarField::updateCoeffs();
}

void WKBCFvPatchScalarField::write(Ostream& os) const
{
    fvPatchScalarField::write(os);
    writeEntry(os, "index", index_);
    writeEntry(os, "value", *this);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

makePatchTypeField(fvPatchScalarField, WKBCFvPatchScalarField);

} // End namespace Foam
