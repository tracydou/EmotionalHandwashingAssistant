// Code found here: http://stackoverflow.com/questions/6142576/sample-from-multivariate-normal-gaussian-distribution-in-c

#include <Eigen/Dense>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/normal_distribution.hpp>    

/*
  We need a functor that can pretend it's const,
  but to be a good random number generator 
  it needs mutable state.
*/
namespace Eigen {
  namespace internal {

    template<typename Scalar> 
    struct scalar_normal_dist_op 
    {
      static boost::mt19937 rng;    // The uniform pseudo-random algorithm
      mutable boost::normal_distribution<Scalar> norm;  // The gaussian combinator

      EIGEN_EMPTY_STRUCT_CTOR(scalar_normal_dist_op)

      template<typename Index>
      inline const Scalar operator() (Index, Index = 0) const { return norm(rng); }
    };

    template<typename Scalar> boost::mt19937 scalar_normal_dist_op<Scalar>::rng;

    template<typename Scalar>
    struct functor_traits<scalar_normal_dist_op<Scalar> >
    {
      enum { Cost = 50 * NumTraits<Scalar>::MulCost, PacketAccess = false, IsRepeatable = false };
    };
  } // end namespace internal
} // end namespace Eigen

namespace bayesact {
  Eigen::internal::scalar_normal_dist_op<double> randN_; // Gaussian functor

  void seedRng_(unsigned s)
  {
    Eigen::internal::scalar_normal_dist_op<double>::rng.seed(s); // Seed the rng
  }

  template<int size>
  Eigen::Matrix<double, size, 1>
  computeMultivariateNormal(Eigen::Matrix<double, size, 1> mean,
                            Eigen::Matrix<double, size, size> covar)
  {

    Eigen::MatrixXd normTransform(size,size);

    Eigen::LLT<Eigen::MatrixXd> cholSolver(covar);

    // We can only use the cholesky decomposition if 
    // the covariance matrix is symmetric, pos-definite.
    // But a covariance matrix might be pos-semi-definite.
    // In that case, we'll go to an EigenSolver
    if (cholSolver.info()==Eigen::Success) {
      // Use cholesky solver
      normTransform = cholSolver.matrixL();
    } else {
      // Use eigen solver
      Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eigenSolver(covar);
      normTransform = eigenSolver.eigenvectors() 
                     * eigenSolver.eigenvalues().cwiseSqrt().asDiagonal();
    }

    Eigen::Matrix<double, size, 1> retval;
    retval = (normTransform
              * Eigen::MatrixXd::NullaryExpr(size,1,randN_)).colwise()
              + mean;

    return retval;
  }
}
