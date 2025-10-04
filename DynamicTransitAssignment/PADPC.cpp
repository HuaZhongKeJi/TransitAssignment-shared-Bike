#include "stdafx.h"
#include "PADPC.h"

#include "myquadprog.h"

#pragma comment( lib, "myquadprog.lib" )

using namespace std;
using namespace Eigen;
using namespace std::chrono;

std::vector<double> myQserver(double* arrayF, std::vector<vector<double>> A1, std::vector<vector<double>> A2,
	std::vector<double> B1, std::vector<double> B2) {

	int Xn = A1[0].size();

	int elementCntH = Xn * Xn;
	double* arrayH = new double[elementCntH] { 1, -1, -1, 2 };
	int tmpN = 0;
	for (int i = 0; i < Xn; i++) {
		for (int j = 0; j < Xn; j++) {
			if (i == j) {
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

	int elementCntA = rowA * colA;
	double* arrayA = new double[elementCntA];
	int countA = 0;
	for (int i = 0; i < colA; i++) {
		for (int j = 0; j < rowA; j++) {
			arrayA[countA] = -1 * A2[j][i];
			countA++;
		}
	}
	mwArray A(rowA, colA, mxDOUBLE_CLASS);
	A.SetData(arrayA, elementCntA);


	int elementCntB = B2.size();
	double* arrayB = new double[elementCntB];
	for (int i = 0; i < elementCntB; i++) {
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
	for (int i = 0; i < Xn; i++) {
		arrayLB[i] = 0;
	}
	mwArray LB(Xn, 1, mxDOUBLE_CLASS);
	LB.SetData(arrayLB, elementCntLB);

	mwArray UB;
	mwArray X;
	mwArray Y;

	myquadprog(2, X, Y, H, F, A, B, AEQ, BEQ, LB, UB);

	std::vector<double> x_n;
	for (int i = 0; i < Xn; i++) {
		x_n.push_back(X.Get(1, i + 1));
	}
	return x_n;
}

/// <summary>
/// ����Rgap
/// </summary>
/// <param name="pathPassenger"></param>
/// <param name="pathTravelTime"></param>
/// <param name="passengerGroups"></param>
/// <param name="T"></param>
/// <param name="ntl"></param>
/// <param name="typeC"> 0�ǲ����ǹ�������gap��1�ǿ��ǹ�������gap </param>
/// <returns></returns> 
double calRgap(vector<vector<double>> pathPassenger, vector<vector<double>> pathTravelTime, vector<passengerOD*> passengerGroups, int T, TransitNetworkLoading* ntl, int typeC = 0) {//����gap
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

			//double testSump = 0;
			//double testShort = 0;

			for (int j = 0; j < passengerGroups[i]->paths.size(); j++) {

				pathType = passengerGroups[i]->paths[j].type;
				int pathBikeDock1 = passengerGroups[i]->paths[j].outShareBicyclePoint1;
				int pathBikeDock2 = passengerGroups[i]->paths[j].outShareBicyclePoint2;

				TpassengerTravelTime += pathTravelTime[t][tmNum] * pathPassenger[t][tmNum];
				//testSump += pathTravelTime[t][tmNum] * pathPassenger[t][tmNum];

				sumP += pathPassenger[t][tmNum];

				if (pathType == 1) {
					if (typeC == 1) {
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
	return gap / passengerShortestTime;
}


/// <summary>
/// ����Rgap
/// </summary>
/// <param name="pathPassenger"></param>
/// <param name="pathTravelTime"></param>
/// <param name="passengerGroups"></param>
/// <param name="T"></param>
/// <param name="ntl"></param>
/// <param name="typeC"> 0�ǲ����ǹ�������gap��1�ǿ��ǹ�������gap </param>
/// <returns></returns>
double calRgap2(vector<vector<double>> pathPassenger, vector<vector<double>> pathTravelTime, vector<passengerOD*> passengerGroups, int T, TransitNetworkLoading* ntl) {//����gap
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

int PADPC::mainPADPC() {

	//���� ********************************
	double tmGap = 0;
	myquadprogInitialize();

	int L = 100;//ѭ����

	clock_t start, end;//����clock_t����
	start = clock();//��ʼʱ��

	//cout << "result is " << Y.Get(1,1) << endl;

	////�ر�myadd.lib
	//myquadprogTerminate();
	////�ر�mcr
	//mclTerminateApplication();


	//����

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
	//double epsilonL = 0.0005;//��ѭ�� �� ������׼
	double epsilonL = 0.001;//��ѭ�� �� ������׼
	double epsilon = 0.0005;//���ι滮�� ������׼
	double tau = 0.3;//����Ȩ��	


	static const int TYPE = 6;//�����


	int path_tpye_1 = 1;//1  share bicycle
	int path_tpye_2 = 2;//2  bus
	int path_tpye_3 = 3;//3  subway-bicycle
	int path_tpye_4 = 4;//4  subway-bus
	int path_tpye_5 = 5;//5  bicycle-bus
	int path_tpye_6 = 6;//6  subway
	std::vector<passengerOD*> passengerGroups;
	std::vector<std::vector<std::string>> allPassegnerPath;

	//----------------------------------------------------------------------------���û�������-----------------------------------------------------------
	bool containsDPC = false;
	bool considerBikeSharing = true;

	static const int T = JiaxingData::T;//ʱ��
	static const int groupNum = JiaxingData::groupNum;
	static const int bicyclePointNum = JiaxingData::bicyclePointNum;
	JiaxingData::setOD(passengerGroups, allPassegnerPath);	
	JiaxingData::setNetwork(ntl, considerBikeSharing);

	//static const int T = BostonData::T;//ʱ��
	//static const int groupNum = BostonData::groupNum;
	//static const int bicyclePointNum = BostonData::bicyclePointNum;
	//BostonData::setOD(passengerGroups, allPassegnerPath);
	//BostonData::setNetwork(ntl, considerBikeSharing);
	//----------------------------------------------------------------------------------------------------------------------------------------------------



	//��ʼ�������˿ͷ��䵽��̵ĵ�·��ȥ
	//Ѱ�ҵ�ǰʱ�ε����·�������˿ͷ��������·��
	for (int t = 0; t < T; t++) {//����ʱ��
		for (int passenger_no = 0; passenger_no < groupNum; passenger_no++) {//�����˿�
			passengerOD* passenger_group = passengerGroups[passenger_no];
			int pathType1Cost = 99999, pathType2Cost = 99999, pathType3Cost = 99999, pathType4Cost = 99999, pathType5Cost = 99999, pathType6Cost = 99999;//���Ƴ�ʼ������
			int pathType1No = -1, pathType2No = -1, pathType3No = -1, pathType4No = -1, pathType5No = -1, pathType6No = -1;

			vector<double> shareBicycleC(ntl->sharedBikeNode.size());//tʱ�̹�����ͣ���������	
			for (int i = 0; i < ntl->sharedBikeNode.size(); i++) {
				shareBicycleC[i] = ntl->getBicyclePointByBS(i);
			}

			//����˿�
			for (int path_no = 0; path_no < passenger_group->paths.size(); path_no++) {//��������·�����������·

				passengerOD::passengerPath passenger_path = passenger_group->paths[path_no];

				int path_cost = (int)ntl->getIniPathTime(passenger_path.links);//����·��ʱ��

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

			//����˿�
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

			double passengerLeft = 0;//ʣ��ĳ˿�����
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
			//���³˿�����
			if (pathType1No != -1) {

				needAssignPassenger = passenger_group->passengerFlow[t] * std::exp(-1 * theta * pathType1Cost) / sumExp;
				assignPassenger = needAssignPassenger;
				if (passenger_group->paths[pathType1No].outShareBicyclePoint1 != -1 && assignPassenger > shareBicycleC[ntl->getShareBicycleSeq(passenger_group->paths[pathType1No].outShareBicyclePoint1)]) {//���outShareBicyclePoint1�ĳ�����С����Ҫ�ĳ�����
					assignPassenger = shareBicycleC[ntl->getShareBicycleSeq(passenger_group->paths[pathType1No].outShareBicyclePoint1)];
				}
				if (passenger_group->paths[pathType1No].outShareBicyclePoint2 != -1 && assignPassenger > shareBicycleC[ntl->getShareBicycleSeq(passenger_group->paths[pathType1No].outShareBicyclePoint2)]) {//���outShareBicyclePoint1�ĳ�����С����Ҫ�ĳ�����
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
				if (passenger_group->paths[pathType3No].outShareBicyclePoint1 != -1 && assignPassenger > shareBicycleC[ntl->getShareBicycleSeq(passenger_group->paths[pathType3No].outShareBicyclePoint1)]) {//���outShareBicyclePoint1�ĳ�����С����Ҫ�ĳ�����
					assignPassenger = shareBicycleC[ntl->getShareBicycleSeq(passenger_group->paths[pathType3No].outShareBicyclePoint1)];
				}
				if (passenger_group->paths[pathType3No].outShareBicyclePoint2 != -1 && assignPassenger > shareBicycleC[ntl->getShareBicycleSeq(passenger_group->paths[pathType3No].outShareBicyclePoint2)]) {//���outShareBicyclePoint1�ĳ�����С����Ҫ�ĳ�����
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
				if (passenger_group->paths[pathType5No].outShareBicyclePoint1 != -1 && assignPassenger > shareBicycleC[ntl->getShareBicycleSeq(passenger_group->paths[pathType5No].outShareBicyclePoint1)]) {//���outShareBicyclePoint1�ĳ�����С����Ҫ�ĳ�����
					assignPassenger = shareBicycleC[ntl->getShareBicycleSeq(passenger_group->paths[pathType5No].outShareBicyclePoint1)];
				}
				if (passenger_group->paths[pathType5No].outShareBicyclePoint2 != -1 && assignPassenger > shareBicycleC[ntl->getShareBicycleSeq(passenger_group->paths[pathType5No].outShareBicyclePoint2)]) {//���outShareBicyclePoint1�ĳ�����С����Ҫ�ĳ�����
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

			//Ҫ������������������й�������ģʽ����������ת��û�й���������·
			if (passengerLeft != 0) {
				//2��4û����������
				if (pathType2No != -1 && pathType4No != -1 && pathType6No != -1) {
					passenger_group->paths[pathType2No].passengerNum[t] += passengerLeft / 3;
					passenger_group->paths[pathType4No].passengerNum[t] += passengerLeft / 3;
					passenger_group->paths[pathType6No].passengerNum[t] += passengerLeft / 3;
				}
				else if (pathType2No != -1 && pathType4No != -1) {
					passenger_group->paths[pathType2No].passengerNum[t] += passengerLeft / 2;
					passenger_group->paths[pathType4No].passengerNum[t] += passengerLeft / 2;
				}
				else if (pathType6No != -1 && pathType4No != -1) {
					passenger_group->paths[pathType6No].passengerNum[t] += passengerLeft / 2;
					passenger_group->paths[pathType4No].passengerNum[t] += passengerLeft / 2;
				}
				else if (pathType2No != -1 && pathType6No != -1) {
					passenger_group->paths[pathType2No].passengerNum[t] += passengerLeft / 2;
					passenger_group->paths[pathType6No].passengerNum[t] += passengerLeft / 2;
				}
				else if (pathType4No != -1) {
					passenger_group->paths[pathType4No].passengerNum[t] += passengerLeft;
				}
				else if (pathType2No != -1) {
					passenger_group->paths[pathType2No].passengerNum[t] += passengerLeft;
				}
				else if (pathType6No != -1) {
					passenger_group->paths[pathType6No].passengerNum[t] += passengerLeft;
				}

			}

			//������·�Ƿ�ʹ�ã���Ҫ������Ӫʱ�䣬��������Ϊ��һ���ļ����ṩ����
			for (int pathno = 0; pathno < passenger_group->paths.size(); pathno++) {
				ntl->addPassenger(passenger_no, pathno,
					passenger_group->paths[pathno].links, passenger_group->paths[pathno].passengerNum[t]);
			}

		}
		ntl->simulation();
	}

	ntl->conSim2End();



	std::vector<vector<double>> alpha;//�����ʵ��A1��ת��
	int sizeAlpha;
	std::vector<vector<double>> traveltime;//��һ��������ʱ�䣬�ڶ�������·
	double hmr[groupNum][TYPE];

	std::vector<vector<double>> A1;
	std::vector<vector<double>> A2;
	std::vector<vector<double>> A3;

	std::vector<double> B1;
	std::vector<double> B2;
	std::vector<double> B3;
	std::vector<double> Bbar;



	std::vector<vector<double>> M;
	std::vector<vector<double>> Mbar;//�洢δ����Լ��

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

	//����
	vector<vector<double>> X_old;
	vector<vector<double>> X_new;

	high_resolution_clock::time_point t1 = high_resolution_clock::now();

	while (L--) {

		//std::sort(supMap->passengerFlowsFinish.begin(), supMap->passengerFlowsFinish.end(), passengerFlowComparer);

		std::vector<vector<double>> oldTravelTime = traveltime;

		//�˿�·�ߵ� running + waitting time		
		if (traveltime.size() > 0) {
			/*double Rgap = calRgap(X_new, traveltime, passengerGroups, T, ctm);
			std::cout << "Rgap: " << Rgap << "\n";*/
			traveltime.clear();
		}

		std::vector<int> oldPathNum;
		for (int gn = 0; gn < groupNum; gn++) {
			oldPathNum.push_back(passengerGroups[gn]->paths.size());
		}

		std::map<std::string, int> newPathMap;
		std::vector<std::vector<double>> travelCostN;
		int routeNo = 0;

		high_resolution_clock::time_point tPathAdd1 = high_resolution_clock::now();
		//�Ͻ�˹����������·===========================================================================================================================
		for (int t = 0; t < T; t++) {
			for (int gn = 0; gn < groupNum; gn++) {//���h
				std::map<int, double> pathTypeMinCost;
				pathTypeMinCost[1] = 9999;
				pathTypeMinCost[2] = 9999;
				pathTypeMinCost[3] = 9999;
				pathTypeMinCost[4] = 9999;
				pathTypeMinCost[5] = 9999;
				pathTypeMinCost[6] = 9999;
				double minPathCost = 9999;

				for (int pn = 0; pn < passengerGroups[gn]->paths.size(); pn++) {
					double travelFee = ntl->calVehTravelTime(gn, pn, t);
					int pathType = passengerGroups[gn]->paths[pn].type;
					if (pathTypeMinCost[pathType] > travelFee) {
						pathTypeMinCost[pathType] = travelFee;
					}
					if (minPathCost > travelFee) {
						minPathCost = travelFee;
					}
				}
				/********************************���·��*********************************/
				//��ѯ���·��				
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

				if (path.size() > 0 && travelCost < pathTypeMinCost[1] && travelCost < (minPathCost + 10) && std::find(allPassegnerPath[gn].begin(), allPassegnerPath[gn].end(), pathIdf) == allPassegnerPath[gn].end()) {//����������Ҫ���path
					passengerGroups[gn]->addPassengerPath(passengerGroups[gn]->paths.size(), 1, path, outShareBicyclePoint1
						, inShareBicyclePoint1, outShareBicyclePoint2, inShareBicyclePoint2);
					allPassegnerPath[gn].push_back(pathIdf);
					newPathMap[pathIdf] = routeNo;
					routeNo++;
					std::vector<double> pathIniTravelC;
					for (int tt = 0; tt < T; tt++) {
						passengerGroups[gn]->paths[passengerGroups[gn]->paths.size() - 1].passengerNum[tt] = 0;//������·�����еĳ˿�
						pathIniTravelC.push_back(9999);
					}
					travelCostN.push_back(pathIniTravelC);

					if (newPathMap.find(pathIdf) != newPathMap.end()) {//����������·
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
				if (path.size() > 0 && travelCost < pathTypeMinCost[2] && travelCost < (minPathCost + 10) && std::find(allPassegnerPath[gn].begin(), allPassegnerPath[gn].end(), pathIdf) == allPassegnerPath[gn].end()) {//����������Ҫ���path
					passengerGroups[gn]->addPassengerPath(passengerGroups[gn]->paths.size(), 2, path, outShareBicyclePoint1
						, inShareBicyclePoint1, outShareBicyclePoint2, inShareBicyclePoint2);
					allPassegnerPath[gn].push_back(pathIdf);
					newPathMap[pathIdf] = routeNo;
					routeNo++;
					std::vector<double> pathIniTravelC;
					for (int tt = 0; tt < T; tt++) {
						passengerGroups[gn]->paths[passengerGroups[gn]->paths.size() - 1].passengerNum[tt] = 0;//������·�����еĳ˿�
						pathIniTravelC.push_back(9999);
					}
					travelCostN.push_back(pathIniTravelC);

					if (newPathMap.find(pathIdf) != newPathMap.end()) {//����������·
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
				if (path.size() > 0 && travelCost < pathTypeMinCost[3] && travelCost < (minPathCost + 10) && std::find(allPassegnerPath[gn].begin(), allPassegnerPath[gn].end(), pathIdf) == allPassegnerPath[gn].end()) {//����������Ҫ���path
					passengerGroups[gn]->addPassengerPath(passengerGroups[gn]->paths.size(), 3, path, outShareBicyclePoint1
						, inShareBicyclePoint1, outShareBicyclePoint2, inShareBicyclePoint2);
					allPassegnerPath[gn].push_back(pathIdf);
					newPathMap[pathIdf] = routeNo;
					routeNo++;
					std::vector<double> pathIniTravelC;
					for (int tt = 0; tt < T; tt++) {
						passengerGroups[gn]->paths[passengerGroups[gn]->paths.size() - 1].passengerNum[tt] = 0;//������·�����еĳ˿�
						pathIniTravelC.push_back(9999);
					}
					travelCostN.push_back(pathIniTravelC);

					if (newPathMap.find(pathIdf) != newPathMap.end()) {//����������·
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
				if (path.size() > 0 && travelCost < pathTypeMinCost[4] && travelCost < (minPathCost + 10) && std::find(allPassegnerPath[gn].begin(), allPassegnerPath[gn].end(), pathIdf) == allPassegnerPath[gn].end()) {//����������Ҫ���path
					passengerGroups[gn]->addPassengerPath(passengerGroups[gn]->paths.size(), 4, path, outShareBicyclePoint1
						, inShareBicyclePoint1, outShareBicyclePoint2, inShareBicyclePoint2);
					allPassegnerPath[gn].push_back(pathIdf);
					newPathMap[pathIdf] = routeNo;
					routeNo++;
					std::vector<double> pathIniTravelC;
					for (int tt = 0; tt < T; tt++) {
						passengerGroups[gn]->paths[passengerGroups[gn]->paths.size() - 1].passengerNum[tt] = 0;//������·�����еĳ˿�
						pathIniTravelC.push_back(9999);
					}
					travelCostN.push_back(pathIniTravelC);

					if (newPathMap.find(pathIdf) != newPathMap.end()) {//����������·
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
				if (path.size() > 0 && travelCost < pathTypeMinCost[5] && travelCost < (minPathCost + 10) && std::find(allPassegnerPath[gn].begin(), allPassegnerPath[gn].end(), pathIdf) == allPassegnerPath[gn].end()) {//����������Ҫ���path
					passengerGroups[gn]->addPassengerPath(passengerGroups[gn]->paths.size(), 5, path, outShareBicyclePoint1
						, inShareBicyclePoint1, outShareBicyclePoint2, inShareBicyclePoint2);
					allPassegnerPath[gn].push_back(pathIdf);
					newPathMap[pathIdf] = routeNo;
					routeNo++;
					std::vector<double> pathIniTravelC;
					for (int tt = 0; tt < T; tt++) {
						passengerGroups[gn]->paths[passengerGroups[gn]->paths.size() - 1].passengerNum[tt] = 0;//������·�����еĳ˿�
						pathIniTravelC.push_back(9999);
					}
					travelCostN.push_back(pathIniTravelC);

					if (newPathMap.find(pathIdf) != newPathMap.end()) {//����������·
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
				if (path.size() > 0 && travelCost < pathTypeMinCost[6] && travelCost < (minPathCost + 10) && std::find(allPassegnerPath[gn].begin(), allPassegnerPath[gn].end(), pathIdf) == allPassegnerPath[gn].end()) {//����������Ҫ���path
					passengerGroups[gn]->addPassengerPath(passengerGroups[gn]->paths.size(), 6, path, outShareBicyclePoint1
						, inShareBicyclePoint1, outShareBicyclePoint2, inShareBicyclePoint2);
					allPassegnerPath[gn].push_back(pathIdf);
					newPathMap[pathIdf] = routeNo;
					routeNo++;
					std::vector<double> pathIniTravelC;
					for (int tt = 0; tt < T; tt++) {
						passengerGroups[gn]->paths[passengerGroups[gn]->paths.size() - 1].passengerNum[tt] = 0;//������·�����еĳ˿�
						pathIniTravelC.push_back(9999);
					}
					travelCostN.push_back(pathIniTravelC);

					if (newPathMap.find(pathIdf) != newPathMap.end()) {//����������·
						travelCostN[newPathMap[pathIdf]][t] = travelCost;
					}
				}


				pathIdf = "";
				typeList.clear();
				path.clear();

			}
		}
		high_resolution_clock::time_point tPathAdd2 = high_resolution_clock::now();
		duration<double, std::milli> time_pathAdd = tPathAdd2 - tPathAdd1;
		//===========================================================================================================================



		//������·����ʱ��===========================================================================================================================
		for (int t = 0; t < T; t++) {
			vector<double> pathsTraveltime;
			for (int gn = 0; gn < groupNum; gn++) {//���h				

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
		for (int t = 0; t < T; t++) {//��ʱ�����
			vector<double> tmX_new;
			for (int gn = 0; gn < groupNum; gn++) {//���h
				for (auto r : passengerGroups[gn]->paths) {
					tmX_new.push_back(r.passengerNum[t]);
				}
			}

			X_new.push_back(tmX_new);
		}

		double Rgap = calRgap2(X_new, traveltime, passengerGroups, T, ntl);
		//double Rgap = calRgap(X_new, traveltime, passengerGroups, T, ntl,0);

		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		duration<double, std::milli> time_span = t2 - t1;
		//std::cout << "It took " << std::fixed << time_span.count() << " milliseconds." << std::endl;

		std::cout << "Rgap: " << Rgap << ", L: " << L << "\n";
		diedai << Rgap << "," << time_span.count() <<",timeAdd,"<< time_pathAdd.count() ;

		//ntl->allODTravelModePassenger;
		//ntl->allTravelModePassenger;


		//��ѭ����ֹ����===================================================================================================================================================
		//if (Rgap < epsilonL ) {
		if (Rgap < -1) {

			std::cout << "������ɣ��������� ���� 1";

			for (int i = 0; i < groupNum; i++) {
				for (int j = 0; j < passengerGroups[i]->paths.size(); j++) {
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

			for (int gn = 0; gn < groupNum; gn++) {//���h
				odModePassenger << passengerGroups[gn]->groupNo << "," << "," << ",";
			}
			odModePassenger << "\n";
			odModePassenger << "bikePassengerNum," << "busPassengerNum," << "railPassengerNum" << "\n";
			for (int j = 0; j < ntl->allODTravelModePassenger.size(); j++) {
				for (int gn = 0; gn < groupNum; gn++) {//���h
					int tmGnn = passengerGroups[gn]->groupNo;
					if (ntl->allODTravelModePassenger[j].find(tmGnn) != ntl->allODTravelModePassenger[j].end()) {//�ҵ���
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

		//��ӡ����ʱ��Ĳ���
		//if (oldTravelTime.size() >0) {
		//	for (int t = 0; t < T; t++) {//��ʱ�����
		//		vector<double> pathsTraveltime;
		//		for (int j = 0; j < sizeAlpha; j++) {//���h
		//			cout << traveltime[t][j] - oldTravelTime[t][j] << " | ";
		//		}
		//	}
		//	cout << "\n";
		//}

		//��ӡ������ͣ��������
		/*for (int i = 0; i < ctm->bicyclePoints.size(); i++) {
			std::cout << ctm->bicyclePoints[i] << " : " << ctm->nodes[ctm->bicyclePoints[i]]->bicycleNum << " || ";
		}
		std::cout << "\n";*/


		ntl->clearNetwork();

		double timeProjection = 0;
		double timeDNL = 0;

		for (int t = 0; t < T; t++) {//��ʱ�����**************************************************************************************************************************************

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




			//����ģ��Լ��
			for (int pn = 0; pn < sizeAlpha; pn++) {
				dmkr[pn] = 0;
			}

			for (int gn = 0; gn < groupNum; gn++) {//���h
				for (int type = 0; type < TYPE; type++) {
					hmr[gn][type] = -1;
				}
			}

			int nowsize = 0;
			for (int gn = 0; gn < groupNum; gn++) {//���h
				for (auto r : passengerGroups[gn]->paths) {
					if (hmr[gn][r.type - 1] == -1) {//��Ҫ����h
						hmr[gn][r.type - 1] = 0;
						for (auto tr : passengerGroups[gn]->paths) {
							if (r.type == tr.type) {
								hmr[gn][r.type - 1] += tr.passengerNum[t];
							}
						}
					}
					if (hmr[gn][r.type - 1] < 0.1) {
						hmr[gn][r.type - 1] = 0.1;//��û�г˿͵�·�����ÿ���
					}
					X_o.push_back(r.passengerNum[t]);
					if (!containsDPC) {
						dmkr[nowsize] = 2 * (-1 * r.passengerNum[t] + tau * (traveltime[t][nowsize] + std::log(hmr[gn][r.type - 1]) / theta));
					}

					nowsize++;
				}
			}

			//��Լ��
			for (auto p : passengerGroups) {
				B1.push_back(p->passengerFlow[t]);
			}

			//��������
			for (int i = 0; i < bicyclePointNum; i++) {
				B2.push_back(-1 * ntl->getBicyclePointByBS(i));
				if (B2[i] > -0.0000001) {
					B2[i] = 0;
				}
			}

			for (int i = 0; i < sizeAlpha; i++) {
				B3.push_back(0);
			}

			//*********************************************************************************************
		//���� t>0 �������Ҫ��X_o���������н⣬���巽�������ڹ����������ı仯�����ܵ��¹��������������㣬���ǽ�Ϊ�������������������Ҫ������·���³���ʱ����̵���·��ȥ
			if (t > 0) {

				for (int i = 0; i < B2.size(); i++) {
					double tmS = 0;
					for (int j = 0; j < sizeAlpha; j++) {
						tmS += A2[i][j] * X_o[j];
					}
					if (tmS - B2[i] < -0.0000001) {//��ʾ������ͣ���� i ������Լ��
						tmS = B2[i] - tmS;//�����ֵ
						for (int k = 0; k < sizeAlpha && tmS > 0.00000001; k++) {
							if (A2[i][k] == -1 && X_o[k] > 0) {//��ʾ�� k ��·����Ҫ��������
								int sumPathno = 0;
								for (int l = 0; l < groupNum && tmS > 0.00000001; l++) {//�ж���һ��od�õ��� k ·��
									if (A1[l][k] == 1) {//��ʾod l �õ�����·k�������Ҫ��od l�е� k��·�������е���
										int minpathNo;
										double pathTravelT = 999999;
										for (int gp = 0; gp < passengerGroups[l]->paths.size(); gp++) {
											if (passengerGroups[l]->paths[gp].type == path_tpye_2 || passengerGroups[l]->paths[gp].type == path_tpye_4) {//ɸѡû�й���������·
												if (traveltime[t][sumPathno + gp] < pathTravelT) {//�ҳ��ɱ���С��·
													minpathNo = gp;
													pathTravelT = traveltime[t][sumPathno + gp];
												}
											}
										}
										//��ʼ������
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
			if (containsDPC && (std::abs(tmGap - Rgap) > 0.02 || L >= 98)) {


				//****************************************************���浱ǰ�ɱ�********************************************************************************************
				TransitNetworkLoading* ntl_new = new TransitNetworkLoading();

				//-------------------------------------------------------------------���û�������----------------------------------------
				JiaxingData::setNetwork(ntl_new, considerBikeSharing);
				//BostonData::setNetwork(ntl_new, considerBikeSharing);
				//------------------------------------------------------------------------------------------------------------------------

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
						for (int gn = 0; gn < groupNum; gn++) {//���h
							for (auto r : passengerGroups[gn]->paths) {
								new_X.push_back(r.passengerNum[t + simS]);
								nowsize++;
							}
						}

						std::vector<double> new_B2;
						//��������
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
								if (tmS - new_B2[i] < -0.0000001) {//��ʾ������ͣ���� i ������Լ��
									tmS = new_B2[i] - tmS;//�����ֵ
									for (int k = 0; k < sizeAlpha && tmS > 0.00000001; k++) {
										if (A2[i][k] == -1 && new_X[k] > 0) {//��ʾ�� k ��·����Ҫ��������
											int sumPathno = 0;
											for (int l = 0; l < groupNum && tmS > 0.00000001; l++) {//�ж���һ��od�õ��� k ·��
												if (A1[l][k] == 1) {//��ʾod l �õ�����·k�������Ҫ��od l�е� k��·�������е���
													int minpathNo;
													double pathTravelT = 999999;
													for (int gp = 0; gp < passengerGroups[l]->paths.size(); gp++) {
														if (passengerGroups[l]->paths[gp].type == path_tpye_2 || passengerGroups[l]->paths[gp].type == path_tpye_4) {//ɸѡû�й���������·
															if (traveltime[t][sumPathno + gp] < pathTravelT) {//�ҳ��ɱ���С��·
																minpathNo = gp;
																pathTravelT = traveltime[t][sumPathno + gp];
															}
														}
													}
													//��ʼ������
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
				for (int gn = 0; gn < groupNum; gn++) {//���h	
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
				for (int gn = 0; gn < groupNum; gn++) {//���h
					for (auto r : passengerGroups[gn]->paths) {
						dmkr[nowsize] = 2 * (-1 * X_o[nowsize] + tau * (pathsTraveltime[nowsize] + std::log(hmr[gn][r.type - 1]) / theta));
						nowsize++;
					}
				}
			}
			else {
				nowsize = 0;
				for (int gn = 0; gn < groupNum; gn++) {//���h
					for (auto r : passengerGroups[gn]->paths) {
						dmkr[nowsize] = 2 * (-1 * X_o[nowsize] + tau * (traveltime[t][nowsize] + std::log(hmr[gn][r.type - 1]) / theta));
						nowsize++;
					}
				}
			}
			//!DPC==========================================================================================================================================

			tmGap = Rgap;

			//****************************************************************************************************

			//nowsize = 0;
			//for (int gn = 0; gn < groupNum; gn++) {//���h
			//	for (auto r : passengerGroups[gn]->paths) {
			//		dmkr[nowsize] = 2 * (-1 * X_o[nowsize] + tau * (traveltime[t][nowsize] + std::log(hmr[gn][r.type - 1]) / theta));
			//		nowsize++;
			//	}
			//}

			high_resolution_clock::time_point tProjection1 = high_resolution_clock::now();
			std::vector<double> new_x = myQserver(dmkr, A1, A2, B1, B2);
			high_resolution_clock::time_point tProjection2 = high_resolution_clock::now();
			duration<double, std::milli> time_spanProjection = tProjection2 - tProjection1;
			timeProjection += time_spanProjection.count();

			for (int i = 0; i < new_x.size(); i++) {
				X_o[i] = new_x[i];
			}



			//****************************************************************************************��������*********************************************************************************
			nowsize = 0;
			for (int i = 0; i < passengerGroups.size(); i++) {
				for (int j = 0; j < passengerGroups[i]->paths.size(); j++) {

					passengerGroups[i]->paths[j].passengerNum[t] = X_o[nowsize];
					ntl->addPassenger(passengerGroups[i]->groupNo, passengerGroups[i]->paths[j].no, passengerGroups[i]->paths[j].links, passengerGroups[i]->paths[j].passengerNum[t]);
					nowsize++;

				}
			}

			high_resolution_clock::time_point tDNL1 = high_resolution_clock::now();
			ntl->simulation();
			high_resolution_clock::time_point tDNL2 = high_resolution_clock::now();
			duration<double, std::milli> time_spanDNL = tDNL2 - tDNL1;
			timeDNL += time_spanDNL.count();

			X_old.push_back(X_o);

		}

		high_resolution_clock::time_point tDNL3 = high_resolution_clock::now();
		ntl->conSim2End();
		high_resolution_clock::time_point tDNL4 = high_resolution_clock::now();
		duration<double, std::milli> time_spanDNL2 = tDNL3 - tDNL4;
		timeDNL += time_spanDNL2.count();

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

		diedai << ",timeProjection," << timeProjection << ",timeDNL," << timeDNL << "\n";

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

		////��ѭ����ֹ����
		//if (tmD < epsilonL && 1>2) {
		////if (tmD < epsilonL || tmDs == tmD) {
		//	if (tmD < epsilonL) {
		//		std::cout << "������ɣ��������� ���� 1";
		//	}
		//	else {
		//		std::cout << "������ɣ��������� ���� 2";
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

	end = clock();   //����ʱ��
	std::cout << "time = " << double(end - start) / CLOCKS_PER_SEC << "s" << endl;  //���ʱ�䣨��λ����

	for (int i = 0; i < groupNum; i++) {
		for (int j = 0; j < passengerGroups[i]->paths.size(); j++) {
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

	for (int gn = 0; gn < groupNum; gn++) {//���h
		odModePassenger << passengerGroups[gn]->groupNo << "," << "," << ",";
	}
	odModePassenger << "\n";
	odModePassenger << "bikePassengerNum," << "busPassengerNum," << "railPassengerNum" << "\n";
	for (int j = 0; j < ntl->allODTravelModePassenger.size(); j++) {
		for (int gn = 0; gn < groupNum; gn++) {//���h
			int tmGnn = passengerGroups[gn]->groupNo;
			if (ntl->allODTravelModePassenger[j].find(tmGnn) != ntl->allODTravelModePassenger[j].end()) {//�ҵ���
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

PADPC::PADPC()
{
}
