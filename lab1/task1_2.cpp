#include <gmsh.h>
#include <set>
#include <vector>

int main(int argc, char **argv) {
  gmsh::initialize();
  gmsh::model::add("task1");
  double lc = 0.1;
  double R = 1.0;
  double r_out = 0.5;
  double r_in = 0.3;
 
  gmsh::model::occ::addTorus(0, 0, 0, R, r_out, 1);
  gmsh::model::occ::addTorus(0, 0, 0, R, r_in, 2);
  
  gmsh::model::occ::synchronize();

  std::vector<std::pair<int, int>> new_volumes;
  std::vector<std::vector<std::pair<int, int>>> deleted;
  gmsh::model::occ::cut({{3, 1}}, {{3, 2}}, new_volumes, deleted, 3, true, true);
  
  gmsh::model::occ::synchronize();
  
  gmsh::model::mesh::generate(3);
  gmsh::write("circle.msh");
  std::set<std::string> args(argv, argv + argc);
  if (!args.count("-nopopup"))
    gmsh::fltk::run();
  gmsh::finalize();
  return 0;
}
