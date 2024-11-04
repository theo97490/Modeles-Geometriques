// Implicits
#pragma once
#include "implicits.h"
#include "mathematics.h"
#include <algorithm>

namespace ImplicitTree {
Vector absp(Vector p) {
  return Vector{abs(p[0]), abs(p[1]), abs(p[2]) };
}

Vector maxp(Vector p, double v){
  return Vector{std::max(p[0], v), std::max(p[1],v), std::max(p[2], v) };
}

Vector minp(Vector p, double v){
  return Vector{std::min(p[0], v), std::min(p[1],v), std::min(p[2], v) };
}

struct UnaryNode : public Implicit {
  UnaryNode(Implicit &a) : Implicit(), a(&a) {}
protected:
  Implicit *a;
};

struct BinaryNode : public Implicit {
  BinaryNode(Implicit &a, Implicit &b) : Implicit(), a(&a), b(&b) {}

protected:
  Implicit *a, *b;
};

struct Union final : public BinaryNode {
  using BinaryNode::BinaryNode;
  double Value(const Vector &pos) const override {
    return std::min(a->Value(pos), b->Value(pos));
  }
};

struct Intersection final : public BinaryNode {
  using BinaryNode::BinaryNode;
  double Value(const Vector &pos) const {
    return std::max(a->Value(pos), b->Value(pos));
  }
};

struct Diff final : public BinaryNode {
  using BinaryNode::BinaryNode;
  double Value(const Vector &pos) const {
    return std::max(a->Value(pos), -b->Value(pos));
  }
};

struct Blend final : public BinaryNode {
  Blend(Implicit &a, Implicit &b, double blend_size)
      : BinaryNode(a, b), blend_size(blend_size) {}

  double Value(const Vector &pos) const {
    double fa = a->Value(pos);
    double fb = b->Value(pos);
    double h = std::max(0., blend_size - std::abs(fa - fb)) / blend_size;
    return std::min(fa, fb) - (blend_size / 6.) * std::pow(h, 3);
  }

private:
  double blend_size;
};

struct Replicate final : public Implicit {
  Replicate(Implicit* a, const Vector& s): a(a), hsize(s) {}
  double Value(const Vector &pos) const {
    return a->Value(Vector{
      fmod(pos[0] + hsize[0], hsize[0] * 2),
      fmod(pos[1] + hsize[1], hsize[1] * 2),
      fmod(pos[2] + hsize[2], hsize[2] * 2),
    } - hsize);
  }
  
private:
  Implicit* a;
  Vector hsize;
};

struct Sphere final : public Implicit {
  Sphere(Vector pos, double size) : Implicit(), pos(pos), size(size) {}
  double Value(const Vector &pos) const {
    return SquaredNorm(this->pos - pos) - size * size;
  }

private:
  Vector pos;
  double size;
};

// https://iquilezles.org/articles/distfunctions/
// Méthode utilisant le quadrant d'une boite centrée sur les axes pour calculer pour tous les autres quadrants
struct InigoBox : public Implicit {
private:
  Vector pos, hsize;
public:
  InigoBox(Vector pos, Vector size) : Implicit(), pos(pos), hsize(size){}
  double Value(const Vector &point) const override {
    Vector relp = point - pos;
    Vector q = absp(relp) - hsize;
    return Norm(maxp(q, 0.)) + std::min(std::max(q[0], std::max(q[1], q[2])), 0.); 
  }
};

// Ma version initiale de Box : Calcule les plans de chaque face et retourne le max du dot avec le point
struct Box final : public Implicit {
  Box(Vector pos, Vector size) : Implicit(), pos(pos), size(size){}
  double Value(const Vector &point) const override {
    double max = -std::numeric_limits<double>::infinity();

    Vector p = point - pos - Vector{-size[0], 0, 0};
    double d = p * Vector{-1, 0, 0};
    max = std::max(max, d);

    p = point - pos - Vector{size[0], 0, 0};
    d = p * Vector{1, 0, 0};
    max = std::max(max, d);

    p = point - pos - Vector{0, -size[1], 0};
    d = p * Vector{0, -1, 0};
    max = std::max(max, d);

    p = point - pos - Vector{0, size[1], 0};
    d = p * Vector{0, 1, 0};
    max = std::max(max, d);

    p = point - pos - Vector{0, 0, -size[2]};
    d = p * Vector{0, 0, -1};
    max = std::max(max, d);

    p = point - pos - Vector{0, 0, size[2]};
    d = p * Vector{0, 0, 1};
    max = std::max(max, d);

    return max;
  }

private:
  Vector pos, size;
};

// Ma version de capule :  une ligne avec une thickness + clamp aux extremités 
struct Capsule final : public Implicit {
  Capsule(Vector pos, Vector hdir, double len, double size) : Implicit(), pos(pos), hdir(Normalized(hdir)), len(len), size(size) {}
  double Value(const Vector &point) const override {
    Vector relp = (point - pos);
    double d = std::clamp(hdir * relp, -len, len);
    Vector pline = hdir * d;
    Vector dist = relp - pline;
    return Norm(dist) - size;
  }

private:
  Vector pos, hdir;
  double len, size;
};

// Version de inigo quilez pour le tore, j'ai pas le temps de refaire les equations moi même...
struct InigoTore final : public Implicit {
  InigoTore(Vector pos, Vector size2d) : Implicit(), pos(pos), t(size2d) {}
  double Value(const Vector &point) const override {
    Vector relp = point - pos;
    Vector l = Vector{relp[0], relp[2], 0};
    Vector q = Vector{Norm(l) - t[0], relp[1], 0};
    return Norm(q) - t[1];
  }
private:
  Vector pos, t;
};


struct Translate : public Implicit {
private:
  Implicit* a;
  Vector c;
public:
  Translate(Implicit* a, Vector c) : Implicit(), a(a), c(c) {}
  double Value(const Vector &point) const override {
    return a->Value(point - c);
  }
};

struct Scale : public Implicit {
private:
  Implicit* a;
  Vector c;
public:
  Scale(Implicit* a, Vector c) : Implicit(), a(a), c(c) {}
  double Value(const Vector &point) const override {
    return a->Value(Vector{point[0] * 1/c[0], point[1] * 1/c[1], point[2] * 1/c[2]});
  }
};

struct Tree : public AnalyticScalarField {
  Tree(Implicit* a): start(a){};
  double Value(const Vector &point) const override {
    return start->Value(point);
  }
private: 
  Implicit* start;
};

} // namespace ImplicitTree
