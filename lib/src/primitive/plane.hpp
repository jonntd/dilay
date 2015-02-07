#ifndef DILAY_PRIMITIVE_PLANE
#define DILAY_PRIMITIVE_PLANE

#include <glm/glm.hpp>
#include <iosfwd>

class PrimPlane {
  public:
    PrimPlane (const glm::vec3&, const glm::vec3&);

    const glm::vec3& point  () const { return this->_point ; }
    const glm::vec3& normal () const { return this->_normal; }

    float distance (const glm::vec3&) const;

  private:
    const glm::vec3 _point;
    const glm::vec3 _normal;
};

std::ostream& operator<<(std::ostream&, const PrimPlane&);

#endif
