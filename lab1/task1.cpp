#include <gmsh.h>
#include <set>
#include <cmath> 

int main(int argc, char **argv) {
  gmsh::initialize();
  gmsh::model::add("task1");
  double lc = 0.2;

  gmsh::model::occ::addPoint(2, 0, 0, lc, 1);
  gmsh::model::occ::addPoint(2, 1, 0, lc, 2);
  gmsh::model::occ::addPoint(2, -1, 0, lc, 3);

  gmsh::model::occ::addCircleArc(2, 1, 3, 1);
  gmsh::model::occ::addCircleArc(3, 1, 2, 2);

  gmsh::model::occ::addCurveLoop({1, 2}, 1);
  gmsh::model::occ::addPlaneSurface({1}, 1);

  gmsh::model::occ::synchronize();

  std::vector<std::pair<int, int>> out;
  gmsh::model::occ::revolve({{2, 1}}, 0, 0, 0, 0, 1, 0, 2 * M_PI, out);

  gmsh::model::occ::synchronize();
  gmsh::model::mesh::generate(3);
  gmsh::write("circle.msh");
  std::set<std::string> args(argv, argv + argc);
  if (!args.count("-nopopup"))
    gmsh::fltk::run();
  gmsh::finalize();
  return 0;
}