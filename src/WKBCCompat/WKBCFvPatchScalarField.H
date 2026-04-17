// Compatibility shim: registers "WKBC" BC type so OpenFOAM 12 can read
// case files from the OF4 Windkessel solver. Sets pressure from the
// stored field value — sufficient for postProcess runs.

#ifndef WKBCFvPatchScalarField_H
#define WKBCFvPatchScalarField_H

#include "fixedValueFvPatchFields.H"

namespace Foam
{

class WKBCFvPatchScalarField
:
    public fixedValueFvPatchScalarField
{
    scalar index_;

public:

    TypeName("WKBC");

    // Constructors
    WKBCFvPatchScalarField
    (
        const fvPatch&,
        const DimensionedField<scalar, volMesh>&
    );

    WKBCFvPatchScalarField
    (
        const fvPatch&,
        const DimensionedField<scalar, volMesh>&,
        const dictionary&
    );

    WKBCFvPatchScalarField
    (
        const WKBCFvPatchScalarField&,
        const fvPatch&,
        const DimensionedField<scalar, volMesh>&,
        const fvPatchFieldMapper&
    );

    WKBCFvPatchScalarField
    (
        const WKBCFvPatchScalarField&,
        const DimensionedField<scalar, volMesh>&
    );

    // No clone — OF12 deleted the copy constructor for fixedValueFvPatchField.
    // The runtime selection table uses the dictionary constructor directly.

    // Member functions
    virtual void updateCoeffs();
    virtual void write(Ostream&) const;
};

} // End namespace Foam

#endif
