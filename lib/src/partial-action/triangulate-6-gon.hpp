#ifndef DILAY_PARTIAL_ACTION_TRIANGULATE_6_GON
#define DILAY_PARTIAL_ACTION_TRIANGULATE_6_GON

#include <vector>
#include "action/on.hpp"
#include "macro.hpp"

class WingedFace;
class WingedMesh;
class Id;

class PATriangulate6Gon : public ActionOn <WingedMesh> {
  public: 
    DECLARE_BIG3 (PATriangulate6Gon)

  /** `run (m,f,n)` triangulates the 6-gon `f`.
   * Note thate `f.edge ()->firstVertex (f)` must be an even vertex, i.e.
   * it must not be generated during the current subdivision step.
   * The new faces are adjacent to `f`.
   * `f` and the new faces are added to `n` if `n` is not `nullptr`.
   */
    void run  (WingedMesh&, WingedFace&, std::vector <Id>* = nullptr);

  private:
    void runUndo (WingedMesh&);
    void runRedo (WingedMesh&);

    class Impl;
    Impl* impl;
};
#endif
