#include <fstream>
#include "itkRANSAC.h"
#include "itkLandmarkRegistrationEstimator.h"
#include "RandomNumberGenerator.h"
#include "itkMesh.h"
#include "itkMeshFileReader.h"


template <unsigned int dimension>
void
GenerateData(unsigned int                                 numInliers,
             unsigned int                                 numOutliers,
             double                                       outlierDistance,
             std::vector<itk::Point<double, dimension>> & data,
             std::vector<double> &                        planeParameters);


int
itkRansacTest_LandmarkRegistration(int argc, char * argv[])
{
  const unsigned int DIMENSION = 6;
  const unsigned int INLIERS = 10;
  const unsigned int OUTLIERS = 0;//10;
  
  typedef itk::RANSAC<itk::Point<double, 6>, double> RANSACType;

  std::vector<itk::Point<double, 6>> data;
  std::vector<double> truePlaneParameters, transformParameters;
  double              outlierDistance = 20.0;

  GenerateData<DIMENSION>(INLIERS, OUTLIERS, outlierDistance, data, truePlaneParameters);

  // create and initialize the parameter estimator
  double                      maximalDistanceFromPlane = 15;
  auto registrationEstimator = itk::LandmarkRegistrationEstimator<6>::New();
  registrationEstimator->SetMinimalForEstimate(250);
  registrationEstimator->SetDelta(maximalDistanceFromPlane);
  registrationEstimator->LeastSquaresEstimate(data, transformParameters);
  
  std::cout << "Least Squares Estimate is " << std::endl;
  unsigned int i = 0;
  for (i = 0; i < transformParameters.size(); i++)
  {
    std::cout << transformParameters[i] << ", ";
  }

  
  // create and initialize the RANSAC algorithm
  double              desiredProbabilityForNoOutliers = 0.01;
  double              percentageOfDataUsed;
  RANSACType::Pointer ransacEstimator = RANSACType::New();
  ransacEstimator->SetData(data);
  ransacEstimator->SetParametersEstimator(registrationEstimator);
  percentageOfDataUsed = ransacEstimator->Compute(transformParameters, desiredProbabilityForNoOutliers);

  // if (planeParameters.empty())
  // {
  //   std::cout << "RANSAC estimate failed, degenerate configuration?\n";
  // }
  // else
  // {
  //   std::cout << "RANSAC hyper(plane) parameters: [n,a]\n\t [ ";
  //   for (i = 0; i < (2 * DIMENSION - 1); i++)
  //   {
  //     std::cout << planeParameters[i] << ", ";
  //   }
  //   std::cout << planeParameters[i] << "]\n\n";
    
  //   dotProduct = 0.0;
  //   for (i = 0; i < DIMENSION; i++)
  //   {
  //     dotProduct += planeParameters[i] * truePlaneParameters[i];
  //   }
  //   std::cout << "\tDot product of real and computed normals[+-1=correct]: ";
  //   std::cout << dotProduct << "\n";
    
  //   dotProduct = 0.0;
  //   for (i = 0; i < DIMENSION; i++)
  //   {
  //     dotProduct += (planeParameters[DIMENSION + i] - truePlaneParameters[DIMENSION + i]) * truePlaneParameters[i];
  //   }
    
  //   std::cout << dotProduct << "\n\n";
    
  //   std::cout << percentageOfDataUsed << "\n\n";
  // }
  return EXIT_SUCCESS;
}


template <unsigned int dimension>
void
GenerateData(unsigned int          numInliers,
             unsigned int          numOutliers,
             double                outlierDistance,
             std::vector<itk::Point<double, dimension>> & data,
             std::vector<double> & planeParameters)
{
  
  // Read the two point sets that are the putative matches
  using CoordinateType = double;
  using MeshType = itk::Mesh<CoordinateType, 3>;
  using ReaderType = itk::MeshFileReader<MeshType>;

  auto reader1 = ReaderType::New();
  reader1->SetFileName("/home/pranjal.sahu/ethicon_aws/deep_learning/deep_learning/train/fixed.vtk");
  reader1->Update();
  auto mesh1 = reader1->GetOutput();

  auto reader2 = ReaderType::New();
  reader2->SetFileName("/home/pranjal.sahu/ethicon_aws/deep_learning/deep_learning/train/moving.vtk");
  reader2->Update();
  auto mesh2 = reader2->GetOutput();
  
  data.reserve(mesh1->GetNumberOfPoints());
 
  // Concatenate corressponding points from two meshes and insert in the data vector
  using PointType = itk::Point<CoordinateType, 6>;
  PointType p0;
  for(unsigned int i = 0; i < mesh1->GetNumberOfPoints(); ++i)
  {
    auto point1 = mesh1->GetPoint(i);
    auto point2 = mesh2->GetPoint(i);

    p0[0] = point1[0];
    p0[1] = point1[1];
    p0[2] = point1[2];

    p0[3] = point2[0];
    p0[4] = point2[1];
    p0[5] = point2[2];

    data.push_back(p0);
  }

  return;
}
