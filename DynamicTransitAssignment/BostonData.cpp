#include "stdafx.h"
#include "BostonData.h"
void BostonData::setOD(std::vector<passengerOD*>& passengerGroups, std::vector<std::vector<std::string>>& allPassegnerPath) {
	//²åÈëOD*********************************************************************
	int ptNo = 0;
	//²åÈë³Ë¿ÍOD****************************************************
	passengerOD* od1 = new passengerOD(ptNo, 1, 0);

	double* pd1 = new double[T];
	pd1[0] = 1;
	pd1[1] = 2;
	pd1[2] = 5;
	pd1[3] = 5;
	pd1[4] = 7;
	pd1[5] = 7;
	pd1[6] = 8;
	pd1[7] = 11;
	pd1[8] = 13;
	pd1[9] = 13;
	pd1[10] = 13;
	pd1[11] = 14;
	pd1[12] = 15;
	pd1[13] = 15;
	pd1[14] = 16;
	pd1[15] = 15;
	pd1[16] = 14;
	pd1[17] = 13;
	pd1[18] = 13;
	pd1[19] = 13;
	pd1[20] = 12;
	pd1[21] = 11;
	pd1[22] = 9;
	pd1[23] = 8;
	pd1[24] = 7;
	pd1[25] = 5;
	pd1[26] = 5;
	pd1[27] = 4;
	pd1[28] = 1;
	pd1[29] = 1;
	od1->setPassengerFlow(pd1);

	std::vector<int> path11;
	path11.push_back(9);
	path11.push_back(99);
	path11.push_back(38);
	od1->addPassengerPath(0, 6, path11);
	passengerGroups.push_back(od1);

	std::vector<std::string> ph1;
	ph1.push_back("9|99|38|");
	allPassegnerPath.push_back(ph1);
	ptNo++;
	//*****************************************************************************
	passengerOD* od2 = new passengerOD(ptNo, 2, 0);

	double* pd2 = new double[T];
	pd2[0] = 1;
	pd2[1] = 2;
	pd2[2] = 3;
	pd2[3] = 5;
	pd2[4] = 6;
	pd2[5] = 7;
	pd2[6] = 9;
	pd2[7] = 9;
	pd2[8] = 9;
	pd2[9] = 10;
	pd2[10] = 11;
	pd2[11] = 14;
	pd2[12] = 15;
	pd2[13] = 15;
	pd2[14] = 14;
	pd2[15] = 13;
	pd2[16] = 15;
	pd2[17] = 12;
	pd2[18] = 14;
	pd2[19] = 12;
	pd2[20] = 13;
	pd2[21] = 9;
	pd2[22] = 12;
	pd2[23] = 8;
	pd2[24] = 6;
	pd2[25] = 7;
	pd2[26] = 5;
	pd2[27] = 3;
	pd2[28] = 2;
	pd2[29] = 1;
	od2->setPassengerFlow(pd2);

	//4-59-58-57-56-55-1
	std::vector<int> path21;
	path21.push_back(13);
	path21.push_back(100);
	path21.push_back(99);
	path21.push_back(38);
	od2->addPassengerPath(0, 6, path21);
	passengerGroups.push_back(od2);

	std::vector<std::string> ph2;
	ph2.push_back("13|100|99|38|");
	allPassegnerPath.push_back(ph2);
	ptNo++;
	//*****************************************************************************

	passengerOD* od3 = new passengerOD(ptNo, 3, 0);

	double* pd3 = new double[T];
	pd3[0] = 1;
	pd3[1] = 2;
	pd3[2] = 4;
	pd3[3] = 3;
	pd3[4] = 5;
	pd3[5] = 5;
	pd3[6] = 6;
	pd3[7] = 7;
	pd3[8] = 7;
	pd3[9] = 8;
	pd3[10] = 11;
	pd3[11] = 11;
	pd3[12] = 13;
	pd3[13] = 14;
	pd3[14] = 16;
	pd3[15] = 15;
	pd3[16] = 13;
	pd3[17] = 12;
	pd3[18] = 11;
	pd3[19] = 9;
	pd3[20] = 8;
	pd3[21] = 7;
	pd3[22] = 6;
	pd3[23] = 6;
	pd3[24] = 6;
	pd3[25] = 4;
	pd3[26] = 2;
	pd3[27] = 3;
	pd3[28] = 1;
	pd3[29] = 1;
	od3->setPassengerFlow(pd3);

	//5-58-57-56-55-1
	std::vector<int> path31;
	path31.push_back(17);
	path31.push_back(101);
	path31.push_back(100);
	path31.push_back(99);
	path31.push_back(38);
	od3->addPassengerPath(0, 6, path31);
	passengerGroups.push_back(od3);

	std::vector<std::string> ph3;
	ph3.push_back("17|101|100|99|38|");
	allPassegnerPath.push_back(ph3);
	ptNo++;
	//*****************************************************************************

	passengerOD* od4 = new passengerOD(ptNo, 4, 0);

	double* pd4 = new double[T];
	pd4[0] = 2;
	pd4[1] = 3;
	pd4[2] = 4;
	pd4[3] = 5;
	pd4[4] = 6;
	pd4[5] = 6;
	pd4[6] = 9;
	pd4[7] = 11;
	pd4[8] = 13;
	pd4[9] = 13;
	pd4[10] = 15;
	pd4[11] = 16;
	pd4[12] = 12;
	pd4[13] = 16;
	pd4[14] = 14;
	pd4[15] = 14;
	pd4[16] = 15;
	pd4[17] = 15;
	pd4[18] = 14;
	pd4[19] = 13;
	pd4[20] = 13;
	pd4[21] = 11;
	pd4[22] = 10;
	pd4[23] = 8;
	pd4[24] = 6;
	pd4[25] = 5;
	pd4[26] = 5;
	pd4[27] = 2;
	pd4[28] = 4;
	pd4[29] = 1;
	od4->setPassengerFlow(pd4);

	//9-63-62-61-1
	std::vector<int> path41;
	path41.push_back(22);
	path41.push_back(102);
	path41.push_back(101);
	path41.push_back(100);
	path41.push_back(99);
	path41.push_back(38);
	od4->addPassengerPath(0, 6, path41);
	passengerGroups.push_back(od4);

	std::vector<std::string> ph4;
	ph4.push_back("22|102|101|100|99|38|");
	allPassegnerPath.push_back(ph4);
	ptNo++;
	//*****************************************************************************

	passengerOD* od5 = new passengerOD(ptNo, 5, 0);

	double* pd5 = new double[T];
	pd5[0] = 2;
	pd5[1] = 3;
	pd5[2] = 4;
	pd5[3] = 7;
	pd5[4] = 8;
	pd5[5] = 9;
	pd5[6] = 9;
	pd5[7] = 10;
	pd5[8] = 10;
	pd5[9] = 11;
	pd5[10] = 12;
	pd5[11] = 13;
	pd5[12] = 14;
	pd5[13] = 16;
	pd5[14] = 15;
	pd5[15] = 15;
	pd5[16] = 14;
	pd5[17] = 13;
	pd5[18] = 12;
	pd5[19] = 10;
	pd5[20] = 9;
	pd5[21] = 9;
	pd5[22] = 8;
	pd5[23] = 8;
	pd5[24] = 7;
	pd5[25] = 5;
	pd5[26] = 1;
	pd5[27] = 1;
	pd5[28] = 3;
	pd5[29] = 2;
	od5->setPassengerFlow(pd5);

	//13-65-64-63-62-61-1
	std::vector<int> path51;
	path51.push_back(25);
	path51.push_back(128);
	path51.push_back(127);
	path51.push_back(42);
	od5->addPassengerPath(0, 2, path51);
	passengerGroups.push_back(od5);

	std::vector<std::string> ph5;
	ph5.push_back("25|128|127|42|");
	allPassegnerPath.push_back(ph5);
	ptNo++;
	//*****************************************************************************

	passengerOD* od6 = new passengerOD(ptNo, 6, 0);

	double* pd6 = new double[T];
	pd6[0] = 3;
	pd6[1] = 4;
	pd6[2] = 5;
	pd6[3] = 6;
	pd6[4] = 8;
	pd6[5] = 9;
	pd6[6] = 10;
	pd6[7] = 12;
	pd6[8] = 13;
	pd6[9] = 14;
	pd6[10] = 13;
	pd6[11] = 14;
	pd6[12] = 14;
	pd6[13] = 15;
	pd6[14] = 15;
	pd6[15] = 14;
	pd6[16] = 13;
	pd6[17] = 15;
	pd6[18] = 13;
	pd6[19] = 12;
	pd6[20] = 11;
	pd6[21] = 9;
	pd6[22] = 7;
	pd6[23] = 6;
	pd6[24] = 6;
	pd6[25] = 5;
	pd6[26] = 4;
	pd6[27] = 2;
	pd6[28] = 3;
	pd6[29] = 1;
	od6->setPassengerFlow(pd6);

	//0-60-61-62-2
	std::vector<int> path61;
	path61.push_back(29);
	path61.push_back(129);
	path61.push_back(128);
	path61.push_back(127);
	path61.push_back(42);
	od6->addPassengerPath(0, 2, path61);
	passengerGroups.push_back(od6);

	std::vector<std::string> ph6;
	ph6.push_back("29|129|128|127|42|");
	allPassegnerPath.push_back(ph6);
	ptNo++;
	//*****************************************************************************

	passengerOD* od7 = new passengerOD(ptNo, 7, 0);

	double* pd7 = new double[T];
	pd7[0] = 2;
	pd7[1] = 3;
	pd7[2] = 5;
	pd7[3] = 7;
	pd7[4] = 8;
	pd7[5] = 9;
	pd7[6] = 11;
	pd7[7] = 11;
	pd7[8] = 11;
	pd7[9] = 13;
	pd7[10] = 14;
	pd7[11] = 15;
	pd7[12] = 17;
	pd7[13] = 17;
	pd7[14] = 15;
	pd7[15] = 17;
	pd7[16] = 18;
	pd7[17] = 16;
	pd7[18] = 14;
	pd7[19] = 14;
	pd7[20] = 12;
	pd7[21] = 11;
	pd7[22] = 11;
	pd7[23] = 11;
	pd7[24] = 9;
	pd7[25] = 7;
	pd7[26] = 6;
	pd7[27] = 5;
	pd7[28] = 3;
	pd7[29] = 1;
	od7->setPassengerFlow(pd7);

	//1-61-62-2
	std::vector<int> path71;
	path71.push_back(33);
	path71.push_back(103);
	path71.push_back(39);
	od7->addPassengerPath(0, 6, path71);
	passengerGroups.push_back(od7);

	std::vector<std::string> ph7;
	ph7.push_back("33|103|39|");
	allPassegnerPath.push_back(ph7);
	ptNo++;

	//*****************************************************************************

	passengerOD* od8 = new passengerOD(ptNo, 0, 4);

	double* pd8 = new double[T];
	for (int i = 0; i < T; i++) {
		pd8[i] = 5;
	}
	od8->setPassengerFlow(pd8);

	//1-61-62-2
	std::vector<int> path81;
	path81.push_back(1);
	path81.push_back(94);
	path81.push_back(95);
	path81.push_back(96);
	path81.push_back(97);
	path81.push_back(59);
	od8->addPassengerPath(0, 6, path81);
	passengerGroups.push_back(od8);

	std::vector<std::string> ph8;
	ph8.push_back("1|94|95|96|97|59|");
	allPassegnerPath.push_back(ph8);
	ptNo++;

	//*****************************************************************************

	passengerOD* od9 = new passengerOD(ptNo, 0, 7);

	double* pd9 = new double[T];
	for (int i = 0; i < T; i++) {
		pd9[i] = 5;
	}
	od9->setPassengerFlow(pd9);

	//1-61-62-2
	std::vector<int> path91;
	path91.push_back(2);
	path91.push_back(98);
	path91.push_back(70);
	od9->addPassengerPath(0, 2, path91);
	passengerGroups.push_back(od9);

	std::vector<std::string> ph9;
	ph9.push_back("2|98|70|");
	allPassegnerPath.push_back(ph9);
	ptNo++;

	//*****************************************************************************
}

void BostonData::setNetwork(TransitNetworkLoading* tnl, bool considerBikeSharing) {
	tnl->outFile.open("bicycle.csv");

	tnl->addNode(0, 0);
	tnl->addNode(1, 0);
	tnl->addNode(2, 0);
	tnl->addNode(3, 0);
	tnl->addNode(4, 0);
	tnl->addNode(5, 0);
	tnl->addNode(6, 0);
	tnl->addNode(7, 0);

	if (considerBikeSharing) {
		tnl->dockCIni.push_back(54);
		tnl->dockCIni.push_back(15);
		tnl->dockCIni.push_back(30);
		tnl->dockCIni.push_back(19);
		tnl->dockCIni.push_back(19);
		tnl->dockCIni.push_back(19);
		tnl->dockCIni.push_back(15);
		tnl->dockCIni.push_back(18);
		tnl->dockCIni.push_back(18);
		tnl->dockCIni.push_back(18);
		tnl->dockCIni.push_back(30);
	}
	else {
		tnl->dockCIni.push_back(0);
		tnl->dockCIni.push_back(0);
		tnl->dockCIni.push_back(0);
		tnl->dockCIni.push_back(0);
		tnl->dockCIni.push_back(0);
		tnl->dockCIni.push_back(0);
		tnl->dockCIni.push_back(0);
		tnl->dockCIni.push_back(0);
		tnl->dockCIni.push_back(0);
		tnl->dockCIni.push_back(0);
		tnl->dockCIni.push_back(0);
	}
	

	tnl->addNode(8, 1, tnl->dockCIni[0]);
	tnl->addNode(9, 1, tnl->dockCIni[1]);
	tnl->addNode(10, 1, tnl->dockCIni[2]);
	tnl->addNode(11, 1, tnl->dockCIni[3]);
	tnl->addNode(12, 1, tnl->dockCIni[4]);
	tnl->addNode(13, 1, tnl->dockCIni[5]);
	tnl->addNode(14, 1, tnl->dockCIni[6]);
	tnl->addNode(15, 1, tnl->dockCIni[7]);
	tnl->addNode(16, 1, tnl->dockCIni[8]);
	tnl->addNode(17, 1, tnl->dockCIni[9]);
	tnl->addNode(18, 1, tnl->dockCIni[10]);


	tnl->addNode(19, 3);
	tnl->addNode(20, 3);
	tnl->addNode(21, 3);
	tnl->addNode(22, 3);
	tnl->addNode(23, 3);
	tnl->addNode(24, 3);
	tnl->addNode(25, 3);
	tnl->addNode(26, 2);
	tnl->addNode(27, 2);
	tnl->addNode(28, 2);
	tnl->addNode(29, 2);
	tnl->addNode(30, 2);
	tnl->addNode(31, 2);
	tnl->addNode(32, 2);
	tnl->addNode(33, 2);
	tnl->addNode(34, 2);
	tnl->addNode(35, 2);
	tnl->addNode(36, 2);
	tnl->addNode(37, 2);
	tnl->addNode(38, 2);
	tnl->addNode(39, 2);
	tnl->addNode(40, 2);
	tnl->addNode(41, 2);
	tnl->addNode(42, 2);
	tnl->addNode(43, 2);
	tnl->addNode(44, 2);


	tnl->addLink(0, 0, 100, 0, 8);
	tnl->addLink(1, 0, 1600, 0, 19);
	tnl->addLink(2, 0, 800, 0, 24);
	tnl->addLink(3, 0, 1000, 0, 26);
	tnl->addLink(4, 0, 800, 0, 31);
	tnl->addLink(5, 0, 1200, 0, 36);
	tnl->addLink(6, 0, 400, 0, 41);
	tnl->addLink(7, 0, 400, 1, 9);
	tnl->addLink(8, 0, 100, 1, 10);
	tnl->addLink(9, 0, 1600, 1, 20);
	tnl->addLink(10, 0, 1000, 1, 27);
	tnl->addLink(11, 0, 800, 1, 32);
	tnl->addLink(12, 0, 1200, 1, 37);
	tnl->addLink(13, 0, 1600, 2, 21);
	tnl->addLink(14, 0, 1000, 2, 28);
	tnl->addLink(15, 0, 800, 2, 33);
	tnl->addLink(16, 0, 100, 3, 11);
	tnl->addLink(17, 0, 1600, 3, 22);
	tnl->addLink(18, 0, 1000, 3, 29);
	tnl->addLink(19, 0, 800, 3, 34);
	tnl->addLink(20, 0, 100, 4, 13);
	tnl->addLink(21, 0, 400, 4, 12);
	tnl->addLink(22, 0, 1900, 4, 23);
	tnl->addLink(23, 0, 1300, 4, 30);
	tnl->addLink(24, 0, 1100, 4, 35);
	tnl->addLink(25, 0, 1500, 5, 38);
	tnl->addLink(26, 0, 400, 5, 42);
	tnl->addLink(27, 0, 100, 5, 14);
	tnl->addLink(28, 0, 400, 5, 15);
	tnl->addLink(29, 0, 1300, 6, 39);
	tnl->addLink(30, 0, 400, 6, 43);
	tnl->addLink(31, 0, 100, 6, 16);
	tnl->addLink(32, 0, 400, 6, 17);
	tnl->addLink(33, 0, 800, 7, 25);
	tnl->addLink(34, 0, 1200, 7, 40);
	tnl->addLink(35, 0, 400, 7, 44);
	tnl->addLink(36, 0, 100, 7, 18);
	tnl->addLink(37, 0, 100, 8, 0);
	tnl->addLink(38, 0, 100, 19, 0);
	tnl->addLink(39, 0, 100, 24, 0);
	tnl->addLink(40, 0, 100, 26, 0);
	tnl->addLink(41, 0, 100, 31, 0);
	tnl->addLink(42, 0, 100, 36, 0);
	tnl->addLink(43, 0, 100, 41, 0);
	tnl->addLink(44, 0, 400, 9, 1);
	tnl->addLink(45, 0, 100, 10, 1);
	tnl->addLink(46, 0, 100, 20, 1);
	tnl->addLink(47, 0, 100, 27, 1);
	tnl->addLink(48, 0, 100, 32, 1);
	tnl->addLink(49, 0, 100, 37, 1);
	tnl->addLink(50, 0, 100, 21, 2);
	tnl->addLink(51, 0, 100, 28, 2);
	tnl->addLink(52, 0, 100, 33, 2);
	tnl->addLink(53, 0, 100, 11, 3);
	tnl->addLink(54, 0, 100, 22, 3);
	tnl->addLink(55, 0, 100, 29, 3);
	tnl->addLink(56, 0, 100, 34, 3);
	tnl->addLink(57, 0, 100, 13, 4);
	tnl->addLink(58, 0, 400, 12, 4);
	tnl->addLink(59, 0, 400, 23, 4);
	tnl->addLink(60, 0, 400, 30, 4);
	tnl->addLink(61, 0, 400, 35, 4);
	tnl->addLink(62, 0, 400, 38, 5);
	tnl->addLink(63, 0, 100, 42, 5);
	tnl->addLink(64, 0, 100, 14, 5);
	tnl->addLink(65, 0, 400, 15, 5);
	tnl->addLink(66, 0, 200, 39, 6);
	tnl->addLink(67, 0, 100, 43, 6);
	tnl->addLink(68, 0, 100, 16, 6);
	tnl->addLink(69, 0, 400, 17, 6);
	tnl->addLink(70, 0, 100, 25, 7);
	tnl->addLink(71, 0, 100, 40, 7);
	tnl->addLink(72, 0, 100, 44, 7);
	tnl->addLink(73, 0, 100, 18, 7);
	tnl->addLink(74, 1, 300, 9, 8);
	tnl->addLink(75, 1, 1500, 10, 8);
	tnl->addLink(76, 1, 2700, 11, 8);
	tnl->addLink(77, 1, 3000, 12, 8);
	tnl->addLink(78, 1, 3600, 13, 8);
	tnl->addLink(79, 1, 1500, 14, 8);
	tnl->addLink(80, 1, 1200, 15, 8);
	tnl->addLink(81, 1, 2100, 16, 8);
	tnl->addLink(82, 1, 2400, 17, 8);
	tnl->addLink(83, 1, 3300, 18, 8);
	tnl->addLink(84, 1, 300, 8, 9);
	tnl->addLink(85, 1, 1500, 8, 10);
	tnl->addLink(86, 1, 2700, 8, 11);
	tnl->addLink(87, 1, 3000, 8, 12);
	tnl->addLink(88, 1, 3600, 8, 13);
	tnl->addLink(89, 1, 1500, 8, 14);
	tnl->addLink(90, 1, 1200, 8, 15);
	tnl->addLink(91, 1, 2100, 8, 16);
	tnl->addLink(92, 1, 2400, 8, 17);
	tnl->addLink(93, 1, 3300, 8, 18);
	tnl->addLink(94, 3, 1200, 19, 20);
	tnl->addLink(95, 3, 1800, 20, 21);
	tnl->addLink(96, 3, 2400, 21, 22);
	tnl->addLink(97, 3, 3000, 22, 23);
	tnl->addLink(98, 3, 3000, 24, 25);
	tnl->addLink(99, 3, 1200, 20, 19);
	tnl->addLink(100, 3, 1800, 21, 20);
	tnl->addLink(101, 3, 2400, 22, 21);
	tnl->addLink(102, 3, 3000, 23, 22);
	tnl->addLink(103, 3, 3000, 25, 24);
	tnl->addLink(104, 2, 1200, 26, 27);
	tnl->addLink(105, 2, 800, 27, 28);
	tnl->addLink(106, 2, 1200, 28, 29);
	tnl->addLink(107, 2, 400, 29, 30);
	tnl->addLink(108, 2, 1200, 31, 32);
	tnl->addLink(109, 2, 800, 32, 33);
	tnl->addLink(110, 2, 1200, 33, 34);
	tnl->addLink(111, 2, 400, 34, 35);
	tnl->addLink(112, 2, 1200, 36, 37);
	tnl->addLink(113, 2, 800, 37, 38);
	tnl->addLink(114, 2, 1600, 38, 39);
	tnl->addLink(115, 2, 400, 39, 40);
	tnl->addLink(116, 2, 1600, 41, 42);
	tnl->addLink(117, 2, 800, 42, 43);
	tnl->addLink(118, 2, 800, 43, 44);
	tnl->addLink(119, 2, 1200, 27, 26);
	tnl->addLink(120, 2, 800, 28, 27);
	tnl->addLink(121, 2, 1200, 29, 28);
	tnl->addLink(122, 2, 400, 30, 29);
	tnl->addLink(123, 2, 1200, 32, 31);
	tnl->addLink(124, 2, 800, 33, 32);
	tnl->addLink(125, 2, 1200, 34, 33);
	tnl->addLink(126, 2, 400, 35, 34);
	tnl->addLink(127, 2, 1200, 37, 36);
	tnl->addLink(128, 2, 800, 38, 37);
	tnl->addLink(129, 2, 1600, 39, 38);
	tnl->addLink(130, 2, 400, 40, 39);
	tnl->addLink(131, 2, 1600, 42, 41);
	tnl->addLink(132, 2, 800, 43, 42);
	tnl->addLink(133, 2, 800, 44, 43);



	tnl->sharedBikeNode[0] = 8;
	tnl->sharedBikeNode[1] = 9;
	tnl->sharedBikeNode[2] = 10;
	tnl->sharedBikeNode[3] = 11;
	tnl->sharedBikeNode[4] = 12;
	tnl->sharedBikeNode[5] = 13;
	tnl->sharedBikeNode[6] = 14;
	tnl->sharedBikeNode[7] = 15;
	tnl->sharedBikeNode[8] = 16;
	tnl->sharedBikeNode[9] = 17;
	tnl->sharedBikeNode[10] = 18;


	std::vector<int> busline14a;
	busline14a.push_back(104);
	busline14a.push_back(105);
	busline14a.push_back(106);
	busline14a.push_back(107);
	std::map<int, double> b14aTimetable;
	b14aTimetable[0] = 800;
	b14aTimetable[6] = 800;
	b14aTimetable[12] = 750;
	b14aTimetable[18] = 650;
	b14aTimetable[24] = 500;
	b14aTimetable[30] = 650;
	b14aTimetable[36] = 750;
	b14aTimetable[42] = 800;
	b14aTimetable[46] = 800;
	b14aTimetable[60] = 800;
	tnl->setVehTimetable(1, 1, 1000, 32, busline14a, b14aTimetable);

	std::vector<int> busline14b;
	busline14b.push_back(122);
	busline14b.push_back(121);
	busline14b.push_back(120);
	busline14b.push_back(119);
	std::map<int, double> b14bTimetable;
	b14bTimetable[0] = 800;
	b14bTimetable[6] = 800;
	b14bTimetable[12] = 750;
	b14bTimetable[18] = 650;
	b14bTimetable[24] = 500;
	b14bTimetable[30] = 650;
	b14bTimetable[36] = 750;
	b14bTimetable[42] = 800;
	b14bTimetable[46] = 800;
	b14bTimetable[60] = 800;
	tnl->setVehTimetable(1, 1, 1000, 32, busline14b, b14bTimetable);

	std::vector<int> busline15a;
	busline15a.push_back(108);
	busline15a.push_back(109);
	busline15a.push_back(110);
	busline15a.push_back(111);
	std::map<int, double> b15aTimetable;
	b15aTimetable[0] = 800;
	b15aTimetable[6] = 800;
	b15aTimetable[12] = 750;
	b15aTimetable[18] = 650;
	b15aTimetable[24] = 500;
	b15aTimetable[30] = 650;
	b15aTimetable[36] = 750;
	b15aTimetable[42] = 800;
	b15aTimetable[46] = 800;
	b15aTimetable[60] = 800;
	tnl->setVehTimetable(2, 1, 1000, 32, busline15a, b15aTimetable);

	std::vector<int> busline15b;
	busline15b.push_back(126);
	busline15b.push_back(125);
	busline15b.push_back(124);
	busline15b.push_back(123);
	std::map<int, double> b15bTimetable;
	b15bTimetable[0] = 800;
	b15bTimetable[6] = 800;
	b15bTimetable[12] = 750;
	b15bTimetable[18] = 650;
	b15bTimetable[24] = 500;
	b15bTimetable[30] = 650;
	b15bTimetable[36] = 750;
	b15bTimetable[42] = 800;
	b15bTimetable[46] = 800;
	b15bTimetable[60] = 800;
	tnl->setVehTimetable(2, 1, 1000, 32, busline15b, b15bTimetable);

	std::vector<int> busline23a;
	busline23a.push_back(112);
	busline23a.push_back(113);
	busline23a.push_back(114);
	busline23a.push_back(115);
	std::map<int, double> b23aTimetable;
	b23aTimetable[0] = 800;
	b23aTimetable[10] = 800;
	b23aTimetable[20] = 700;
	b23aTimetable[30] = 600;
	b23aTimetable[40] = 650;
	b23aTimetable[50] = 700;
	b23aTimetable[60] = 800;
	tnl->setVehTimetable(3, 1, 1000, 32, busline23a, b23aTimetable);

	std::vector<int> busline23b;
	busline23b.push_back(130);
	busline23b.push_back(129);
	busline23b.push_back(128);
	busline23b.push_back(127);
	std::map<int, double> b23bTimetable;
	b23bTimetable[0] = 800;
	b23bTimetable[10] = 800;
	b23bTimetable[20] = 700;
	b23bTimetable[30] = 600;
	b23bTimetable[40] = 650;
	b23bTimetable[50] = 700;
	b23bTimetable[60] = 800;
	tnl->setVehTimetable(3, 1, 1000, 32, busline23b, b23bTimetable);

	std::vector<int> busline24a;
	busline24a.push_back(116);
	busline24a.push_back(117);
	busline24a.push_back(118);
	std::map<int, double> b24aTimetable;
	b24aTimetable[0] = 800;
	b24aTimetable[10] = 800;
	b24aTimetable[20] = 700;
	b24aTimetable[30] = 600;
	b24aTimetable[40] = 650;
	b24aTimetable[50] = 700;
	b24aTimetable[60] = 800;
	tnl->setVehTimetable(4, 1, 1000, 32, busline24a, b24aTimetable);

	std::vector<int> busline24b;
	busline24b.push_back(133);
	busline24b.push_back(132);
	busline24b.push_back(131);
	std::map<int, double> b24bTimetable;
	b24bTimetable[0] = 800;
	b24bTimetable[10] = 800;
	b24bTimetable[20] = 700;
	b24bTimetable[30] = 600;
	b24bTimetable[40] = 650;
	b24bTimetable[50] = 700;
	b24bTimetable[60] = 800;
	tnl->setVehTimetable(4, 1, 1000, 32, busline24b, b24bTimetable);



	std::vector<int> metroline1a;
	metroline1a.push_back(94);
	metroline1a.push_back(95);
	metroline1a.push_back(96);
	metroline1a.push_back(97);
	std::map<int, double> m1aTimetable;
	m1aTimetable[0] = 1000;
	m1aTimetable[5] = 1000;
	m1aTimetable[10] = 1000;
	m1aTimetable[15] = 1000;
	m1aTimetable[20] = 1000;
	m1aTimetable[25] = 1000;
	m1aTimetable[30] = 1000;
	m1aTimetable[35] = 1000;
	m1aTimetable[40] = 1000;
	m1aTimetable[45] = 1000;
	m1aTimetable[55] = 1000;
	m1aTimetable[65] = 1000;
	tnl->setVehTimetable(15, 2, 1000, 200, metroline1a, m1aTimetable);

	std::vector<int> metroline1b;
	metroline1b.push_back(102);
	metroline1b.push_back(101);
	metroline1b.push_back(100);
	metroline1b.push_back(99);
	std::map<int, double> m1bTimetable;
	m1bTimetable[0] = 1000;
	m1bTimetable[5] = 1000;
	m1bTimetable[10] = 1000;
	m1bTimetable[15] = 1000;
	m1bTimetable[20] = 1000;
	m1bTimetable[25] = 1000;
	m1bTimetable[30] = 1000;
	m1bTimetable[35] = 1000;
	m1bTimetable[40] = 1000;
	m1bTimetable[45] = 1000;
	m1bTimetable[55] = 1000;
	m1bTimetable[65] = 1000;
	tnl->setVehTimetable(16, 2, 1000, 200, metroline1b, m1bTimetable);

	std::vector<int> metroline2a;
	metroline2a.push_back(98);
	std::map<int, double> m2aTimetable;
	m2aTimetable[0] = 1000;
	m2aTimetable[5] = 1000;
	m2aTimetable[10] = 1000;
	m2aTimetable[15] = 1000;
	m2aTimetable[20] = 1000;
	m2aTimetable[25] = 1000;
	m2aTimetable[30] = 1000;
	m2aTimetable[35] = 1000;
	m2aTimetable[40] = 1000;
	m2aTimetable[45] = 1000;
	m2aTimetable[55] = 1000;
	m2aTimetable[65] = 1000;
	tnl->setVehTimetable(15, 2, 1000, 200, metroline2a, m2aTimetable);

	std::vector<int> metroline2b;
	metroline2b.push_back(103);
	std::map<int, double> m2bTimetable;
	m2bTimetable[0] = 1000;
	m2bTimetable[5] = 1000;
	m2bTimetable[10] = 1000;
	m2bTimetable[15] = 1000;
	m2bTimetable[20] = 1000;
	m2bTimetable[25] = 1000;
	m2bTimetable[30] = 1000;
	m2bTimetable[35] = 1000;
	m2bTimetable[40] = 1000;
	m2bTimetable[45] = 1000;
	m2bTimetable[55] = 1000;
	m2bTimetable[65] = 1000;
	tnl->setVehTimetable(16, 2, 1000, 200, metroline2b, m2bTimetable);
}