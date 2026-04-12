#include "poisson.h"
#include <basix/finite-element.h>
#include <cmath>
#include <dolfinx.h>
#include <dolfinx/fem/Constant.h>
#include <dolfinx/fem/petsc.h>
#include <dolfinx/la/petsc.h>
#include <petscmat.h>
#include <petscsys.h>
#include <petscsystypes.h>
#include <utility>
#include <vector>

using namespace dolfinx;
using T = PetscScalar;
using U = typename dolfinx::scalar_value_t<T>;

int main(int argc, char* argv[]) {
  dolfinx::init_logging(argc, argv);
  PetscInitialize(&argc, &argv, nullptr, nullptr);
  {
    auto part = mesh::create_cell_partitioner(mesh::GhostMode::shared_facet);
    auto mesh = std::make_shared<mesh::Mesh<U>>(
        mesh::create_rectangle<U>(MPI_COMM_WORLD, {{{0.0, 0.0}, {2.0, 1.0}}},
                                  {32, 16}, mesh::CellType::triangle, part));
    auto element = basix::create_element<U>(
        basix::element::family::P, basix::cell::type::triangle, 1,
        basix::element::lagrange_variant::unset,
        basix::element::dpc_variant::unset, false);
    auto V =
        std::make_shared<fem::FunctionSpace<U>>(fem::create_functionspace<U>(
            mesh, std::make_shared<fem::FiniteElement<U>>(element)));
    auto kappa = std::make_shared<fem::Constant<T>>(1.0);
    auto f = std::make_shared<fem::Function<T>>(V);
    auto g = std::make_shared<fem::Function<T>>(V);
    fem::Form<T> a = fem::create_form<T>(*form_poisson_a, {V, V}, {},
                                         {{"kappa", kappa}}, {}, {});
    fem::Form<T> L = fem::create_form<T>(*form_poisson_L, {V},
                                         {{"f", f}, {"g", g}}, {}, {}, {});

    f->interpolate(
        [](auto x) -> std::pair<std::vector<T>, std::vector<std::size_t>> {
          std::vector<T> f;
          for (std::size_t p = 0; p < x.extent(1); ++p) f.push_back(0.0);
          return {f, {f.size()}};
        });
    g->interpolate(
        [](auto x) -> std::pair<std::vector<T>, std::vector<std::size_t>> {
          std::vector<T> g;
          for (std::size_t p = 0; p < x.extent(1); ++p) g.push_back(0.0);
          return {g, {g.size()}};
        });

    std::vector facets_left =
        mesh::locate_entities_boundary(*mesh, 1, [](auto x) {
          constexpr U eps = 1.0e-8;
          std::vector<std::int8_t> marker(x.extent(1), false);
          for (std::size_t p = 0; p < x.extent(1); ++p)
            if (std::abs(x(0, p)) < eps) marker[p] = true;
          return marker;
        });
    std::vector facets_right =
        mesh::locate_entities_boundary(*mesh, 1, [](auto x) {
          constexpr U eps = 1.0e-8;
          std::vector<std::int8_t> marker(x.extent(1), false);
          for (std::size_t p = 0; p < x.extent(1); ++p)
            if (std::abs(x(0, p) - 2.0) < eps) marker[p] = true;
          return marker;
        });
    std::vector bdofs_left = fem::locate_dofs_topological(
        *V->mesh()->topology_mutable(), *V->dofmap(), 1, facets_left);
    std::vector bdofs_right = fem::locate_dofs_topological(
        *V->mesh()->topology_mutable(), *V->dofmap(), 1, facets_right);

    for (int frame = 0; frame < 200; ++frame) {
      double time_factor = frame / 199.0;
      double left_temp = 200.0 * time_factor;
      fem::DirichletBC<T> bc_left(left_temp, bdofs_left, V);
      fem::DirichletBC<T> bc_right(0.0, bdofs_right, V);
      std::vector<std::reference_wrapper<const fem::DirichletBC<T>>> bcs = {
          bc_left, bc_right};

      la::petsc::Matrix A(fem::petsc::create_matrix(a), false);
      MatZeroEntries(A.mat());
      fem::assemble_matrix(la::petsc::Matrix::set_block_fn(A.mat(), ADD_VALUES),
                           a, bcs);
      MatAssemblyBegin(A.mat(), MAT_FLUSH_ASSEMBLY);
      MatAssemblyEnd(A.mat(), MAT_FLUSH_ASSEMBLY);
      fem::set_diagonal<T>(la::petsc::Matrix::set_fn(A.mat(), INSERT_VALUES),
                           *V, bcs);
      MatAssemblyBegin(A.mat(), MAT_FINAL_ASSEMBLY);
      MatAssemblyEnd(A.mat(), MAT_FINAL_ASSEMBLY);

      la::Vector<T> b(L.function_spaces()[0]->dofmap()->index_map,
                      L.function_spaces()[0]->dofmap()->index_map_bs());
      std::ranges::fill(b.array(), 0);
      fem::assemble_vector(b.array(), L);
      fem::apply_lifting(b.array(), {a}, {bcs}, {}, T(1));
      b.scatter_rev(std::plus<T>());
      for (auto& bc : bcs) bc.get().set(b.array(), std::nullopt);
      auto u = std::make_shared<fem::Function<T>>(V);
      la::petsc::KrylovSolver lu(MPI_COMM_WORLD);
      la::petsc::options::set("ksp_type", "preonly");
      la::petsc::options::set("pc_type", "lu");
      lu.set_from_options();
      lu.set_operator(A.mat());

      la::petsc::Vector _u(la::petsc::create_vector_wrap(*u->x()), false);
      la::petsc::Vector _b(la::petsc::create_vector_wrap(b), false);
      lu.solve(_u.vec(), _b.vec());
      u->x()->scatter_fwd();

      std::string filename = "frame_" + std::to_string(frame) + "/u.pvd";
      std::filesystem::create_directory("frame_" + std::to_string(frame));
      io::VTKFile file(MPI_COMM_WORLD, filename, "w");
      file.write<T>({*u}, 0.0);
    }
  }
  PetscFinalize();
  return 0;
}
