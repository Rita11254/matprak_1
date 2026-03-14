#include <set>
#include <gmsh.h>

int main(int argc, char **argv)
{
  gmsh::initialize();
  gmsh::option::setNumber("Mesh.Algorithm", 1);      // Frontal-Delaunay
  gmsh::option::setNumber("Mesh.CharacteristicLengthMin", 0.05);
  gmsh::option::setNumber("Mesh.CharacteristicLengthMax", 1.0);
  gmsh::option::setNumber("Mesh.Optimize", 1); 

  gmsh::model::add("task2");

  try {
    gmsh::merge("cow.stl");
  } catch(...) {
    gmsh::logger::write("Could not load STL mesh: bye!");
    gmsh::finalize();
    return 0;
  }

  double angle = 25;
  bool forceParametrizablePatches = false;
  bool includeBoundary = true;
  double curveAngle = 180;

  gmsh::model::mesh::classifySurfaces(angle * M_PI / 180., includeBoundary,
                                      forceParametrizablePatches,
                                      curveAngle * M_PI / 180.);
  gmsh::model::mesh::createGeometry();

  std::vector<std::pair<int, int> > s;
  gmsh::model::getEntities(s, 2);
  std::vector<int> sl;
  for(auto surf : s) sl.push_back(surf.second);
  int l = gmsh::model::geo::addSurfaceLoop(sl);
  gmsh::model::geo::addVolume({l});

  gmsh::model::geo::synchronize();

  bool funny = false;
  int f = gmsh::model::mesh::field::add("MathEval");
  if(funny)
    gmsh::model::mesh::field::setString(f, "F", "2*sin((x+y)/5) + 3");
  else
    gmsh::model::mesh::field::setString(f, "F", "5");
  gmsh::model::mesh::field::setAsBackgroundMesh(f);
  
  gmsh::model::mesh::generate(3);
  gmsh::model::mesh::optimize("Netgen");

  gmsh::write("task2.msh");

  std::set<std::string> args(argv, argv + argc);
  if(!args.count("-nopopup")) gmsh::fltk::run();

  gmsh::finalize();
  return 0;
}