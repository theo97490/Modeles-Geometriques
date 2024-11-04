#pragma once
#define _USE_MATH_DEFINES

#include "color.h"
#include "mesh.h"
#include "meshcolor.h"
#include <cassert>
#include <cmath>
#include <functional>
#include <vector>
#include <array>
#include <mathematics.h>
#include <math.h>

using uchar = unsigned char;
using uint = unsigned int;

class Factorial{
  static std::vector<size_t> cache;
public:
  inline static size_t compute(uint k){
    if (k == 0) return 1;
    if (k-1 < cache.size()) return cache[k-1];
    size_t ret = k * compute(k-1);
    cache.push_back(ret);
    return ret;
  }
};

class Binomial{
  static std::vector<size_t> cache;

  static void compute_line(uint sum, uint n){
    uint sum_n1 = ((n-1) * (n-1) + (n-1))/2;
    if (sum != 0 and sum > cache.size()) compute_line(sum_n1, n-1);
    assert(sum == cache.size());

    for (uint k = 0; k <= n; k++){
      size_t fn = Factorial::compute(n);
      size_t fk = Factorial::compute(k);
      size_t fnk = Factorial::compute(n - k);
      cache.push_back(fn / (fk * fnk));
    }
  }
public:
  inline static size_t compute(uint n, uint k){
    uint sum = (n * n + n)/2;
    if (sum >= cache.size()) compute_line(sum, n);
    return cache[sum + k];
  }
};

// todo : faire binomial sans factoriel

inline std::vector<size_t> Binomial::cache = {};
inline std::vector<size_t> Factorial::cache = {};

inline double bernstein(uint n, uint k, double u){
  return Binomial::compute(n,k) * std::pow(u, k) * std::pow(1 - u, n - k);
}

class Curve {
  static constexpr double epsilon = 0.001;
public:
  virtual Vector position(double t) = 0;

  // Default methods for any curve

  virtual Vector delta_1(double t, double e0 = Curve::epsilon, double e1 = Curve::epsilon){
    Vector p0 = position(t-e0);
    Vector p1 = position(t+e1);
    return (p1-p0)/(e0+e1);
  };

  virtual Vector delta_2(double t, double e0 = Curve::epsilon, double e1 = Curve::epsilon){
    Vector p0 = delta_1(t-e0);
    Vector p1 = delta_1(t+e1);
    return (p1-p0)/(e0+e1);
  };

  virtual double length(double t0, double t1) {
    double sum = 0;
    if (t0 < epsilon) t0 = epsilon;
    float i = t0;
    while (i < t1){
      double e0 = epsilon;
      double e1 = i + epsilon < t1 ? epsilon : t1 - i;
      sum += Norm(delta_1(i, e0, e1));
    };

    return sum;
  };

  virtual Vector tangente(double t){
    return Normalized(delta_1(t));
  }

  virtual Vector normal(double t){
    return Normalized(delta_2(t));
  }

  virtual Vector binormal(double t){
    return Normalized(tangente(t) / normal(t));
  }
};

class BezierCurve : public Curve{
  std::vector<Vector> controls;
public:
  BezierCurve(const std::vector<Vector>& controls) : controls(controls) {}
  BezierCurve(std::vector<Vector>&& controls) : controls(controls) {}
  BezierCurve(){}

  Vector position(double t) override{
    Vector sum(0,0,0);
    assert(controls.size() > 0);
    for (uint i = 0; i < controls.size(); i++){
      sum += controls[i] * bernstein(controls.size() - 1, i, t);
    }
    return sum;
  }

  Vector delta_1(double t, double e0, double e1) override {
    Vector sum(0,0,0);
    assert(controls.size() > 0);
    uint n = controls.size() - 1;
    for (uint i = 0; i < controls.size() - 1; i++){
      sum += n * (controls[i+1] - controls[i]) * bernstein(n - 1, i, t);
    }
    return sum;
  }

  Vector delta_2(double t, double e0, double e1) override {
    Vector sum(0,0,0);
    assert(controls.size() > 0);
    uint n = controls.size() - 1;
    for (uint i = 0; i < controls.size() - 2; i++){
      sum += (n - 1) * n
        * (controls[i+2] - 2*controls[i+1] + controls[i])
        * bernstein(n - 2, i, t);
    }
    return sum;
  }

  Vector normal(double t) override{
    return Normalized(tangente(t)/Vector(0,1,0));
  }
};

class BezierSurface {
  std::vector<Vector> controls; // 2D array
  uint size_x, size_y;
public:
  BezierSurface(uint sx, uint sy, const std::vector<Vector>& controls) : controls(controls), size_x(sx), size_y(sy) {}
  BezierSurface(uint sx, uint sy, std::vector<Vector>&& controls) : controls(controls), size_x(sx), size_y(sy){}
  BezierSurface(){}

  const Vector& control(uint x, uint y) const {
    assert(x < size_x && y < size_y);
    return controls[y * size_x + x];
  }

  Vector& control(uint x, uint y) {
    assert(x < size_x && y < size_y);
    return controls[y * size_x + x];
  }

  Vector position(double u, double v) const{
    Vector sum(0,0,0);
    assert(controls.size() > 0);
    for (uint y = 0; y < size_y; y++){
      for (uint x = 0; x < size_x; x++){
        sum += control(x,y) * bernstein(size_x - 1, x, u) * bernstein(size_y - 1, y, v);
      }
    }
    return sum;
  }

  Vector normal(double u, double v) const{
    Vector sum_u(0,0,0), sum_v(0,0,0);
    assert(controls.size() > 0);

    // derive u
    for (uint y = 0; y < size_y; y++){
      for (uint x = 0; x < size_x-1; x++){
        sum_u += (size_x-1) * (control(x+1,y) - control(x,y)) * bernstein(size_x-2, x, u) * bernstein(size_y-1, y, v);
      }
    }

    // derive v
    for (uint y = 0; y < size_y-1; y++){
      for (uint x = 0; x < size_x; x++){
        sum_v += (size_y-1) * (control(x,y+1) - control(x,y)) * bernstein(size_x-1, x, u) * bernstein(size_y-2, y, v);
      }
    }

    // /!\ cross product
    return Normalized(sum_u / sum_v);
  };
};

struct ExtrusionSurface {
  using RadialFunction = std::function<double(double)>;

  ExtrusionSurface(){};
  ExtrusionSurface(Curve* curve, const RadialFunction& rad): curve(curve), radial(rad) {};

  Vector position(double u, double v) const {
    double tpiv = 2 * M_PI * v;
    return curve->position(u) + radial(tpiv) * (cos(tpiv) * curve->normal(u) + sin(tpiv) * curve->binormal(u));
  }

  Vector normal(double u, double v) const {
    double tpiv = 2 * M_PI * v;
    return Normalized(cos(tpiv) * curve->normal(u) + sin(tpiv) * curve->binormal(u));
  }

private:
  Curve* curve;
  RadialFunction radial;
};

inline MeshColor mesh_bezier_surface(const BezierSurface& bezier, uint dim_x, uint dim_y){
  std::vector<Vector> vertices;
  std::vector<Vector> normals;
  std::vector<size_t> indices;
  std::vector<size_t> normal_indices;

  const auto id = [&](uint x, uint y){
    return y * dim_x + x;
  };

  for (uint x = 0; x < dim_x; x++){
    for (uint y = 0; y < dim_y; y++){
      double u = double(x)/(dim_x-1);
      double v = double(y)/(dim_y-1);
      // Vertices
      vertices.push_back(bezier.position(u,v));
      // Normals
      normals.push_back(bezier.normal(u,v));
      // Triangles
      if (x < dim_x - 1 && y < dim_y - 1){
        indices.push_back(id(x,y));
        indices.push_back(id(x+1,y));
        indices.push_back(id(x+1,y+1));

        indices.push_back(id(x,y));
        indices.push_back(id(x+1, y+1));
        indices.push_back(id(x,y+1));
      }
    }
  }

  normal_indices = indices;

  Mesh mesh = Mesh(vertices, normals, indices, normal_indices);
  std::vector<Color> cols;
  cols.resize(mesh.Vertexes());
  for (size_t i = 0; i < cols.size(); i++)
    cols[i] = Color(0.8, 0.8, 0.8);

  return MeshColor(mesh, cols, mesh.VertexIndexes());
}



inline MeshColor mesh_extrusion_surface(const ExtrusionSurface& surf, uint div_curve, uint div_radius){
  std::vector<Vector> vertices;
  std::vector<Vector> normals;
  std::vector<size_t> indices;
  std::vector<size_t> normal_indices;

  const auto id = [&](uint c, uint r){
    return c * div_radius + (r % div_radius);
  };

  for (uint c = 0; c < div_curve; c++){
    for (uint r = 0; r < div_radius; r++){
      double u = double(c)/(div_curve-1);
      double v = double(r)/(div_radius-1);
      // Vertices
      vertices.push_back(surf.position(u,v));
      // Normals
      normals.push_back(surf.normal(u,v));
      // Triangles
      if (c < div_curve - 1){
        indices.push_back(id(c,r));
        indices.push_back(id(c+1,r));
        indices.push_back(id(c+1,r+1));

        indices.push_back(id(c,r));
        indices.push_back(id(c+1, r+1));
        indices.push_back(id(c,r+1));
      }
    }
  }

  normal_indices = indices;

  Mesh mesh = Mesh(vertices, normals, indices, normal_indices);
  std::vector<Color> cols;
  cols.resize(mesh.Vertexes());
  for (size_t i = 0; i < cols.size(); i++)
    cols[i] = Color(0.8, 0.8, 0.8);

  return MeshColor(mesh, cols, mesh.VertexIndexes());
}
