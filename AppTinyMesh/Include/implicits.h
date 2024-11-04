// Implicits

#pragma once

#include <iostream>

#include "mesh.h"

struct Implicit {
  virtual double Value(const Vector&) const  { return 0; };
};

class AnalyticScalarField : public Implicit
{
protected:
public:
  AnalyticScalarField();
  double Value(const Vector&) const override;
  virtual Vector Gradient(const Vector&) const;

  // Normal
  virtual Vector Normal(const Vector&) const;

  // Dichotomy
  Vector Dichotomy(Vector, Vector, double, double, double, const double& = 1.0e-4) const;

  virtual void Polygonize(int, Mesh&, const Box&, const double& = 1e-4) const;
protected:
  static const double Epsilon; //!< Epsilon value for partial derivatives
protected:
  static int TriangleTable[256][16]; //!< Two dimensionnal array storing the straddling edges for every marching cubes configuration.
  static int edgeTable[256];    //!< Array storing straddling edges for every marching cubes configuration.
};
