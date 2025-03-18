#include "stdafx.h"
#include "CTM.h"


double CTM::calVehTravelTime(int passengerGroup, int pathNo, int startTime)
{
	int pathNum = 0;
	double sumRunningTime =0;

	for (int i = 0; i < vehs.size();i++) {
		if (vehs[i].passengerGroup == passengerGroup && vehs[i].pathNo == pathNo && vehs[i].startTime == startTime) {
			sumRunningTime += vehs[i].endTime - startTime;
			pathNum++;
		}
	}
	return sumRunningTime/ pathNum;

}

void CTM::conSim2End()
{
	while (veh_on_network) {
		simulation(); 
	}
}

void CTM::addLink(int type, double v, double w, double l, double veh_len, int veh_capacity, int Q, int startNodeSeq, int endNodeSeq)
{
	Link* link = new Link();
	link->type = type;
	link->endNode = nodes[endNodeSeq];
	link->startNode = nodes[startNodeSeq];
	link->linkLen = l;

	nodes[startNodeSeq]->outNodeLink.push_back(links.size());
	nodes[endNodeSeq]->inNodeLink.push_back(links.size());

	for (int i = 0; i < l/v;i++) {//根据长度和流速确定元胞数
		Cell *cell = new Cell();
		cell->type = 1;
		cell->N = veh_capacity;
		cell->Q = Q;
		cell->v = v;
		cell->w = w;
		cell->linkOrNodeSeq = links.size();

		if (i != 0) {
			link->cell_on_link[i - 1]->nextCell.push_back(cell);
		}

		link->cell_on_link.push_back(cell);		

		cells.push_back(cell);
	}

	link->cell_on_link[link->cell_on_link.size() - 1]->nextCell.push_back(nodes[endNodeSeq]->cell_on_node[0]);
	nodes[startNodeSeq]->cell_on_node[nodes[startNodeSeq]->cell_on_node.size() - 1]->nextCell.push_back(link->cell_on_link[0]);

	links.push_back(link);
}

void CTM::addNode(int type, double v, double w, int veh_capacity, int Q)
{
	Node* node = new Node();
	node->type = type;
	Cell* cell = new Cell();
	cell->type = 0;
	cell->N = veh_capacity;
	cell->Q = Q;
	cell->v = v;
	cell->w = w;
	cell->linkOrNodeSeq = nodes.size();
	cells.push_back(cell);
	node->cell_on_node.push_back(cell);
	nodes.push_back(node);
}

void CTM::addBicyclePoint(int pointSeq, double bicycleNum)
{
	bicyclePoints.push_back(pointSeq);
	nodes[pointSeq]->bicycleNum = bicycleNum;
}

double CTM::getBicyclePointByBS(int bicyclePointSeq)
{
	return nodes[bicyclePoints[bicyclePointSeq]]->bicycleNum;
}

double CTM::getBicyclePointByNS(int nodeSeq)
{
	return nodes[nodeSeq]->bicycleNum;
}



void CTM::addVeh(int type, int pathType, int passengerGroup, int pathNo, int startNode, int endNode, std::vector<int> vehLinks, double vehNum,
	int inShareBicyclePoint1, int inShareBicyclePoint2, int outShareBicyclePoint1, int outShareBicyclePoint2)
{
	//更新共享单车容量
	if (outShareBicyclePoint1 != -1) {
		nodes[outShareBicyclePoint1]->bicycleNum -= vehNum;
	}
	if (outShareBicyclePoint2 != -1) {
		nodes[outShareBicyclePoint2]->bicycleNum -= vehNum;
	}
	if (vehNum == 0 && type == 0) {
		VehOrPegr veh_on_cell;
		veh_on_cell.vehid = defVehId;
		veh_on_cell.type = type;
		veh_on_cell.pathType = pathType;
		veh_on_cell.passengerGroup = passengerGroup;
		veh_on_cell.pathNo = pathNo;
		veh_on_cell.startNode = startNode;
		veh_on_cell.endNode = endNode;
		veh_on_cell.inShareBicyclePoint1 = inShareBicyclePoint1;
		veh_on_cell.inShareBicyclePoint2 = inShareBicyclePoint2;
		veh_on_cell.outShareBicyclePoint1 = outShareBicyclePoint1;
		veh_on_cell.outShareBicyclePoint2 = outShareBicyclePoint2;
		for (int i = 0; i < vehLinks.size(); i++) {
			veh_on_cell.links.push_back(vehLinks[i]);
		}
		veh_on_cell.startTime = simTime;
		veh_on_cell.vehNum = 0;
		nodes[startNode]->cell_on_node[0]->veh_on_cell.push_back(veh_on_cell);
		defVehId++;
		veh_on_network++;
	}
	else {
		for (int i = 0; i < (int)vehNum; i++) {
			VehOrPegr veh_on_cell;
			veh_on_cell.vehid = defVehId;
			veh_on_cell.type = type;
			veh_on_cell.pathType = pathType;
			veh_on_cell.passengerGroup = passengerGroup;
			veh_on_cell.pathNo = pathNo;
			veh_on_cell.startNode = startNode;
			veh_on_cell.inShareBicyclePoint1 = inShareBicyclePoint1;
			veh_on_cell.inShareBicyclePoint2 = inShareBicyclePoint2;
			veh_on_cell.outShareBicyclePoint1 = outShareBicyclePoint1;
			veh_on_cell.outShareBicyclePoint2 = outShareBicyclePoint2;
			veh_on_cell.endNode = endNode;
			for (int i = 0; i < vehLinks.size(); i++) {
				veh_on_cell.links.push_back(vehLinks[i]);
			}
			veh_on_cell.startTime = simTime;
			veh_on_cell.vehNum = 1;

			nodes[startNode]->cell_on_node[0]->veh_on_cell.push_back(veh_on_cell);
			defVehId++;
			if (type == 0)
				veh_on_network++;
		}
		if (vehNum - (int)vehNum > 0) {

			VehOrPegr veh_on_cell;
			veh_on_cell.vehid = defVehId;
			veh_on_cell.type = type;
			veh_on_cell.pathType = pathType;
			veh_on_cell.passengerGroup = passengerGroup;
			veh_on_cell.pathNo = pathNo;
			veh_on_cell.startNode = startNode;
			veh_on_cell.inShareBicyclePoint1 = inShareBicyclePoint1;
			veh_on_cell.inShareBicyclePoint2 = inShareBicyclePoint2;
			veh_on_cell.outShareBicyclePoint1 = outShareBicyclePoint1;
			veh_on_cell.outShareBicyclePoint2 = outShareBicyclePoint2;
			veh_on_cell.endNode = endNode;
			for (int i = 0; i < vehLinks.size(); i++) {
				veh_on_cell.links.push_back(vehLinks[i]);
			}
			veh_on_cell.startTime = simTime;
			veh_on_cell.vehNum = vehNum - (int)vehNum;
			
			defVehId++;
			nodes[startNode]->cell_on_node[0]->veh_on_cell.push_back(veh_on_cell);
			if (type == 0)
				veh_on_network ++;
		}
	}	
}

void CTM::simulation()
{
	std::vector<double> tmDocks;
	for (int i = 0; i < bicyclePoints.size(); i++) {

		tmDocks.push_back(nodes[bicyclePoints[i]]->bicycleNum);

		if (i == bicyclePoints.size() - 1)
			outFile << nodes[bicyclePoints[i]]->bicycleNum << "\n";
		else
			outFile << nodes[bicyclePoints[i]]->bicycleNum << " , ";
	}
	dockCStar.push_back(tmDocks);

	//分成两步走，第一步计算每个cell能够进入的车辆数，能够出的最大车辆数
	for (int i = 0; i < cells.size(); i++) {

		Cell *tmCell = cells[i];

		//设置最大离开车辆
		double maxOutCarNum = 999999;
		double tmMaxCarNum = 0;
		for (int j = 0;j < tmCell->veh_on_cell.size(); j++) {
			tmCell->veh_on_cell[j].moved = 0;
			tmMaxCarNum += tmCell->veh_on_cell[j].vehNum;
		}
		if (tmMaxCarNum < maxOutCarNum) {
			maxOutCarNum = tmMaxCarNum;
		}
		if (tmCell->Q < maxOutCarNum) {
			maxOutCarNum = tmCell->Q;
		}
		//if (tmCell->veh_on_cell.size() > 0 && maxOutCarNum < 0.000001) {
		//	maxOutCarNum = 1;//这里为了给虚拟车辆一个出去的机会
		//}
		
		tmCell->maxOutVeh = maxOutCarNum;

		//设置最大进入车辆
		double maxInCarNum = 999999;
		if (tmCell->Q < maxInCarNum) {
			maxInCarNum = tmCell->Q;
		}
		if ((tmCell->N - tmMaxCarNum) * tmCell->w / tmCell->v < maxInCarNum) {
			maxInCarNum = (tmCell->N - tmMaxCarNum) * tmCell->w / tmCell->v;
		}
		tmCell->maxInVeh = maxInCarNum;
	}

	//第二步，根据车辆数，更新所有节点
	for (int i = 0; i < cells.size(); i++) {		

		Cell* tmCell = cells[i];
		if (tmCell->type == 0) {//0:node cell, node cell的进出都要在这里计算
			//node节点有车辆进入，这里需要判断终点
			int conF = 1;
			while(tmCell->maxInVeh>0.0000000001 && conF){
				conF = 0;
				for (int j = 0; j < nodes[tmCell->linkOrNodeSeq]->inNodeLink.size(); j++) {//遍历所有入链接
					
					Cell* tmOutC = links[nodes[tmCell->linkOrNodeSeq]->inNodeLink[j]]->cell_on_link.back();
					
					if (tmOutC->veh_on_cell.size()>0 && tmOutC->maxOutVeh > -0.0000001 && tmOutC->maxOutVeh - tmOutC->veh_on_cell[0].vehNum > -0.0000001
						&& tmCell->maxInVeh - tmOutC->veh_on_cell[0].vehNum > -0.000001 && tmOutC->veh_on_cell[0].moved == 0) {//可以驶出
						VehOrPegr tmV = tmOutC->veh_on_cell[0];
						
						//打印车辆路径**********************************************************************************
						if (printVehId == tmV.vehid) {
							std::cout << "time:" << simTime << "; link:" << tmOutC->linkOrNodeSeq << "- node:" << tmCell->linkOrNodeSeq<<"\n";
						}
						tmOutC->veh_on_cell[0].moved = 1;

						if (nodes[tmCell->linkOrNodeSeq]->type == 1) {//更新共享单车停靠点容量
							if (tmCell->linkOrNodeSeq == tmV.inShareBicyclePoint1) {
								nodes[tmCell->linkOrNodeSeq]->bicycleNum += tmV.vehNum;
							}else if (tmCell->linkOrNodeSeq == tmV.inShareBicyclePoint2) {
								nodes[tmCell->linkOrNodeSeq]->bicycleNum += tmV.vehNum;
							}
						}

						if (tmV.endNode == tmCell->linkOrNodeSeq) {//表示车辆到达终点
							if (tmV.type == 0) {								
								tmV.endTime = simTime;
								vehs.push_back(tmV);
								veh_on_network --;
							}							
						}
						else {//车辆未结束
							tmCell->veh_on_cell.push_back(tmOutC->veh_on_cell[0]);
						}
						tmOutC->veh_on_cell.erase(tmOutC->veh_on_cell.begin());
						tmOutC->maxOutVeh -= tmV.vehNum;
						tmCell->maxInVeh -= tmV.vehNum;
						conF = 1;
					}
					if (tmCell->maxInVeh == 0) {
						break;
					}
				}
			}
			

			//node节点驶出
			int outVeh = 0;//漏掉的车辆
			int finded = 0;
			double testFlow = 0;
			double maxFlow = tmCell->maxOutVeh;
			int cellVehNo = tmCell->veh_on_cell.size();
			while (maxFlow - testFlow > -0.00000001 && tmCell->veh_on_cell.size()>0 &&outVeh < tmCell->veh_on_cell.size()) {
				finded = 0;				

				VehOrPegr v = tmCell->veh_on_cell[outVeh];

				testFlow += v.vehNum;

				for (int l = 0; l < v.links.size()&& finded == 0;l++) {//车辆途径站点					
					for (int j = 0; j < tmCell->nextCell.size();j++) {//遍历cell的下游节点
						if (v.links[l] == tmCell->nextCell[j]->linkOrNodeSeq) {//表示车辆要前往该cell
							
							if (tmCell->nextCell[j]->maxInVeh - v.vehNum > -0.00000001  && tmCell->maxOutVeh - v.vehNum > -0.000001 && v.moved == 0) {//表示可以进入车辆
								v.moved = 1;
								tmCell->nextCell[j]->maxInVeh -= v.vehNum;
								tmCell->maxOutVeh -= v.vehNum;
								tmCell->nextCell[j]->veh_on_cell.push_back(v);
								tmCell->veh_on_cell.erase(tmCell->veh_on_cell.begin()+ outVeh);

								//打印车辆路径**********************************************************************************
								if (printVehId == v.vehid) {
									std::cout << "time:" << simTime << "; node:" << tmCell->linkOrNodeSeq << "- link:" << tmCell->nextCell[j]->linkOrNodeSeq << "\n";
								}

							}
							else {
								outVeh++;
							}
							finded = 1;
							break;

						}
					}					
				}

				cellVehNo = tmCell->veh_on_cell.size();
			}
		}
		else if (tmCell->type == 1) {//1:link cell 
			if (tmCell->nextCell[0]->type == 1) {//link转link

				while (tmCell->veh_on_cell.size()>0  && tmCell->maxOutVeh - tmCell->veh_on_cell[0].vehNum > -0.0000001
					&& tmCell->nextCell[0]->maxInVeh - tmCell->veh_on_cell[0].vehNum > -0.0000001 && tmCell->veh_on_cell[0].moved == 0) {

					//打印车辆路径**********************************************************************************
					if (printVehId == tmCell->veh_on_cell[0].vehid) {
						std::cout << "time:" << simTime << "; link:" << tmCell->linkOrNodeSeq << "- link:" << tmCell->nextCell[0]->linkOrNodeSeq << "\n";
					}
					tmCell->veh_on_cell[0].moved = 1;

					tmCell->maxOutVeh -= tmCell->veh_on_cell[0].vehNum;
					tmCell->nextCell[0]->maxInVeh -= tmCell->veh_on_cell[0].vehNum;

					tmCell->nextCell[0]->veh_on_cell.push_back(tmCell->veh_on_cell[0]);
					tmCell->veh_on_cell.erase(tmCell->veh_on_cell.begin());

					
					
				}
			}
		}
	}

	//执行调度
	if (startSs) {
		dispatchStrategySs();
	}

	//打印共享单车容量
	if (simTime == 0) {
		outFile << "---------------------------------------------------------------------------------------------------------------------" << "\n";
	}
	std::vector<double> tmDock;
	for (int i = 0; i < bicyclePoints.size(); i++) {

		tmDock.push_back(nodes[bicyclePoints[i]]->bicycleNum);

		if (i == bicyclePoints.size() - 1)
			outFile << nodes[bicyclePoints[i]]->bicycleNum << "\n";
		else
			outFile << nodes[bicyclePoints[i]]->bicycleNum << " , ";
	}
	dockC.push_back(tmDock);
	
	simTime++;


}

double CTM::getIniPathTime(std::vector<int> pathLinks)
{
	double timePath = 0;
	for (int i = 0; i < pathLinks.size();i++) {		
		if (links[pathLinks[i]]->type == 0) {
			timePath += links[pathLinks[i]]->linkLen/5;
		}else if (links[pathLinks[i]]->type == 1) {
			timePath += links[pathLinks[i]]->linkLen / 25;
		}
		else if (links[pathLinks[i]]->type == 2) {
			timePath += links[pathLinks[i]]->linkLen / 30;
		}
		else if (links[pathLinks[i]]->type == 3) {
			timePath += links[pathLinks[i]]->linkLen / 60;
		}
		
	}
	
	return timePath;
}

int CTM::getShareBicycleSeq(int pointSeq)
{
	for (int i = 0; i < bicyclePoints.size();i++) {
		if (bicyclePoints[i] == pointSeq) {
			return i;
		}
	}
	return -1;
}

void CTM::clearNetwork()
{
	defVehId = 0;
	simTime = 0;
	vehs.clear();
	dispatchTask.clear();
	for (int i = 0; i < bicyclePoints.size(); i++) {//要针对问题修改*********************************************************************
		nodes[bicyclePoints[i]]->bicycleNum = dockCIni[i];
	}
	dispatchCost = 0;
	dockC.clear();
	dockCStar.clear();
}

void CTM::dispatchStrategySs()
{
	
	//计算预期容量
	std::vector<double> tmVehs;
	std::vector<double> tmVehsPer;//容量百分比
	if (handS == 0) {
		for (int i = 0; i < bicyclePoints.size(); i++) {
			tmVehs.push_back(getBicyclePointByBS(i));
		}
	}
	else if (handS == 1 && simTime == 0) {
		BikeDispatch tmBikeDispatch;
		tmBikeDispatch.startTime = simTime;
		tmBikeDispatch.bikeNum = nodes[bicyclePoints[2]]->bicycleNum;
		tmBikeDispatch.dock = 1;
		tmBikeDispatch.time = simTime + 5;
		dispatchTask.push_back(tmBikeDispatch);
		dispatchCost += dispatchCostPerTime * 5;
		nodes[bicyclePoints[2]]->bicycleNum = 0;


		BikeDispatch tmBikeDispatch1;
		tmBikeDispatch1.startTime = simTime;
		tmBikeDispatch1.bikeNum = nodes[bicyclePoints[4]]->bicycleNum;
		tmBikeDispatch1.dock = 5;
		tmBikeDispatch1.time = simTime + 5;
		dispatchTask.push_back(tmBikeDispatch1);
		dispatchCost += dispatchCostPerTime * 5;
		nodes[bicyclePoints[4]]->bicycleNum = 0;


		BikeDispatch tmBikeDispatch2;
		tmBikeDispatch2.startTime = simTime;
		tmBikeDispatch2.bikeNum = nodes[bicyclePoints[7]]->bicycleNum;
		tmBikeDispatch2.dock = 6;
		tmBikeDispatch2.time = simTime + 5;
		dispatchTask.push_back(tmBikeDispatch2);
		dispatchCost += dispatchCostPerTime * 5;
		nodes[bicyclePoints[7]]->bicycleNum = 0;


		BikeDispatch tmBikeDispatch3;
		tmBikeDispatch3.startTime = simTime;
		tmBikeDispatch3.bikeNum = nodes[bicyclePoints[9]]->bicycleNum;
		tmBikeDispatch3.dock = 8;
		tmBikeDispatch3.time = simTime + 5;
		dispatchTask.push_back(tmBikeDispatch3);
		dispatchCost += dispatchCostPerTime * 5;
		nodes[bicyclePoints[9]]->bicycleNum = 0;
	}
	else if (handS == 1 && simTime == 15) {
		BikeDispatch tmBikeDispatch;
		tmBikeDispatch.startTime = simTime;
		tmBikeDispatch.bikeNum = nodes[bicyclePoints[0]]->bicycleNum/4;
		tmBikeDispatch.dock = 1;
		tmBikeDispatch.time = simTime + 5;
		dispatchTask.push_back(tmBikeDispatch);
		dispatchCost += dispatchCostPerTime * 5;

		BikeDispatch tmBikeDispatch1;
		tmBikeDispatch1.startTime = simTime;
		tmBikeDispatch1.bikeNum = nodes[bicyclePoints[0]]->bicycleNum / 4;
		tmBikeDispatch1.dock = 3;
		tmBikeDispatch1.time = simTime + 5;
		dispatchTask.push_back(tmBikeDispatch1);
		dispatchCost += dispatchCostPerTime * 5;

		BikeDispatch tmBikeDispatch2;
		tmBikeDispatch2.startTime = simTime;
		tmBikeDispatch2.bikeNum = nodes[bicyclePoints[0]]->bicycleNum / 4;
		tmBikeDispatch2.dock = 6;
		tmBikeDispatch2.time = simTime + 5;
		dispatchTask.push_back(tmBikeDispatch2);
		dispatchCost += dispatchCostPerTime * 5;
		
		BikeDispatch tmBikeDispatch3;
		tmBikeDispatch3.startTime = simTime;
		tmBikeDispatch3.bikeNum = nodes[bicyclePoints[0]]->bicycleNum / 4;
		tmBikeDispatch3.dock = 8;
		tmBikeDispatch3.time = simTime + 5;
		dispatchTask.push_back(tmBikeDispatch3);
		dispatchCost += dispatchCostPerTime * 5;

		nodes[bicyclePoints[0]]->bicycleNum = 0;
	}
	

	//容量填充
	for (int i = dispatchTask.size()-1; i >= 0;i--) {		
		BikeDispatch s = dispatchTask[i];
		if (handS == 0) {
			tmVehs[s.dock] += s.bikeNum;
		}
		if (s.time == simTime) {//到达了调度目标dock
			nodes[bicyclePoints[s.dock]]->bicycleNum += s.bikeNum;
			dispatchTask.erase(begin(dispatchTask)+i);
		}
	}

	if (handS == 0) {
		for (int i = 0; i < bicyclePoints.size(); i++) {
			tmVehsPer.push_back(tmVehs[i] / dockCIni[i]);
		}

		//容量调度
		for (int i = 0; i < bicyclePoints.size(); i++) {
			if (getBicyclePointByBS(i) > dockS[i]) {//大于S需要调度

				double depBikes;
				double tmOldDepBikes = 10000;

				//if(i==0)//需要根据案例进行修改
				//	depBikes = getBicyclePointByBS(i);
				//else
				//	depBikes = getBicyclePointByBS(i) - dockCIni[i];

				depBikes = getBicyclePointByBS(i) - dockCIni[i];


				while (depBikes > 0.00001 && depBikes < tmOldDepBikes) {

					tmOldDepBikes = depBikes;

					double minBikes = 10000;
					int dockSep = -1;


					//寻找最小容量的停靠点
					for (int j = 0; j < bicyclePoints.size(); j++) {
						if (tmVehsPer[j] < minBikes) {
							minBikes = tmVehsPer[j];
							dockSep = j;
						}
					}


					//进行车辆调度
					double tmdepB = 0;
					if (depBikes < (dockCIni[dockSep] - tmVehs[dockSep])) {
						tmdepB = depBikes;
					}
					else {
						tmdepB = dockCIni[dockSep] - tmVehs[dockSep];
					}
					depBikes -= tmdepB;

					BikeDispatch tmBikeDispatch;
					tmBikeDispatch.startTime = simTime;
					tmBikeDispatch.bikeNum = tmdepB;
					tmBikeDispatch.dock = dockSep;
					tmBikeDispatch.time = simTime + dispatchTime[i][dockSep];
					dispatchTask.push_back(tmBikeDispatch);
					dispatchCost += dispatchCostPerTime * dispatchTime[i][dockSep];

					tmVehs[dockSep] += tmdepB;
					tmVehsPer[dockSep] = tmVehs[dockSep] / dockCIni[dockSep];


				}

				nodes[bicyclePoints[i]]->bicycleNum = dockCIni[i];
				tmVehsPer[i] = 1;
				tmVehs[i] = dockCIni[i];

				//if (i==0) {//需要根据案例进行修改
				//	nodes[bicyclePoints[i]]->bicycleNum = depBikes;
				//	tmVehsPer[i] = depBikes/ dockCIni[i];
				//	tmVehs[i] = depBikes;
				//}
				//else {
				//	nodes[bicyclePoints[i]]->bicycleNum = dockCIni[i];
				//	tmVehsPer[i] = 1;
				//	tmVehs[i] = dockCIni[i];
				//}

			}
		}	
	}

	
}

CTM::CTM()//初始化
{
	outFile.open("bicycle.csv");
	
	/***************************************************************
	  *  @brief     插入节点
	  *  @param[in] type 节点类型 0乘客起终点，1共享单车停靠点,2公交节点，3地铁
	  *  @param[in] v 链路自由流速
	  *  @param[in] w 链路反向传播速度
	  *  @param[in] veh_capacity 最大车容量
	  *  @param[in] Q 链路元胞的最大通行能力（与几车道有关系）
	  *  @note      备注
	  *  @Sample usage:     网络初始化时，自动添加
	 **************************************************************/
	//void addNode(int type, double v, double w, double veh_capacity, int Q);

	addNode(0, 20, 20, 100, 100);
	addNode(0, 20, 20, 100, 100);
	addNode(0, 20, 20, 100, 100);
	addNode(0, 20, 20, 100, 100);
	addNode(0, 20, 20, 100, 100);
	addNode(0, 20, 20, 100, 100);
	addNode(0, 20, 20, 100, 100);
	addNode(0, 20, 20, 100, 100);
	addNode(1, 20, 20, 100, 100);
	addNode(1, 20, 20, 100, 100);
	addNode(1, 20, 20, 100, 100);
	addNode(1, 20, 20, 100, 100);
	addNode(1, 20, 20, 100, 100);
	addNode(1, 20, 20, 100, 100);
	addNode(1, 20, 20, 100, 100);
	addNode(1, 20, 20, 100, 100);
	addNode(1, 20, 20, 100, 100);
	addNode(1, 20, 20, 100, 100);
	addNode(1, 20, 20, 100, 100);
	addNode(3, 20, 20, 100, 100);
	addNode(3, 20, 20, 100, 100);
	addNode(3, 20, 20, 100, 100);
	addNode(3, 20, 20, 100, 100);
	addNode(3, 20, 20, 100, 100);
	addNode(3, 20, 20, 100, 100);
	addNode(3, 20, 20, 100, 100);
	addNode(2, 20, 20, 100, 100);
	addNode(2, 20, 20, 100, 100);
	addNode(2, 20, 20, 100, 100);
	addNode(2, 20, 20, 100, 100);
	addNode(2, 20, 20, 100, 100);
	addNode(2, 20, 20, 100, 100);
	addNode(2, 20, 20, 100, 100);
	addNode(2, 20, 20, 100, 100);
	addNode(2, 20, 20, 100, 100);
	addNode(2, 20, 20, 100, 100);
	addNode(2, 20, 20, 100, 100);
	addNode(2, 20, 20, 100, 100);
	addNode(2, 20, 20, 100, 100);
	addNode(2, 20, 20, 100, 100);
	addNode(2, 20, 20, 100, 100);
	addNode(2, 20, 20, 100, 100);
	addNode(2, 20, 20, 100, 100);
	addNode(2, 20, 20, 100, 100);
	addNode(2, 20, 20, 100, 100);


	/***************************************************************
	  *  @brief     插入连接
	  *  @param[in] type 链路类型 ，0步行链接，1共享单车链接，2公交链接，3地铁链接
	  *  @param[in] v 链路自由流速
	  *  @param[in] w 链路反向传播速度
	  *  @param[in] l 链路长度
	  *  @param[in] veh_len 链路行驶的车辆长度
	  *  @param[in] veh_capacity Link中单位cell的车容量
	  *  @param[in] Q 链路元胞的最大通行能力（与几车道有关系）
	  *  @param[in] startNodeSeq 开始节点的序号
	  *  @param[in] endNodeSeq 结束节点的序号
	  *  @note      备注
	  *  @Sample usage:     网络初始化时，自动添加
	 **************************************************************/
	//void addLink(int type, double v, double w, double l, double veh_len, int veh_capacity,int Q, int startNodeSeq, int endNodeSeq);

	addLink(0, 1, 1, 1, 0.1, 80, 80, 0, 8);
	addLink(0, 1, 1, 16, 0.1, 80, 80, 0, 19);
	addLink(0, 1, 1, 8, 0.1, 80, 80, 0, 24);
	addLink(0, 1, 1, 10, 0.1, 80, 80, 0, 26);
	addLink(0, 1, 1, 8, 0.1, 80, 80, 0, 31);
	addLink(0, 1, 1, 12, 0.1, 80, 80, 0, 36);
	addLink(0, 1, 1, 4, 0.1, 80, 80, 0, 41);
	addLink(0, 1, 1, 4, 0.1, 80, 80, 1, 9);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 1, 10);
	addLink(0, 1, 1, 16, 0.1, 80, 80, 1, 20);
	addLink(0, 1, 1, 10, 0.1, 80, 80, 1, 27);
	addLink(0, 1, 1, 8, 0.1, 80, 80, 1, 32);
	addLink(0, 1, 1, 12, 0.1, 80, 80, 1, 37);
	addLink(0, 1, 1, 16, 0.1, 80, 80, 2, 21);
	addLink(0, 1, 1, 10, 0.1, 80, 80, 2, 28);
	addLink(0, 1, 1, 8, 0.1, 80, 80, 2, 33);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 3, 11);
	addLink(0, 1, 1, 16, 0.1, 80, 80, 3, 22);
	addLink(0, 1, 1, 10, 0.1, 80, 80, 3, 29);
	addLink(0, 1, 1, 8, 0.1, 80, 80, 3, 34);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 4, 13);
	addLink(0, 1, 1, 4, 0.1, 80, 80, 4, 12);
	addLink(0, 1, 1, 19, 0.1, 80, 80, 4, 23);
	addLink(0, 1, 1, 13, 0.1, 80, 80, 4, 30);
	addLink(0, 1, 1, 11, 0.1, 80, 80, 4, 35);
	addLink(0, 1, 1, 15, 0.1, 80, 80, 5, 38);
	addLink(0, 1, 1, 4, 0.1, 80, 80, 5, 42);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 5, 14);
	addLink(0, 1, 1, 4, 0.1, 80, 80, 5, 15);
	addLink(0, 1, 1, 13, 0.1, 80, 80, 6, 39);
	addLink(0, 1, 1, 4, 0.1, 80, 80, 6, 43);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 6, 16);
	addLink(0, 1, 1, 4, 0.1, 80, 80, 6, 17);
	addLink(0, 1, 1, 8, 0.1, 80, 80, 7, 25);
	addLink(0, 1, 1, 12, 0.1, 80, 80, 7, 40);
	addLink(0, 1, 1, 4, 0.1, 80, 80, 7, 44);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 7, 18);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 8, 0);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 19, 0);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 24, 0);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 26, 0);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 31, 0);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 36, 0);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 41, 0);
	addLink(0, 1, 1, 4, 0.1, 80, 80, 9, 1);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 10, 1);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 20, 1);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 27, 1);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 32, 1);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 37, 1);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 21, 2);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 28, 2);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 33, 2);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 11, 3);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 22, 3);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 29, 3);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 34, 3);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 13, 4);
	addLink(0, 1, 1, 4, 0.1, 80, 80, 12, 4);
	addLink(0, 1, 1, 4, 0.1, 80, 80, 23, 4);
	addLink(0, 1, 1, 4, 0.1, 80, 80, 30, 4);
	addLink(0, 1, 1, 4, 0.1, 80, 80, 35, 4);
	addLink(0, 1, 1, 4, 0.1, 80, 80, 38, 5);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 42, 5);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 14, 5);
	addLink(0, 1, 1, 4, 0.1, 80, 80, 15, 5);
	addLink(0, 1, 1, 2, 0.1, 80, 80, 39, 6);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 43, 6);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 16, 6);
	addLink(0, 1, 1, 4, 0.1, 80, 80, 17, 6);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 25, 7);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 40, 7);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 44, 7);
	addLink(0, 1, 1, 1, 0.1, 80, 80, 18, 7);
	addLink(1, 1, 1, 1, 0.1, 80, 80, 9, 8);
	addLink(1, 1, 1, 5, 0.1, 80, 80, 10, 8);
	addLink(1, 1, 1, 9, 0.1, 80, 80, 11, 8);
	addLink(1, 1, 1, 10, 0.1, 80, 80, 12, 8);
	addLink(1, 1, 1, 12, 0.1, 80, 80, 13, 8);
	addLink(1, 1, 1, 5, 0.1, 80, 80, 14, 8);
	addLink(1, 1, 1, 4, 0.1, 80, 80, 15, 8);
	addLink(1, 1, 1, 7, 0.1, 80, 80, 16, 8);
	addLink(1, 1, 1, 8, 0.1, 80, 80, 17, 8);
	addLink(1, 1, 1, 11, 0.1, 80, 80, 18, 8);
	addLink(1, 1, 1, 1, 0.1, 80, 80, 8, 9);
	addLink(1, 1, 1, 5, 0.1, 80, 80, 8, 10);
	addLink(1, 1, 1, 9, 0.1, 80, 80, 8, 11);
	addLink(1, 1, 1, 10, 0.1, 80, 80, 8, 12);
	addLink(1, 1, 1, 12, 0.1, 80, 80, 8, 13);
	addLink(1, 1, 1, 5, 0.1, 80, 80, 8, 14);
	addLink(1, 1, 1, 4, 0.1, 80, 80, 8, 15);
	addLink(1, 1, 1, 7, 0.1, 80, 80, 8, 16);
	addLink(1, 1, 1, 8, 0.1, 80, 80, 8, 17);
	addLink(1, 1, 1, 11, 0.1, 80, 80, 8, 18);
	addLink(3, 1, 1, 2, 0.1, 40, 40, 19, 20);
	addLink(3, 1, 1, 3, 0.1, 40, 40, 20, 21);
	addLink(3, 1, 1, 4, 0.1, 40, 40, 21, 22);
	addLink(3, 1, 1, 5, 0.1, 40, 40, 22, 23);
	addLink(3, 1, 1, 5, 0.1, 40, 40, 24, 25);
	addLink(3, 1, 1, 2, 0.1, 40, 40, 20, 19);
	addLink(3, 1, 1, 3, 0.1, 40, 40, 21, 20);
	addLink(3, 1, 1, 4, 0.1, 40, 40, 22, 21);
	addLink(3, 1, 1, 5, 0.1, 40, 40, 23, 22);
	addLink(3, 1, 1, 5, 0.1, 40, 40, 25, 24);
	addLink(2, 1, 1, 3, 0.1, 30, 30, 26, 27);
	addLink(2, 1, 1, 2, 0.1, 30, 30, 27, 28);
	addLink(2, 1, 1, 3, 0.1, 30, 30, 28, 29);
	addLink(2, 1, 1, 1, 0.1, 30, 30, 29, 30);
	addLink(2, 1, 1, 3, 0.1, 30, 30, 31, 32);
	addLink(2, 1, 1, 2, 0.1, 30, 30, 32, 33);
	addLink(2, 1, 1, 3, 0.1, 30, 30, 33, 34);
	addLink(2, 1, 1, 1, 0.1, 30, 30, 34, 35);
	addLink(2, 1, 1, 3, 0.1, 30, 30, 36, 37);
	addLink(2, 1, 1, 2, 0.1, 30, 30, 37, 38);
	addLink(2, 1, 1, 4, 0.1, 30, 30, 38, 39);
	addLink(2, 1, 1, 1, 0.1, 30, 30, 39, 40);
	addLink(2, 1, 1, 4, 0.1, 30, 30, 41, 42);
	addLink(2, 1, 1, 2, 0.1, 30, 30, 42, 43);
	addLink(2, 1, 1, 2, 0.1, 30, 30, 43, 44);
	addLink(2, 1, 1, 3, 0.1, 30, 30, 27, 26);
	addLink(2, 1, 1, 2, 0.1, 30, 30, 28, 27);
	addLink(2, 1, 1, 3, 0.1, 30, 30, 29, 28);
	addLink(2, 1, 1, 1, 0.1, 30, 30, 30, 29);
	addLink(2, 1, 1, 3, 0.1, 30, 30, 32, 31);
	addLink(2, 1, 1, 2, 0.1, 30, 30, 33, 32);
	addLink(2, 1, 1, 3, 0.1, 30, 30, 34, 33);
	addLink(2, 1, 1, 1, 0.1, 30, 30, 35, 34);
	addLink(2, 1, 1, 3, 0.1, 30, 30, 37, 36);
	addLink(2, 1, 1, 2, 0.1, 30, 30, 38, 37);
	addLink(2, 1, 1, 4, 0.1, 30, 30, 39, 38);
	addLink(2, 1, 1, 1, 0.1, 30, 30, 40, 39);
	addLink(2, 1, 1, 4, 0.1, 30, 30, 42, 41);
	addLink(2, 1, 1, 2, 0.1, 30, 30, 43, 42);
	addLink(2, 1, 1, 2, 0.1, 30, 30, 44, 43);
		

	//添加共享单车点***************************************************************************

	//设置共享单车停靠点容量
	dockCIni.push_back(54);
	dockCIni.push_back(15);
	dockCIni.push_back(30);
	dockCIni.push_back(19);
	dockCIni.push_back(19);
	dockCIni.push_back(19);
	dockCIni.push_back(15);
	dockCIni.push_back(18);
	dockCIni.push_back(18);
	dockCIni.push_back(18);
	dockCIni.push_back(30);

	/*dockCIni.push_back(1200);
	dockCIni.push_back(1200);
	dockCIni.push_back(1200);
	dockCIni.push_back(1200);
	dockCIni.push_back(1200);
	dockCIni.push_back(1200);
	dockCIni.push_back(1200);
	dockCIni.push_back(1200);
	dockCIni.push_back(1200);
	dockCIni.push_back(1200);
	dockCIni.push_back(1200);*/

	//添加停靠点
	addBicyclePoint(8, dockCIni[0]);
	addBicyclePoint(9, dockCIni[1]);
	addBicyclePoint(10, dockCIni[2]);
	addBicyclePoint(11, dockCIni[3]);
	addBicyclePoint(12, dockCIni[4]);
	addBicyclePoint(13, dockCIni[5]);
	addBicyclePoint(14, dockCIni[6]);
	addBicyclePoint(15, dockCIni[7]);
	addBicyclePoint(16, dockCIni[8]);
	addBicyclePoint(17, dockCIni[9]);
	addBicyclePoint(18, dockCIni[10]);

	//设置共享单车调度时间
	std::vector<double> dock1DeptTime;
	dock1DeptTime.push_back(0);
	dock1DeptTime.push_back(5);
	dock1DeptTime.push_back(5);
	dock1DeptTime.push_back(5);
	dock1DeptTime.push_back(5);
	dock1DeptTime.push_back(5);
	dock1DeptTime.push_back(5);
	dock1DeptTime.push_back(5);
	dock1DeptTime.push_back(5);
	dock1DeptTime.push_back(5);
	dock1DeptTime.push_back(5);
	dispatchTime.push_back(dock1DeptTime);

	std::vector<double> dock2DeptTime;
	dock2DeptTime.push_back(5);
	dock2DeptTime.push_back(0);
	dock2DeptTime.push_back(5);
	dock2DeptTime.push_back(5);
	dock2DeptTime.push_back(5);
	dock2DeptTime.push_back(5);
	dock2DeptTime.push_back(5);
	dock2DeptTime.push_back(5);
	dock2DeptTime.push_back(5);
	dock2DeptTime.push_back(5);
	dock2DeptTime.push_back(5);
	dispatchTime.push_back(dock2DeptTime);

	std::vector<double> dock3DeptTime;
	dock3DeptTime.push_back(5);
	dock3DeptTime.push_back(5);
	dock3DeptTime.push_back(0);
	dock3DeptTime.push_back(5);
	dock3DeptTime.push_back(5);
	dock3DeptTime.push_back(5);
	dock3DeptTime.push_back(5);
	dock3DeptTime.push_back(5);
	dock3DeptTime.push_back(5);
	dock3DeptTime.push_back(5);
	dock3DeptTime.push_back(5);
	dispatchTime.push_back(dock3DeptTime);

	std::vector<double> dock4DeptTime;
	dock4DeptTime.push_back(5);
	dock4DeptTime.push_back(5);
	dock4DeptTime.push_back(5);
	dock4DeptTime.push_back(0);
	dock4DeptTime.push_back(5);
	dock4DeptTime.push_back(5);
	dock4DeptTime.push_back(5);
	dock4DeptTime.push_back(5);
	dock4DeptTime.push_back(5);
	dock4DeptTime.push_back(5);
	dock4DeptTime.push_back(5);
	dispatchTime.push_back(dock4DeptTime);

	std::vector<double> dock5DeptTime;
	dock5DeptTime.push_back(5);
	dock5DeptTime.push_back(5);
	dock5DeptTime.push_back(5);
	dock5DeptTime.push_back(5);
	dock5DeptTime.push_back(0);
	dock5DeptTime.push_back(5);
	dock5DeptTime.push_back(5);
	dock5DeptTime.push_back(5);
	dock5DeptTime.push_back(5);
	dock5DeptTime.push_back(5);
	dock5DeptTime.push_back(5);
	dispatchTime.push_back(dock5DeptTime);

	std::vector<double> dock6DeptTime;
	dock6DeptTime.push_back(5);
	dock6DeptTime.push_back(5);
	dock6DeptTime.push_back(5);
	dock6DeptTime.push_back(5);
	dock6DeptTime.push_back(5);
	dock6DeptTime.push_back(0);
	dock6DeptTime.push_back(5);
	dock6DeptTime.push_back(5);
	dock6DeptTime.push_back(5);
	dock6DeptTime.push_back(5);
	dock6DeptTime.push_back(5);
	dispatchTime.push_back(dock6DeptTime);

	std::vector<double> dock7DeptTime;
	dock7DeptTime.push_back(5);
	dock7DeptTime.push_back(5);
	dock7DeptTime.push_back(5);
	dock7DeptTime.push_back(5);
	dock7DeptTime.push_back(5);
	dock7DeptTime.push_back(5);
	dock7DeptTime.push_back(0);
	dock7DeptTime.push_back(5);
	dock7DeptTime.push_back(5);
	dock7DeptTime.push_back(5);
	dock7DeptTime.push_back(5);
	dispatchTime.push_back(dock7DeptTime);

	std::vector<double> dock8DeptTime;
	dock8DeptTime.push_back(5);
	dock8DeptTime.push_back(5);
	dock8DeptTime.push_back(5);
	dock8DeptTime.push_back(5);
	dock8DeptTime.push_back(5);
	dock8DeptTime.push_back(5);
	dock8DeptTime.push_back(5);
	dock8DeptTime.push_back(0);
	dock8DeptTime.push_back(5);
	dock8DeptTime.push_back(5);
	dock8DeptTime.push_back(5);
	dispatchTime.push_back(dock8DeptTime);

	std::vector<double> dock9DeptTime;
	dock9DeptTime.push_back(5);
	dock9DeptTime.push_back(5);
	dock9DeptTime.push_back(5);
	dock9DeptTime.push_back(5);
	dock9DeptTime.push_back(5);
	dock9DeptTime.push_back(5);
	dock9DeptTime.push_back(5);
	dock9DeptTime.push_back(5);
	dock9DeptTime.push_back(0);
	dock9DeptTime.push_back(5);
	dock9DeptTime.push_back(5);
	dispatchTime.push_back(dock9DeptTime);

	std::vector<double> dock10DeptTime;
	dock10DeptTime.push_back(5);
	dock10DeptTime.push_back(5);
	dock10DeptTime.push_back(5);
	dock10DeptTime.push_back(5);
	dock10DeptTime.push_back(5);
	dock10DeptTime.push_back(5);
	dock10DeptTime.push_back(5);
	dock10DeptTime.push_back(5);
	dock10DeptTime.push_back(5);
	dock10DeptTime.push_back(0);
	dock10DeptTime.push_back(5);
	dispatchTime.push_back(dock10DeptTime);

	std::vector<double> dock11DeptTime;
	dock11DeptTime.push_back(5);
	dock11DeptTime.push_back(5);
	dock11DeptTime.push_back(5);
	dock11DeptTime.push_back(5);
	dock11DeptTime.push_back(5);
	dock11DeptTime.push_back(5);
	dock11DeptTime.push_back(5);
	dock11DeptTime.push_back(5);
	dock11DeptTime.push_back(5);
	dock11DeptTime.push_back(5);
	dock11DeptTime.push_back(0);
	dispatchTime.push_back(dock11DeptTime);

	//添加Ss
	dockS.push_back(80);
	dockS.push_back(30);
	dockS.push_back(50);
	dockS.push_back(40);
	dockS.push_back(40);
	dockS.push_back(40);
	dockS.push_back(30);
	dockS.push_back(40);
	dockS.push_back(40);
	dockS.push_back(40);
	dockS.push_back(50);

	docks.push_back(20);
	docks.push_back(10);
	docks.push_back(15);
	docks.push_back(10);
	docks.push_back(10);
	docks.push_back(10);
	docks.push_back(10);
	docks.push_back(10);
	docks.push_back(10);
	docks.push_back(10);
	docks.push_back(15);
}


CTM::~CTM()
{
	outFile.close();
}
