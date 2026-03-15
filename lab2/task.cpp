#include <gmsh.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkTetra.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLUnstructuredGridWriter.h>

#include <cmath>
#include <iostream>
#include <vector>

using namespace std;

// Класс расчётной точки
class CalcNode {
  // Класс сетки будет friend-ом точки
  friend class CalcMesh;

 protected:
  // Координаты
  double x;
  double y;
  double z;
  // Некая величина, в попугаях
  double smth;
  // Скорость
  double vx;
  double vy;
  double vz;

 public:
  // Конструктор по умолчанию
  CalcNode() : x(0.0), y(0.0), z(0.0), smth(0.0), vx(0.0), vy(0.0), vz(0.0) {}

  // Конструктор с указанием всех параметров
  CalcNode(double x, double y, double z, double smth, double vx, double vy,
           double vz)
      : x(x), y(y), z(z), smth(smth), vx(vx), vy(vy), vz(vz) {}

  // Метод отвечает за перемещение точки
  // Движемся время tau из текущего положения с текущей скоростью
  void move(double tau) {
    x += vx * tau;
    y += vy * tau;
    z += vz * tau;
  }
};

// Класс элемента сетки
class Element {
  // Класс сетки будет friend-ом и элемента тоже
  // (и вообще будет нагло считать его просто структурой)
  friend class CalcMesh;

 protected:
  // Индексы узлов, образующих этот элемент сетки
  unsigned long nodesIds[4];
};

// Класс расчётной сетки
class CalcMesh {
 protected:
  // 3D-сетка из расчётных точек
  vector<CalcNode> nodes;
  vector<Element> elements;

  int type;
  double nowTime;

 public:
  // void SwapType(int new_type) { type = new_type; }
  // Конструктор сетки из заданного stl-файла
  CalcMesh(const std::vector<double>& nodesCoords,
           const std::vector<std::size_t>& tetrsPoints) {
    // Пройдём по узлам в модели gmsh
    nodes.resize(nodesCoords.size() / 3);
    for (unsigned int i = 0; i < nodesCoords.size() / 3; i++) {
      // Координаты заберём из gmsh
      double pointX = nodesCoords[i * 3];
      double pointY = nodesCoords[i * 3 + 1];
      double pointZ = nodesCoords[i * 3 + 2];
      // Модельная скалярная величина распределена как-то вот так
      double smth = pow(pointX, 2) + pow(pointY, 2) + pow(pointZ, 2);
      nodes[i] = CalcNode(pointX, pointY, pointZ, smth, 0.0, 0.0, 0.0);
    }

    // Пройдём по элементам в модели gmsh
    elements.resize(tetrsPoints.size() / 4);
    for (unsigned int i = 0; i < tetrsPoints.size() / 4; i++) {
      elements[i].nodesIds[0] = tetrsPoints[i * 4] - 1;
      elements[i].nodesIds[1] = tetrsPoints[i * 4 + 1] - 1;
      elements[i].nodesIds[2] = tetrsPoints[i * 4 + 2] - 1;
      elements[i].nodesIds[3] = tetrsPoints[i * 4 + 3] - 1;
    }
  }

  void UpdateSpeed(int new_type) {
    switch (new_type) {
      case 1:
        for (auto& node : nodes) {
          node.vx = (1.0) * node.x;
          node.vy = 0.0;
          node.vz = 0.0;
        }
        break;

      case 2:
        for (auto& node : nodes) {
          node.vx = 0.0;
          node.vy = (1.0) * node.y;
          node.vz = 0.0;
        }
        break;

      case 3:
        for (auto& node : nodes) {
          node.vx = (-1.0) * node.x;
          node.vy = 0.0;
          node.vz = 0.0;
        }
        break;
      case 4:
        for (auto& node : nodes) {
          node.vx = 0.0;
          node.vy = (-1.0) * node.y;
          node.vz = 0.0;
        }
        break;

      case 5:
        for (auto& node : nodes) {
          node.vz = 0.0;
          double dx = node.x - 10.0;
          double dy = node.y - 25.0;
          double r = sqrt(dx * dx + dy * dy);

          if (r < 1e-10) {
            node.vx = 0.0;
            node.vy = 0.0;
            continue;
          }
          double omega = 10.0;
          node.vx = -omega * dy;
          node.vy = omega * dx;
        }
        break;
      case 6:
        for (auto& node : nodes) {
          node.vx = 0.0;
          node.vy = 0.0;
          if ((node.x - 5.0) * (node.x - 5.0) + (node.y - 15.0) * (node.y - 15.0) < 9.6) {
            node.vz = 7.0;
          } else {
            node.vz = 0.0;
          }
        }
        break;
      case 7:
        for (auto& node : nodes) {
          node.vx = 0.0;
          node.vy = 0.0;
          if ((node.x - 15.0) * (node.x - 15.0) + (node.y - 5.0) * (node.y - 5.0) < 9.6) {
            node.vz = 7.0;
          } else {
            node.vz = 0.0;
          }
        }
        break;
      case 8:
        for (auto& node : nodes) {
          node.vx = 0.0;
          node.vy = 0.0;
          if ((node.x - 5.0) * (node.x - 5.0) + (node.y - 5.0) * (node.y - 5.0) < 9.6) {
            node.vz = 7.0;
          } else {
            node.vz = 0.0;
          }
        }
        break;
      case 9:
        for (auto& node : nodes) {
          node.vx = 0.0;
          node.vy = 0.0;
          if ((node.x - 15.0) * (node.x - 15.0) + (node.y - 15.0) * (node.y - 15.0) < 9.6) {
            node.vz = 7.0;
          } else {
            node.vz = 0.0;
          }
        }
        break;
      case 10:
        for (auto& node : nodes) {
          node.vx = 0.0;
          node.vy = 0.0;
          if ((node.x - 5.0) * (node.x - 5.0) + (node.y - 15.0) * (node.y - 15.0) < 9.6) {
            node.vz = -7.0;
          } else {
            node.vz = 0.0;
          }
        }
        break;
      case 11:
        for (auto& node : nodes) {
          node.vx = 0.0;
          node.vy = 0.0;
          if ((node.x - 15.0) * (node.x - 15.0) + (node.y - 5.0) * (node.y - 5.0) < 9.6) {
            node.vz = -7.0;
          } else {
            node.vz = 0.0;
          }
        }
        break;
      case 12:
        for (auto& node : nodes) {
          node.vx = 0.0;
          node.vy = 0.0;
          if ((node.x - 5.0) * (node.x - 5.0) + (node.y - 5.0) * (node.y - 5.0) < 9.6) {
            node.vz = -7.0;
          } else {
            node.vz = 0.0;
          }
        }
        break;
      case 13:
        for (auto& node : nodes) {
          node.vx = 0.0;
          node.vy = 0.0;
          if ((node.x - 15.0) * (node.x - 15.0) + (node.y - 15.0) * (node.y - 15.0) < 9.6) {
            node.vz = -7.0;
          } else {
            node.vz = 0.0;
          }
        }
        break;
      default:
        for (auto& node : nodes) {
          node.vx = node.vy = node.vz = 0.0;
        }
        break;
    }
  }

  // Метод отвечает за выполнение для всей сетки шага по времени величиной tau
  void doTimeStep(double tau, int new_type) {
    UpdateSpeed(new_type);
    // По сути метод просто двигает все точки
    for (unsigned int i = 0; i < nodes.size(); i++) {
      nodes[i].move(tau);
    }
  }

  // Метод отвечает за запись текущего состояния сетки в снапшот в формате VTK
  void snapshot(unsigned int snap_number) {
    // Сетка в терминах VTK
    vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid =
        vtkSmartPointer<vtkUnstructuredGrid>::New();
    // Точки сетки в терминах VTK
    vtkSmartPointer<vtkPoints> dumpPoints = vtkSmartPointer<vtkPoints>::New();
    // Скалярное поле на точках сетки
    auto smth = vtkSmartPointer<vtkDoubleArray>::New();
    smth->SetName("smth");
    // Векторное поле на точках сетки
    auto vel = vtkSmartPointer<vtkDoubleArray>::New();
    vel->SetName("velocity");
    vel->SetNumberOfComponents(3);
    // Обходим все точки нашей расчётной сетки
    for (unsigned int i = 0; i < nodes.size(); i++) {
      // Вставляем новую точку в сетку VTK-снапшота
      dumpPoints->InsertNextPoint(nodes[i].x, nodes[i].y, nodes[i].z);
      // Добавляем значение векторного поля в этой точке
      double _vel[3] = {nodes[i].vx, nodes[i].vy, nodes[i].vz};
      vel->InsertNextTuple(_vel);
      // И значение скалярного поля тоже
      smth->InsertNextValue(nodes[i].smth);
    }
    // Грузим точки в сетку
    unstructuredGrid->SetPoints(dumpPoints);
    // Присоединяем векторное и скалярное поля к точкам
    unstructuredGrid->GetPointData()->AddArray(vel);
    unstructuredGrid->GetPointData()->AddArray(smth);
    // А теперь пишем, как наши точки объединены в тетраэдры
    for (unsigned int i = 0; i < elements.size(); i++) {
      auto tetra = vtkSmartPointer<vtkTetra>::New();
      tetra->GetPointIds()->SetId(0, elements[i].nodesIds[0]);
      tetra->GetPointIds()->SetId(1, elements[i].nodesIds[1]);
      tetra->GetPointIds()->SetId(2, elements[i].nodesIds[2]);
      tetra->GetPointIds()->SetId(3, elements[i].nodesIds[3]);
      unstructuredGrid->InsertNextCell(tetra->GetCellType(),
                                       tetra->GetPointIds());
    }
    // Создаём снапшот в файле с заданным именем
    string fileName = "lego-step-" + std::to_string(snap_number) + ".vtu";
    vtkSmartPointer<vtkXMLUnstructuredGridWriter> writer =
        vtkSmartPointer<vtkXMLUnstructuredGridWriter>::New();
    writer->SetFileName(fileName.c_str());
    writer->SetInputData(unstructuredGrid);
    writer->Write();
  }
};

int main() {
  // Шаг точек по пространству
  double h = 4.0;
  // Шаг по времени
  double tau = 0.01;
  const unsigned int GMSH_TETR_CODE = 4;
  gmsh::initialize();
  gmsh::model::add("lab2");

  try {
    gmsh::merge("lego.stl");
  } catch (...) {
    gmsh::logger::write("Could not load STL mesh: bye!");
    gmsh::finalize();
    return -1;
  }

  double angle = 40;
  bool forceParametrizablePatches = false;
  bool includeBoundary = true;
  double curveAngle = 180;
  gmsh::model::mesh::classifySurfaces(angle * M_PI / 180., includeBoundary,
                                      forceParametrizablePatches,
                                      curveAngle * M_PI / 180.);
  gmsh::model::mesh::createGeometry();

  std::vector<std::pair<int, int>> s;
  gmsh::model::getEntities(s, 2);
  std::vector<int> sl;
  for (auto surf : s) sl.push_back(surf.second);
  int l = gmsh::model::geo::addSurfaceLoop(sl);
  gmsh::model::geo::addVolume({l});

  gmsh::model::geo::synchronize();

  int f = gmsh::model::mesh::field::add("MathEval");
  gmsh::model::mesh::field::setString(f, "F", "4");
  gmsh::model::mesh::field::setAsBackgroundMesh(f);

  gmsh::model::mesh::generate(3);

  std::vector<double> nodesCoord;
  std::vector<std::size_t> nodeTags;
  std::vector<double> parametricCoord;
  gmsh::model::mesh::getNodes(nodeTags, nodesCoord, parametricCoord);

  std::vector<std::size_t>* tetrsNodesTags = nullptr;
  std::vector<int> elementTypes;
  std::vector<std::vector<std::size_t>> elementTags;
  std::vector<std::vector<std::size_t>> elementNodeTags;
  gmsh::model::mesh::getElements(elementTypes, elementTags, elementNodeTags);
  for (unsigned int i = 0; i < elementTypes.size(); i++) {
    if (elementTypes[i] != GMSH_TETR_CODE) continue;
    tetrsNodesTags = &elementNodeTags[i];
  }

  if (tetrsNodesTags == nullptr) {
    cout << "Can not find tetra data. Exiting." << endl;
    gmsh::finalize();
    return -2;
  }

  cout << "The model has " << nodeTags.size() << " nodes and "
       << tetrsNodesTags->size() / 4 << " tetrs." << endl;

  for (int i = 0; i < nodeTags.size(); ++i) {
    assert(i == nodeTags[i] - 1);
  }
  assert(tetrsNodesTags->size() % 4 == 0);

  CalcMesh mesh(nodesCoord, *tetrsNodesTags);

  gmsh::finalize();

  mesh.snapshot(0);

  for (unsigned int step = 1; step < 100; step++) {
    mesh.snapshot(step);
  }

  for (unsigned int step = 100; step < 125; step++) {
    mesh.doTimeStep(tau, 8);
    mesh.snapshot(step);
  }

  for (unsigned int step = 125; step < 150; step++) {
    mesh.doTimeStep(tau, 9);
    mesh.snapshot(step);
  }

  for (unsigned int step = 150; step < 175; step++) {
    mesh.doTimeStep(tau, 6);
    mesh.snapshot(step);
  }

  for (unsigned int step = 175; step < 200; step++) {
    mesh.doTimeStep(tau, 7);
    mesh.snapshot(step);
  }

  for (unsigned int step = 200; step < 300; step++) {
    if (step % 4 == 0) {
      mesh.doTimeStep(tau, 10);
      mesh.snapshot(step);
    }
    if (step % 4 == 1) {
      mesh.doTimeStep(tau, 11);
      mesh.snapshot(step);
    }
    if (step % 4 == 2) {
      mesh.doTimeStep(tau, 12);
      mesh.snapshot(step);
    }
    if (step % 4 == 3) {
      mesh.doTimeStep(tau, 13);
      mesh.snapshot(step);
    }    
  }

  for (unsigned int step = 300; step < 350; step++) {
    mesh.doTimeStep(tau, 1);
    mesh.snapshot(step);
  }

  for (unsigned int step = 350; step < 400; step++) {
    mesh.doTimeStep(tau, 2);
    mesh.snapshot(step);
  }

  for (unsigned int step = 400; step < 450; step++) {
    mesh.doTimeStep(tau, 3);
    mesh.snapshot(step);
  }

  for (unsigned int step = 450; step < 500; step++) {
    mesh.doTimeStep(tau, 4);
    mesh.snapshot(step);
  }

  for (unsigned int step = 500; step < 650; step++) {
    mesh.doTimeStep(tau, 5);
    mesh.snapshot(step);
  }

  return 0;
}
