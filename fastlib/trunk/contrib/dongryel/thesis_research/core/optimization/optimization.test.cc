/** @file optimization.test.cc
 *
 *  @brief The test driver for the L-BFGS optimizer and the trust
 *         region optimizer on some popular test functions.
 *
 *  @author Dongryeol Lee (dongryel@cc.gatech.edu)
 */

#include "core/math/math_lib.h"
#include "core/optimization/lbfgs_dev.h"
#include "core/optimization/trust_region_dev.h"
#include <stdexcept>

namespace core {
namespace optimization {
namespace optimization_test {

class ExtendedRosenbrockFunction {

  private:

    int num_dimensions_;

  public:
    double Evaluate(const arma::vec &x) {
      double fval = 0;
      for(int i = 0; i < num_dimensions() - 1; i++) {
        fval = fval + 100 * core::math::Sqr(x[i] * x[i] - x[i + 1]) +
               core::math::Sqr(x[i] - 1);
      }
      return fval;
    }

    void Gradient(const arma::vec &x, arma::vec *gradient) {

      gradient->zeros();
      for(int k = 0; k < num_dimensions() - 1; k++) {
        (*gradient)[k] = 400 * x[k] * (x[k] * x[k] - x[k+1]) + 2 * (x[k] - 1);
        if(k > 0) {
          (*gradient)[k] = (*gradient)[k] + 200 * (x[k] - x[k - 1] * x[k - 1]);
        }
      }
      (*gradient)[num_dimensions() - 1] =
        200 * (x[num_dimensions() - 1] -
               core::math::Sqr(x[num_dimensions() - 2]));
    }

    void Hessian(const arma::vec &x, arma::mat *hessian) {
      hessian->zeros(num_dimensions_, num_dimensions_);
    }

    int num_dimensions() const {
      return num_dimensions_;
    }

    void InitStartingIterate(arma::vec *iterate) {
      num_dimensions_ = 2 * core::math::RandInt(2, 100);
      iterate->set_size(num_dimensions_);
      for(int i = 0; i < num_dimensions_; i++) {
        if(i % 2 == 0) {
          (*iterate)[i] = -1.2;
        }
        else {
          (*iterate)[i] = 1.0;
        }
      }
    }
};

class WoodFunction {

  public:
    double Evaluate(const arma::vec &x) {
      return 100 * core::math::Sqr(x[0] * x[0] - x[1]) +
             core::math::Sqr(1 - x[0]) +
             90 * core::math::Sqr(x[2] * x[2] - x[3]) +
             core::math::Sqr(1 - x[2]) +
             10.1 * (core::math::Sqr(1 - x[1]) + core::math::Sqr(1 - x[3])) +
             19.8 * (1 - x[1]) * (1 - x[3]);
    }

    void Gradient(const arma::vec &x, arma::vec *gradient) {
      (*gradient)[0] = 400 * x[0] * (x[0] * x[0] - x[1]) + 2 * (x[0] - 1);
      (*gradient)[1] = 200 * (x[1] - x[0] * x[0]) + 20.2 * (x[1] - 1) +
                       19.8 * (x[3] - 1);
      (*gradient)[2] = 360 * x[2] * (x[2] * x[2] - x[3]) + 2 * (x[2] - 1);
      (*gradient)[3] = 180 * (x[3] - x[2] * x[2]) + 20.2 * (x[3] - 1) +
                       19.8 * (x[1] - 1);
    }

    void Hessian(const arma::vec &x, arma::mat *hessian) {
      hessian->zeros(this->num_dimensions(), this->num_dimensions());
    }

    int num_dimensions() const {
      return 4;
    }

    void InitStartingIterate(arma::vec *iterate) {

      iterate->set_size(num_dimensions());
      (*iterate)[0] = (*iterate)[2] = -3;
      (*iterate)[1] = (*iterate)[3] = -1;
    }
};

template< template<typename> class OptimizerType>
class OptimizerInitTrait {
  public:
    template<typename FunctionType>
    static void Init(
      OptimizerType<FunctionType> &optimizer_in,
      FunctionType &function_in,
      int num_lbfgs_basis,
      core::optimization::TrustRegionSearchMethod::SearchType
      trust_region_search_method_in);
};

template<>
class OptimizerInitTrait< core::optimization::Lbfgs > {
  public:
    template<typename FunctionType>
    static void Init(
      core::optimization::Lbfgs<FunctionType> &optimizer_in,
      FunctionType &function_in,
      int num_lbfgs_basis,
      core::optimization::TrustRegionSearchMethod::SearchType
      trust_region_search_method_in) {
      optimizer_in.Init(function_in, num_lbfgs_basis);
    }
};

template<>
class OptimizerInitTrait< core::optimization::TrustRegion > {
  public:
    template<typename FunctionType>
    static void Init(
      core::optimization::TrustRegion<FunctionType> &optimizer_in,
      FunctionType &function_in,
      int num_lbfgs_basis,
      core::optimization::TrustRegionSearchMethod::SearchType
      trust_region_search_method_in) {
      optimizer_in.Init(function_in, trust_region_search_method_in);
    }
};

template< template<typename> class OptimizerType >
class OptimizationTest {
  public:

    void TestExtendedRosenbrockFunction() {

      std::cout << "Testing extended Rosenbrock function: optimal value: 0.\n";
      for(int i = 0; i < 10; i++) {
        core::optimization::optimization_test::ExtendedRosenbrockFunction
        extended_rosenbrock_function;
        OptimizerType <
        core::optimization::optimization_test::ExtendedRosenbrockFunction >
        extended_rosenbrock_function_optimizer;
        arma::vec extended_rosenbrock_function_optimized;
        extended_rosenbrock_function.InitStartingIterate(
          &extended_rosenbrock_function_optimized);
        core::optimization::optimization_test::
        OptimizerInitTrait<OptimizerType>::Init(
          extended_rosenbrock_function_optimizer,
          extended_rosenbrock_function,
          std::min(extended_rosenbrock_function.num_dimensions() / 2, 20),
          core::optimization::TrustRegionSearchMethod::CAUCHY);
        extended_rosenbrock_function_optimizer.Optimize(
          -1, &extended_rosenbrock_function_optimized);

        // Test whether the evaluation is close to the zero.
        double function_value = extended_rosenbrock_function.Evaluate(
                                  extended_rosenbrock_function_optimized);
        printf(
          "%d dimensional extended Rosenbrock function optimized to the "
          "function value of %g\n",
          extended_rosenbrock_function.num_dimensions(), function_value);
        if(function_value > 0.5 || function_value < -0.5) {
          throw std::runtime_error("Aborted in extended Rosenbrock test");
        }

        // It should converge to something close to all 1's.
        for(unsigned int i = 0;
            i < extended_rosenbrock_function_optimized.n_elem; i++) {
          if(extended_rosenbrock_function_optimized[i] > 1.5 ||
              extended_rosenbrock_function_optimized[i] < 0.5) {
            throw std::runtime_error("Invalid optimal point");
          }
        }
      }
    }

    void TestWoodFunction() {
      printf("Testing wood function: optimal value: 0.\n");
      core::optimization::optimization_test::WoodFunction wood_function;
      arma::vec wood_function_optimized;
      OptimizerType <
      core::optimization::optimization_test::WoodFunction > wood_function_optimizer;
      wood_function.InitStartingIterate(&wood_function_optimized);

      core::optimization::optimization_test::
      OptimizerInitTrait <
      OptimizerType >::Init(
        wood_function_optimizer,
        wood_function, 3,
        core::optimization::TrustRegionSearchMethod::CAUCHY);
      wood_function_optimizer.Optimize(-1, &wood_function_optimized);

      // It should converge to something close to (1, 1, 1, 1)^T
      for(unsigned int i = 0; i < wood_function_optimized.n_elem; i++) {
        if(wood_function_optimized[i] < 0.5 ||
            wood_function_optimized[i] > 1.5) {
          throw std::runtime_error("Failed in wood function");
        }
      }
    }
};
}
}
}

int main(int argc, char *argv[]) {
  printf("Starting L-BFGS tests.\n");
  core::optimization::optimization_test::OptimizationTest <
  core::optimization::Lbfgs > lbfgs_test;
  lbfgs_test.TestExtendedRosenbrockFunction();
  lbfgs_test.TestWoodFunction();
  printf("Starting trust region tests.\n");
  core::optimization::optimization_test::OptimizationTest <
  core::optimization::TrustRegion > trust_region_test;
  trust_region_test.TestExtendedRosenbrockFunction();
  trust_region_test.TestWoodFunction();
  printf("All tests passed!");
}
