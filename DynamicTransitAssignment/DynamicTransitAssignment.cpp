// DynamicTransitAssignment.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
//#include "MapSupNtw.h"
#include "passengerOD.h"
#include "CTM.h"
#include "TransitNetworkLoading.h"
#include "myquadprog.h"

#pragma comment( lib, "myquadprog.lib" )

using namespace std;
using namespace Eigen;

//bool passengerFlowComparer(MapSupNtw::passengerFlow& pf_a, MapSupNtw::passengerFlow& pf_b)
//{
//	if (pf_a.startTime != pf_b.startTime)
//		return (pf_a.startTime < pf_b.startTime);
//	else if (pf_a.passengerGroup != pf_b.passengerGroup)
//		return (pf_a.passengerGroup < pf_b.passengerGroup);
//	else
//		return (pf_a.pathNo < pf_b.pathNo);
//}

template<typename T>
vector<vector<T>> MatrixMultiply(vector<vector<T>> arg1, std::vector<std::vector<T>> arg2) {
	int M = arg1.size();
	int N = arg2[0].size();
	int K = arg2.size();

	vector<vector<T>> res;
	for (int m = 0; m < M;m++) {
		vector<T> tm;
		for (int n = 0; n < N; n++) {
			tm.push_back(0);
			for (int k = 0; k < K;k++) {
				tm[n] += arg1[m][k]* arg2[k][n];
			}			
		}
		res.push_back(tm);
	}
	return res;
}

template<typename T>
vector<vector<T>> MatrixT(vector<vector<T>> arg1) {
	int M = arg1.size();
	int N = arg1[0].size();

	vector<vector<T>> res;
	for (int n = 0; n < N; n++) {
		vector<T> tm;
		for (int m = 0; m < M; m++) {
			tm.push_back(arg1[m][n]);
		}
		res.push_back(tm);
	}
	return res;
}

template<typename T>
T Matrix2Parametrics(vector<T> arg1) {
	T tm = 0;
	for (int i = 0; i < arg1.size(); i++) {
		tm += arg1[i]* arg1[i];
	}
	return sqrt(tm);
}


void removeRow(Eigen::MatrixXd& matrix, unsigned int rowToRemove)
{
	unsigned int numRows = matrix.rows() - 1;
	unsigned int numCols = matrix.cols();

	if (rowToRemove < numRows)
		matrix.block(rowToRemove, 0, numRows - rowToRemove, numCols) = matrix.block(rowToRemove + 1, 0, numRows - rowToRemove, numCols);

	matrix.conservativeResize(numRows, numCols);
}


std::vector<std::vector<double>> inverseMatrixLU(const std::vector<std::vector<double>>& matrix) {
	int n = matrix.size();

	// Allocate memory for the LU decomposition and inverse matrix
	std::vector<std::vector<double>> LU(n, std::vector<double>(n, 0.0));
	std::vector<std::vector<double>> invMatrix(n, std::vector<double>(n, 0.0));
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			LU[i][j] = matrix[i][j];
			invMatrix[i][j] = (i == j) ? 1.0 : 0.0;
		}
	}

	// Perform LU decomposition
	for (int k = 0; k < n - 1; k++) {
		for (int i = k + 1; i < n; i++) {
			double factor = LU[i][k] / LU[k][k];
			for (int j = k + 1; j < n; j++) {
				LU[i][j] -= factor * LU[k][j];
			}
			LU[i][k] = factor;
			for (int j = 0; j < n; j++) {
				invMatrix[i][j] -= factor * invMatrix[k][j];
			}
		}
	}

	// Solve for the inverse using backward substitution
	for (int k = n - 1; k >= 0; k--) {
		for (int j = 0; j < n; j++) {
			double sum = 0.0;
			for (int i = k + 1; i < n; i++) {
				sum += LU[k][i] * invMatrix[i][j];
			}
			invMatrix[k][j] = (invMatrix[k][j] - sum) / LU[k][k];
		}
	}

	// Return the inverse matrix
	return invMatrix;
}

int matrix_rank(const MatrixXd& matrix) {
	int rank = matrix.rows();
	MatrixXd reduced_matrix = matrix.fullPivLu().matrixLU().triangularView<Upper>();
	for (int i = rank - 1; i >= 0; i--) {
		int allZero = 1;
		for (int j = 0; j < reduced_matrix.cols();j++) {
			if (reduced_matrix(i,j)<-0.000001 || reduced_matrix(i, j)>0.000001) {
				allZero = 0;
			}
		}

		if (allZero == 1) {
			rank--;
		}
		else {
			break;
		}
	}
	return rank;
}

/// <summary>
/// 
/// </summary>
/// <param name="pathPassenger"></param>
/// <param name="pathTravelTime"></param>
/// <param name="passengerGroups"></param>
/// <param name="T"></param>
/// <param name="ntl"></param>
/// <param name="typeC"> 0是不考虑共享单车的gap，1是考虑共享单车的gap </param>
/// <returns></returns>
double calRgap(vector<vector<double>> pathPassenger, vector<vector<double>> pathTravelTime, vector<passengerOD*> passengerGroups, int T, TransitNetworkLoading* ntl, int typeC = 0) {
	double path_tpye_1_minTime = 9999;
	double path_tpye_2_minTime = 9999;
	double path_tpye_3_minTime = 9999;
	double path_tpye_4_minTime = 9999;
	double path_tpye_5_minTime = 9999;
	double path_tpye_6_minTime = 9999;


	double passengerTravelTime = 0;
	double passengerShortestTime = 0;

	double TpassengerTravelTime = 0;
	double TpassengerShortestTime = 0;

	double sumPassengerCost = 0;

	int pathType = 0;
	int tmNum = 0;
	double sumExp = 0;
	double theta = 0.5;
	double gap = 0;
	//std::cout << "t_gap";
	for (int t = 0; t < T; t++) {

		TpassengerTravelTime = 0;
		TpassengerShortestTime = 0;

		for (int i = 0; i < passengerGroups.size(); i++) {

			double sumP = 0;
			double nunCountp = 0;
			double nunCountpTime = 0;

			/*double testSump = 0;
			double testShort = 0;*/

			for (int j = 0; j < passengerGroups[i]->paths.size(); j++) {

				pathType = passengerGroups[i]->paths[j].type;
				int pathBikeDock1 = passengerGroups[i]->paths[j].outShareBicyclePoint1;
				int pathBikeDock2 = passengerGroups[i]->paths[j].outShareBicyclePoint2;
				/*pathType:
					1  share bicycle
					2  bus
					3  subway-bicycle
					4  subway-bus
					5  bicycle-bus
					6  subway
				*/
				TpassengerTravelTime += pathTravelTime[t][tmNum] * pathPassenger[t][tmNum];
				//testSump += pathTravelTime[t][tmNum] * pathPassenger[t][tmNum];

				sumP += pathPassenger[t][tmNum];

				if (pathType == 1) {
					if (pathBikeDock1 != -1) {
						if (ntl->dockCStar[t][ntl->getShareBicycleSeq(pathBikeDock1)] > 0.00001) {
							if (pathBikeDock2 != -1) {
								if (ntl->dockCStar[t][ntl->getShareBicycleSeq(pathBikeDock2)] > 0.00001) {
									if (path_tpye_1_minTime > pathTravelTime[t][tmNum]) {
										path_tpye_1_minTime = pathTravelTime[t][tmNum];
									}
								}
								else {
									nunCountp += pathPassenger[t][tmNum];
									nunCountpTime += pathTravelTime[t][tmNum] * pathPassenger[t][tmNum];
								}
							}
							else {
								if (path_tpye_1_minTime > pathTravelTime[t][tmNum]) {
									path_tpye_1_minTime = pathTravelTime[t][tmNum];
								}
							}
						}
						else {
							nunCountp += pathPassenger[t][tmNum];
							nunCountpTime += pathTravelTime[t][tmNum] * pathPassenger[t][tmNum];
						}

					}
				}
				else if (pathType == 2) {
					if (path_tpye_2_minTime > pathTravelTime[t][tmNum]) {
						path_tpye_2_minTime = pathTravelTime[t][tmNum];
					}
				}
				else if (pathType == 3) {

					if (pathBikeDock1 != -1) {
						if (ntl->dockCStar[t][ntl->getShareBicycleSeq(pathBikeDock1)] > 0.00001) {
							if (pathBikeDock2 != -1) {
								if (ntl->dockCStar[t][ntl->getShareBicycleSeq(pathBikeDock2)] > 0.00001) {
									if (path_tpye_3_minTime > pathTravelTime[t][tmNum]) {
										path_tpye_3_minTime = pathTravelTime[t][tmNum];
									}
								}
								else {
									nunCountp += pathPassenger[t][tmNum];
									nunCountpTime += pathTravelTime[t][tmNum] * pathPassenger[t][tmNum];
								}
							}
							else {
								if (path_tpye_3_minTime > pathTravelTime[t][tmNum]) {
									path_tpye_3_minTime = pathTravelTime[t][tmNum];
								}
							}
						}
						else {
							nunCountp += pathPassenger[t][tmNum];
							nunCountpTime += pathTravelTime[t][tmNum] * pathPassenger[t][tmNum];
						}

					}

				}
				else if (pathType == 4) {
					if (path_tpye_4_minTime > pathTravelTime[t][tmNum]) {
						path_tpye_4_minTime = pathTravelTime[t][tmNum];
					}
				}
				else if (pathType == 5) {

					if (pathBikeDock1 != -1) {
						if (ntl->dockCStar[t][ntl->getShareBicycleSeq(pathBikeDock1)] > 0.00001) {
							if (pathBikeDock2 != -1) {
								if (ntl->dockCStar[t][ntl->getShareBicycleSeq(pathBikeDock2)] > 0.00001) {
									if (path_tpye_5_minTime > pathTravelTime[t][tmNum]) {
										path_tpye_5_minTime = pathTravelTime[t][tmNum];
									}
								}
								else {
									nunCountp += pathPassenger[t][tmNum];
									nunCountpTime += pathTravelTime[t][tmNum] * pathPassenger[t][tmNum];
								}
							}
							else {
								if (path_tpye_5_minTime > pathTravelTime[t][tmNum]) {
									path_tpye_5_minTime = pathTravelTime[t][tmNum];
								}
							}
						}
						else {
							nunCountp += pathPassenger[t][tmNum];
							nunCountpTime += pathTravelTime[t][tmNum] * pathPassenger[t][tmNum];
						}

					}

				}
				else if (pathType == 6) {
					if (path_tpye_6_minTime > pathTravelTime[t][tmNum]) {
						path_tpye_6_minTime = pathTravelTime[t][tmNum];
					}
				}

				tmNum++;
			}

			if (path_tpye_1_minTime != 9999) {
				sumExp += std::exp(-1 * theta * path_tpye_1_minTime);
			}
			if (path_tpye_2_minTime != 9999) {
				sumExp += std::exp(-1 * theta * path_tpye_2_minTime);
			}
			if (path_tpye_3_minTime != 9999) {
				sumExp += std::exp(-1 * theta * path_tpye_3_minTime);
			}
			if (path_tpye_4_minTime != 9999) {
				sumExp += std::exp(-1 * theta * path_tpye_4_minTime);
			}
			if (path_tpye_5_minTime != 9999) {
				sumExp += std::exp(-1 * theta * path_tpye_5_minTime);
			}
			if (path_tpye_6_minTime != 9999) {
				sumExp += std::exp(-1 * theta * path_tpye_6_minTime);
			}

			if (path_tpye_1_minTime != 9999) {
				TpassengerShortestTime += path_tpye_1_minTime * (sumP - nunCountp) * exp(-1 * theta * path_tpye_1_minTime) / sumExp;
				//testShort += path_tpye_1_minTime * (sumP - nunCountp) * exp(-1 * theta * path_tpye_1_minTime) / sumExp;
			}
			if (path_tpye_2_minTime != 9999) {
				TpassengerShortestTime += path_tpye_2_minTime * (sumP - nunCountp) * exp(-1 * theta * path_tpye_2_minTime) / sumExp;
				//testShort += path_tpye_2_minTime * (sumP - nunCountp) * exp(-1 * theta * path_tpye_2_minTime) / sumExp;
			}
			if (path_tpye_3_minTime != 9999) {
				TpassengerShortestTime += path_tpye_3_minTime * (sumP - nunCountp) * exp(-1 * theta * path_tpye_3_minTime) / sumExp;
				//testShort += path_tpye_3_minTime * (sumP - nunCountp) * exp(-1 * theta * path_tpye_3_minTime) / sumExp;
			}
			if (path_tpye_4_minTime != 9999) {
				TpassengerShortestTime += path_tpye_4_minTime * (sumP - nunCountp) * exp(-1 * theta * path_tpye_4_minTime) / sumExp;
				//testShort += path_tpye_4_minTime * (sumP - nunCountp) * exp(-1 * theta * path_tpye_4_minTime) / sumExp;
			}
			if (path_tpye_5_minTime != 9999) {
				TpassengerShortestTime += path_tpye_5_minTime * (sumP - nunCountp) * exp(-1 * theta * path_tpye_5_minTime) / sumExp;
				//testShort += path_tpye_5_minTime * (sumP - nunCountp) * exp(-1 * theta * path_tpye_5_minTime) / sumExp;
			}
			if (path_tpye_6_minTime != 9999) {
				TpassengerShortestTime += path_tpye_6_minTime * (sumP - nunCountp) * exp(-1 * theta * path_tpye_6_minTime) / sumExp;
				//testShort += path_tpye_6_minTime * (sumP - nunCountp) * exp(-1 * theta * path_tpye_6_minTime) / sumExp;
			}

			TpassengerShortestTime += nunCountpTime;
			//testShort += nunCountpTime;

			sumExp = 0;


			path_tpye_1_minTime = 9999;
			path_tpye_2_minTime = 9999;
			path_tpye_3_minTime = 9999;
			path_tpye_4_minTime = 9999;
			path_tpye_5_minTime = 9999;
			path_tpye_6_minTime = 9999;

			//std::cout << i << ":" << testSump - testShort << " ";

		}
		tmNum = 0;
		//std::cout << "\n";
		/*if(t==0)
			std::cout << t <<":" << (TpassengerTravelTime - TpassengerShortestTime) / TpassengerShortestTime <<"    ";*/

			//std::cout <<"{" << t << ":" << abs(TpassengerTravelTime - TpassengerShortestTime) / TpassengerShortestTime << "} ";

		gap += abs(TpassengerTravelTime - TpassengerShortestTime);
		/*passengerTravelTime += TpassengerTravelTime;*/
		passengerShortestTime += TpassengerShortestTime;

		sumPassengerCost += TpassengerTravelTime;

	}
	//std::cout << "dispatchCost" << ":" << ctm->dispatchCost << ",";
	std::cout << "passengerCost" << ":" << sumPassengerCost << ",";
	//std::cout << "\n";
	//return (passengerTravelTime - passengerShortestTime) / passengerShortestTime;
	return gap / passengerShortestTime;
}

/*double calRgap(vector<vector<double>> pathPassenger, vector<vector<double>> pathTravelTime, vector<passengerOD*> passengerGroups, int T, TransitNetworkLoading* ntl, int typeC = 0) {//计算gap
	double path_tpye_1_minTime = 9999;
	double path_tpye_2_minTime = 9999;
	double path_tpye_3_minTime = 9999;
	double path_tpye_4_minTime = 9999;
	double path_tpye_5_minTime = 9999;
	double path_tpye_6_minTime = 9999;


	double passengerTravelTime = 0;
	double passengerShortestTime = 0;

	double TpassengerTravelTime = 0;
	double TpassengerShortestTime = 0;

	double sumPassengerCost = 0;

	int pathType = 0;
	int tmNum = 0;
	double sumExp = 0;
	double theta = 0.5;
	double gap = 0;
	//std::cout << "t_gap";
	for (int t = 0; t < T;t++) {

		TpassengerTravelTime = 0;
		TpassengerShortestTime = 0;

		for (int i = 0; i < passengerGroups.size(); i++) {

			double sumP = 0;
			double nunCountp = 0;
			double nunCountpTime = 0;

			//double testSump = 0;
			//double testShort = 0;

			for (int j = 0; j < passengerGroups[i]->paths.size(); j++) {
								
				pathType = passengerGroups[i]->paths[j].type;
				int pathBikeDock1 = passengerGroups[i]->paths[j].outShareBicyclePoint1;
				int pathBikeDock2 = passengerGroups[i]->paths[j].outShareBicyclePoint2;
				
				TpassengerTravelTime += pathTravelTime[t][tmNum] * pathPassenger[t][tmNum];
				//testSump += pathTravelTime[t][tmNum] * pathPassenger[t][tmNum];

				sumP += pathPassenger[t][tmNum];

				if (pathType == 1 ) {
					if (typeC == 1){
						if (path_tpye_1_minTime > pathTravelTime[t][tmNum]) {
							path_tpye_1_minTime = pathTravelTime[t][tmNum];
						}
					}
					else {					
						if (pathBikeDock1 != -1) {
							if (ntl->dockCStar[t][ntl->getShareBicycleSeq(pathBikeDock1)] > 0.00001) {
								if (pathBikeDock2 != -1) {
									if (ntl->dockCStar[t][ntl->getShareBicycleSeq(pathBikeDock2)] > 0.00001) {
										if (path_tpye_1_minTime > pathTravelTime[t][tmNum]) {
											path_tpye_1_minTime = pathTravelTime[t][tmNum];
										}
									}
									else {
										nunCountp += pathPassenger[t][tmNum];
										nunCountpTime += pathTravelTime[t][tmNum] * pathPassenger[t][tmNum];
									}
								}
								else {
									if (path_tpye_1_minTime > pathTravelTime[t][tmNum]) {
										path_tpye_1_minTime = pathTravelTime[t][tmNum];
									}
								}
							}
							else {
								nunCountp += pathPassenger[t][tmNum];
								nunCountpTime += pathTravelTime[t][tmNum] * pathPassenger[t][tmNum];
							}

						}
					}							
				}
				else if (pathType == 2) {
					if (path_tpye_2_minTime > pathTravelTime[t][tmNum]) {
						path_tpye_2_minTime = pathTravelTime[t][tmNum];
					}
				}
				else if (pathType == 3) {

					if (typeC == 1) {
						if (path_tpye_3_minTime > pathTravelTime[t][tmNum]) {
							path_tpye_3_minTime = pathTravelTime[t][tmNum];
						}
					}
					else {
						if (pathBikeDock1 != -1) {
							if (ntl->dockCStar[t][ntl->getShareBicycleSeq(pathBikeDock1)] > 0.00001) {
								if (pathBikeDock2 != -1) {
									if (ntl->dockCStar[t][ntl->getShareBicycleSeq(pathBikeDock2)] > 0.00001) {
										if (path_tpye_3_minTime > pathTravelTime[t][tmNum]) {
											path_tpye_3_minTime = pathTravelTime[t][tmNum];
										}
									}
									else {
										nunCountp += pathPassenger[t][tmNum];
										nunCountpTime += pathTravelTime[t][tmNum] * pathPassenger[t][tmNum];
									}
								}
								else {
									if (path_tpye_3_minTime > pathTravelTime[t][tmNum]) {
										path_tpye_3_minTime = pathTravelTime[t][tmNum];
									}
								}
							}
							else {
								nunCountp += pathPassenger[t][tmNum];
								nunCountpTime += pathTravelTime[t][tmNum] * pathPassenger[t][tmNum];
							}

						}
					}
				}
				else if (pathType == 4) {
					if (path_tpye_4_minTime > pathTravelTime[t][tmNum]) {
						path_tpye_4_minTime = pathTravelTime[t][tmNum];
					}
				}
				else if (pathType == 5) {
					if (typeC == 1) {
						if (pathBikeDock1 != -1) {
							if (ntl->dockCStar[t][ntl->getShareBicycleSeq(pathBikeDock1)] > 0.00001) {
								if (pathBikeDock2 != -1) {
									if (ntl->dockCStar[t][ntl->getShareBicycleSeq(pathBikeDock2)] > 0.00001) {
										if (path_tpye_5_minTime > pathTravelTime[t][tmNum]) {
											path_tpye_5_minTime = pathTravelTime[t][tmNum];
										}
									}
									else {
										nunCountp += pathPassenger[t][tmNum];
										nunCountpTime += pathTravelTime[t][tmNum] * pathPassenger[t][tmNum];
									}
								}
								else {
									if (path_tpye_5_minTime > pathTravelTime[t][tmNum]) {
										path_tpye_5_minTime = pathTravelTime[t][tmNum];
									}
								}
							}
							else {
								nunCountp += pathPassenger[t][tmNum];
								nunCountpTime += pathTravelTime[t][tmNum] * pathPassenger[t][tmNum];
							}

						}
					}
					else {
						if (path_tpye_5_minTime > pathTravelTime[t][tmNum]) {
							path_tpye_5_minTime = pathTravelTime[t][tmNum];
						}
					}
				}
				else if (pathType == 6) {
					if (path_tpye_6_minTime > pathTravelTime[t][tmNum]) {
						path_tpye_6_minTime = pathTravelTime[t][tmNum];
					}
				}

				tmNum++;
			}

			if (path_tpye_1_minTime != 9999) {
				sumExp += std::exp(-1 * theta * path_tpye_1_minTime);
			}
			if (path_tpye_2_minTime != 9999) {
				sumExp += std::exp(-1 * theta * path_tpye_2_minTime);
			}
			if (path_tpye_3_minTime != 9999) {
				sumExp += std::exp(-1 * theta * path_tpye_3_minTime);
			}
			if (path_tpye_4_minTime != 9999) {
				sumExp += std::exp(-1 * theta * path_tpye_4_minTime);
			}
			if (path_tpye_5_minTime != 9999) {
				sumExp += std::exp(-1 * theta * path_tpye_5_minTime);
			}
			if (path_tpye_6_minTime != 9999) {
				sumExp += std::exp(-1 * theta * path_tpye_6_minTime);
			}

			if (path_tpye_1_minTime != 9999) {
				TpassengerShortestTime += path_tpye_1_minTime * (sumP- nunCountp) * exp(-1 * theta * path_tpye_1_minTime) / sumExp;
				//testShort += path_tpye_1_minTime * (sumP - nunCountp) * exp(-1 * theta * path_tpye_1_minTime) / sumExp;
			}
			if (path_tpye_2_minTime != 9999) {
				TpassengerShortestTime += path_tpye_2_minTime * (sumP - nunCountp) * exp(-1 * theta * path_tpye_2_minTime) / sumExp;
				//testShort += path_tpye_2_minTime * (sumP - nunCountp) * exp(-1 * theta * path_tpye_2_minTime) / sumExp;
			}
			if (path_tpye_3_minTime != 9999) {
				TpassengerShortestTime += path_tpye_3_minTime * (sumP - nunCountp) * exp(-1 * theta * path_tpye_3_minTime) / sumExp;
				//testShort += path_tpye_3_minTime * (sumP - nunCountp) * exp(-1 * theta * path_tpye_3_minTime) / sumExp;
			}
			if (path_tpye_4_minTime != 9999) {
				TpassengerShortestTime += path_tpye_4_minTime * (sumP - nunCountp) * exp(-1 * theta * path_tpye_4_minTime) / sumExp;
				//testShort += path_tpye_4_minTime * (sumP - nunCountp) * exp(-1 * theta * path_tpye_4_minTime) / sumExp;
			}
			if (path_tpye_5_minTime != 9999) {
				TpassengerShortestTime += path_tpye_5_minTime * (sumP - nunCountp) * exp(-1 * theta * path_tpye_5_minTime) / sumExp;
				//testShort += path_tpye_5_minTime * (sumP - nunCountp) * exp(-1 * theta * path_tpye_5_minTime) / sumExp;
			}
			if (path_tpye_6_minTime != 9999) {
				TpassengerShortestTime += path_tpye_6_minTime * (sumP - nunCountp) * exp(-1 * theta * path_tpye_6_minTime) / sumExp;
				//testShort += path_tpye_6_minTime * (sumP - nunCountp) * exp(-1 * theta * path_tpye_6_minTime) / sumExp;
			}
			
			TpassengerShortestTime += nunCountpTime;
			//testShort += nunCountpTime;

			sumExp = 0;


			path_tpye_1_minTime = 9999;
			path_tpye_2_minTime = 9999;
			path_tpye_3_minTime = 9999;
			path_tpye_4_minTime = 9999;
			path_tpye_5_minTime = 9999;
			path_tpye_6_minTime = 9999;

			//std::cout << i << ":" << testSump - testShort << " ";

		}
		tmNum = 0;
		//std::cout << "\n";
		//if(t==0)
		//	std::cout << t <<":" << (TpassengerTravelTime - TpassengerShortestTime) / TpassengerShortestTime <<"    ";

		//std::cout <<"{" << t << ":" << abs(TpassengerTravelTime - TpassengerShortestTime) / TpassengerShortestTime << "} ";

		gap += abs(TpassengerTravelTime - TpassengerShortestTime);
		//passengerTravelTime += TpassengerTravelTime;
		passengerShortestTime += TpassengerShortestTime;

		sumPassengerCost += TpassengerTravelTime;

	}
	//std::cout << "dispatchCost" << ":" << ctm->dispatchCost << ",";
	std::cout << "passengerCost" << ":" << sumPassengerCost << ",";
	//std::cout << "\n";
	//return (passengerTravelTime - passengerShortestTime) / passengerShortestTime;
	return gap/ passengerShortestTime;
}*/

std::vector<double> myQserver(double* arrayF,std::vector<vector<double>> A1, std::vector<vector<double>> A2,
	 std::vector<double> B1, std::vector<double> B2){

	int Xn = A1[0].size();

	int elementCntH = Xn* Xn;
	double* arrayH = new double[elementCntH] { 1, -1, -1, 2 };
	int tmpN = 0;
	for (int i = 0; i < Xn;i++) {
		for (int j = 0; j < Xn; j++) {
			if (i==j) {
				arrayH[tmpN] = 2;
			}
			else {
				arrayH[tmpN] = 0;
			}
			tmpN++;
		}
	}
	mwArray H(Xn, Xn, mxDOUBLE_CLASS);
	H.SetData(arrayH, elementCntH);


	int elementCntF = Xn;
	mwArray F(Xn, 1, mxDOUBLE_CLASS);
	F.SetData(arrayF, elementCntF);

	int rowA = A2.size();
	int colA = A2[0].size();

	int elementCntA = rowA* colA;
	double* arrayA = new double[elementCntA];
	int countA = 0;
	for (int i = 0; i < colA;i++) {
		for (int j = 0; j < rowA; j++) {
			arrayA[countA] = -1*A2[j][i];
			countA++;
		}
	}
	mwArray A(rowA, colA, mxDOUBLE_CLASS);
	A.SetData(arrayA, elementCntA);

	
	int elementCntB = B2.size();
	double* arrayB = new double[elementCntB];
	for (int i = 0; i < elementCntB;i++) {
		arrayB[i] = -1 * B2[i];
	}
	mwArray B(elementCntB, 1, mxDOUBLE_CLASS);
	B.SetData(arrayB, elementCntB);


	int rowAEQ = A1.size();
	int colAEQ = A1[0].size();

	int elementCntAEQ = rowAEQ * colAEQ;
	double* arrayAEQ = new double[elementCntAEQ];
	int countAEQ = 0;
	for (int i = 0; i < colAEQ; i++) {
		for (int j = 0; j < rowAEQ; j++) {
			arrayAEQ[countAEQ] = A1[j][i];
			countAEQ++;
		}
	}
	mwArray AEQ(rowAEQ, colAEQ, mxDOUBLE_CLASS);
	AEQ.SetData(arrayAEQ, elementCntAEQ);

	int elementCntBEQ = B1.size();
	double* arrayBEQ = new double[elementCntBEQ];
	for (int i = 0; i < elementCntBEQ; i++) {
		arrayBEQ[i] = B1[i];
	}
	mwArray BEQ(elementCntBEQ, 1, mxDOUBLE_CLASS);
	BEQ.SetData(arrayBEQ, elementCntBEQ);

	int elementCntLB = Xn;
	double* arrayLB = new double[elementCntLB];
	for (int i = 0; i < Xn;i++) {
		arrayLB[i] = 0;
	}
	mwArray LB(Xn, 1, mxDOUBLE_CLASS);
	LB.SetData(arrayLB, elementCntLB);

	mwArray UB;	
	mwArray X;
	mwArray Y;

	myquadprog(2, X, Y, H, F, A, B, AEQ, BEQ, LB, UB);
	
	std::vector<double> x_n;
	for (int i = 0; i < Xn;i++) {
		x_n.push_back(X.Get(1,i+1));
	}
	return x_n;
}

int main() {
	//测试 ********************************
	myquadprogInitialize();

	bool containsDPC = true;

	clock_t start, end;//定义clock_t变量
	start = clock();//开始时间

	//cout << "result is " << Y.Get(1,1) << endl;

	////关闭myadd.lib
	//myquadprogTerminate();
	////关闭mcr
	//mclTerminateApplication();


	//测试
	
	TransitNetworkLoading* ntl = new TransitNetworkLoading();
	//CTM* ctm = new CTM();
	std::ofstream outFileS;
	outFileS.open("pathTravelTime.csv");
	std::ofstream outFilePS;
	outFilePS.open("pathPassenger.csv");
	std::ofstream outFilePLink;
	outFilePLink.open("passengerPath.csv");
	std::ofstream diedai;
	diedai.open("diedai.csv");

	std::ofstream modePassenger;
	modePassenger.open("modePassenger.csv");

	std::ofstream odModePassenger;
	odModePassenger.open("odModePassenger.csv");

	
	/*
	pathType:
		1  share bicycle
		2  bus
		3  subway-bicycle
		4  subway-bus
		5  bicycle-bus
		6  subway
	*/
	double tmDs = 0;
	double theta = 0.5;
	//double epsilonL = 0.0005;//大循环 的 收敛标准
	double epsilonL = 0.001;//大循环 的 收敛标准
	double epsilon = 0.0005;//二次规划的 收敛标准
	double tau = 0.3;//正则化权重	
	int L = 100;//循环数

	static const int T = 30;//时段
	static const int TYPE = 6;//类别数
	
	static const int bicyclePointNum = 19;//共享单车停靠点数

	int path_tpye_1 = 1;//1  share bicycle
	int path_tpye_2 = 2;//2  bus
	int path_tpye_3 = 3;//3  subway-bicycle
	int path_tpye_4 = 4;//4  subway-bus
	int path_tpye_5 = 5;//5  bicycle-bus
	int path_tpye_6 = 6;//6  subway
	std::vector<passengerOD*> passengerGroups;
	std::vector<std::vector<std::string>> allPassegnerPath;

	//插入OD*********************************************************************
	int ptNo = 0;
	//插入乘客OD****************************************************
	passengerOD* od1 = new passengerOD(ptNo, 0, 1);
		
	double pd1[T];
	pd1[0] = 0;
	pd1[1] = 0;
	pd1[2] = 0;
	pd1[3] = 0;
	pd1[4] = 0;
	pd1[5] = 1;
	pd1[6] = 1;
	pd1[7] = 1;
	pd1[8] = 2;
	pd1[9] = 2;
	pd1[10] = 2;
	pd1[11] = 2;
	pd1[12] = 5;
	pd1[13] = 5;
	pd1[14] = 5;
	pd1[15] = 5;
	pd1[16] = 5;
	pd1[17] = 2;
	pd1[18] = 2;
	pd1[19] = 2;
	pd1[20] = 2;
	pd1[21] = 2;
	pd1[22] = 1;
	pd1[23] = 1;
	pd1[24] = 1;
	pd1[25] = 1;
	pd1[26] = 1;
	pd1[27] = 1;
	pd1[28] = 1;
	pd1[29] = 1;
	od1->setPassengerFlow(pd1);

	vector<int> path11;
	path11.push_back(419);
	path11.push_back(22);
	path11.push_back(454);
	od1->addPassengerPath(0, 2, path11);
	passengerGroups.push_back(od1);

	std::vector<std::string> ph1;
	ph1.push_back("419|22|454|");
	allPassegnerPath.push_back(ph1);
	ptNo++;
	//*****************************************************************************
	passengerOD* od2 = new passengerOD(ptNo, 4, 1);
	
	double pd2[T];
	pd2[0] = 5;
	pd2[1] = 5;
	pd2[2] = 5;
	pd2[3] = 6;
	pd2[4] = 6;
	pd2[5] = 6;
	pd2[6] = 6;
	pd2[7] = 12;
	pd2[8] = 12;
	pd2[9] = 12;
	pd2[10] = 12;
	pd2[11] = 20;
	pd2[12] = 30;
	pd2[13] = 30;
	pd2[14] = 15;
	pd2[15] = 15;
	pd2[16] = 15;
	pd2[17] = 12;
	pd2[18] = 12;
	pd2[19] = 12;
	pd2[20] = 12;
	pd2[21] = 5;
	pd2[22] = 5;
	pd2[23] = 5;
	pd2[24] = 5;
	pd2[25] = 5;
	pd2[26] = 5;
	pd2[27] = 5;
	pd2[28] = 5;
	pd2[29] = 1;
	od2->setPassengerFlow(pd2);

	//4-59-58-57-56-55-1
	vector<int> path21;
	path21.push_back(519);
	path21.push_back(17);
	path21.push_back(18);
	path21.push_back(19);
	path21.push_back(20);
	path21.push_back(455);
	od2->addPassengerPath(0, 2, path21);
	passengerGroups.push_back(od2);

	std::vector<std::string> ph2;
	ph2.push_back("519|17|18|19|20|455|");
	allPassegnerPath.push_back(ph2);
	ptNo++;
	//*****************************************************************************

	passengerOD* od3 = new passengerOD(ptNo, 5, 1);

	double pd3[T];
	pd3[0] = 0;
	pd3[1] = 0;
	pd3[2] = 0;
	pd3[3] = 0;
	pd3[4] = 0;
	pd3[5] = 1;
	pd3[6] = 1;
	pd3[7] = 1;
	pd3[8] = 2;
	pd3[9] = 2;
	pd3[10] = 2;
	pd3[11] = 4;
	pd3[12] = 5;
	pd3[13] = 5;
	pd3[14] = 5;
	pd3[15] = 5;
	pd3[16] = 3;
	pd3[17] = 3;
	pd3[18] = 2;
	pd3[19] = 2;
	pd3[20] = 2;
	pd3[21] = 2;
	pd3[22] = 1;
	pd3[23] = 1;
	pd3[24] = 1;
	pd3[25] = 1;
	pd3[26] = 1;
	pd3[27] = 1;
	pd3[28] = 0;
	pd3[29] = 0;
	od3->setPassengerFlow(pd3);

	//5-58-57-56-55-1
	vector<int> path31;
	path31.push_back(540);
	path31.push_back(18);
	path31.push_back(19);
	path31.push_back(20);
	path31.push_back(455);
	od3->addPassengerPath(0, 2, path31);
	passengerGroups.push_back(od3);

	std::vector<std::string> ph3;
	ph3.push_back("540|18|19|20|455|");
	allPassegnerPath.push_back(ph3);
	ptNo++;
	//*****************************************************************************

	passengerOD* od4 = new passengerOD(ptNo, 9, 1);

	double pd4[T];
	pd4[0] = 0;
	pd4[1] = 0;
	pd4[2] = 0;
	pd4[3] = 0;
	pd4[4] = 0;
	pd4[5] = 2;
	pd4[6] = 2;
	pd4[7] = 2;
	pd4[8] = 4;
	pd4[9] = 4;
	pd4[10] = 4;
	pd4[11] = 8;
	pd4[12] = 10;
	pd4[13] = 10;
	pd4[14] = 10;
	pd4[15] = 10;
	pd4[16] = 6;
	pd4[17] = 6;
	pd4[18] = 4;
	pd4[19] = 4;
	pd4[20] = 2;
	pd4[21] = 2;
	pd4[22] = 2;
	pd4[23] = 2;
	pd4[24] = 1;
	pd4[25] = 1;
	pd4[26] = 1;
	pd4[27] = 1;
	pd4[28] = 0;
	pd4[29] = 0;
	od4->setPassengerFlow(pd4);

	//9-63-62-61-1
	vector<int> path41;
	path41.push_back(607);
	path41.push_back(33);
	path41.push_back(34);
	path41.push_back(454);
	od4->addPassengerPath(0, 2, path41);
	passengerGroups.push_back(od4);

	std::vector<std::string> ph4;
	ph4.push_back("607|33|34|454|");
	allPassegnerPath.push_back(ph4);
	ptNo++;
	//*****************************************************************************

	passengerOD* od5 = new passengerOD(ptNo, 13, 1);

	double pd5[T];
	pd5[0] = 0;
	pd5[1] = 0;
	pd5[2] = 0;
	pd5[3] = 0;
	pd5[4] = 0;
	pd5[5] = 2;
	pd5[6] = 2;
	pd5[7] = 2;
	pd5[8] = 4;
	pd5[9] = 4;
	pd5[10] = 4;
	pd5[11] = 8;
	pd5[12] = 10;
	pd5[13] = 10;
	pd5[14] = 10;
	pd5[15] = 10;
	pd5[16] = 6;
	pd5[17] = 6;
	pd5[18] = 4;
	pd5[19] = 4;
	pd5[20] = 2;
	pd5[21] = 2;
	pd5[22] = 2;
	pd5[23] = 2;
	pd5[24] = 1;
	pd5[25] = 1;
	pd5[26] = 1;
	pd5[27] = 1;
	pd5[28] = 0;
	pd5[29] = 0;
	od5->setPassengerFlow(pd5);

	//13-65-64-63-62-61-1
	vector<int> path51;
	path51.push_back(653);
	path51.push_back(31);
	path51.push_back(32);
	path51.push_back(33);
	path51.push_back(34);
	path51.push_back(454);
	od5->addPassengerPath(0, 2, path51);
	passengerGroups.push_back(od5);

	std::vector<std::string> ph5;
	ph5.push_back("653|31|32|33|34|454|");
	allPassegnerPath.push_back(ph5);
	ptNo++;
	//*****************************************************************************

	passengerOD* od6 = new passengerOD(ptNo, 0, 2);

	double pd6[T];
	pd6[0] = 5;
	pd6[1] = 5;
	pd6[2] = 5;
	pd6[3] = 6;
	pd6[4] = 6;
	pd6[5] = 6;
	pd6[6] = 6;
	pd6[7] = 12;
	pd6[8] = 12;
	pd6[9] = 12;
	pd6[10] = 12;
	pd6[11] = 15;
	pd6[12] = 20;
	pd6[13] = 20;
	pd6[14] = 20;
	pd6[15] = 20;
	pd6[16] = 15;
	pd6[17] = 12;
	pd6[18] = 12;
	pd6[19] = 12;
	pd6[20] = 12;
	pd6[21] = 5;
	pd6[22] = 5;
	pd6[23] = 5;
	pd6[24] = 5;
	pd6[25] = 5;
	pd6[26] = 5;
	pd6[27] = 5;
	pd6[28] = 5;
	pd6[29] = 1;
	od6->setPassengerFlow(pd6);

	//0-60-61-62-2
	vector<int> path61;
	path61.push_back(419);
	path61.push_back(22);
	path61.push_back(23);
	path61.push_back(485);
	od6->addPassengerPath(0, 2, path61);
	passengerGroups.push_back(od6);

	std::vector<std::string> ph6;
	ph6.push_back("419|22|23|485|");
	allPassegnerPath.push_back(ph6);
	ptNo++;
	//*****************************************************************************

	passengerOD* od7 = new passengerOD(ptNo, 1, 2);

	double pd7[T];
	pd7[0] = 5;
	pd7[1] = 5;
	pd7[2] = 5;
	pd7[3] = 6;
	pd7[4] = 10;
	pd7[5] = 10;
	pd7[6] = 12;
	pd7[7] = 12;
	pd7[8] = 12;
	pd7[9] = 12;
	pd7[10] = 12;
	pd7[11] = 15;
	pd7[12] = 20;
	pd7[13] = 30;
	pd7[14] = 30;
	pd7[15] = 20;
	pd7[16] = 20;
	pd7[17] = 15;
	pd7[18] = 12;
	pd7[19] = 12;
	pd7[20] = 12;
	pd7[21] = 10;
	pd7[22] = 10;
	pd7[23] = 10;
	pd7[24] = 10;
	pd7[25] = 10;
	pd7[26] = 5;
	pd7[27] = 5;
	pd7[28] = 5;
	pd7[29] = 1;
	od7->setPassengerFlow(pd7);

	//1-61-62-2
	vector<int> path71;
	path71.push_back(439);
	path71.push_back(23);
	path71.push_back(485);
	od7->addPassengerPath(0, 2, path71);
	passengerGroups.push_back(od7);

	std::vector<std::string> ph7;
	ph7.push_back("439|23|485|");
	allPassegnerPath.push_back(ph7);
	ptNo++;
	//*****************************************************************************

	passengerOD* od8 = new passengerOD(ptNo, 4, 2);

	double pd8[T];
	pd8[0] = 0;
	pd8[1] = 0;
	pd8[2] = 0;
	pd8[3] = 0;
	pd8[4] = 0;
	pd8[5] = 2;
	pd8[6] = 2;
	pd8[7] = 2;
	pd8[8] = 4;
	pd8[9] = 4;
	pd8[10] = 4;
	pd8[11] = 8;
	pd8[12] = 10;
	pd8[13] = 10;
	pd8[14] = 10;
	pd8[15] = 10;
	pd8[16] = 6;
	pd8[17] = 6;
	pd8[18] = 4;
	pd8[19] = 4;
	pd8[20] = 2;
	pd8[21] = 2;
	pd8[22] = 2;
	pd8[23] = 2;
	pd8[24] = 1;
	pd8[25] = 1;
	pd8[26] = 1;
	pd8[27] = 1;
	pd8[28] = 0;
	pd8[29] = 0;
	od8->setPassengerFlow(pd8);

	//4-59-58-57-56-2
	vector<int> path81;
	path81.push_back(519);
	path81.push_back(17);
	path81.push_back(18);
	path81.push_back(19);
	path81.push_back(486);
	od8->addPassengerPath(0, 2, path81);
	passengerGroups.push_back(od8);

	std::vector<std::string> ph8;
	ph8.push_back("519|17|18|19|486|");
	allPassegnerPath.push_back(ph8);
	ptNo++;
	//*****************************************************************************

	passengerOD* od9 = new passengerOD(ptNo, 12, 2);

	double pd9[T];
	pd9[0] = 0;
	pd9[1] = 0;
	pd9[2] = 0;
	pd9[3] = 0;
	pd9[4] = 0;
	pd9[5] = 1;
	pd9[6] = 1;
	pd9[7] = 1;
	pd9[8] = 2;
	pd9[9] = 2;
	pd9[10] = 2;
	pd9[11] = 2;
	pd9[12] = 5;
	pd9[13] = 5;
	pd9[14] = 5;
	pd9[15] = 5;
	pd9[16] = 5;
	pd9[17] = 2;
	pd9[18] = 2;
	pd9[19] = 2;
	pd9[20] = 2;
	pd9[21] = 2;
	pd9[22] = 1;
	pd9[23] = 1;
	pd9[24] = 1;
	pd9[25] = 1;
	pd9[26] = 1;
	pd9[27] = 1;
	pd9[28] = 1;
	pd9[29] = 1;
	od9->setPassengerFlow(pd9);

	//12-49-48-47-51-52-2
	vector<int> path91;
	path91.push_back(645);
	path91.push_back(3);
	path91.push_back(4);
	path91.push_back(467);
	path91.push_back(7);
	path91.push_back(487);
	od9->addPassengerPath(0, 2, path91);
	passengerGroups.push_back(od9);

	std::vector<std::string> ph9;
	ph9.push_back("645|3|4|467|7|487|");
	allPassegnerPath.push_back(ph9);
	ptNo++;
	//*****************************************************************************

	passengerOD* od10 = new passengerOD(ptNo, 4, 3);

	double pd10[T];
	pd10[0] = 0;
	pd10[1] = 0;
	pd10[2] = 0;
	pd10[3] = 1;
	pd10[4] = 1;
	pd10[5] = 2;
	pd10[6] = 2;
	pd10[7] = 2;
	pd10[8] = 2;
	pd10[9] = 2;
	pd10[10] = 4;
	pd10[11] = 4;
	pd10[12] = 5;
	pd10[13] = 5;
	pd10[14] = 10;
	pd10[15] = 10;
	pd10[16] = 5;
	pd10[17] = 5;
	pd10[18] = 5;
	pd10[19] = 2;
	pd10[20] = 2;
	pd10[21] = 2;
	pd10[22] = 2;
	pd10[23] = 2;
	pd10[24] = 1;
	pd10[25] = 1;
	pd10[26] = 1;
	pd10[27] = 1;
	pd10[28] = 1;
	pd10[29] = 1;
	od10->setPassengerFlow(pd10);

	//4-59-58-57-3
	vector<int> path101;
	path101.push_back(519);
	path101.push_back(17);
	path101.push_back(18);
	path101.push_back(510);
	od10->addPassengerPath(0, 2, path101);
	passengerGroups.push_back(od10);

	std::vector<std::string> ph10;
	ph10.push_back("519|17|18|510|");
	allPassegnerPath.push_back(ph10);
	ptNo++;
	//*****************************************************************************

	passengerOD* od11 = new passengerOD(ptNo, 13, 3);

	double pd11[T];
	pd11[0] = 0;
	pd11[1] = 0;
	pd11[2] = 0;
	pd11[3] = 1;
	pd11[4] = 1;
	pd11[5] = 1;
	pd11[6] = 2;
	pd11[7] = 2;
	pd11[8] = 2;
	pd11[9] = 2;
	pd11[10] = 3;
	pd11[11] = 3;
	pd11[12] = 5;
	pd11[13] = 5;
	pd11[14] = 8;
	pd11[15] = 8;
	pd11[16] = 5;
	pd11[17] = 5;
	pd11[18] = 3;
	pd11[19] = 3;
	pd11[20] = 2;
	pd11[21] = 2;
	pd11[22] = 1;
	pd11[23] = 1;
	pd11[24] = 1;
	pd11[25] = 1;
	pd11[26] = 1;
	pd11[27] = 0;
	pd11[28] = 0;
	pd11[29] = 0;
	od11->setPassengerFlow(pd11);

	//13-43-42-41-40-39-3
	vector<int> path111;
	path111.push_back(651);
	path111.push_back(71);
	path111.push_back(72);
	path111.push_back(73);
	path111.push_back(74);
	path111.push_back(509);
	od11->addPassengerPath(0, 6, path111);
	passengerGroups.push_back(od11);

	std::vector<std::string> ph11;
	ph11.push_back("651|71|72|73|74|509|");
	allPassegnerPath.push_back(ph11);
	ptNo++;
	//*****************************************************************************

	passengerOD* od12 = new passengerOD(ptNo, 1, 4);

	double pd12[T];
	pd12[0] = 1;
	pd12[1] = 1;
	pd12[2] = 1;
	pd12[3] = 2;
	pd12[4] = 2;
	pd12[5] = 2;
	pd12[6] = 4;
	pd12[7] = 4;
	pd12[8] = 4;
	pd12[9] = 4;
	pd12[10] = 6;
	pd12[11] = 6;
	pd12[12] = 12;
	pd12[13] = 12;
	pd12[14] = 20;
	pd12[15] = 20;
	pd12[16] = 12;
	pd12[17] = 12;
	pd12[18] = 6;
	pd12[19] = 6;
	pd12[20] = 4;
	pd12[21] = 4;
	pd12[22] = 4;
	pd12[23] = 2;
	pd12[24] = 2;
	pd12[25] = 2;
	pd12[26] = 2;
	pd12[27] = 1;
	pd12[28] = 1;
	pd12[29] = 1;
	od12->setPassengerFlow(pd12);

	//1-55-56-57-58-59-4
	vector<int> path121;
	path121.push_back(440);
	path121.push_back(13);
	path121.push_back(14);
	path121.push_back(15);
	path121.push_back(16);
	path121.push_back(529);
	od12->addPassengerPath(0, 2, path121);
	passengerGroups.push_back(od12);

	std::vector<std::string> ph12;
	ph12.push_back("440|13|14|15|16|529|");
	allPassegnerPath.push_back(ph12);
	ptNo++;
	//*****************************************************************************

	passengerOD* od13 = new passengerOD(ptNo, 2, 4);

	double pd13[T];
	pd13[0] = 1;
	pd13[1] = 1;
	pd13[2] = 2;
	pd13[3] = 2;
	pd13[4] = 3;
	pd13[5] = 3;
	pd13[6] = 4;
	pd13[7] = 4;
	pd13[8] = 6;
	pd13[9] = 6;
	pd13[10] = 8;
	pd13[11] = 8;
	pd13[12] = 15;
	pd13[13] = 15;
	pd13[14] = 23;
	pd13[15] = 23;
	pd13[16] = 15;
	pd13[17] = 15;
	pd13[18] = 8;
	pd13[19] = 8;
	pd13[20] = 6;
	pd13[21] = 6;
	pd13[22] = 4;
	pd13[23] = 4;
	pd13[24] = 4;
	pd13[25] = 2;
	pd13[26] = 2;
	pd13[27] = 2;
	pd13[28] = 1;
	pd13[29] = 1;
	od13->setPassengerFlow(pd13);

	//2-56-57-58-59-4
	vector<int> path131;
	path131.push_back(471);
	path131.push_back(14);
	path131.push_back(15);
	path131.push_back(16);
	path131.push_back(529);
	od13->addPassengerPath(0, 2, path131);
	passengerGroups.push_back(od13);

	std::vector<std::string> ph13;
	ph13.push_back("471|14|15|16|529|");
	allPassegnerPath.push_back(ph13);
	ptNo++;
	//*****************************************************************************

	passengerOD* od14 = new passengerOD(ptNo, 3, 4);

	double pd14[T];
	pd14[0] = 1;
	pd14[1] = 1;
	pd14[2] = 2;
	pd14[3] = 2;
	pd14[4] = 3;
	pd14[5] = 3;
	pd14[6] = 4;
	pd14[7] = 4;
	pd14[8] = 6;
	pd14[9] = 6;
	pd14[10] = 8;
	pd14[11] = 8;
	pd14[12] = 15;
	pd14[13] = 15;
	pd14[14] = 23;
	pd14[15] = 23;
	pd14[16] = 15;
	pd14[17] = 15;
	pd14[18] = 8;
	pd14[19] = 8;
	pd14[20] = 6;
	pd14[21] = 6;
	pd14[22] = 4;
	pd14[23] = 4;
	pd14[24] = 4;
	pd14[25] = 2;
	pd14[26] = 2;
	pd14[27] = 2;
	pd14[28] = 1;
	pd14[29] = 1;
	od14->setPassengerFlow(pd14);

	//3-57-58-59-4
	vector<int> path141;
	path141.push_back(500);
	path141.push_back(15);
	path141.push_back(16);
	path141.push_back(529);
	od14->addPassengerPath(0, 2, path141);
	passengerGroups.push_back(od14);

	std::vector<std::string> ph14;
	ph14.push_back("500|15|16|529|");
	allPassegnerPath.push_back(ph14);
	ptNo++;
	//*****************************************************************************

	passengerOD* od15 = new passengerOD(ptNo, 8, 5);

	double pd15[T];
	pd15[0] = 1;
	pd15[1] = 1;
	pd15[2] = 1;
	pd15[3] = 2;
	pd15[4] = 2;
	pd15[5] = 2;
	pd15[6] = 2;
	pd15[7] = 2;
	pd15[8] = 3;
	pd15[9] = 3;
	pd15[10] = 4;
	pd15[11] = 4;
	pd15[12] = 7;
	pd15[13] = 7;
	pd15[14] = 10;
	pd15[15] = 10;
	pd15[16] = 7;
	pd15[17] = 7;
	pd15[18] = 4;
	pd15[19] = 4;
	pd15[20] = 3;
	pd15[21] = 3;
	pd15[22] = 3;
	pd15[23] = 3;
	pd15[24] = 2;
	pd15[25] = 2;
	pd15[26] = 2;
	pd15[27] = 2;
	pd15[28] = 1;
	pd15[29] = 1;
	od15->setPassengerFlow(pd15);

	//8-54-55-56-57-58-5
	vector<int> path151;
	path151.push_back(595);
	path151.push_back(12);
	path151.push_back(13);
	path151.push_back(14);
	path151.push_back(15);
	path151.push_back(555);
	od15->addPassengerPath(0, 2, path151);
	passengerGroups.push_back(od15);

	std::vector<std::string> ph15;
	ph15.push_back("595|12|13|14|15|555|");
	allPassegnerPath.push_back(ph15);
	ptNo++;
	//*****************************************************************************

	//passengerOD* od16 = new passengerOD(ptNo, 12, 5);

	//double pd16[T];
	//pd16[0] = 0;
	//pd16[1] = 0;
	//pd16[2] = 1;
	//pd16[3] = 1;
	//pd16[4] = 1;
	//pd16[5] = 2;
	//pd16[6] = 2;
	//pd16[7] = 2;
	//pd16[8] = 2;
	//pd16[9] = 3;
	//pd16[10] = 3;
	//pd16[11] = 3;
	//pd16[12] = 8;
	//pd16[13] = 8;
	//pd16[14] = 10;
	//pd16[15] = 10;
	//pd16[16] = 8;
	//pd16[17] = 8;
	//pd16[18] = 4;
	//pd16[19] = 4;
	//pd16[20] = 3;
	//pd16[21] = 3;
	//pd16[22] = 2;
	//pd16[23] = 2;
	//pd16[24] = 2;
	//pd16[25] = 1;
	//pd16[26] = 1;
	//pd16[27] = 1;
	//pd16[28] = 0;
	//pd16[29] = 0;
	//od16->setPassengerFlow(pd16);

	////12-49-48-54-55-56-57-58-5
	//vector<int> path161;
	//path161.push_back(645);
	//path161.push_back(3);
	//path161.push_back(605);
	//path161.push_back(12);
	//path161.push_back(13);
	//path161.push_back(14);
	//path161.push_back(15);
	//path161.push_back(555);
	//od16->addPassengerPath(0, 2, path161);
	//passengerGroups.push_back(od16);

	//std::vector<std::string> ph16;
	//ph16.push_back("645|3|605|12|13|14|15|555|");
	//allPassegnerPath.push_back(ph16);
	//ptNo++;
	//*****************************************************************************

	passengerOD* od17 = new passengerOD(ptNo, 4, 6);

	double pd17[T];
	pd17[0] = 0;
	pd17[1] = 0;
	pd17[2] = 1;
	pd17[3] = 1;
	pd17[4] = 1;
	pd17[5] = 2;
	pd17[6] = 2;
	pd17[7] = 2;
	pd17[8] = 2;
	pd17[9] = 3;
	pd17[10] = 3;
	pd17[11] = 3;
	pd17[12] = 6;
	pd17[13] = 6;
	pd17[14] = 6;
	pd17[15] = 6;
	pd17[16] = 8;
	pd17[17] = 8;
	pd17[18] = 10;
	pd17[19] = 10;
	pd17[20] = 10;
	pd17[21] = 8;
	pd17[22] = 8;
	pd17[23] = 4;
	pd17[24] = 4;
	pd17[25] = 2;
	pd17[26] = 2;
	pd17[27] = 1;
	pd17[28] = 0;
	pd17[29] = 0;
	od17->setPassengerFlow(pd17);

	//4-73-74-6
	vector<int> path171;
	path171.push_back(521);
	path171.push_back(43);
	path171.push_back(581);
	od17->addPassengerPath(0, 2, path171);
	passengerGroups.push_back(od17);

	std::vector<std::string> ph17;
	ph17.push_back("521|43|581|");
	allPassegnerPath.push_back(ph17);
	ptNo++;
	//*****************************************************************************

	passengerOD* od18 = new passengerOD(ptNo, 2, 7);

	double pd18[T];
	pd18[0] = 0;
	pd18[1] = 0;
	pd18[2] = 1;
	pd18[3] = 1;
	pd18[4] = 1;
	pd18[5] = 1;
	pd18[6] = 1;
	pd18[7] = 2;
	pd18[8] = 2;
	pd18[9] = 2;
	pd18[10] = 2;
	pd18[11] = 3;
	pd18[12] = 3;
	pd18[13] = 3;
	pd18[14] = 4;
	pd18[15] = 4;
	pd18[16] = 5;
	pd18[17] = 5;
	pd18[18] = 5;
	pd18[19] = 5;
	pd18[20] = 3;
	pd18[21] = 3;
	pd18[22] = 1;
	pd18[23] = 1;
	pd18[24] = 0;
	pd18[25] = 0;
	pd18[26] = 0;
	pd18[27] = 0;
	pd18[28] = 0;
	pd18[29] = 0;
	od18->setPassengerFlow(pd18);

	//2-56-57-58-59-73-74-75-7
	vector<int> path181;
	path181.push_back(471);
	path181.push_back(14);
	path181.push_back(15);
	path181.push_back(16);
	path181.push_back(526);
	path181.push_back(43);
	path181.push_back(44);
	path181.push_back(592);
	od18->addPassengerPath(0, 2, path181);
	passengerGroups.push_back(od18);

	std::vector<std::string> ph18;
	ph18.push_back("471|14|15|16|526|43|44|592|");
	allPassegnerPath.push_back(ph18);
	ptNo++;
	//*****************************************************************************

	passengerOD* od19 = new passengerOD(ptNo, 1, 9);

	double pd19[T];
	pd19[0] = 0;
	pd19[1] = 0;
	pd19[2] = 1;
	pd19[3] = 1;
	pd19[4] = 1;
	pd19[5] = 2;
	pd19[6] = 2;
	pd19[7] = 4;
	pd19[8] = 8;
	pd19[9] = 10;
	pd19[10] = 10;
	pd19[11] = 10;
	pd19[12] = 8;
	pd19[13] = 8;
	pd19[14] = 6;
	pd19[15] = 6;
	pd19[16] = 6;
	pd19[17] = 6;
	pd19[18] = 5;
	pd19[19] = 5;
	pd19[20] = 4;
	pd19[21] = 4;
	pd19[22] = 4;
	pd19[23] = 4;
	pd19[24] = 2;
	pd19[25] = 1;
	pd19[26] = 0;
	pd19[27] = 0;
	pd19[28] = 0;
	pd19[29] = 0;
	od19->setPassengerFlow(pd19);

	//1-61-62-63-9
	vector<int> path191;
	path191.push_back(439);
	path191.push_back(23);
	path191.push_back(24);
	path191.push_back(610);
	od19->addPassengerPath(0, 2, path191);
	passengerGroups.push_back(od19);

	std::vector<std::string> ph19;
	ph19.push_back("439|23|24|610|");
	allPassegnerPath.push_back(ph19);
	ptNo++;
	//*****************************************************************************

	passengerOD* od20 = new passengerOD(ptNo, 13, 9);

	double pd20[T];
	pd20[0] = 0;
	pd20[1] = 0;
	pd20[2] = 1;
	pd20[3] = 1;
	pd20[4] = 1;
	pd20[5] = 2;
	pd20[6] = 2;
	pd20[7] = 4;
	pd20[8] = 4;
	pd20[9] = 4;
	pd20[10] = 4;
	pd20[11] = 4;
	pd20[12] = 8;
	pd20[13] = 8;
	pd20[14] = 10;
	pd20[15] = 10;
	pd20[16] = 10;
	pd20[17] = 6;
	pd20[18] = 6;
	pd20[19] = 6;
	pd20[20] = 4;
	pd20[21] = 4;
	pd20[22] = 4;
	pd20[23] = 4;
	pd20[24] = 4;
	pd20[25] = 4;
	pd20[26] = 2;
	pd20[27] = 2;
	pd20[28] = 1;
	pd20[29] = 1;
	od20->setPassengerFlow(pd20);

	//13-65-64-63-9
	vector<int> path201;
	path201.push_back(653);
	path201.push_back(31);
	path201.push_back(32);
	path201.push_back(610);
	od20->addPassengerPath(0, 2, path201);
	passengerGroups.push_back(od20);

	std::vector<std::string> ph20;
	ph20.push_back("653|31|32|610|");
	allPassegnerPath.push_back(ph20);
	ptNo++;
	//*****************************************************************************

	passengerOD* od21 = new passengerOD(ptNo, 7, 10);

	double pd21[T];
	pd21[0] = 0;
	pd21[1] = 0;
	pd21[2] = 1;
	pd21[3] = 1;
	pd21[4] = 1;
	pd21[5] = 2;
	pd21[6] = 2;
	pd21[7] = 4;
	pd21[8] = 6;
	pd21[9] = 6;
	pd21[10] = 8;
	pd21[11] = 8;
	pd21[12] = 8;
	pd21[13] = 10;
	pd21[14] = 10;
	pd21[15] = 12;
	pd21[16] = 12;
	pd21[17] = 8;
	pd21[18] = 8;
	pd21[19] = 6;
	pd21[20] = 6;
	pd21[21] = 6;
	pd21[22] = 6;
	pd21[23] = 4;
	pd21[24] = 4;
	pd21[25] = 4;
	pd21[26] = 2;
	pd21[27] = 2;
	pd21[28] = 1;
	pd21[29] = 1;
	od21->setPassengerFlow(pd21);

	//7-75-76-70-69-68-10
	vector<int> path211;
	path211.push_back(589);
	path211.push_back(45);
	path211.push_back(687);
	path211.push_back(40);
	path211.push_back(41);
	path211.push_back(620);
	od21->addPassengerPath(0, 2, path211);
	passengerGroups.push_back(od21);

	std::vector<std::string> ph21;
	ph21.push_back("589|45|687|40|41|620|");
	allPassegnerPath.push_back(ph21);
	ptNo++;
	//*****************************************************************************

	passengerOD* od22 = new passengerOD(ptNo, 13, 10);

	double pd22[T];
	pd22[0] = 0;
	pd22[1] = 0;
	pd22[2] = 1;
	pd22[3] = 1;
	pd22[4] = 1;
	pd22[5] = 2;
	pd22[6] = 2;
	pd22[7] = 2;
	pd22[8] = 2;
	pd22[9] = 2;
	pd22[10] = 4;
	pd22[11] = 4;
	pd22[12] = 4;
	pd22[13] = 6;
	pd22[14] = 6;
	pd22[15] = 6;
	pd22[16] = 6;
	pd22[17] = 3;
	pd22[18] = 3;
	pd22[19] = 3;
	pd22[20] = 2;
	pd22[21] = 2;
	pd22[22] = 2;
	pd22[23] = 1;
	pd22[24] = 1;
	pd22[25] = 1;
	pd22[26] = 0;
	pd22[27] = 0;
	pd22[28] = 0;
	pd22[29] = 0;
	od22->setPassengerFlow(pd22);

	//13-65-64-10
	vector<int> path221;
	path221.push_back(653);
	path221.push_back(31);
	path221.push_back(619);
	od22->addPassengerPath(0, 2, path221);
	passengerGroups.push_back(od22);

	std::vector<std::string> ph22;
	ph22.push_back("653|31|619|");
	allPassegnerPath.push_back(ph22);
	ptNo++;
	//*****************************************************************************

	//passengerOD* od23 = new passengerOD(ptNo, 4, 12);

	//double pd23[T];
	//pd23[0] = 0;
	//pd23[1] = 0;
	//pd23[2] = 1;
	//pd23[3] = 1;
	//pd23[4] = 1;
	//pd23[5] = 2;
	//pd23[6] = 2;
	//pd23[7] = 2;
	//pd23[8] = 2;
	//pd23[9] = 2;
	//pd23[10] = 4;
	//pd23[11] = 4;
	//pd23[12] = 4;
	//pd23[13] = 6;
	//pd23[14] = 6;
	//pd23[15] = 6;
	//pd23[16] = 6;
	//pd23[17] = 3;
	//pd23[18] = 3;
	//pd23[19] = 3;
	//pd23[20] = 2;
	//pd23[21] = 2;
	//pd23[22] = 2;
	//pd23[23] = 1;
	//pd23[24] = 1;
	//pd23[25] = 1;
	//pd23[26] = 0;
	//pd23[27] = 0;
	//pd23[28] = 0;
	//pd23[29] = 0;
	//od23->setPassengerFlow(pd23);

	////4-59-58-57-56-55-54-48-49-12
	//vector<int> path231;
	//path231.push_back(519);
	//path231.push_back(17);
	//path231.push_back(18);
	//path231.push_back(19);
	//path231.push_back(20);
	//path231.push_back(21);
	//path231.push_back(599);
	//path231.push_back(2);
	//path231.push_back(648);
	//od23->addPassengerPath(0, 2, path231);
	//passengerGroups.push_back(od23);

	//std::vector<std::string> ph23;
	//ph23.push_back("519|17|18|19|20|21|599|2|648|");
	//allPassegnerPath.push_back(ph23);
	//ptNo++;
	//*****************************************************************************

	//passengerOD* od24 = new passengerOD(ptNo, 13, 12);

	//double pd24[T];
	//pd24[0] = 0;
	//pd24[1] = 0;
	//pd24[2] = 1;
	//pd24[3] = 1;
	//pd24[4] = 1;
	//pd24[5] = 2;
	//pd24[6] = 2;
	//pd24[7] = 2;
	//pd24[8] = 2;
	//pd24[9] = 2;
	//pd24[10] = 4;
	//pd24[11] = 4;
	//pd24[12] = 4;
	//pd24[13] = 6;
	//pd24[14] = 6;
	//pd24[15] = 6;
	//pd24[16] = 6;
	//pd24[17] = 3;
	//pd24[18] = 3;
	//pd24[19] = 3;
	//pd24[20] = 2;
	//pd24[21] = 2;
	//pd24[22] = 2;
	//pd24[23] = 1;
	//pd24[24] = 1;
	//pd24[25] = 1;
	//pd24[26] = 0;
	//pd24[27] = 0;
	//pd24[28] = 0;
	//pd24[29] = 0;
	//od24->setPassengerFlow(pd24);

	////13-65-64-63-62-61-47-48-49-12
	//vector<int> path241;
	//path241.push_back(653);
	//path241.push_back(31);
	//path241.push_back(32);
	//path241.push_back(33);
	//path241.push_back(34);
	//path241.push_back(449);
	//path241.push_back(1);
	//path241.push_back(2);
	//path241.push_back(648);
	//od24->addPassengerPath(0, 2, path241);
	//passengerGroups.push_back(od24);

	//std::vector<std::string> ph24;
	//ph24.push_back("653|31|32|33|34|449|1|2|648|");
	//allPassegnerPath.push_back(ph24);
	//ptNo++;
	//*****************************************************************************

	/*passengerOD* od25 = new passengerOD(ptNo, 18, 13);

	double pd25[T];
	pd25[0] = 0;
	pd25[1] = 0;
	pd25[2] = 1;
	pd25[3] = 1;
	pd25[4] = 1;
	pd25[5] = 2;
	pd25[6] = 2;
	pd25[7] = 2;
	pd25[8] = 2;
	pd25[9] = 2;
	pd25[10] = 4;
	pd25[11] = 4;
	pd25[12] = 4;
	pd25[13] = 6;
	pd25[14] = 6;
	pd25[15] = 6;
	pd25[16] = 6;
	pd25[17] = 3;
	pd25[18] = 3;
	pd25[19] = 3;
	pd25[20] = 2;
	pd25[21] = 2;
	pd25[22] = 2;
	pd25[23] = 1;
	pd25[24] = 1;
	pd25[25] = 1;
	pd25[26] = 0;
	pd25[27] = 0;
	pd25[28] = 0;
	pd25[29] = 0;
	od25->setPassengerFlow(pd25);

	//18-83-82-81-13
	vector<int> path251;
	path251.push_back(709);
	path251.push_back(56);
	path251.push_back(57);
	path251.push_back(662);
	od25->addPassengerPath(0, 2, path251);
	passengerGroups.push_back(od25);

	std::vector<std::string> ph25;
	ph25.push_back("709|56|57|662|");
	allPassegnerPath.push_back(ph25);
	ptNo++;*/
	//*****************************************************************************

	/*????????????????????????????????????????????passengerOD* od26 = new passengerOD(ptNo, 18, 16);

	double pd26[T];
	pd26[0] = 0;
	pd26[1] = 0;
	pd26[2] = 1;
	pd26[3] = 1;
	pd26[4] = 1;
	pd26[5] = 2;
	pd26[6] = 2;
	pd26[7] = 2;
	pd26[8] = 2;
	pd26[9] = 2;
	pd26[10] = 4;
	pd26[11] = 4;
	pd26[12] = 4;
	pd26[13] = 6;
	pd26[14] = 6;
	pd26[15] = 6;
	pd26[16] = 6;
	pd26[17] = 3;
	pd26[18] = 3;
	pd26[19] = 3;
	pd26[20] = 2;
	pd26[21] = 2;
	pd26[22] = 2;
	pd26[23] = 1;
	pd26[24] = 1;
	pd26[25] = 1;
	pd26[26] = 0;
	pd26[27] = 0;
	pd26[28] = 0;
	pd26[29] = 0;
	od26->setPassengerFlow(pd26);

	//18-83-82-16
	vector<int> path261;
	path261.push_back(709);
	path261.push_back(56);
	path261.push_back(696);
	od26->addPassengerPath(0, 2, path261);
	passengerGroups.push_back(od26);

	std::vector<std::string> ph26;
	ph26.push_back("709|56|696|");
	allPassegnerPath.push_back(ph26);
	ptNo++;*/
	//*****************************************************************************

	passengerOD* od27 = new passengerOD(ptNo, 16, 17);

	double pd27[T];
	pd27[0] = 2;
	pd27[1] = 2;
	pd27[2] = 4;
	pd27[3] = 8;
	pd27[4] = 10;
	pd27[5] = 15;
	pd27[6] = 20;
	pd27[7] = 30;
	pd27[8] = 30;
	pd27[9] = 30;
	pd27[10] = 20;
	pd27[11] = 20;
	pd27[12] = 10;
	pd27[13] = 10;
	pd27[14] = 8;
	pd27[15] = 8;
	pd27[16] = 6;
	pd27[17] = 6;
	pd27[18] = 6;
	pd27[19] = 3;
	pd27[20] = 3;
	pd27[21] = 3;
	pd27[22] = 2;
	pd27[23] = 2;
	pd27[24] = 2;
	pd27[25] = 2;
	pd27[26] = 1;
	pd27[27] = 1;
	pd27[28] = 1;
	pd27[29] = 1;
	od27->setPassengerFlow(pd27);

	//16-82-81-65-66-67-17
	vector<int> path271;
	path271.push_back(690);
	path271.push_back(57);
	path271.push_back(659);
	path271.push_back(27);
	path271.push_back(28);
	path271.push_back(704);
	od27->addPassengerPath(0, 2, path271);
	passengerGroups.push_back(od27);

	std::vector<std::string> ph27;
	ph27.push_back("690|57|659|27|28|704|");
	allPassegnerPath.push_back(ph27);
	ptNo++;

	//*****************************************************************************

	/*passengerOD* od28 = new passengerOD(ptNo, 13, 18);

	double pd28[T];
	pd28[0] = 0;
	pd28[1] = 0;
	pd28[2] = 1;
	pd28[3] = 1;
	pd28[4] = 1;
	pd28[5] = 2;
	pd28[6] = 2;
	pd28[7] = 2;
	pd28[8] = 2;
	pd28[9] = 2;
	pd28[10] = 4;
	pd28[11] = 4;
	pd28[12] = 4;
	pd28[13] = 6;
	pd28[14] = 6;
	pd28[15] = 6;
	pd28[16] = 6;
	pd28[17] = 3;
	pd28[18] = 3;
	pd28[19] = 3;
	pd28[20] = 2;
	pd28[21] = 2;
	pd28[22] = 2;
	pd28[23] = 1;
	pd28[24] = 1;
	pd28[25] = 1;
	pd28[26] = 0;
	pd28[27] = 0;
	pd28[28] = 0;
	pd28[29] = 0;
	od28->setPassengerFlow(pd28);

	//13-81-82-83-18
	vector<int> path281;
	path281.push_back(652);
	path281.push_back(54);
	path281.push_back(55);
	path281.push_back(719);
	od28->addPassengerPath(0, 2, path281);
	passengerGroups.push_back(od28);

	std::vector<std::string> ph28;
	ph28.push_back("652|54|55|719|");
	allPassegnerPath.push_back(ph28);
	ptNo++;*/

	//*****************************************************************************
	static const int groupNum = 22;//用户组数

	//初始化，将乘客分配到最短的道路上去
	//寻找当前时段的最短路，并将乘客分配至最短路中
	for (int t = 0; t < T; t++) {//遍历时段
		for (int passenger_no = 0; passenger_no < groupNum; passenger_no++) {//遍历乘客
			passengerOD* passenger_group = passengerGroups[passenger_no];
			int pathType1Cost = 99999, pathType2Cost = 99999, pathType3Cost = 99999, pathType4Cost = 99999, pathType5Cost = 99999, pathType6Cost = 99999;//控制初始化方法
			int pathType1No = -1, pathType2No = -1, pathType3No = -1, pathType4No = -1, pathType5No = -1, pathType6No = -1;

			vector<double> shareBicycleC(ntl->sharedBikeNode.size());//t时刻共享单车停靠点的容量	
			for (int i = 0; i < ntl->sharedBikeNode.size(); i++) {
				shareBicycleC[i] = ntl->getBicyclePointByBS(i);
			}

			//计算乘客
			for (int path_no = 0; path_no < passenger_group->paths.size(); path_no++) {//遍历所有路径，计算最短路

				passengerOD::passengerPath passenger_path = passenger_group->paths[path_no];

				int path_cost = (int)ntl->getIniPathTime(passenger_path.links);//估计路径时间

				if (passenger_path.type == path_tpye_1) {
					if (path_cost < pathType1Cost) {
						pathType1Cost = path_cost;
						pathType1No = path_no;
					}
				}
				else if (passenger_path.type == path_tpye_2) {
					if (path_cost < pathType2Cost) {
						pathType2Cost = path_cost;
						pathType2No = path_no;
					}
				}
				else if (passenger_path.type == path_tpye_3) {
					if (path_cost < pathType3Cost) {
						pathType3Cost = path_cost;
						pathType3No = path_no;
					}
				}
				else if (passenger_path.type == path_tpye_4) {
					if (path_cost < pathType4Cost) {
						pathType4Cost = path_cost;
						pathType4No = path_no;
					}
				}
				else if (passenger_path.type == path_tpye_5) {
					if (path_cost < pathType5Cost) {
						pathType5Cost = path_cost;
						pathType5No = path_no;
					}
				}
				else if (passenger_path.type == path_tpye_6) {
					if (path_cost < pathType6Cost) {
						pathType6Cost = path_cost;
						pathType6No = path_no;
					}
				}
			}

			//分配乘客
			double sumExp = 0;

			if (pathType1No != -1) {
				sumExp += std::exp(-1 * theta * pathType1Cost);
			}
			if (pathType2No != -1) {
				sumExp += std::exp(-1 * theta * pathType2Cost);
			}
			if (pathType3No != -1) {
				sumExp += std::exp(-1 * theta * pathType3Cost);
			}
			if (pathType4No != -1) {
				sumExp += std::exp(-1 * theta * pathType4Cost);
			}
			if (pathType5No != -1) {
				sumExp += std::exp(-1 * theta * pathType5Cost);
			}
			if (pathType6No != -1) {
				sumExp += std::exp(-1 * theta * pathType6Cost);
			}

			double passengerLeft = 0;//剩余的乘客数量
			double assignPassenger = 0;
			double needAssignPassenger = 0;


			/*
			pathType:
				1  share bicycle
				2  bus
				3  subway-bicycle
				4  subway-bus
				5  bicycle-bus
			*/
			//更新乘客容量
			if (pathType1No != -1) {

				needAssignPassenger = passenger_group->passengerFlow[t] * std::exp(-1 * theta * pathType1Cost) / sumExp;
				assignPassenger = needAssignPassenger;
				if (passenger_group->paths[pathType1No].outShareBicyclePoint1 != -1 && assignPassenger > shareBicycleC[ntl->getShareBicycleSeq(passenger_group->paths[pathType1No].outShareBicyclePoint1)]) {//如果outShareBicyclePoint1的车辆数小于需要的车辆数
					assignPassenger = shareBicycleC[ntl->getShareBicycleSeq(passenger_group->paths[pathType1No].outShareBicyclePoint1)];
				}
				if (passenger_group->paths[pathType1No].outShareBicyclePoint2 != -1 && assignPassenger > shareBicycleC[ntl->getShareBicycleSeq(passenger_group->paths[pathType1No].outShareBicyclePoint2)]) {//如果outShareBicyclePoint1的车辆数小于需要的车辆数
					assignPassenger = shareBicycleC[ntl->getShareBicycleSeq(passenger_group->paths[pathType1No].outShareBicyclePoint2)];
				}

				if (passenger_group->paths[pathType1No].outShareBicyclePoint1 != -1) {
					shareBicycleC[ntl->getShareBicycleSeq(passenger_group->paths[pathType1No].outShareBicyclePoint1)] -= assignPassenger;
				}
				if (passenger_group->paths[pathType1No].outShareBicyclePoint2 != -1) {
					shareBicycleC[ntl->getShareBicycleSeq(passenger_group->paths[pathType1No].outShareBicyclePoint2)] -= assignPassenger;
				}

				passengerLeft += needAssignPassenger - assignPassenger;
				passenger_group->paths[pathType1No].passengerNum[t] = assignPassenger;
			}
			if (pathType2No != -1) {

				needAssignPassenger = passenger_group->passengerFlow[t] * std::exp(-1 * theta * pathType2Cost) / sumExp;
				passenger_group->paths[pathType2No].passengerNum[t] = needAssignPassenger;

			}
			if (pathType3No != -1) {

				needAssignPassenger = passenger_group->passengerFlow[t] * std::exp(-1 * theta * pathType3Cost) / sumExp;
				assignPassenger = needAssignPassenger;
				if (passenger_group->paths[pathType3No].outShareBicyclePoint1 != -1 && assignPassenger > shareBicycleC[ntl->getShareBicycleSeq(passenger_group->paths[pathType3No].outShareBicyclePoint1)]) {//如果outShareBicyclePoint1的车辆数小于需要的车辆数
					assignPassenger = shareBicycleC[ntl->getShareBicycleSeq(passenger_group->paths[pathType3No].outShareBicyclePoint1)];
				}
				if (passenger_group->paths[pathType3No].outShareBicyclePoint2 != -1 && assignPassenger > shareBicycleC[ntl->getShareBicycleSeq(passenger_group->paths[pathType3No].outShareBicyclePoint2)]) {//如果outShareBicyclePoint1的车辆数小于需要的车辆数
					assignPassenger = shareBicycleC[ntl->getShareBicycleSeq(passenger_group->paths[pathType3No].outShareBicyclePoint2)];
				}

				if (passenger_group->paths[pathType3No].outShareBicyclePoint1 != -1) {
					shareBicycleC[ntl->getShareBicycleSeq(passenger_group->paths[pathType3No].outShareBicyclePoint1)] -= assignPassenger;
				}
				if (passenger_group->paths[pathType3No].outShareBicyclePoint2 != -1) {
					shareBicycleC[ntl->getShareBicycleSeq(passenger_group->paths[pathType3No].outShareBicyclePoint2)] -= assignPassenger;
				}

				passengerLeft += needAssignPassenger - assignPassenger;
				passenger_group->paths[pathType3No].passengerNum[t] = assignPassenger;

			}
			if (pathType4No != -1) {

				needAssignPassenger = passenger_group->passengerFlow[t] * std::exp(-1 * theta * pathType4Cost) / sumExp;
				passenger_group->paths[pathType4No].passengerNum[t] = needAssignPassenger;

			}
			if (pathType5No != -1) {

				needAssignPassenger = passenger_group->passengerFlow[t] * std::exp(-1 * theta * pathType5Cost) / sumExp;
				assignPassenger = needAssignPassenger;
				if (passenger_group->paths[pathType5No].outShareBicyclePoint1 != -1 && assignPassenger > shareBicycleC[ntl->getShareBicycleSeq(passenger_group->paths[pathType5No].outShareBicyclePoint1)]) {//如果outShareBicyclePoint1的车辆数小于需要的车辆数
					assignPassenger = shareBicycleC[ntl->getShareBicycleSeq(passenger_group->paths[pathType5No].outShareBicyclePoint1)];
				}
				if (passenger_group->paths[pathType5No].outShareBicyclePoint2 != -1 && assignPassenger > shareBicycleC[ntl->getShareBicycleSeq(passenger_group->paths[pathType5No].outShareBicyclePoint2)]) {//如果outShareBicyclePoint1的车辆数小于需要的车辆数
					assignPassenger = shareBicycleC[ntl->getShareBicycleSeq(passenger_group->paths[pathType5No].outShareBicyclePoint2)];
				}

				if (passenger_group->paths[pathType5No].outShareBicyclePoint1 != -1) {
					shareBicycleC[ntl->getShareBicycleSeq(passenger_group->paths[pathType5No].outShareBicyclePoint1)] -= assignPassenger;
				}
				if (passenger_group->paths[pathType5No].outShareBicyclePoint2 != -1) {
					shareBicycleC[ntl->getShareBicycleSeq(passenger_group->paths[pathType5No].outShareBicyclePoint2)] -= assignPassenger;
				}

				passengerLeft += needAssignPassenger - assignPassenger;
				passenger_group->paths[pathType5No].passengerNum[t] = assignPassenger;

			}
			if (pathType6No != -1) {

				needAssignPassenger = passenger_group->passengerFlow[t] * std::exp(-1 * theta * pathType6Cost) / sumExp;
				passenger_group->paths[pathType6No].passengerNum[t] = needAssignPassenger;

			}

			//要处理容量超出，如果有共享单车的模式超出，则将其转向没有共享单车的线路
			if (passengerLeft != 0) {
				//2和4没有容量限制
				if (pathType2No != -1 && pathType4No != -1 && pathType6No != -1) {
					passenger_group->paths[pathType2No].passengerNum[t] += passengerLeft / 3;
					passenger_group->paths[pathType4No].passengerNum[t] += passengerLeft / 3;
					passenger_group->paths[pathType6No].passengerNum[t] += passengerLeft / 3;
				}
				else if (pathType2No != -1 && pathType4No != -1) {
					passenger_group->paths[pathType2No].passengerNum[t] += passengerLeft/2;
					passenger_group->paths[pathType4No].passengerNum[t] += passengerLeft/2;
				}
				else if (pathType6No != -1 && pathType4No != -1) {
					passenger_group->paths[pathType6No].passengerNum[t] += passengerLeft / 2;
					passenger_group->paths[pathType4No].passengerNum[t] += passengerLeft / 2;
				}
				else if (pathType2No != -1 && pathType6No != -1) {
					passenger_group->paths[pathType2No].passengerNum[t] += passengerLeft / 2;
					passenger_group->paths[pathType6No].passengerNum[t] += passengerLeft / 2;
				}
				else if(pathType4No != -1){
					passenger_group->paths[pathType4No].passengerNum[t] += passengerLeft;
				}
				else if (pathType2No != -1) {
					passenger_group->paths[pathType2No].passengerNum[t] += passengerLeft;
				}
				else if (pathType6No != -1) {
					passenger_group->paths[pathType6No].passengerNum[t] += passengerLeft;
				}

			}

			//无论线路是否使用，都要更新运营时间，这样才能为下一步的计算提供依据
			for (int pathno = 0; pathno < passenger_group->paths.size(); pathno++) {				
				ntl->addPassenger(passenger_no, pathno, 
					passenger_group->paths[pathno].links, passenger_group->paths[pathno].passengerNum[t]);
			}

		}
		ntl->simulation();
	}

	ntl->conSim2End();


	
	std::vector<vector<double>> alpha;//这个其实是A1的转置
	int sizeAlpha;
	std::vector<vector<double>> traveltime;//第一级索引是时间，第二级是线路
	double hmr[groupNum][TYPE];

	std::vector<vector<double>> A1;
	std::vector<vector<double>> A2;
	std::vector<vector<double>> A3;

	std::vector<double> B1;
	std::vector<double> B2;
	std::vector<double> B3;
	std::vector<double> Bbar;



	std::vector<vector<double>> M;
	std::vector<vector<double>> Mbar;//存储未满足约束

	for (auto p : passengerGroups) {

		for (auto r : p->paths) {
			vector<double> alphaSingle;

			for (int s = 0; s < bicyclePointNum; s++) {
				alphaSingle.push_back(0);
			}
			if (r.outShareBicyclePoint1 != -1) {
				alphaSingle[ntl->getShareBicycleSeq(r.outShareBicyclePoint1)] = 1;
			}
			if (r.outShareBicyclePoint2 != -1) {
				alphaSingle[ntl->getShareBicycleSeq(r.outShareBicyclePoint2)] = 1;
			}
			alpha.push_back(alphaSingle);
		}
	}

	sizeAlpha = alpha.size();

	int tmp = 0;
	for (auto p : passengerGroups) {
		vector<double> a;
		for (int i = 0; i < sizeAlpha; i++) {
			a.push_back(0);
		}
		for (auto r : p->paths) {
			a[tmp] = 1;
			tmp++;
		}
		A1.push_back(a);
	}

	for (int i = 0; i < bicyclePointNum; i++) {
		vector<double> a;
		for (int j = 0; j < sizeAlpha; j++) {
			a.push_back(-1 * alpha[j][i]);
		}
		A2.push_back(a);
	}

	for (int i = 0; i < sizeAlpha; i++) {
		vector<double> a;
		for (int j = 0; j < sizeAlpha; j++) {
			if (i == j)
				a.push_back(1);
			else
				a.push_back(0);
		}
		A3.push_back(a);
	}


	double* dmkr = new double[sizeAlpha];

	//计算
	vector<vector<double>> X_old;
	vector<vector<double>> X_new;

	

	while (L--) {

		//std::sort(supMap->passengerFlowsFinish.begin(), supMap->passengerFlowsFinish.end(), passengerFlowComparer);
		
		std::vector<vector<double>> oldTravelTime = traveltime;

		//乘客路线的 running + waitting time		
		if (traveltime.size() > 0) {
			/*double Rgap = calRgap(X_new, traveltime, passengerGroups, T, ctm);
			std::cout << "Rgap: " << Rgap << "\n";*/
			traveltime.clear();
		}

		std::vector<int> oldPathNum;
		for (int gn = 0; gn < groupNum; gn++) {
			oldPathNum.push_back(passengerGroups[gn]->paths.size());
		}

		std::map<std::string,int> newPathMap;
		std::vector<std::vector<double>> travelCostN;
		int routeNo = 0;


		//迪杰斯特拉新增线路===========================================================================================================================
		for (int t = 0; t < T; t++) {
			for (int gn = 0; gn < groupNum; gn++) {//清空h
				std::map<int, double> pathTypeMinCost;
				pathTypeMinCost[1] = 9999;
				pathTypeMinCost[2] = 9999;
				pathTypeMinCost[3] = 9999;
				pathTypeMinCost[4] = 9999;
				pathTypeMinCost[5] = 9999;
				pathTypeMinCost[6] = 9999;
				double minPathCost = 9999;

				for (int pn = 0; pn < passengerGroups[gn]->paths.size();pn++) {
					double travelFee = ntl->calVehTravelTime(gn, pn, t);
					int pathType = passengerGroups[gn]->paths[pn].type;
					if (pathTypeMinCost[pathType]> travelFee) {
						pathTypeMinCost[pathType] = travelFee;
					}
					if (minPathCost> travelFee) {
						minPathCost = travelFee;
					}
				}
				/********************************添加路径*********************************/
				//查询最短路径				
				std::vector<int> path;
				std::vector<int> typeList;	
				std::string pathIdf;

				int outShareBicyclePoint1 = -1;
				int inShareBicyclePoint1 = -1;
				int outShareBicyclePoint2 = -1;
				int inShareBicyclePoint2 = -1;
				int travelTime = 0;
				double travelCost = 0;
				//type1 share bike				
				typeList.push_back(0);
				typeList.push_back(1);
				path = ntl->Dijkstra(passengerGroups[gn]->startPonit, passengerGroups[gn]->endPonit, t, typeList, travelTime, travelCost, pathIdf, outShareBicyclePoint1
				, inShareBicyclePoint1, outShareBicyclePoint2, inShareBicyclePoint2);

				if (path.size()>0 && travelCost< pathTypeMinCost[1] && travelCost<(minPathCost+ 10) && std::find(allPassegnerPath[gn].begin(), allPassegnerPath[gn].end(), pathIdf) == allPassegnerPath[gn].end()) {//满足条件需要添加path
					passengerGroups[gn]->addPassengerPath(passengerGroups[gn]->paths.size(),1, path, outShareBicyclePoint1
						, inShareBicyclePoint1, outShareBicyclePoint2, inShareBicyclePoint2);
					allPassegnerPath[gn].push_back(pathIdf);
					newPathMap[pathIdf] = routeNo;
					routeNo++;
					std::vector<double> pathIniTravelC;
					for (int tt = 0; tt < T;tt++) {
						passengerGroups[gn]->paths[passengerGroups[gn]->paths.size()-1].passengerNum[tt] = 0;//设置新路线所有的乘客
						pathIniTravelC.push_back(9999);
					}
					travelCostN.push_back(pathIniTravelC);

					if (newPathMap.find(pathIdf) != newPathMap.end()) {//是新增的线路
						travelCostN[newPathMap[pathIdf]][t] = travelCost;
					}
				}

				

				pathIdf = "";
				typeList.clear();
				path.clear();

				//type2 bus	
				typeList.push_back(0);
				typeList.push_back(2);
				path = ntl->Dijkstra(passengerGroups[gn]->startPonit, passengerGroups[gn]->endPonit, t, typeList, travelTime, travelCost, pathIdf, outShareBicyclePoint1
					, inShareBicyclePoint1, outShareBicyclePoint2, inShareBicyclePoint2);
				if (path.size() > 0 && travelCost < pathTypeMinCost[2] && travelCost < (minPathCost + 10) && std::find(allPassegnerPath[gn].begin(), allPassegnerPath[gn].end(), pathIdf) == allPassegnerPath[gn].end()) {//满足条件需要添加path
					passengerGroups[gn]->addPassengerPath(passengerGroups[gn]->paths.size(), 2, path, outShareBicyclePoint1
						, inShareBicyclePoint1, outShareBicyclePoint2, inShareBicyclePoint2);
					allPassegnerPath[gn].push_back(pathIdf);
					newPathMap[pathIdf] = routeNo;
					routeNo++;
					std::vector<double> pathIniTravelC;
					for (int tt = 0; tt < T; tt++) {
						passengerGroups[gn]->paths[passengerGroups[gn]->paths.size() - 1].passengerNum[tt] = 0;//设置新路线所有的乘客
						pathIniTravelC.push_back(9999);
					}
					travelCostN.push_back(pathIniTravelC);

					if (newPathMap.find(pathIdf) != newPathMap.end()) {//是新增的线路
						travelCostN[newPathMap[pathIdf]][t] = travelCost;
					}
				}

				
				pathIdf = "";
				typeList.clear();
				path.clear();

				//type3 subway-bike	
				typeList.push_back(0);
				typeList.push_back(1);
				typeList.push_back(3);
				path = ntl->Dijkstra(passengerGroups[gn]->startPonit, passengerGroups[gn]->endPonit, t, typeList, travelTime, travelCost, pathIdf, outShareBicyclePoint1
					, inShareBicyclePoint1, outShareBicyclePoint2, inShareBicyclePoint2);
				if (path.size() > 0 && travelCost < pathTypeMinCost[3] && travelCost < (minPathCost + 10) && std::find(allPassegnerPath[gn].begin(), allPassegnerPath[gn].end(), pathIdf) == allPassegnerPath[gn].end()) {//满足条件需要添加path
					passengerGroups[gn]->addPassengerPath(passengerGroups[gn]->paths.size(), 3, path, outShareBicyclePoint1
						, inShareBicyclePoint1, outShareBicyclePoint2, inShareBicyclePoint2);
					allPassegnerPath[gn].push_back(pathIdf);
					newPathMap[pathIdf] = routeNo;
					routeNo++;
					std::vector<double> pathIniTravelC;
					for (int tt = 0; tt < T; tt++) {
						passengerGroups[gn]->paths[passengerGroups[gn]->paths.size() - 1].passengerNum[tt] = 0;//设置新路线所有的乘客
						pathIniTravelC.push_back(9999);
					}
					travelCostN.push_back(pathIniTravelC);

					if (newPathMap.find(pathIdf) != newPathMap.end()) {//是新增的线路
						travelCostN[newPathMap[pathIdf]][t] = travelCost;
					}
				}
				
				pathIdf = "";
				typeList.clear();
				path.clear();

				//type4 subway-bus	
				typeList.push_back(0);
				typeList.push_back(2);
				typeList.push_back(3);
				path = ntl->Dijkstra(passengerGroups[gn]->startPonit, passengerGroups[gn]->endPonit, t, typeList, travelTime, travelCost, pathIdf, outShareBicyclePoint1
					, inShareBicyclePoint1, outShareBicyclePoint2, inShareBicyclePoint2);
				if (path.size() > 0 && travelCost < pathTypeMinCost[4] && travelCost < (minPathCost + 10) && std::find(allPassegnerPath[gn].begin(), allPassegnerPath[gn].end(), pathIdf) == allPassegnerPath[gn].end()) {//满足条件需要添加path
					passengerGroups[gn]->addPassengerPath(passengerGroups[gn]->paths.size(), 4, path, outShareBicyclePoint1
						, inShareBicyclePoint1, outShareBicyclePoint2, inShareBicyclePoint2);
					allPassegnerPath[gn].push_back(pathIdf);
					newPathMap[pathIdf] = routeNo;
					routeNo++;
					std::vector<double> pathIniTravelC;
					for (int tt = 0; tt < T; tt++) {
						passengerGroups[gn]->paths[passengerGroups[gn]->paths.size() - 1].passengerNum[tt] = 0;//设置新路线所有的乘客
						pathIniTravelC.push_back(9999);
					}
					travelCostN.push_back(pathIniTravelC);

					if (newPathMap.find(pathIdf) != newPathMap.end()) {//是新增的线路
						travelCostN[newPathMap[pathIdf]][t] = travelCost;
					}
				}

				
				pathIdf = "";
				typeList.clear();
				path.clear();

				//type5 bus-bike	
				typeList.push_back(0);
				typeList.push_back(1);
				typeList.push_back(2);
				path = ntl->Dijkstra(passengerGroups[gn]->startPonit, passengerGroups[gn]->endPonit, t, typeList, travelTime, travelCost, pathIdf, outShareBicyclePoint1
					, inShareBicyclePoint1, outShareBicyclePoint2, inShareBicyclePoint2);
				if (path.size() > 0 && travelCost < pathTypeMinCost[5] && travelCost < (minPathCost + 10) && std::find(allPassegnerPath[gn].begin(), allPassegnerPath[gn].end(), pathIdf) == allPassegnerPath[gn].end()) {//满足条件需要添加path
					passengerGroups[gn]->addPassengerPath(passengerGroups[gn]->paths.size(), 5, path, outShareBicyclePoint1
						, inShareBicyclePoint1, outShareBicyclePoint2, inShareBicyclePoint2);
					allPassegnerPath[gn].push_back(pathIdf);
					newPathMap[pathIdf] = routeNo;
					routeNo++;
					std::vector<double> pathIniTravelC;
					for (int tt = 0; tt < T; tt++) {
						passengerGroups[gn]->paths[passengerGroups[gn]->paths.size() - 1].passengerNum[tt] = 0;//设置新路线所有的乘客
						pathIniTravelC.push_back(9999);
					}
					travelCostN.push_back(pathIniTravelC);

					if (newPathMap.find(pathIdf) != newPathMap.end()) {//是新增的线路
						travelCostN[newPathMap[pathIdf]][t] = travelCost;
					}
				}

				
				pathIdf = "";
				typeList.clear();
				path.clear();

				//type6 subway	
				typeList.push_back(0);
				typeList.push_back(3);
				path = ntl->Dijkstra(passengerGroups[gn]->startPonit, passengerGroups[gn]->endPonit, t, typeList, travelTime, travelCost, pathIdf, outShareBicyclePoint1
					, inShareBicyclePoint1, outShareBicyclePoint2, inShareBicyclePoint2);
				if (path.size() > 0 && travelCost < pathTypeMinCost[6] && travelCost < (minPathCost + 10) && std::find(allPassegnerPath[gn].begin(), allPassegnerPath[gn].end(), pathIdf) == allPassegnerPath[gn].end()) {//满足条件需要添加path
					passengerGroups[gn]->addPassengerPath(passengerGroups[gn]->paths.size(), 6, path, outShareBicyclePoint1
						, inShareBicyclePoint1, outShareBicyclePoint2, inShareBicyclePoint2);
					allPassegnerPath[gn].push_back(pathIdf);
					newPathMap[pathIdf] = routeNo;
					routeNo++;
					std::vector<double> pathIniTravelC;
					for (int tt = 0; tt < T; tt++) {
						passengerGroups[gn]->paths[passengerGroups[gn]->paths.size() - 1].passengerNum[tt] = 0;//设置新路线所有的乘客
						pathIniTravelC.push_back(9999);
					}
					travelCostN.push_back(pathIniTravelC);

					if (newPathMap.find(pathIdf) != newPathMap.end()) {//是新增的线路
						travelCostN[newPathMap[pathIdf]][t] = travelCost;
					}
				}

				
				pathIdf = "";
				typeList.clear();
				path.clear();

			}
		}
		//===========================================================================================================================

		//计算线路出行时间===========================================================================================================================
		for (int t = 0; t < T; t++) {
			vector<double> pathsTraveltime;
			for (int gn = 0; gn < groupNum; gn++) {//清空h				

				for (int pn = 0; pn < passengerGroups[gn]->paths.size(); pn++) {
					if (pn < oldPathNum[gn]) {
						double travelFee = ntl->calVehTravelTime(gn, pn, t);
						pathsTraveltime.push_back(travelFee);
					}
					else {
						double travelFee = travelCostN[newPathMap[passengerGroups[gn]->paths[pn].pathIdf]][t];
						pathsTraveltime.push_back(travelFee);
					}
				}				
			}
			traveltime.push_back(pathsTraveltime);
		}


		X_new.clear();
		for (int t = 0; t < T; t++) {//分时段求解
			vector<double> tmX_new;
			for (int gn = 0; gn < groupNum; gn++) {//清空h
				for (auto r : passengerGroups[gn]->paths) {
					tmX_new.push_back(r.passengerNum[t]);
				}
			}

			X_new.push_back(tmX_new);
		}
				
		double Rgap = calRgap(X_new, traveltime, passengerGroups, T, ntl,0);
		//double Rgap = calRgap(X_new, traveltime, passengerGroups, T, ntl,1);
		std::cout << "Rgap: "<< Rgap << ", L: " << L << "\n";
		diedai << Rgap << "\n";

		//ntl->allODTravelModePassenger;
		//ntl->allTravelModePassenger;


		//大循环终止条件===================================================================================================================================================
		if (Rgap < epsilonL && L!=99) {

			std::cout << "迭代完成，程序收敛 条件 1";			

			for (int i = 0; i < groupNum; i++) {
				for (int j = 0; j < passengerGroups[i]->paths.size(); j++) {
					outFilePS << passengerGroups[i]->startPonit<<"~"<< passengerGroups[i]->endPonit << ":" << i << "_" << j << ",";
					outFilePLink << passengerGroups[i]->startPonit << "~" << passengerGroups[i]->endPonit << ":" << i << "_" << j << ","; outFileS;
					outFileS << passengerGroups[i]->startPonit << "~" << passengerGroups[i]->endPonit << ":" << i << "_" << j << ","; outFileS;
				}
			}
			outFilePS << "\n";
			outFilePLink << "\n";
			outFileS << "\n";
			for (int i = 0; i < groupNum; i++) {
				for (int j = 0; j < passengerGroups[i]->paths.size(); j++) {
					
					outFilePLink << ntl->links[passengerGroups[i]->paths[j].links[0]]->startNode->nodeID;
					for (int k = 0; k < passengerGroups[i]->paths[j].links.size(); k++) {
						int linkNo = passengerGroups[i]->paths[j].links[k];
						outFilePLink << "_" << ntl->links[linkNo]->endNode->nodeID;
					}
					outFilePLink << ",";
				}
			}

			for (int i = 0; i < X_new.size(); i++) {
				for (int j = 0; j < X_new[i].size(); j++) {
					outFilePS << X_new[i][j] << ",";
				}
				outFilePS << "\n";
			}

			for (int i = 0; i < traveltime.size(); i++) {
				for (int j = 0; j < traveltime[i].size(); j++) {
					outFileS << traveltime[i][j] << ",";
				}
				outFileS << "\n";
			}

			modePassenger << "bikePassengerNum," << "busPassengerNum," << "railPassengerNum" << "\n";
			for (int j = 0; j < ntl->allTravelModePassenger.size(); j++) {
				modePassenger << ntl->allTravelModePassenger[j].bikePassengerNum << "," << ntl->allTravelModePassenger[j].busPassengerNum << "," << ntl->allTravelModePassenger[j].railPassengerNum;
				modePassenger << "\n";
			}

			for (int gn = 0; gn < groupNum; gn++) {//清空h
				odModePassenger << passengerGroups[gn]->groupNo << "," << "," << ",";
			}
			odModePassenger << "\n";
			odModePassenger << "bikePassengerNum," << "busPassengerNum," << "railPassengerNum" << "\n";
			for (int j = 0; j < ntl->allODTravelModePassenger.size(); j++) {
				for (int gn = 0; gn < groupNum; gn++) {//清空h
					int tmGnn = passengerGroups[gn]->groupNo;
					if (ntl->allODTravelModePassenger[j].find(tmGnn) != ntl->allODTravelModePassenger[j].end()) {//找到了
						odModePassenger << ntl->allODTravelModePassenger[j][tmGnn].bikePassengerNum << "," << ntl->allODTravelModePassenger[j][tmGnn].busPassengerNum << "," << ntl->allODTravelModePassenger[j][tmGnn].railPassengerNum << ",";
					}
					else {
						odModePassenger << "0," << "0," << "0,";
					}
				}
				odModePassenger << "\n";
			}

			
			ntl->printLaRPassenger(50);
			ntl->~TransitNetworkLoading();

			return 0;
		}
		//===================================================================================================================================================

		alpha.clear();
		for (auto p : passengerGroups) {

			for (auto r : p->paths) {
				vector<double> alphaSingle;

				for (int s = 0; s < bicyclePointNum; s++) {
					alphaSingle.push_back(0);
				}
				if (r.outShareBicyclePoint1 != -1) {
					alphaSingle[ntl->getShareBicycleSeq(r.outShareBicyclePoint1)] = 1;
				}
				if (r.outShareBicyclePoint2 != -1) {
					alphaSingle[ntl->getShareBicycleSeq(r.outShareBicyclePoint2)] = 1;
				}
				alpha.push_back(alphaSingle);
			}
		}

		sizeAlpha = alpha.size();
		A1.clear();
		A2.clear();
		A3.clear();

		int tmp = 0;
		for (auto p : passengerGroups) {
			vector<double> a;
			for (int i = 0; i < sizeAlpha; i++) {
				a.push_back(0);
			}
			for (auto r : p->paths) {
				a[tmp] = 1;
				tmp++;
			}
			A1.push_back(a);
		}

		for (int i = 0; i < bicyclePointNum; i++) {
			vector<double> a;
			for (int j = 0; j < sizeAlpha; j++) {
				a.push_back(-1 * alpha[j][i]);
			}
			A2.push_back(a);
		}

		for (int i = 0; i < sizeAlpha; i++) {
			vector<double> a;
			for (int j = 0; j < sizeAlpha; j++) {
				if (i == j)
					a.push_back(1);
				else
					a.push_back(0);
			}
			A3.push_back(a);
		}

		delete dmkr;
		dmkr = new double[sizeAlpha];

		//打印出行时间的差异
		//if (oldTravelTime.size() >0) {
		//	for (int t = 0; t < T; t++) {//分时段求解
		//		vector<double> pathsTraveltime;
		//		for (int j = 0; j < sizeAlpha; j++) {//清空h
		//			cout << traveltime[t][j] - oldTravelTime[t][j] << " | ";
		//		}
		//	}
		//	cout << "\n";
		//}
		
		//打印共享单车停靠点容量
		/*for (int i = 0; i < ctm->bicyclePoints.size(); i++) {
			std::cout << ctm->bicyclePoints[i] << " : " << ctm->nodes[ctm->bicyclePoints[i]]->bicycleNum << " || ";
		}
		std::cout << "\n";*/


		ntl->clearNetwork();

		for (int t = 0; t < T; t++) {//分时段求解**************************************************************************************************************************************

			vector<double> X_o;

			if (B1.size() != 0) {
				B1.clear();
			}
			if (B2.size() != 0) {
				B2.clear();
			}
			if (B3.size() != 0) {
				B3.clear();
			}

			


			//更新模型约束
			for (int pn = 0; pn < sizeAlpha; pn++) {
				dmkr[pn] = 0;
			}

			for (int gn = 0; gn < groupNum; gn++) {//清空h
				for (int type = 0; type < TYPE; type++) {
					hmr[gn][type] = -1;
				}
			}

			int nowsize = 0;
			for (int gn = 0; gn < groupNum; gn++) {//清空h
				for (auto r : passengerGroups[gn]->paths) {
					if (hmr[gn][r.type - 1] == -1) {//需要计算h
						hmr[gn][r.type - 1] = 0;
						for (auto tr : passengerGroups[gn]->paths) {
							if (r.type == tr.type) {
								hmr[gn][r.type - 1] += tr.passengerNum[t];
							}
						}
					}
					if (hmr[gn][r.type - 1] < 0.1) {
						hmr[gn][r.type - 1] = 0.1;//给没有乘客的路径设置客流
					}
					X_o.push_back(r.passengerNum[t]);
					if (!containsDPC) {
						dmkr[nowsize] = 2 * (-1 * r.passengerNum[t] + tau * (traveltime[t][nowsize] + std::log(hmr[gn][r.type - 1]) / theta));
					}
					
					nowsize++;
				}
			}

			//流约束
			for (auto p : passengerGroups) {
				B1.push_back(p->passengerFlow[t]);
			}

			//车场容量
			for (int i = 0; i < bicyclePointNum; i++) {
				B2.push_back(-1 * ntl->getBicyclePointByBS(i));
				if (B2[i]>-0.0000001) {
					B2[i] = 0;
				}
			}

			for (int i = 0; i < sizeAlpha; i++) {
				B3.push_back(0);
			}

			//*********************************************************************************************
		//对于 t>0 的情况，要讲X_o调整至可行解，具体方法：由于共享单车容量的变化，可能导致共享单车容量不满足，我们将为满足的容量，给到不需要共享单车路径下出行时间最短的线路中去
			if (t > 0) {

				for (int i = 0; i < B2.size(); i++) {
					double tmS = 0;
					for (int j = 0; j < sizeAlpha; j++) {
						tmS += A2[i][j] * X_o[j];
					}
					if (tmS - B2[i] < -0.0000001) {//表示不满足停靠点 i 的容量约束
						tmS = B2[i] - tmS;//计算差值
						for (int k = 0; k < sizeAlpha && tmS > 0.00000001; k++) {
							if (A2[i][k] == -1 && X_o[k] > 0) {//表示第 k 条路径需要调出客流
								int sumPathno = 0;
								for (int l = 0; l < groupNum && tmS > 0.00000001; l++) {//判断哪一个od用到了 k 路径
									if (A1[l][k] == 1) {//表示od l 用到了线路k，这里就要对od l中的 k线路客流进行调整
										int minpathNo;
										double pathTravelT = 999999;
										for (int gp = 0; gp < passengerGroups[l]->paths.size(); gp++) {
											if (passengerGroups[l]->paths[gp].type == path_tpye_2 || passengerGroups[l]->paths[gp].type == path_tpye_4) {//筛选没有共享单车的线路
												if (traveltime[t][sumPathno + gp] < pathTravelT) {//找出成本最小线路
													minpathNo = gp;
													pathTravelT = traveltime[t][sumPathno + gp];
												}
											}
										}
										//开始调整解
										if (X_o[k] >= tmS) {
											X_o[k] -= tmS;
											X_o[sumPathno + minpathNo] += tmS;
											tmS = 0;
										}
										else {
											X_o[sumPathno + minpathNo] += X_o[k];
											tmS -= X_o[k];
											X_o[k] = 0;
										}
									}
									sumPathno += passengerGroups[l]->paths.size();
								}
							}
						}

					}
				}

			}

			//DPC==========================================================================================================================================
			if (containsDPC) {
			
			
				//****************************************************仿真当前成本********************************************************************************************
				TransitNetworkLoading* ntl_new = new TransitNetworkLoading();
				for (int t_new = 0; t_new < t; t_new++) {
					nowsize = 0;
					for (int i = 0; i < passengerGroups.size(); i++) {
						for (int j = 0; j < passengerGroups[i]->paths.size(); j++) {
							ntl_new->addPassenger(passengerGroups[i]->groupNo, passengerGroups[i]->paths[j].no, passengerGroups[i]->paths[j].links, passengerGroups[i]->paths[j].passengerNum[t_new]);
							nowsize++;
						}
					}
					ntl_new->simulation();
				}

				for (int simS = 0; simS < 20; simS++) {

					if (t + simS < T) {
						std::vector<double> new_X;
						nowsize = 0;
						for (int gn = 0; gn < groupNum; gn++) {//清空h
							for (auto r : passengerGroups[gn]->paths) {
								new_X.push_back(r.passengerNum[t + simS]);
								nowsize++;
							}
						}

						std::vector<double> new_B2;
						//车场容量
						for (int i = 0; i < bicyclePointNum; i++) {
							new_B2.push_back(-1 * ntl_new->getBicyclePointByBS(i));
							if (new_B2[i] > -0.0000001) {
								new_B2[i] = 0;
							}
						}

						if (t > 0) {
							for (int i = 0; i < new_B2.size(); i++) {
								double tmS = 0;
								for (int j = 0; j < sizeAlpha; j++) {
									tmS += A2[i][j] * new_X[j];
								}
								if (tmS - new_B2[i] < -0.0000001) {//表示不满足停靠点 i 的容量约束
									tmS = new_B2[i] - tmS;//计算差值
									for (int k = 0; k < sizeAlpha && tmS > 0.00000001; k++) {
										if (A2[i][k] == -1 && new_X[k] > 0) {//表示第 k 条路径需要调出客流
											int sumPathno = 0;
											for (int l = 0; l < groupNum && tmS > 0.00000001; l++) {//判断哪一个od用到了 k 路径
												if (A1[l][k] == 1) {//表示od l 用到了线路k，这里就要对od l中的 k线路客流进行调整
													int minpathNo;
													double pathTravelT = 999999;
													for (int gp = 0; gp < passengerGroups[l]->paths.size(); gp++) {
														if (passengerGroups[l]->paths[gp].type == path_tpye_2 || passengerGroups[l]->paths[gp].type == path_tpye_4) {//筛选没有共享单车的线路
															if (traveltime[t][sumPathno + gp] < pathTravelT) {//找出成本最小线路
																minpathNo = gp;
																pathTravelT = traveltime[t][sumPathno + gp];
															}
														}
													}
													//开始调整解
													if (new_X[k] >= tmS) {
														new_X[k] -= tmS;
														new_X[sumPathno + minpathNo] += tmS;
														tmS = 0;
													}
													else {
														new_X[sumPathno + minpathNo] += new_X[k];
														tmS -= new_X[k];
														new_X[k] = 0;
													}
												}
												sumPathno += passengerGroups[l]->paths.size();
											}
										}
									}
								}
							}
						}

						nowsize = 0;
						for (int i = 0; i < passengerGroups.size(); i++) {
							for (int j = 0; j < passengerGroups[i]->paths.size(); j++) {
								ntl_new->addPassenger(passengerGroups[i]->groupNo, passengerGroups[i]->paths[j].no, passengerGroups[i]->paths[j].links, new_X[nowsize]);
								nowsize++;

							}
						}
					}

					ntl_new->simulation();
				}

				nowsize = 0;
				vector<double> pathsTraveltime;
				for (int gn = 0; gn < groupNum; gn++) {//清空h	
					for (int pn = 0; pn < passengerGroups[gn]->paths.size(); pn++) {
						double travelFee = ntl_new->calVehTravelTime(gn, pn, t);
						if (travelFee == 0) {
							double travelFee = traveltime[t][nowsize];
							pathsTraveltime.push_back(travelFee);
						}
						else {
							pathsTraveltime.push_back(travelFee);
						}
						nowsize++;
					}
				}

				delete ntl_new;

				nowsize = 0;
				for (int gn = 0; gn < groupNum; gn++) {//清空h
					for (auto r : passengerGroups[gn]->paths) {
						dmkr[nowsize] = 2 * (-1 * X_o[nowsize] + tau * (pathsTraveltime[nowsize] + std::log(hmr[gn][r.type - 1]) / theta));
						nowsize++;
					}
				}
			}
			else {
				nowsize = 0;
				for (int gn = 0; gn < groupNum; gn++) {//清空h
					for (auto r : passengerGroups[gn]->paths) {
						dmkr[nowsize] = 2 * (-1 * X_o[nowsize] + tau * (traveltime[t][nowsize] + std::log(hmr[gn][r.type - 1]) / theta));
						nowsize++;
					}
				}
			}
			//!DPC==========================================================================================================================================

			//****************************************************************************************************

			/*nowsize = 0;
			for (int gn = 0; gn < groupNum; gn++) {//清空h
				for (auto r : passengerGroups[gn]->paths) {
					dmkr[nowsize] = 2 * (-1 * X_o[nowsize] + tau * (traveltime[t][nowsize] + std::log(hmr[gn][r.type - 1]) / theta));
					nowsize++;
				}
			}


			std::vector<double> new_x = myQserver(dmkr, A1, A2, B1, B2);
			for (int i = 0; i < new_x.size(); i++) {
				X_o[i] = new_x[i];
			}*/

			


			/*std::vector<double> new_x = myQserver(dmkr, A1, A2, B1, B2);
			for (int i = 0; i < new_x.size(); i++) {
				X_o[i] = new_x[i];
			}*/
			

			//*****************************************************************求解二次规划

			//对变量以及约束进行筛选，保证M为满秩矩阵

			//****************************************************************************
			

			int loopFlag = 1;
			int step2Flag = 0;
			vector<double> gradientF(sizeAlpha);

			MatrixXd matrix(0, sizeAlpha + 1);//用来判断M的秩
			vector<vector<double>> lostM;

			while (loopFlag) {

				if (!step2Flag) {

					if (M.size() != 0) {
						M.clear();
					}
					if (Mbar.size() != 0) {
						Mbar.clear();
					}
					if (Bbar.size() != 0) {
						Bbar.clear();
					}


					matrix.conservativeResize(0, matrix.cols());
					lostM.clear();

					//添加A1
					for (int i = 0; i < groupNum; i++) {
						if (B1[i] < -0.0000000001 || B1[i] > 0.0000000001) {//等于0的约束不再添加
							M.push_back(A1[i]);
							int mR = matrix.rows();
							matrix.conservativeResize(mR + 1, matrix.cols());
							for (int j = 0; j < sizeAlpha + 1; j++) {
								if (j < sizeAlpha) {
									matrix(mR, j) = A1[i][j];
								}
								else {
									matrix(mR, j) = B1[i];
								}
							}
						}
					}



					for (int i = 0; i < sizeAlpha; i++) {
						if (X_o[i] > -0.0000000001 && X_o[i] < 0.0000000001) {
							M.push_back(A3[i]);
							int mR = matrix.rows();
							matrix.conservativeResize(mR + 1, matrix.cols());
							for (int j = 0; j < sizeAlpha + 1; j++) {
								if (j < sizeAlpha) {
									matrix(mR, j) = A3[i][j];
								}
								else {
									matrix(mR, j) = B3[i];
								}
							}
						}
						else {
							Mbar.push_back(A3[i]);
							Bbar.push_back(B3[i]);
						}
					}

					int oldRank = matrix_rank(matrix);
					int newRank = 0;
					for (int i = 0; i < bicyclePointNum; i++) {
						double s = 0;
						for (int j = 0; j < sizeAlpha; j++) {
							s += A2[i][j] * X_o[j];
						}
						if (s - B2[i] < 0.0000001) {
							if ((B2[i] < -0.00000001 || B2[i] > 0.00000001) && oldRank < sizeAlpha) {//等于0的约束不再添加


								int mR = matrix.rows();
								matrix.conservativeResize(mR + 1, matrix.cols());
								for (int j = 0; j < sizeAlpha + 1; j++) {
									if (j < sizeAlpha) {
										matrix(mR, j) = A2[i][j];
									}
									else {
										matrix(mR, j) = B2[i];
									}
								}
								newRank = matrix_rank(matrix);
								if (newRank > oldRank) {
									M.push_back(A2[i]);
									oldRank = newRank;
								}
								else {
									matrix.conservativeResize(mR , matrix.cols());
									vector<double> tmAB(sizeAlpha + 1);
									for (int s = 0; s < sizeAlpha; s++) {
										tmAB[s] = A2[i][s];
									}
									tmAB[sizeAlpha] = B2[i];
									lostM.push_back(tmAB);
								}

							}
							else {
								vector<double> tmAB(sizeAlpha+1);
								for (int s = 0; s < sizeAlpha;s++) {
									tmAB[s] = A2[i][s];
								}
								tmAB[sizeAlpha] = B2[i];
								lostM.push_back(tmAB);
							}
						}
						else if (s - B2[i] < 0) {
							std::cout << "容量约束出错";
						}
						else {
							Mbar.push_back(A2[i]);
							Bbar.push_back(B2[i]);
						}
					}



					//求解梯度方向
					nowsize = 0;
					for (int gn = 0; gn < groupNum; gn++) {//清空h
						for (auto r : passengerGroups[gn]->paths) {
							gradientF[nowsize] = 2 * X_o[nowsize] + dmkr[nowsize];
							nowsize++;
						}
					}
				}


				//求解下降方向dk

				vector<vector<double>> MTMinv;
				vector<vector<double>> P(sizeAlpha, vector<double>(sizeAlpha));
				if (M.size() == 0) {
					for (int i = 0; i < sizeAlpha; i++) {
						for (int j = 0; j < sizeAlpha; j++) {
							if (i == j)
								P[i][j] = 1;
							else
								P[i][j] = 0;
						}
					}
				}
				else {

					vector<vector<double>> MT = MatrixT(M);

					vector<vector<double>> MTM = MatrixMultiply(M, MT);

					MTMinv = inverseMatrixLU(MTM);

					vector<vector<double>> MTMTMinv = MatrixMultiply(MT, MTMinv);

					vector<vector<double>> MTMTMinvM = MatrixMultiply(MTMTMinv, M);

					for (int i = 0; i < sizeAlpha; i++) {
						for (int j = 0; j < sizeAlpha; j++) {
							if (i == j)
								P[i][j] = 1 - MTMTMinvM[i][j];
							else
								P[i][j] = -1 * MTMTMinvM[i][j];
						}
					}
				}


				vector<double> dk(sizeAlpha);

				for (int i = 0; i < sizeAlpha; i++) {
					dk[i] = 0;
					for (int j = 0; j < sizeAlpha; j++) {
						dk[i] -= P[i][j] * gradientF[j];
					}
					if (isnan(dk[i])) {
						std::cout << "迭代方向错误";
						for (int n = 0; n < matrix.rows();n++) {
							std::cout << "\n";
							for (int m = 0; m < matrix.cols();m++) {
								cout << matrix(n,m);
							}
						}
					}
				}

				

				if (Matrix2Parametrics(dk) < epsilon) {//判断收敛标准******************************************************************************

					vector<vector<double>> MTMinvM = MatrixMultiply(MTMinv, M);
					double lambda = 0;
					int minNo = 0;
					for (int i = MTMinvM.size() - 1; i >= groupNum; i--) {
						double tmLambda = 0;
						for (int j = 0; j < sizeAlpha; j++) {
							tmLambda += MTMinvM[i][j] * gradientF[j];
						}
						//这里只去除 最小的 lamba对应的行
						if (tmLambda < lambda) {
							lambda = tmLambda;
							minNo = i;
						}
					}

					if (lambda < -0.000000001) {

						//为什么有不能删除？？？？？？？
							//删除之后替换进入一个能够使其行满秩的候补行。。。。。。。。。。。
						//MatrixXd matrixN(M.size(), sizeAlpha + 1);
						//M.erase(M.begin() + minNo);
						int MR = lostM.size();
						if (lostM.size()!=0) {							
							for (int k = 0; k < lostM.size();k++) {
								for (int l = 0; l < sizeAlpha + 1; l++) {
									if (l < sizeAlpha)
										M[minNo][l] = lostM[k][l];
									matrix(minNo,l) = lostM[k][l];
								}	
								if (matrix_rank(matrix) == M.size()) {//补充之后满秩
									lostM.erase(lostM.begin() + k);									
									break;
								}
							}
						}
						if (lostM.size()== MR) {
							removeRow(matrix, minNo);
							M.erase(M.begin() + minNo);		
						}
						step2Flag = 1;
					}
					else {
						loopFlag = 0;
					}
				}
				else {
					//求解步长alphabar

					double maxAlphaBar = 999999999999999999;

					for (int i = 0; i < Mbar.size(); i++) {
						double Ad = 0;
						double Ax = 0;
						for (int j = 0; j < sizeAlpha; j++) {
							Ad += Mbar[i][j] * dk[j];
							Ax += Mbar[i][j] * X_o[j];
						}
						if (Ad < 0) {
							double tm = (Bbar[i] - Ax) / Ad;
							if (tm < maxAlphaBar) {
								maxAlphaBar = tm;
							}
						}
					}


					double tm1 = 0;
					double tm2 = 0;
					for (int i = 0; i < sizeAlpha; i++) {
						tm1 += 2 * dk[i] * X_o[i] + dk[i] * dmkr[i];
						tm2 += 2 * dk[i] * dk[i];
					}
					double ak1 = -1 * tm1 / tm2;
					if (ak1 > maxAlphaBar)
						ak1 = maxAlphaBar;
					double ak2 = 0;
					double ak3 = maxAlphaBar;


					//测试MaxAlphaBar求解对不对
					//maxAlphaBar += 0.1;
					vector<double> tmB(B1.size() + B2.size() + B3.size());
					for (int i = 0; i < groupNum;i++) {
						double tm = 0;
						for (int j = 0; j < sizeAlpha; j++) {
							double tma = X_o[j] + maxAlphaBar * dk[j];
							tm += tma*A1[i][j];
						}
						tmB[i] = tm;
						if (B1[i] - tm >= 0.0001)
							std::cout << "未满足约束1\n";
					}

					for (int i = 0; i < ntl->sharedBikeNode.size(); i++) {
						double tm = 0;
						for (int j = 0; j < sizeAlpha; j++) {
							double tma = X_o[j] + maxAlphaBar * dk[j];
							tm += tma * A2[i][j];
						}
						tmB[groupNum + i] = tm;
						if (B2[i] - tm >= 0.0001)
							std::cout << "未满足约束2\n";
					}

					for (int i = 0; i < sizeAlpha; i++) {
						double tm = 0;
						for (int j = 0; j < sizeAlpha; j++) {
							double tma = X_o[j] + maxAlphaBar * dk[j];
							tm += tma * A3[i][j];
						}
						tmB[groupNum + ntl->sharedBikeNode.size() + i] = tm;
						if (B3[i] - tm >= 0.0001)
							std::cout << "未满足约束3\n";
					}


					//测试结束


					double value1 = 0;
					double value2 = 0;
					double value3 = 0;

					for (int i = 0; i < sizeAlpha; i++) {
						double mt = X_o[i] + ak1 * dk[i] + 0.5 * dmkr[i];
						value1 += mt * mt;
					}
					for (int i = 0; i < sizeAlpha; i++) {
						double mt = X_o[i] + ak2 * dk[i] + 0.5 * dmkr[i];
						value2 += mt * mt;
					}
					for (int i = 0; i < sizeAlpha; i++) {
						double mt = X_o[i] + ak3 * dk[i] + 0.5 * dmkr[i];
						value3 += mt * mt;
					}



					double minAlpha = 0;
					if (value1 <= value2) {
						if (value1 <= value3)minAlpha = ak1;
						else minAlpha = ak3;
					}
					else if (value2 <= value3)minAlpha = ak2;
					else minAlpha = ak3;

					//步长与方向确定，要更新x
					for (int i = 0; i < sizeAlpha; i++) {
						double tma = X_o[i] + minAlpha * dk[i];
						if (tma < 0.00000001)
							X_o[i] = 0;
						else
							X_o[i] = tma;
					}
					step2Flag = 0;
				}


			}

			double ssss = 0;
			for (int i = 0; i < X_o.size();i++) {
				ssss += X_o[i]* X_o[i] + dmkr[i]* X_o[i];
			}


			
			//****************************************************************************************更新网络*********************************************************************************
			nowsize = 0;
			for (int i = 0; i < passengerGroups.size();i++) {
				for (int j = 0; j < passengerGroups[i]->paths.size();j++) {
					
					passengerGroups[i]->paths[j].passengerNum[t] = X_o[nowsize];
					ntl->addPassenger(passengerGroups[i]->groupNo, passengerGroups[i]->paths[j].no, passengerGroups[i]->paths[j].links, passengerGroups[i]->paths[j].passengerNum[t]);
					nowsize++;

				}
			}

			ntl->simulation();

			X_old.push_back(X_o);

		}

		ntl->conSim2End();

		if (X_new.size() != 0) {
			X_new.clear();
			for (int i = 0; i < X_old.size(); i++) {
				X_new.push_back(X_old[i]);
			}
			X_old.clear();
		}
		else {
			for (int i = 0; i < X_old.size(); i++) {
				X_new.push_back(X_old[i]);
			}
			X_old.clear();
		}

		//double tmD = 0;
		//if (X_new.size() != 0) {
		//	for (int i = 0; i < X_old.size(); i++) {
		//		for (int j = 0; j < X_old[i].size(); j++) {
		//			tmD += (X_old[i][j] - X_new[i][j]) * (X_old[i][j] - X_new[i][j]);
		//		}
		//	}
		//	tmD = std::sqrt(tmD);
		//	//std::cout << tmD << '\n';
		//	X_new.clear();
		//	for (int i = 0; i < X_old.size(); i++) {
		//		X_new.push_back(X_old[i]);
		//	}
		//	X_old.clear();
		//}
		//else {
		//	for (int i = 0; i < X_old.size(); i++) {
		//		X_new.push_back(X_old[i]);
		//	}
		//	X_old.clear();
		//}

		////大循环终止条件
		//if (tmD < epsilonL && 1>2) {
		////if (tmD < epsilonL || tmDs == tmD) {
		//	if (tmD < epsilonL) {
		//		std::cout << "迭代完成，程序收敛 条件 1";
		//	}
		//	else {
		//		std::cout << "迭代完成，程序收敛 条件 2";
		//	}
		//	
		//	ctm->~CTM();

		//	
		//	for (int i = 0; i < X_new.size(); i++) {
		//		for (int j = 0; j < X_new[i].size(); j++) {
		//			outFilePS << X_new[i][j] << ",";
		//		}
		//		outFilePS << "\n";
		//	}

		//	for (int i = 0; i < traveltime.size();i++) {
		//		for (int j = 0; j < traveltime[i].size(); j++) {
		//			outFileS<< traveltime[i][j]<< ",";
		//		}
		//		outFileS << "\n";
		//	}			

		//	return 0;
		//}
		//tmDs = tmD;
	}

	end = clock();   //结束时间
	std::cout << "time = " << double(end - start) / CLOCKS_PER_SEC << "s" << endl;  //输出时间（单位：ｓ）

	for (int i = 0; i < groupNum;i++) {
		for (int j = 0; j < passengerGroups[i]->paths.size();j++) {
			outFilePS << passengerGroups[i]->startPonit << "~" << passengerGroups[i]->endPonit << ":" << i << "_" << j << ",";
			outFilePLink << passengerGroups[i]->startPonit << "~" << passengerGroups[i]->endPonit << ":" << i << "_" << j << ","; outFileS;
			outFileS << passengerGroups[i]->startPonit << "~" << passengerGroups[i]->endPonit << ":" << i << "_" << j << ","; outFileS;
		}
	}
	outFilePS << "\n";
	outFilePLink << "\n";
	outFileS << "\n";
	for (int i = 0; i < groupNum; i++) {
		for (int j = 0; j < passengerGroups[i]->paths.size(); j++) {
			outFilePLink << ntl->links[passengerGroups[i]->paths[j].links[0]]->startNode->nodeID;
			for (int k = 0; k < passengerGroups[i]->paths[j].links.size(); k++) {
				int linkNo = passengerGroups[i]->paths[j].links[k];
				outFilePLink << "_" << ntl->links[linkNo]->endNode->nodeID;
			}
			outFilePLink << ",";
		}
	}

	for (int i = 0; i < X_new.size(); i++) {
		for (int j = 0; j < X_new[i].size(); j++) {
			outFilePS << X_new[i][j] << ",";
		}
		outFilePS << "\n";
	}

	for (int i = 0; i < traveltime.size(); i++) {
		for (int j = 0; j < traveltime[i].size(); j++) {
			outFileS << traveltime[i][j] << ",";
		}
		outFileS << "\n";
	}

	modePassenger << "bikePassengerNum," << "busPassengerNum," << "railPassengerNum" << "\n";
	for (int j = 0; j < ntl->allTravelModePassenger.size(); j++) {
		modePassenger << ntl->allTravelModePassenger[j].bikePassengerNum << "," << ntl->allTravelModePassenger[j].busPassengerNum << "," << ntl->allTravelModePassenger[j].railPassengerNum;
		modePassenger << "\n";
	}

	for (int gn = 0; gn < groupNum; gn++) {//清空h
		odModePassenger << passengerGroups[gn]->groupNo << "," << "," << ",";
	}
	odModePassenger << "\n";
	odModePassenger << "bikePassengerNum," << "busPassengerNum," << "railPassengerNum" << "\n";
	for (int j = 0; j < ntl->allODTravelModePassenger.size(); j++) {
		for (int gn = 0; gn < groupNum; gn++) {//清空h
			int tmGnn = passengerGroups[gn]->groupNo;
			if (ntl->allODTravelModePassenger[j].find(tmGnn) != ntl->allODTravelModePassenger[j].end()) {//找到了
				odModePassenger << ntl->allODTravelModePassenger[j][tmGnn].bikePassengerNum << "," << ntl->allODTravelModePassenger[j][tmGnn].busPassengerNum << "," << ntl->allODTravelModePassenger[j][tmGnn].railPassengerNum << ",";
			}
			else {
				odModePassenger << "0," << "0," << "0,";
			}
		}
		odModePassenger << "\n";
	}

	ntl->printLaRPassenger(50);
	ntl->~TransitNetworkLoading();


	

	return 0;



}
