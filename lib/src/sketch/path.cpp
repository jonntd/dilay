#include "../mesh.hpp"
#include "intersection.hpp"
#include "primitive/aabox.hpp"
#include "primitive/plane.hpp"
#include "primitive/ray.hpp"
#include "sketch/path.hpp"
#include "sketch/path-intersection.hpp"
#include "util.hpp"

struct SketchPath :: Impl {
  SketchPath*         self;
  SketchPath::Spheres spheres;
  glm::vec3           minimum;
  glm::vec3           maximum;

  Impl (SketchPath* s)
    : self (s)
  {
    this->resetMinMax ();
  }

  void resetMinMax () {
    this->minimum = glm::vec3 (std::numeric_limits <float>::max ());
    this->maximum = glm::vec3 (std::numeric_limits <float>::lowest ());
  }

  void reset () {
    this->resetMinMax ();
    this->spheres.clear ();
  }

  void setMinMax () {
    this->resetMinMax ();

    for (const PrimSphere& s : this->spheres) {
      this->maximum = glm::max (this->maximum, s.center () + glm::vec3 (s.radius ()));
      this->minimum = glm::min (this->minimum, s.center () - glm::vec3 (s.radius ()));
    }
  }

  bool isEmpty () const {
    return this->spheres.empty ();
  }

  PrimAABox aabox () const {
    assert (this->isEmpty () == false);
    return PrimAABox (this->minimum, this->maximum);
  }

  void addSphere (const glm::vec3& position, float radius) {
    this->maximum = glm::max (this->maximum, position + glm::vec3 (radius));
    this->minimum = glm::min (this->minimum, position - glm::vec3 (radius));

    this->spheres.emplace_back (position, radius);
  }

  void render (Camera& camera, Mesh& mesh) const {
    for (const PrimSphere& s : this->spheres) {
      mesh.position (s.center ());
      mesh.scaling  (glm::vec3 (s.radius ()));
      mesh.render   (camera);
    }
  }

  bool intersects (const PrimRay& ray, SketchMesh& mesh, SketchPathIntersection& intersection) {
    if (IntersectionUtil::intersects (ray, PrimAABox (this->minimum, this->maximum))) {
      for (const PrimSphere& s : this->spheres) {
        float t;
        if (IntersectionUtil::intersects (ray, s, &t)) {
          intersection.update (t, ray.pointAt (t)
                                , glm::normalize (ray.pointAt (t) - s.center ())
                                , mesh
                                , *this->self );
        }
      }
    }
    return intersection.isIntersection ();
  }

  SketchPath mirror (const PrimPlane& mPlane) {
    SketchPath::Spheres oldSpheres (std::move (this->spheres));
    SketchPath          mirrored;

    this->reset ();
    for (const PrimSphere& s : oldSpheres) {
      if (mPlane.distance (s.center ()) > -Util::epsilon ()) {
        this->addSphere (s.center (), s.radius ());
        mirrored.addSphere (mPlane.mirror (s.center ()), s.radius ());
      }
    }
    return mirrored;
  }

  void smooth ( const PrimSphere& range, unsigned int halfWidth, SketchPathSmoothEffect effect
              , const PrimSphere* nearestToFirst, const PrimSphere* nearestToLast )
  {
    const unsigned int numS = this->spheres.size ();

    for (unsigned int i = 0; i < numS; i++) {
      if (IntersectionUtil::intersects (range, this->spheres.at (i))) {
        const unsigned int hW = i < halfWidth
                              ? i
                              : ( i >= numS - halfWidth 
                                ? numS - i - 1
                                : halfWidth );
        glm::vec3 center (0.0f);
        float     radius (0.0f);
        for (unsigned int j = i-hW; j <= i+hW; j++) {
          center += this->spheres.at (j).center ();
          radius += this->spheres.at (j).radius ();
        }

        const bool   effectEmbeds      = effect == SketchPathSmoothEffect::Embed 
                                      || effect == SketchPathSmoothEffect::EmbedAndAdjust;
        unsigned int numAffectedCenter = 0;
        unsigned int numAffectedRadius = 0;

        if (effect != SketchPathSmoothEffect::None) {
          if (i < halfWidth) {
            if (nearestToFirst && effectEmbeds) {
              numAffectedCenter++;
              center += nearestToFirst->center ();
            }

            if (nearestToFirst && effect == SketchPathSmoothEffect::EmbedAndAdjust) {
              numAffectedRadius++;
              radius += nearestToFirst->radius ();
            }
            else if (effect == SketchPathSmoothEffect::Pinch) {
              numAffectedRadius++;
            }
          }

          if (i >= numS - halfWidth) {
            if (nearestToLast && effectEmbeds) {
              numAffectedCenter++;
              center += nearestToLast->center ();
            }

            if (nearestToLast && effect == SketchPathSmoothEffect::EmbedAndAdjust) {
              numAffectedRadius++;
              radius += nearestToLast->radius ();
            }
            else if (effect == SketchPathSmoothEffect::Pinch) {
              numAffectedRadius++;
            }
          }
        }
        this->spheres.at (i).center (center / float ((2 * hW) + 1 + numAffectedCenter));
        this->spheres.at (i).radius (radius / float ((2 * hW) + 1 + numAffectedRadius));
      }
    }
    this->setMinMax ();
  }
};

DELEGATE_BIG6_SELF (SketchPath)
GETTER_CONST    (const SketchPath::Spheres&, SketchPath, spheres)
GETTER_CONST    (const glm::vec3&          , SketchPath, minimum)
GETTER_CONST    (const glm::vec3&          , SketchPath, maximum)
DELEGATE_CONST  (bool                      , SketchPath, isEmpty)
DELEGATE_CONST  (PrimAABox                 , SketchPath, aabox)
DELEGATE2       (void                      , SketchPath, addSphere, const glm::vec3&, float)
DELEGATE2_CONST (void                      , SketchPath, render, Camera&, Mesh&)
DELEGATE3       (bool                      , SketchPath, intersects, const PrimRay&, SketchMesh&, SketchPathIntersection&)
DELEGATE1       (SketchPath                , SketchPath, mirror, const PrimPlane&)
DELEGATE5       (void                      , SketchPath, smooth, const PrimSphere&, unsigned int, SketchPathSmoothEffect, const PrimSphere*, const PrimSphere*)
