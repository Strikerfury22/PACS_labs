#include <iostream>
#include <Eigen/Dense>
//This line will deactivate vectorization
//#define EIGEN_DONT_VECTORIZE
//This line will activate vectorization
//#define EIGEN_VECTORIZE
//generate random matrix at: https://onlinemathtools.com/generate-random-matrix
//check this in the future ALSO DELETE: https://stackoverflow.com/questions/35827926/eigen-matrix-library-filling-a-matrix-with-random-float-values-in-a-given-range
int main(int argc, char* argv[])
{
  if(argc == 2){
		int id = atoi(argv[1]);
		switch(id){
		case 1:
      {
      Eigen::MatrixXd mat1(32,32);
      mat1 << Eigen::MatrixXd::Random(32,32);
      Eigen::MatrixXd mat2(32,32);
      mat2 << Eigen::MatrixXd::Random(32,32);
      Eigen::MatrixXd mat3 = mat1 * mat2;
      }
      break;
    case 2:
      {
      Eigen::MatrixXd mat1(100,100);
      mat1 << Eigen::MatrixXd::Random(100,100);
      Eigen::MatrixXd mat2(100,100);
      mat2 << Eigen::MatrixXd::Random(100,100);
      Eigen::MatrixXd mat3 = mat1 * mat2;
      }
      break;
    case 3:
      {
      Eigen::MatrixXd mat1(150,150);
      mat1 << Eigen::MatrixXd::Random(150,150);
      Eigen::MatrixXd mat2(150,150);
      mat2 << Eigen::MatrixXd::Random(150,150);
      Eigen::MatrixXd mat3 = mat1 * mat2;
      }
      break;
    case 4:
      {
      Eigen::MatrixXd mat1(708,708);
      mat1 << Eigen::MatrixXd::Random(708,708);
      Eigen::MatrixXd mat2(708,708);
      mat2 << Eigen::MatrixXd::Random(708,708);
      Eigen::MatrixXd mat3 = mat1 * mat2;
      }
      break;
    case 5:
      {
      Eigen::MatrixXd mat1(900,900);
      mat1 << Eigen::MatrixXd::Random(900,900);
      Eigen::MatrixXd mat2(900,900);
      mat2 << Eigen::MatrixXd::Random(900,900);
      Eigen::MatrixXd mat3 = mat1 * mat2;
      }
      break;
    case 6:
      {
      Eigen::MatrixXd mat1(1200,1200);
      mat1 << Eigen::MatrixXd::Random(1200,1200);
      Eigen::MatrixXd mat2(1200,1200);
      mat2 << Eigen::MatrixXd::Random(1200,1200);
      Eigen::MatrixXd mat3 = mat1 * mat2;
      }
      break;
    case 7:
      {
      Eigen::MatrixXd mat1(2500,2500);
      mat1 << Eigen::MatrixXd::Random(2500,2500);
      Eigen::MatrixXd mat2(2500,2500);
      mat2 << Eigen::MatrixXd::Random(2500,2500);
      Eigen::MatrixXd mat3 = mat1 * mat2;
      }
      break;
    case 8:
      {
      Eigen::MatrixXd mat1(5000,5000);
      mat1 << Eigen::MatrixXd::Random(5000,5000);
      Eigen::MatrixXd mat2(5000,5000);
      mat2 << Eigen::MatrixXd::Random(5000,5000);
      Eigen::MatrixXd mat3 = mat1 * mat2;
      }
      break;
    case 9:
      {
      Eigen::MatrixXd mat1(10000,10000);
      mat1 << Eigen::MatrixXd::Random(10000,10000);
      Eigen::MatrixXd mat2(10000,10000);
      mat2 << Eigen::MatrixXd::Random(10000,10000);
      Eigen::MatrixXd mat3 = mat1 * mat2;
      }
      break;
    default:
      std::cout << "ID de prueba no válido. Su valor debe estar entre  1 y 9 incluidos." << std::endl;
		}
		std::cout << "Saliendo del programa." << std::endl;
	} else {
		std::cout << "La estructura de llamada es: ./programa <id_prueba>. Donde <id_prueba> es un valor entre 1 y 9 incluidos." << std::endl;
	}
  }
