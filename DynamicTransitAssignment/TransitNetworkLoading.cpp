#include "stdafx.h"
#include "TransitNetworkLoading.h"
void TransitNetworkLoading::addNode(int nodeID, int type, double bicycleNum) {
	Node* node = new Node();
	node->nodeID = nodeID;
	node->type = type;
	if (type==1) {
		node->bicycleNum = bicycleNum;
	}
	TransitNetworkLoading::nodes.insert(std::pair<int, Node*>(nodeID,node));
}

void TransitNetworkLoading::addLink(int linkID, int type, double linkLen, int startNode, int endNode) {
	Link* link = new Link();
	link->linkID = linkID;
	link->type = type;
	link->linkLen = linkLen;
	//添加连接
	link->startNode = nodes[startNode];
	link->endNode = nodes[endNode];
	nodes[startNode]->outNodeLink.push_back(linkID);
	nodes[endNode]->inNodeLink.push_back(linkID);
	TransitNetworkLoading::links.insert(std::pair<int, Link*>(linkID, link));
}



void TransitNetworkLoading::addPassenger( int passengerGroup, int pathNo, std::vector<int> pathLinks, double passengerNum) {

	int startNode = 0;
	int endNode = 0;
	int inShareBicyclePoint1 = -1;
	int inShareBicyclePoint2 = -1;
	int outShareBicyclePoint1 = -1;
	int outShareBicyclePoint2 = -1;
	for (int i = 0; i < pathLinks.size();i++) {
		if (i == 0) {
			startNode = links[pathLinks[i]]->startNode->nodeID;
		}
		else if (i == pathLinks.size()-1) {
			endNode = links[pathLinks[i]]->endNode->nodeID;
		}
		if (links[pathLinks[i]]->startNode->type==1 && links[pathLinks[i]]->type==1) {
			if (outShareBicyclePoint1 == -1) {
				outShareBicyclePoint1 = links[pathLinks[i]]->startNode->nodeID;
			}
			else {
				outShareBicyclePoint2 = links[pathLinks[i]]->startNode->nodeID;
			}
		}
		if (links[pathLinks[i]]->endNode->type == 1 && links[pathLinks[i]]->type == 1) {
			if (inShareBicyclePoint1 == -1) {
				inShareBicyclePoint1 = links[pathLinks[i]]->endNode->nodeID;
			}
			else {
				inShareBicyclePoint2 = links[pathLinks[i]]->endNode->nodeID;
			}
		}
	}


	//设置共享单车数量（分配到线路时就直接减少，后面就不再减少）
	if (outShareBicyclePoint1 != -1) {
		nodes[outShareBicyclePoint1]->bicycleNum -= passengerNum;
	}
	if (outShareBicyclePoint2 != -1) {
		nodes[outShareBicyclePoint2]->bicycleNum -= passengerNum;
	}
	PassengerTravel *pt = new PassengerTravel();
	pt->passengerGroup = passengerGroup;
	pt->pathNo = pathNo;
	pt->type = nodes[startNode]->type;
	pt->startTime = simTime;
	pt->startNode = startNode;
	pt->endNode = endNode;
	pt->passengerNum = passengerNum;
	pt->travelCost = 0;
	pt->vehNo = -1;
	pt->nowNode = startNode;
	for (int i = 0; i < pathLinks.size(); i++) {
		pt->links.push_back(pathLinks[i]);
	}
	nodes[startNode]->passengers.push_back(pt);
	passengerTravels.push_back(pt);
}

void TransitNetworkLoading::simulation() {
	
	std::map<int, int> tlinkPassenger;
	std::map<int, int> troutePassenger;
	for (int i = 0; i < links.size();i++) {
		tlinkPassenger[i] = 0;
	}

	std::vector<double> tmDocks;
	for (int i = 0; i < sharedBikeNode.size(); i++) {

		if (nodes[sharedBikeNode[i]]->bicycleNum>-0.000001&& nodes[sharedBikeNode[i]]->bicycleNum < 0.000001 ) {
			nodes[sharedBikeNode[i]]->bicycleNum = 0;
		}

		tmDocks.push_back(nodes[sharedBikeNode[i]]->bicycleNum);

		if (i == sharedBikeNode.size() - 1)
			outFile << nodes[sharedBikeNode[i]]->bicycleNum << "\n";
		else
			outFile << nodes[sharedBikeNode[i]]->bicycleNum << " , ";
	}
	dockCStar.push_back(tmDocks);

	//根据时刻表发车
	for each (VehTimetable vt in vehTimetable)
	{
		troutePassenger[vt.vehicle.routeNo] = 0;
		if (vt.timetable.find(simTime)!= vt.timetable.end()) {//表示找到了
			Vehicle* veh = new Vehicle();
			veh->seatNum = vt.vehicle.seatNum;
			veh->capacity = vt.vehicle.capacity;
			veh->deptTime = simTime;
			veh->links = vt.vehicle.links;
			veh->nowLink = veh->links[0];
			veh->linkDis = 0;
			veh->routeNo = vt.vehicle.routeNo;
			veh->speed = vt.timetable[simTime];
			veh->vehNo = vehNo;
			veh->type = vt.vehicle.type;
			veh->nodeTime[links[veh->links[0]]->startNode->nodeID]= simTime;
			vehNo++;
			vehicles.push_back(veh);
			vehicleBeifen.push_back(veh);
		}
	}
	//std::cout << simTime << std::endl;
	//for (int i = 0; i < vehicles.size(); i++) {
	//	std::cout << "Route"<< vehicles[i]->routeNo << ";  vehID:" << vehicles[i]->vehNo 
	//		<<";  Vehpos:"<< vehicles[i]->nowLink 
	//		<< ";  passengerNum:" << vehicles[i]->passengerNum << std::endl;
	//}

	simTime++;
	//便利所有乘客，步行或者是骑车
	int ptsSize = passengerTravels.size();
	for(int i=0;i< ptsSize;i++)
	{
		
		PassengerTravel* pt = passengerTravels[i];

		pt->justArrNode = false;
		if (pt->type==0) {//乘客在步行站点或者步行link,步行会到其他站点
			if (pt->nowNode!=-1 && nodes[pt->nowNode]->type==0) {//在节点,需要前行
				for each (int link in nodes[pt->nowNode]->outNodeLink)//便利当前节点的所有可行link
				{
					if (std::find(pt->links.begin(), pt->links.end(), link) != pt->links.end()) {//如果连接在path的连接内，表示当前连接找到了
						//删除node中的乘客
						for (int k = 0; k < nodes[pt->nowNode]->passengers.size(); k++) {
							PassengerTravel* tmPtn = nodes[pt->nowNode]->passengers[k];
							if (tmPtn->passengerGroup == pt->passengerGroup && tmPtn->pathNo == pt->pathNo
								&& tmPtn->startTime == pt->startTime && tmPtn->passengerNum == pt->passengerNum) {
								nodes[pt->nowNode]->passengers.erase(nodes[pt->nowNode]->passengers.begin() + k);
								break;
							}
						}

						if (links[link]->linkLen<=walkSpeed) {//如果单位时间就走到了下一个节点
							pt->nowNode = links[link]->endNode->nodeID;
							if(links[link]->linkLen!=0)
								pt->justArrNode = true;
							pt->type = nodes[pt->nowNode]->type;
							if (pt->nowNode == pt->endNode) {//********************************************************************************************************//乘客到达终点
								pt->endTime = simTime;
								finishPassengerTravels.push_back(pt);
								passengerTravels.erase(passengerTravels.begin() + i);
								ptsSize--;
								i--;
							}
							else {
								nodes[pt->nowNode]->passengers.push_back(pt);
							}
						}
						else {//还在link里面
							pt->nowNode = -1;
							pt->nowLink = link;
							pt->linkDis += walkSpeed;
							tlinkPassenger[pt->nowLink] += pt->passengerNum;
						}
					}
				}
			}
			else {//在link继续前行
				if (pt->linkDis + walkSpeed >= links[pt->nowLink]->linkLen) {//如果走完了link了
					int link = pt->nowLink;
					pt->nowNode = links[link]->endNode->nodeID;
					pt->type = nodes[pt->nowNode]->type;
					pt->nowLink = -1;
					pt->linkDis = 0;
					if (links[link]->linkLen != 0)
						pt->justArrNode = true;
					
					if (pt->nowNode == pt->endNode) {//********************************************************************************************************//乘客到达终点
						pt->endTime = simTime;
						finishPassengerTravels.push_back(pt);
						passengerTravels.erase(passengerTravels.begin() + i);
						ptsSize--;
						i--;
					}
					else {
						nodes[pt->nowNode]->passengers.push_back(pt);
					}
				}
				else {//还在link只需要前进
					pt->linkDis += walkSpeed;
					tlinkPassenger[pt->nowLink] += pt->passengerNum;
				}
			
			}
		}
		else if (pt->type == 1) {//骑车只会到自行车站点
			if (pt->nowNode != -1 && nodes[pt->nowNode]->type == 1) {//在节点,需要前行
				for each (int link in nodes[pt->nowNode]->outNodeLink)//便利当前节点的所有可行link
				{
					if (std::find(pt->links.begin(), pt->links.end(), link) != pt->links.end()) {//如果连接在path的连接内，表示当前连接找到了

						//删除node中的乘客
						for (int k = 0; k < nodes[pt->nowNode]->passengers.size(); k++) {
							PassengerTravel* tmPtn = nodes[pt->nowNode]->passengers[k];
							if (tmPtn->passengerGroup == pt->passengerGroup && tmPtn->pathNo == pt->pathNo
								&& tmPtn->startTime == pt->startTime && tmPtn->passengerNum == pt->passengerNum) {
								nodes[pt->nowNode]->passengers.erase(nodes[pt->nowNode]->passengers.begin() + k);
								break;
							}
						}

						double speed = 0;
						if (links[link]->type==0) {//步行
							speed = walkSpeed;
							pt->type = 0;
						}
						else if (links[link]->type == 1) {//共享单车
							speed = bicycleSpeed;
							pt->type = 1;
						}

						if (links[link]->linkLen <= speed) {//如果单位时间就走到了下一个节点
							if (pt->type == 1) {
								pt->bikeDis += links[link]->linkLen;
								pt->bikeTime += 1;
							}								
							pt->nowNode = links[link]->endNode->nodeID;
							if (links[link]->linkLen != 0)
								pt->justArrNode = true;
							if (links[link]->type == 1) {//共享单车
								nodes[pt->nowNode]->bicycleNum += pt->passengerNum;//还车*************************************************************还车
							}
							//pt.type = nodes[pt.nowNode]->type;
							if (pt->nowNode == pt->endNode) {//********************************************************************************************************//乘客到达终点
								pt->endTime = simTime;
								finishPassengerTravels.push_back(pt);
								passengerTravels.erase(passengerTravels.begin() + i);
								ptsSize--;
								i--;
							}
							else {
								nodes[pt->nowNode]->passengers.push_back(pt);
							}
						}
						else {//还在link里面
							
							pt->nowNode = -1;
							pt->nowLink = link;
							pt->linkDis += speed;
							if (pt->type == 1) {
								pt->bikeDis += speed;
								pt->bikeTime += 1;
							}	
							tlinkPassenger[pt->nowLink] += pt->passengerNum;
						}
					}
				}
			}
			else {
				if (pt->linkDis + bicycleSpeed >= links[pt->nowLink]->linkLen) {//如果走完了link了
					int link = pt->nowLink;
					pt->nowNode = links[link]->endNode->nodeID;
					pt->bikeDis += links[pt->nowLink]->linkLen - pt->linkDis;
					if (links[link]->linkLen != 0)
						pt->justArrNode = true;
					pt->bikeTime += 1;
					pt->nowLink = -1;
					pt->linkDis = 0;					
					nodes[pt->nowNode]->bicycleNum += pt->passengerNum;//还车*************************************************************还车
					//pt.type = nodes[pt.nowNode]->type;
					if (pt->nowNode == pt->endNode) {//********************************************************************************************************//乘客到达终点
						pt->endTime = simTime;
						finishPassengerTravels.push_back(pt);
						passengerTravels.erase(passengerTravels.begin() + i);
						ptsSize--;
						i--;
					}
					else {
						nodes[pt->nowNode]->passengers.push_back(pt);
					}
				}
				else {//还在link只需要前进
					tlinkPassenger[pt->nowLink] += pt->passengerNum;
					pt->linkDis += bicycleSpeed;
					pt->bikeDis += bicycleSpeed;
					pt->bikeTime += 1;
				}
			}
		}
		else if ((pt->type==2|| pt->type == 3)&&pt->nowNode!=-1) {//在公交或者是BRT站点，已经到达终点或者是需要步行
			for (int j = 0; j < pt->links.size();j++) {
				int linkId = pt->links[j];
				if (links[linkId]->startNode->nodeID == pt->nowNode && links[linkId]->type==0) {//是步行
					//需要从之前的node里面删除
					for (int k = 0; k < nodes[pt->nowNode]->passengers.size();k++) {
						PassengerTravel* tmPtn = nodes[pt->nowNode]->passengers[k];
						if (tmPtn->passengerGroup == pt->passengerGroup && tmPtn->pathNo == pt->pathNo
							&& tmPtn->startTime == pt->startTime && tmPtn->passengerNum == pt->passengerNum) {
							nodes[pt->nowNode]->passengers.erase(nodes[pt->nowNode]->passengers.begin()+k);
							break;
						}
					}
					if (links[linkId]->linkLen<=walkSpeed) {//直接步行到了下一个节点
						pt->nowNode = links[linkId]->endNode->nodeID;
						pt->type = nodes[pt->nowNode]->type;
						if (links[linkId]->linkLen != 0)
							pt->justArrNode = true;
						//判断是不是终点
						if (pt->nowNode == pt->endNode) {
							pt->endTime = simTime;
							finishPassengerTravels.push_back(pt);
							passengerTravels.erase(passengerTravels.begin() + i);
							ptsSize--;
							i--;
						}
						else {
							nodes[pt->nowNode]->passengers.push_back(pt);
						}
					}
					else {//在link里面
						pt->type = links[linkId]->type;
						pt->nowNode = -1;
						pt->nowLink = linkId;
						pt->linkDis += walkSpeed;
						tlinkPassenger[pt->nowLink] += pt->passengerNum;
					}
				}
			}
		}
	}
	
	//遍历所有车辆，只运行与上下客
	for (int i = 0; i < vehicles.size();i++) {
		Vehicle* v = vehicles[i];
		//运行车辆，上客
		if (v->linkDis == 0) {//表示出发，需要上客
			int node = links[v->nowLink]->startNode->nodeID;
			int pn = nodes[node]->passengers.size();
			for (int j = 0; j < pn;j++) {
				PassengerTravel* pt = nodes[node]->passengers[j];
				if (pt->justArrNode==false&&std::find(pt->links.begin(), pt->links.end(), v->nowLink) != pt->links.end()) {//表示乘客在等这个车，需要上车
										
					//要删除node中的乘客
					if (v->capacity >= v->passengerNum + pt->passengerNum) {//所有乘客已经上车

						pt->nowLink = v->nowLink;
						pt->nowNode = -1;
						pt->vehNo = v->vehNo;
						pt->type = links[v->nowLink]->type;
						v->passengers.push_back(pt);

						v->passengerNum += pt->passengerNum;
						nodes[node]->passengers.erase(nodes[node]->passengers.begin() + j);
						pn--;
						j--;
					}
					else {//有乘客不能够上车
						double asPn = v->capacity - v->passengerNum - pt->passengerNum;
						pt->passengerNum -= asPn;
						v->passengerNum = v->capacity;
						PassengerTravel *nPt = new PassengerTravel();
						nPt->pathNo = pt->pathNo;
						nPt->passengerGroup = pt->passengerGroup;
						nPt->endNode = pt->endNode;
						nPt->linkDis = pt->linkDis;
						nPt->links = pt->links;
						nPt->nowLink = v->nowLink;
						nPt->nowNode = -1;
						nPt->passengerNum = asPn;
						nPt->startNode = pt->startNode;
						nPt->startTime = pt->startTime;
						nPt->travelCost = pt->travelCost;
						nPt->type = links[v->nowLink]->type;
						nPt->vehNo = v->vehNo;
						v->passengers.push_back(nPt);
						passengerTravels.push_back(nPt);
					}
				}
			}
			v->linkPassengerNum[v->nowLink] = v->passengerNum;
		}
		//更新车位置
		if (v->linkDis + v->speed >= links[v->nowLink]->linkLen) {//如果直接到了下一个连接
			for (int j = 0; j < v->links.size();j++) {
				if (v->links[j] == v->nowLink) {//找到了当前的link
					if (j== v->links.size()-1) {//最后一个连接
						v->finish = true;
						v->nodeTime[links[v->nowLink]->endNode->nodeID] = simTime;
						//v->nodeTime[links[v->nowLink]->endNode->nodeID] = simTime + 1;
					}
					else {
						
						v->linkDis = 0;
						v->nowLink = v->links[j + 1];
						v->nodeTime[links[v->nowLink]->startNode->nodeID] = simTime;
						//v->nodeTime[links[v->nowLink]->startNode->nodeID] = simTime + 1;

						break;
					}
				}
			}				
		}
		else {
			v->linkDis += v->speed;
		}

		//更新乘客成本
		if (v->passengerNum > v->seatNum) {
			for (int j = 0; j < v->passengers.size();j++) {
				if(v->passengerNum > v->seatNum)
					v->passengers[j]->travelCost += theta*(v->passengerNum - v->seatNum) / v->seatNum;
			}			
		}
		
	}

	int vS = vehicles.size();
	for (int i = 0; i < vS; i++) {
		Vehicle* v = vehicles[i];
		
		
		//下客,更新乘客的link
		if (v->linkDis == 0 && !v->finish) {//表示到达，需要下客
			int pn = v->passengers.size();
			for (int j = 0; j < pn;j++) {//遍历车上的乘客
				PassengerTravel* pt = v->passengers[j];
				if (std::find(pt->links.begin(), pt->links.end(), v->nowLink) != pt->links.end()) {//表示乘客需要走当前的连接，不需要下车
					pt->nowLink = v->nowLink;
					pt->type = links[v->nowLink]->type;
					pt->linkDis = 0;
				}
				else {//表示乘客需要下车
					int altNode = links[v->nowLink]->startNode->nodeID;//下车站点	
					pt->nowLink = -1;
					pt->linkDis = -1;
					pt->justArrNode = true;
					pt->nowNode = altNode;
					pt->vehNo = -1;
					if (altNode == pt->endNode) {//表示乘客结束*********************************************************************************************************************
						pt->endTime = simTime;
						finishPassengerTravels.push_back(pt);
						for (int k = 0; k < passengerTravels.size();k++) {
							if (passengerTravels[k]->passengerGroup == pt->passengerGroup && passengerTravels[k]->pathNo == pt->pathNo
								&& passengerTravels[k]->startTime == pt->startTime && passengerTravels[k]->passengerNum == pt->passengerNum) {
								passengerTravels.erase(passengerTravels.begin()+k);
								break;
							}
						}
					}
					else {
						nodes[altNode]->passengers.push_back(pt);						
					}
					v->passengerNum -= pt->passengerNum;
					v->passengers.erase(v->passengers.begin()+j);
					j--;
					pn--;
				}				
			}
		}
		else if (v->finish) {//表示车到站了
			int pn = v->passengers.size();
			for (int j = 0; j < pn; j++) {//遍历车上的乘客
				PassengerTravel* pt = v->passengers[j];
				int altNode = links[v->nowLink]->endNode->nodeID;//下车站点	
				pt->nowLink = -1;
				pt->linkDis = -1;
				pt->nowNode = altNode;
				pt->justArrNode = true;
				pt->vehNo = -1;
				if (altNode == pt->endNode) {//表示乘客结束*********************************************************************************************************************
					pt->endTime = simTime;
					finishPassengerTravels.push_back(pt);
					for (int k = 0; k < passengerTravels.size(); k++) {
						if (passengerTravels[k]->passengerGroup == pt->passengerGroup && passengerTravels[k]->pathNo == pt->pathNo
							&& passengerTravels[k]->startTime == pt->startTime && passengerTravels[k]->passengerNum == pt->passengerNum) {
							passengerTravels.erase(passengerTravels.begin() + k);
							break;
						}
					}
				}
				else {
					nodes[altNode]->passengers.push_back(pt);
				}
				v->passengerNum -= pt->passengerNum;
				v->passengers.erase(v->passengers.begin() + j);
				j--;
				pn--;
			}
			vehicles.erase(vehicles.begin()+i);
			//delete v;
			vS--;
			i--;
		}

		if (!v->finish) {
			tlinkPassenger[v->nowLink] += v->passengerNum;
			troutePassenger[v->routeNo] += v->passengerNum;
		}
	}

	std::map<int, TravelModePassenger> odTravelModePassenger;

	TravelModePassenger travelModePassenger; 
	travelModePassenger.bikePassengerNum = 0;
	travelModePassenger.busPassengerNum = 0;
	travelModePassenger.railPassengerNum = 0;



	/*vS = vehicles.size();
	for (int i = 0; i < vS; i++) {
		Vehicle* v = vehicles[i];
		if (v->passengerNum > 0.0001) {
			if (v->type == 1) {//公交
				travelModePassenger.busPassengerNum += v->passengerNum;
			}
			else if (v->type == 2) {//地铁
				travelModePassenger.railPassengerNum += v->passengerNum;
			}
		}

		//int tpn = v->passengers.size();
		//for (int j = 0; j < tpn;j++) {
		//	PassengerTravel* pt = v->passengers[j];
		//	if (pt->passengerNum > 0.0001) {

		//		int pGn = pt->passengerGroup;
		//		TravelModePassenger odTmTravelModePassenger;
		//		if (odTravelModePassenger.find(pGn) != odTravelModePassenger.end()) {//找到了key
		//			odTmTravelModePassenger = odTravelModePassenger[pGn];
		//		}
		//		else {
		//			odTmTravelModePassenger.bikePassengerNum = 0;
		//			odTmTravelModePassenger.busPassengerNum = 0;
		//			odTmTravelModePassenger.railPassengerNum = 0;
		//		}

		//		if (v->type == 1) {//公交
		//			odTmTravelModePassenger.busPassengerNum += pt->passengerNum;
		//		}
		//		else if (v->type == 2) {//地铁
		//			odTmTravelModePassenger.railPassengerNum += pt->passengerNum;
		//		}

		//		odTravelModePassenger[pGn] = odTmTravelModePassenger;
		//	}
		//}
		
	}*/

	ptsSize = passengerTravels.size();
	for (int i = 0; i < ptsSize; i++)
	{
		PassengerTravel* pt = passengerTravels[i];
		if (pt->passengerNum > 0.0001) {
			int vehN = pt->vehNo;
			int pGn = pt->passengerGroup;
			TravelModePassenger odTmTravelModePassenger;
			if (odTravelModePassenger.find(pGn) != odTravelModePassenger.end()) {//找到了key
				odTmTravelModePassenger = odTravelModePassenger[pGn];
			}
			else {
				odTmTravelModePassenger.bikePassengerNum = 0;
				odTmTravelModePassenger.busPassengerNum = 0;
				odTmTravelModePassenger.railPassengerNum = 0;
			}

			if (pt->type == 1 ) {
				travelModePassenger.bikePassengerNum += pt->passengerNum;
				odTmTravelModePassenger.bikePassengerNum += pt->passengerNum;
			}else if (pt->type == 2) {//在公交
				travelModePassenger.busPassengerNum += pt->passengerNum;
				odTmTravelModePassenger.busPassengerNum += pt->passengerNum;
			}
			else if (pt->type == 3) {//地铁
				travelModePassenger.railPassengerNum += pt->passengerNum;
				odTmTravelModePassenger.railPassengerNum += pt->passengerNum;
			}
			odTravelModePassenger[pGn] = odTmTravelModePassenger;
		}
	}

	allTravelModePassenger.push_back(travelModePassenger);
	allODTravelModePassenger.push_back(odTravelModePassenger);


	

	linkPassenger.push_back(tlinkPassenger);
	routePassenger.push_back(troutePassenger);

	//simTime++;
}

void TransitNetworkLoading::clearNetwork() {
	simTime = 0;
	vehNo = 0;

	linkPassenger.clear();
	routePassenger.clear();

	linkVehicleLastBeifen.clear();
	

	allTravelModePassenger.clear();


	allODTravelModePassenger.clear();

	int vn = vehicleLastBeifen.size() - 1;
	for (; vn >= 0; vn--) {
		Vehicle* v = vehicleLastBeifen[vn];
		vehicleLastBeifen.erase(vehicleLastBeifen.begin() + vn);
		delete v;
	}	
	vehicleLastBeifen.clear();
	vehicleLastBeifenMap.clear();

	vn = vehicleBeifen.size() - 1;
	for (; vn >= 0; vn--) {
		Vehicle* v = vehicleBeifen[vn];
		vehicleBeifen.erase(vehicleBeifen.begin() + vn);
		for (int l : v->links) {
			if (linkVehicleLastBeifen.find(l) == linkVehicleLastBeifen.end()) {//没有这个元素
				std::vector<Vehicle*> t;
				t.push_back(v);
				linkVehicleLastBeifen[l] = t;
			}
			else {
				linkVehicleLastBeifen[l].push_back(v);
			}
		}
		vehicleLastBeifenMap[v->vehNo] = v;
		vehicleLastBeifen.push_back(v);
	}
	vehicleBeifen.clear();
	vehicles.clear();

	//double travelC = calVehTravelTime(6, 2, 20);

	int ptn = finishPassengerTravels.size()-1;
	for (; ptn>=0; ptn--) {
		PassengerTravel* pt = finishPassengerTravels[ptn];
		finishPassengerTravels.erase(finishPassengerTravels.begin()+ ptn);
		delete pt;
	}
	for (int i = 0; i < dockCIni.size();i++) {
		nodes[sharedBikeNode[i]]->bicycleNum = dockCIni[i];
	}
	for (auto& n : nodes) {
		n.second->passengers.clear();
	}
	for (auto& l : links) {
		l.second->vehicles.clear();
	}
	finishPassengerTravels.clear();
	dockCStar.clear();
	//test

	/*std::vector<int> path;
	std::vector<int> typeList;
	typeList.push_back(0);
	typeList.push_back(2);
	int travelTime = 0;
	double travelCost = 0;
	path = Dijkstra(7,0,20, typeList, travelTime, travelCost);	*/
}

void TransitNetworkLoading::setVehTimetable(int routeNo, int type, int capacity, int seatNum, std::vector<int> links, std::map<int, double> timetable)
{
	Vehicle v;
	v.capacity = capacity;
	v.routeNo = routeNo;
	v.type = type;
	v.seatNum = seatNum;
	v.links = links;
	VehTimetable vt;
	vt.vehicle = v;
	vt.timetable = timetable;
	vehTimetable.push_back(vt);
}

TransitNetworkLoading::TransitNetworkLoading() {
	outFile.open("bicycle.csv");
	addNode(0, 0);
	addNode(1, 0);
	addNode(2, 0);
	addNode(3, 0);
	addNode(4, 0);
	addNode(5, 0);
	addNode(6, 0);
	addNode(7, 0);
	addNode(8, 0);
	addNode(9, 0);
	addNode(10, 0);
	addNode(11, 0);
	addNode(12, 0);
	addNode(13, 0);
	addNode(14, 0);
	addNode(15, 0);
	addNode(16, 0);
	addNode(17, 0);
	addNode(18, 0);

	dockCIni.push_back(20);
	dockCIni.push_back(20);
	dockCIni.push_back(50);
	dockCIni.push_back(80);
	dockCIni.push_back(20);
	dockCIni.push_back(50);
	dockCIni.push_back(10);
	dockCIni.push_back(25);
	dockCIni.push_back(200);
	dockCIni.push_back(30);
	dockCIni.push_back(35);
	dockCIni.push_back(20);
	dockCIni.push_back(40);
	dockCIni.push_back(10);
	dockCIni.push_back(150);
	dockCIni.push_back(10);
	dockCIni.push_back(80);
	dockCIni.push_back(20);
	dockCIni.push_back(0);

	/*dockCIni.push_back(9999);
	dockCIni.push_back(9999);
	dockCIni.push_back(9999);
	dockCIni.push_back(9999);
	dockCIni.push_back(9999);
	dockCIni.push_back(9999);
	dockCIni.push_back(9999);
	dockCIni.push_back(9999);
	dockCIni.push_back(9999);
	dockCIni.push_back(9999);
	dockCIni.push_back(9999);
	dockCIni.push_back(9999);
	dockCIni.push_back(9999);
	dockCIni.push_back(9999);
	dockCIni.push_back(9999);
	dockCIni.push_back(9999);
	dockCIni.push_back(9999);
	dockCIni.push_back(9999);
	dockCIni.push_back(9999);*/

	/*dockCIni.push_back(0);
	dockCIni.push_back(0);
	dockCIni.push_back(0);
	dockCIni.push_back(0);
	dockCIni.push_back(0);
	dockCIni.push_back(0);
	dockCIni.push_back(0);
	dockCIni.push_back(0);
	dockCIni.push_back(0);
	dockCIni.push_back(0);
	dockCIni.push_back(0);
	dockCIni.push_back(0);
	dockCIni.push_back(0);
	dockCIni.push_back(0);
	dockCIni.push_back(0);
	dockCIni.push_back(0);
	dockCIni.push_back(0);
	dockCIni.push_back(0);
	dockCIni.push_back(0);*/

	addNode(19, 1, dockCIni[0]);
	addNode(20, 1, dockCIni[1]);
	addNode(21, 1, dockCIni[2]);
	addNode(22, 1, dockCIni[3]);
	addNode(23, 1, dockCIni[4]);
	addNode(24, 1, dockCIni[5]);
	addNode(25, 1, dockCIni[6]);
	addNode(26, 1, dockCIni[7]);
	addNode(27, 1, dockCIni[8]);
	addNode(28, 1, dockCIni[9]);
	addNode(29, 1, dockCIni[10]);
	addNode(30, 1, dockCIni[11]);
	addNode(31, 1, dockCIni[12]);
	addNode(32, 1, dockCIni[13]);
	addNode(33, 1, dockCIni[14]);
	addNode(34, 1, dockCIni[15]);
	addNode(35, 1, dockCIni[16]);
	addNode(36, 1, dockCIni[17]);
	addNode(37, 1, dockCIni[18]);


	addNode(38, 3);
	addNode(39, 3);
	addNode(40, 3);
	addNode(41, 3);
	addNode(42, 3);
	addNode(43, 3);
	addNode(44, 3);
	addNode(45, 3);
	addNode(46, 2);
	addNode(47, 2);
	addNode(48, 2);
	addNode(49, 2);
	addNode(50, 2);
	addNode(51, 2);
	addNode(52, 2);
	addNode(53, 2);
	addNode(54, 2);
	addNode(55, 2);
	addNode(56, 2);
	addNode(57, 2);
	addNode(58, 2);
	addNode(59, 2);
	addNode(60, 2);
	addNode(61, 2);
	addNode(62, 2);
	addNode(63, 2);
	addNode(64, 2);
	addNode(65, 2);
	addNode(66, 2);
	addNode(67, 2);
	addNode(68, 2);
	addNode(69, 2);
	addNode(70, 2);
	addNode(71, 2);
	addNode(72, 2);
	addNode(73, 2);
	addNode(74, 2);
	addNode(75, 2);
	addNode(76, 2);
	addNode(77, 2);
	addNode(78, 2);
	addNode(79, 2);
	addNode(80, 2);
	addNode(81, 2);
	addNode(82, 2);
	addNode(83, 2);

	addLink(0, 2, 1800, 46, 47);
	addLink(1, 2, 1700, 47, 48);
	addLink(2, 2, 1500, 48, 49);
	addLink(3, 2, 1500, 49, 48);
	addLink(4, 2, 1700, 48, 47);
	addLink(5, 2, 1800, 47, 46);
	addLink(6, 2, 1800, 50, 51);
	addLink(7, 2, 1500, 51, 52);
	addLink(8, 2, 700, 52, 53);
	addLink(9, 2, 700, 53, 52);
	addLink(10, 2, 1500, 52, 51);
	addLink(11, 2, 1800, 51, 50);
	addLink(12, 2, 1700, 54, 55);
	addLink(13, 2, 1500, 55, 56);
	addLink(14, 2, 700, 56, 57);
	addLink(15, 2, 900, 57, 58);
	addLink(16, 2, 800, 58, 59);
	addLink(17, 2, 800, 59, 58);
	addLink(18, 2, 900, 58, 57);
	addLink(19, 2, 700, 57, 56);
	addLink(20, 2, 1500, 56, 55);
	addLink(21, 2, 1700, 55, 54);
	addLink(22, 2, 1800, 60, 61);
	addLink(23, 2, 1500, 61, 62);
	addLink(24, 2, 750, 62, 63);
	addLink(25, 2, 2100, 63, 64);
	addLink(26, 2, 2200, 64, 65);
	addLink(27, 2, 2800, 65, 66);
	addLink(28, 2, 1800, 66, 67);
	addLink(29, 2, 1800, 67, 66);
	addLink(30, 2, 2800, 66, 65);
	addLink(31, 2, 2200, 65, 64);
	addLink(32, 2, 2100, 64, 63);
	addLink(33, 2, 750, 63, 62);
	addLink(34, 2, 1500, 62, 61);
	addLink(35, 2, 1800, 61, 60);
	addLink(36, 2, 750, 68, 69);
	addLink(37, 2, 750, 69, 70);
	addLink(38, 2, 9400, 70, 71);
	addLink(39, 2, 9400, 71, 70);
	addLink(40, 2, 750, 70, 69);
	addLink(41, 2, 750, 69, 68);
	addLink(42, 2, 800, 72, 73);
	addLink(43, 2, 1000, 73, 74);
	addLink(44, 2, 1300, 74, 75);
	addLink(45, 2, 800, 75, 76);
	addLink(46, 2, 800, 76, 75);
	addLink(47, 2, 1300, 75, 74);
	addLink(48, 2, 1000, 74, 73);
	addLink(49, 2, 800, 73, 72);
	addLink(50, 2, 800, 77, 78);
	addLink(51, 2, 1100, 78, 79);
	addLink(52, 2, 1200, 79, 80);
	addLink(53, 2, 1500, 80, 81);
	addLink(54, 2, 950, 81, 82);
	addLink(55, 2, 6700, 82, 83);
	addLink(56, 2, 6700, 83, 82);
	addLink(57, 2, 950, 82, 81);
	addLink(58, 2, 1500, 81, 80);
	addLink(59, 2, 1200, 80, 79);
	addLink(60, 2, 1100, 79, 78);
	addLink(61, 2, 800, 78, 77);
	addLink(62, 3, 700, 38, 39);
	addLink(63, 3, 900, 39, 40);
	addLink(64, 3, 1100, 40, 41);
	addLink(65, 3, 1200, 41, 42);
	addLink(66, 3, 1500, 42, 43);
	addLink(67, 3, 950, 43, 44);
	addLink(68, 3, 6700, 44, 45);
	addLink(69, 3, 6700, 45, 44);
	addLink(70, 3, 950, 44, 43);
	addLink(71, 3, 1500, 43, 42);
	addLink(72, 3, 1200, 42, 41);
	addLink(73, 3, 1100, 41, 40);
	addLink(74, 3, 900, 40, 39);
	addLink(75, 3, 700, 39, 38);
	addLink(76, 1, 1800, 19, 20);
	addLink(77, 1, 3300, 19, 21);
	addLink(78, 1, 4000, 19, 22);
	addLink(79, 1, 5700, 19, 23);
	addLink(80, 1, 4900, 19, 24);
	addLink(81, 1, 6000, 19, 25);
	addLink(82, 1, 7300, 19, 26);
	addLink(83, 1, 3500, 19, 27);
	addLink(84, 1, 4050, 19, 28);
	addLink(85, 1, 6150, 19, 29);
	addLink(86, 1, 6900, 19, 30);
	addLink(87, 1, 5000, 19, 31);
	addLink(88, 1, 8400, 19, 32);
	addLink(89, 1, 11200, 19, 33);
	addLink(90, 1, 7650, 19, 34);
	addLink(91, 1, 9350, 19, 35);
	addLink(92, 1, 13000, 19, 36);
	addLink(93, 1, 16050, 19, 37);
	addLink(94, 1, 1500, 20, 21);
	addLink(95, 1, 2200, 20, 22);
	addLink(96, 1, 3900, 20, 23);
	addLink(97, 1, 3100, 20, 24);
	addLink(98, 1, 4200, 20, 25);
	addLink(99, 1, 5500, 20, 26);
	addLink(100, 1, 1700, 20, 27);
	addLink(101, 1, 2250, 20, 28);
	addLink(102, 1, 4350, 20, 29);
	addLink(103, 1, 5100, 20, 30);
	addLink(104, 1, 3200, 20, 31);
	addLink(105, 1, 6600, 20, 32);
	addLink(106, 1, 9400, 20, 33);
	addLink(107, 1, 5850, 20, 34);
	addLink(108, 1, 7550, 20, 35);
	addLink(109, 1, 11200, 20, 36);
	addLink(110, 1, 14250, 20, 37);
	addLink(111, 1, 700, 21, 22);
	addLink(112, 1, 2400, 21, 23);
	addLink(113, 1, 1600, 21, 24);
	addLink(114, 1, 2700, 21, 25);
	addLink(115, 1, 4000, 21, 26);
	addLink(116, 1, 3200, 21, 27);
	addLink(117, 1, 750, 21, 28);
	addLink(118, 1, 2850, 21, 29);
	addLink(119, 1, 3600, 21, 30);
	addLink(120, 1, 4700, 21, 31);
	addLink(121, 1, 5100, 21, 32);
	addLink(122, 1, 7900, 21, 33);
	addLink(123, 1, 4350, 21, 34);
	addLink(124, 1, 6050, 21, 35);
	addLink(125, 1, 9700, 21, 36);
	addLink(126, 1, 12750, 21, 37);
	addLink(127, 1, 1700, 22, 23);
	addLink(128, 1, 900, 22, 24);
	addLink(129, 1, 2000, 22, 25);
	addLink(130, 1, 3300, 22, 26);
	addLink(131, 1, 3900, 22, 27);
	addLink(132, 1, 1450, 22, 28);
	addLink(133, 1, 3550, 22, 29);
	addLink(134, 1, 3200, 22, 30);
	addLink(135, 1, 5400, 22, 31);
	addLink(136, 1, 4700, 22, 32);
	addLink(137, 1, 7500, 22, 33);
	addLink(138, 1, 3950, 22, 34);
	addLink(139, 1, 5650, 22, 35);
	addLink(140, 1, 9300, 22, 36);
	addLink(141, 1, 12350, 22, 37);
	addLink(142, 1, 800, 23, 24);
	addLink(143, 1, 1000, 23, 25);
	addLink(144, 1, 2300, 23, 26);
	addLink(145, 1, 5600, 23, 27);
	addLink(146, 1, 3150, 23, 28);
	addLink(147, 1, 2950, 23, 29);
	addLink(148, 1, 2200, 23, 30);
	addLink(149, 1, 7100, 23, 31);
	addLink(150, 1, 3700, 23, 32);
	addLink(151, 1, 6500, 23, 33);
	addLink(152, 1, 2950, 23, 34);
	addLink(153, 1, 4650, 23, 35);
	addLink(154, 1, 8300, 23, 36);
	addLink(155, 1, 11350, 23, 37);
	addLink(156, 1, 1100, 24, 25);
	addLink(157, 1, 2400, 24, 26);
	addLink(158, 1, 4800, 24, 27);
	addLink(159, 1, 2350, 24, 28);
	addLink(160, 1, 3050, 24, 29);
	addLink(161, 1, 2300, 24, 30);
	addLink(162, 1, 6300, 24, 31);
	addLink(163, 1, 3800, 24, 32);
	addLink(164, 1, 6600, 24, 33);
	addLink(165, 1, 3050, 24, 34);
	addLink(166, 1, 4750, 24, 35);
	addLink(167, 1, 8400, 24, 36);
	addLink(168, 1, 11450, 24, 37);
	addLink(169, 1, 1300, 25, 26);
	addLink(170, 1, 5900, 25, 27);
	addLink(171, 1, 3450, 25, 28);
	addLink(172, 1, 1950, 25, 29);
	addLink(173, 1, 1200, 25, 30);
	addLink(174, 1, 7400, 25, 31);
	addLink(175, 1, 2700, 25, 32);
	addLink(176, 1, 5500, 25, 33);
	addLink(177, 1, 1950, 25, 34);
	addLink(178, 1, 3650, 25, 35);
	addLink(179, 1, 7300, 25, 36);
	addLink(180, 1, 10350, 25, 37);
	addLink(181, 1, 7200, 26, 27);
	addLink(182, 1, 4750, 26, 28);
	addLink(183, 1, 2300, 26, 29);
	addLink(184, 1, 1550, 26, 30);
	addLink(185, 1, 8700, 26, 31);
	addLink(186, 1, 3050, 26, 32);
	addLink(187, 1, 5850, 26, 33);
	addLink(188, 1, 800, 26, 34);
	addLink(189, 1, 4000, 26, 35);
	addLink(190, 1, 7650, 26, 36);
	addLink(191, 1, 10200, 26, 37);
	addLink(192, 1, 3950, 27, 28);
	addLink(193, 1, 6050, 27, 29);
	addLink(194, 1, 6800, 27, 30);
	addLink(195, 1, 1500, 27, 31);
	addLink(196, 1, 8300, 27, 32);
	addLink(197, 1, 11100, 27, 33);
	addLink(198, 1, 7550, 27, 34);
	addLink(199, 1, 9250, 27, 35);
	addLink(200, 1, 12900, 27, 36);
	addLink(201, 1, 15950, 27, 37);
	addLink(202, 1, 2100, 28, 29);
	addLink(203, 1, 2850, 28, 30);
	addLink(204, 1, 5450, 28, 31);
	addLink(205, 1, 4300, 28, 32);
	addLink(206, 1, 7100, 28, 33);
	addLink(207, 1, 3600, 28, 34);
	addLink(208, 1, 5250, 28, 35);
	addLink(209, 1, 8900, 28, 36);
	addLink(210, 1, 11950, 28, 37);
	addLink(211, 1, 750, 29, 30);
	addLink(212, 1, 7550, 29, 31);
	addLink(213, 1, 2200, 29, 32);
	addLink(214, 1, 5000, 29, 33);
	addLink(215, 1, 1500, 29, 34);
	addLink(216, 1, 3150, 29, 35);
	addLink(217, 1, 6800, 29, 36);
	addLink(218, 1, 9850, 29, 37);
	addLink(219, 1, 8300, 30, 31);
	addLink(220, 1, 1500, 30, 32);
	addLink(221, 1, 4300, 30, 33);
	addLink(222, 1, 750, 30, 34);
	addLink(223, 1, 2450, 30, 35);
	addLink(224, 1, 6100, 30, 36);
	addLink(225, 1, 9150, 30, 37);
	addLink(226, 1, 9800, 31, 32);
	addLink(227, 1, 12600, 31, 33);
	addLink(228, 1, 9050, 31, 34);
	addLink(229, 1, 10750, 31, 35);
	addLink(230, 1, 14400, 31, 36);
	addLink(231, 1, 17450, 31, 37);
	addLink(232, 1, 2800, 32, 33);
	addLink(233, 1, 2250, 32, 34);
	addLink(234, 1, 950, 32, 35);
	addLink(235, 1, 4600, 32, 36);
	addLink(236, 1, 7650, 32, 37);
	addLink(237, 1, 5050, 33, 34);
	addLink(238, 1, 3750, 33, 35);
	addLink(239, 1, 1800, 33, 36);
	addLink(240, 1, 10450, 33, 37);
	addLink(241, 1, 3200, 34, 35);
	addLink(242, 1, 6850, 34, 36);
	addLink(243, 1, 9400, 34, 37);
	addLink(244, 1, 5550, 35, 36);
	addLink(245, 1, 6700, 35, 37);
	addLink(246, 1, 12250, 36, 37);
	addLink(247, 1, 1800, 20, 19);
	addLink(248, 1, 3300, 21, 19);
	addLink(249, 1, 4000, 22, 19);
	addLink(250, 1, 5700, 23, 19);
	addLink(251, 1, 4900, 24, 19);
	addLink(252, 1, 6000, 25, 19);
	addLink(253, 1, 7300, 26, 19);
	addLink(254, 1, 3500, 27, 19);
	addLink(255, 1, 4050, 28, 19);
	addLink(256, 1, 6150, 29, 19);
	addLink(257, 1, 6900, 30, 19);
	addLink(258, 1, 5000, 31, 19);
	addLink(259, 1, 8400, 32, 19);
	addLink(260, 1, 11200, 33, 19);
	addLink(261, 1, 7650, 34, 19);
	addLink(262, 1, 9350, 35, 19);
	addLink(263, 1, 13000, 36, 19);
	addLink(264, 1, 16050, 37, 19);
	addLink(265, 1, 1500, 21, 20);
	addLink(266, 1, 2200, 22, 20);
	addLink(267, 1, 3900, 23, 20);
	addLink(268, 1, 3100, 24, 20);
	addLink(269, 1, 4200, 25, 20);
	addLink(270, 1, 5500, 26, 20);
	addLink(271, 1, 1700, 27, 20);
	addLink(272, 1, 2250, 28, 20);
	addLink(273, 1, 4350, 29, 20);
	addLink(274, 1, 5100, 30, 20);
	addLink(275, 1, 3200, 31, 20);
	addLink(276, 1, 6600, 32, 20);
	addLink(277, 1, 9400, 33, 20);
	addLink(278, 1, 5850, 34, 20);
	addLink(279, 1, 7550, 35, 20);
	addLink(280, 1, 11200, 36, 20);
	addLink(281, 1, 14250, 37, 20);
	addLink(282, 1, 700, 22, 21);
	addLink(283, 1, 2400, 23, 21);
	addLink(284, 1, 1600, 24, 21);
	addLink(285, 1, 2700, 25, 21);
	addLink(286, 1, 4000, 26, 21);
	addLink(287, 1, 3200, 27, 21);
	addLink(288, 1, 750, 28, 21);
	addLink(289, 1, 2850, 29, 21);
	addLink(290, 1, 3600, 30, 21);
	addLink(291, 1, 4700, 31, 21);
	addLink(292, 1, 5100, 32, 21);
	addLink(293, 1, 7900, 33, 21);
	addLink(294, 1, 4350, 34, 21);
	addLink(295, 1, 6050, 35, 21);
	addLink(296, 1, 9700, 36, 21);
	addLink(297, 1, 12750, 37, 21);
	addLink(298, 1, 1700, 23, 22);
	addLink(299, 1, 900, 24, 22);
	addLink(300, 1, 2000, 25, 22);
	addLink(301, 1, 3300, 26, 22);
	addLink(302, 1, 3900, 27, 22);
	addLink(303, 1, 1450, 28, 22);
	addLink(304, 1, 3550, 29, 22);
	addLink(305, 1, 3200, 30, 22);
	addLink(306, 1, 5400, 31, 22);
	addLink(307, 1, 4700, 32, 22);
	addLink(308, 1, 7500, 33, 22);
	addLink(309, 1, 3950, 34, 22);
	addLink(310, 1, 5650, 35, 22);
	addLink(311, 1, 9300, 36, 22);
	addLink(312, 1, 12350, 37, 22);
	addLink(313, 1, 800, 24, 23);
	addLink(314, 1, 1000, 25, 23);
	addLink(315, 1, 2300, 26, 23);
	addLink(316, 1, 5600, 27, 23);
	addLink(317, 1, 3150, 28, 23);
	addLink(318, 1, 2950, 29, 23);
	addLink(319, 1, 2200, 30, 23);
	addLink(320, 1, 7100, 31, 23);
	addLink(321, 1, 3700, 32, 23);
	addLink(322, 1, 6500, 33, 23);
	addLink(323, 1, 2950, 34, 23);
	addLink(324, 1, 4650, 35, 23);
	addLink(325, 1, 8300, 36, 23);
	addLink(326, 1, 11350, 37, 23);
	addLink(327, 1, 1100, 25, 24);
	addLink(328, 1, 2400, 26, 24);
	addLink(329, 1, 4800, 27, 24);
	addLink(330, 1, 2350, 28, 24);
	addLink(331, 1, 3050, 29, 24);
	addLink(332, 1, 2300, 30, 24);
	addLink(333, 1, 6300, 31, 24);
	addLink(334, 1, 3800, 32, 24);
	addLink(335, 1, 6600, 33, 24);
	addLink(336, 1, 3050, 34, 24);
	addLink(337, 1, 4750, 35, 24);
	addLink(338, 1, 8400, 36, 24);
	addLink(339, 1, 11450, 37, 24);
	addLink(340, 1, 1300, 26, 25);
	addLink(341, 1, 5900, 27, 25);
	addLink(342, 1, 3450, 28, 25);
	addLink(343, 1, 1950, 29, 25);
	addLink(344, 1, 1200, 30, 25);
	addLink(345, 1, 7400, 31, 25);
	addLink(346, 1, 2700, 32, 25);
	addLink(347, 1, 5500, 33, 25);
	addLink(348, 1, 1950, 34, 25);
	addLink(349, 1, 3650, 35, 25);
	addLink(350, 1, 7300, 36, 25);
	addLink(351, 1, 10350, 37, 25);
	addLink(352, 1, 7200, 27, 26);
	addLink(353, 1, 4750, 28, 26);
	addLink(354, 1, 2300, 29, 26);
	addLink(355, 1, 1550, 30, 26);
	addLink(356, 1, 8700, 31, 26);
	addLink(357, 1, 3050, 32, 26);
	addLink(358, 1, 5850, 33, 26);
	addLink(359, 1, 800, 34, 26);
	addLink(360, 1, 4000, 35, 26);
	addLink(361, 1, 7650, 36, 26);
	addLink(362, 1, 10200, 37, 26);
	addLink(363, 1, 3950, 28, 27);
	addLink(364, 1, 6050, 29, 27);
	addLink(365, 1, 6800, 30, 27);
	addLink(366, 1, 1500, 31, 27);
	addLink(367, 1, 8300, 32, 27);
	addLink(368, 1, 11100, 33, 27);
	addLink(369, 1, 7550, 34, 27);
	addLink(370, 1, 9250, 35, 27);
	addLink(371, 1, 12900, 36, 27);
	addLink(372, 1, 15950, 37, 27);
	addLink(373, 1, 2100, 29, 28);
	addLink(374, 1, 2850, 30, 28);
	addLink(375, 1, 5450, 31, 28);
	addLink(376, 1, 4300, 32, 28);
	addLink(377, 1, 7100, 33, 28);
	addLink(378, 1, 3600, 34, 28);
	addLink(379, 1, 5250, 35, 28);
	addLink(380, 1, 8900, 36, 28);
	addLink(381, 1, 11950, 37, 28);
	addLink(382, 1, 750, 30, 29);
	addLink(383, 1, 7550, 31, 29);
	addLink(384, 1, 2200, 32, 29);
	addLink(385, 1, 5000, 33, 29);
	addLink(386, 1, 1500, 34, 29);
	addLink(387, 1, 3150, 35, 29);
	addLink(388, 1, 6800, 36, 29);
	addLink(389, 1, 9850, 37, 29);
	addLink(390, 1, 8300, 31, 30);
	addLink(391, 1, 1500, 32, 30);
	addLink(392, 1, 4300, 33, 30);
	addLink(393, 1, 750, 34, 30);
	addLink(394, 1, 2450, 35, 30);
	addLink(395, 1, 6100, 36, 30);
	addLink(396, 1, 9150, 37, 30);
	addLink(397, 1, 9800, 32, 31);
	addLink(398, 1, 12600, 33, 31);
	addLink(399, 1, 9050, 34, 31);
	addLink(400, 1, 10750, 35, 31);
	addLink(401, 1, 14400, 36, 31);
	addLink(402, 1, 17450, 37, 31);
	addLink(403, 1, 2800, 33, 32);
	addLink(404, 1, 2250, 34, 32);
	addLink(405, 1, 950, 35, 32);
	addLink(406, 1, 4600, 36, 32);
	addLink(407, 1, 7650, 37, 32);
	addLink(408, 1, 5050, 34, 33);
	addLink(409, 1, 3750, 35, 33);
	addLink(410, 1, 1800, 36, 33);
	addLink(411, 1, 10450, 37, 33);
	addLink(412, 1, 3200, 35, 34);
	addLink(413, 1, 6850, 36, 34);
	addLink(414, 1, 9400, 37, 34);
	addLink(415, 1, 5550, 36, 35);
	addLink(416, 1, 6700, 37, 35);
	addLink(417, 1, 12250, 37, 36);
	addLink(418, 0, 200, 0, 19);
	addLink(419, 0, 200, 0, 60);
	addLink(420, 0, 200, 0, 50);
	addLink(421, 0, 200, 0, 46);
	addLink(422, 0, 200, 19, 60);
	addLink(423, 0, 200, 19, 50);
	addLink(424, 0, 200, 19, 46);
	addLink(425, 0, 200, 60, 50);
	addLink(426, 0, 200, 60, 46);
	addLink(427, 0, 200, 50, 46);
	addLink(428, 0, 200, 19, 0);
	addLink(429, 0, 200, 60, 0);
	addLink(430, 0, 200, 50, 0);
	addLink(431, 0, 200, 46, 0);
	addLink(432, 0, 200, 60, 19);
	addLink(433, 0, 200, 50, 19);
	addLink(434, 0, 200, 46, 19);
	addLink(435, 0, 200, 50, 60);
	addLink(436, 0, 200, 46, 60);
	addLink(437, 0, 200, 46, 50);
	addLink(438, 0, 200, 1, 20);
	addLink(439, 0, 200, 1, 61);
	addLink(440, 0, 200, 1, 55);
	addLink(441, 0, 200, 1, 51);
	addLink(442, 0, 200, 1, 47);
	addLink(443, 0, 200, 20, 61);
	addLink(444, 0, 200, 20, 55);
	addLink(445, 0, 200, 20, 51);
	addLink(446, 0, 200, 20, 47);
	addLink(447, 0, 200, 61, 55);
	addLink(448, 0, 200, 61, 51);
	addLink(449, 0, 200, 61, 47);
	addLink(450, 0, 200, 55, 51);
	addLink(451, 0, 200, 55, 47);
	addLink(452, 0, 200, 51, 47);
	addLink(453, 0, 200, 20, 1);
	addLink(454, 0, 200, 61, 1);
	addLink(455, 0, 200, 55, 1);
	addLink(456, 0, 200, 51, 1);
	addLink(457, 0, 200, 47, 1);
	addLink(458, 0, 200, 61, 20);
	addLink(459, 0, 200, 55, 20);
	addLink(460, 0, 200, 51, 20);
	addLink(461, 0, 200, 47, 20);
	addLink(462, 0, 200, 55, 61);
	addLink(463, 0, 200, 51, 61);
	addLink(464, 0, 200, 47, 61);
	addLink(465, 0, 200, 51, 55);
	addLink(466, 0, 200, 47, 55);
	addLink(467, 0, 200, 47, 51);
	addLink(468, 0, 200, 2, 21);
	addLink(469, 0, 200, 2, 38);
	addLink(470, 0, 200, 2, 62);
	addLink(471, 0, 200, 2, 56);
	addLink(472, 0, 200, 2, 52);
	addLink(473, 0, 200, 21, 38);
	addLink(474, 0, 200, 21, 62);
	addLink(475, 0, 200, 21, 56);
	addLink(476, 0, 200, 21, 52);
	addLink(477, 0, 200, 38, 62);
	addLink(478, 0, 200, 38, 56);
	addLink(479, 0, 200, 38, 52);
	addLink(480, 0, 200, 62, 56);
	addLink(481, 0, 200, 62, 52);
	addLink(482, 0, 200, 56, 52);
	addLink(483, 0, 200, 21, 2);
	addLink(484, 0, 200, 38, 2);
	addLink(485, 0, 200, 62, 2);
	addLink(486, 0, 200, 56, 2);
	addLink(487, 0, 200, 52, 2);
	addLink(488, 0, 200, 38, 21);
	addLink(489, 0, 200, 62, 21);
	addLink(490, 0, 200, 56, 21);
	addLink(491, 0, 200, 52, 21);
	addLink(492, 0, 200, 62, 38);
	addLink(493, 0, 200, 56, 38);
	addLink(494, 0, 200, 52, 38);
	addLink(495, 0, 200, 56, 62);
	addLink(496, 0, 200, 52, 62);
	addLink(497, 0, 200, 52, 56);
	addLink(498, 0, 200, 3, 22);
	addLink(499, 0, 200, 3, 39);
	addLink(500, 0, 200, 3, 57);
	addLink(501, 0, 200, 3, 53);
	addLink(502, 0, 200, 22, 39);
	addLink(503, 0, 200, 22, 57);
	addLink(504, 0, 200, 22, 53);
	addLink(505, 0, 200, 39, 57);
	addLink(506, 0, 200, 39, 53);
	addLink(507, 0, 200, 57, 53);
	addLink(508, 0, 200, 22, 3);
	addLink(509, 0, 200, 39, 3);
	addLink(510, 0, 200, 57, 3);
	addLink(511, 0, 200, 53, 3);
	addLink(512, 0, 200, 39, 22);
	addLink(513, 0, 200, 57, 22);
	addLink(514, 0, 200, 53, 22);
	addLink(515, 0, 200, 57, 39);
	addLink(516, 0, 200, 53, 39);
	addLink(517, 0, 200, 53, 57);
	addLink(518, 0, 200, 4, 23);
	addLink(519, 0, 200, 4, 59);
	addLink(520, 0, 200, 4, 77);
	addLink(521, 0, 200, 4, 73);
	addLink(522, 0, 200, 23, 59);
	addLink(523, 0, 200, 23, 77);
	addLink(524, 0, 200, 23, 73);
	addLink(525, 0, 200, 59, 77);
	addLink(526, 0, 200, 59, 73);
	addLink(527, 0, 200, 77, 73);
	addLink(528, 0, 200, 23, 4);
	addLink(529, 0, 200, 59, 4);
	addLink(530, 0, 200, 77, 4);
	addLink(531, 0, 200, 73, 4);
	addLink(532, 0, 200, 59, 23);
	addLink(533, 0, 200, 77, 23);
	addLink(534, 0, 200, 73, 23);
	addLink(535, 0, 200, 77, 59);
	addLink(536, 0, 200, 73, 59);
	addLink(537, 0, 200, 73, 77);
	addLink(538, 0, 200, 5, 24);
	addLink(539, 0, 200, 5, 40);
	addLink(540, 0, 200, 5, 58);
	addLink(541, 0, 200, 5, 78);
	addLink(542, 0, 200, 5, 72);
	addLink(543, 0, 200, 24, 40);
	addLink(544, 0, 200, 24, 58);
	addLink(545, 0, 200, 24, 78);
	addLink(546, 0, 200, 24, 72);
	addLink(547, 0, 200, 40, 58);
	addLink(548, 0, 200, 40, 78);
	addLink(549, 0, 200, 40, 72);
	addLink(550, 0, 200, 58, 78);
	addLink(551, 0, 200, 58, 72);
	addLink(552, 0, 200, 78, 72);
	addLink(553, 0, 200, 24, 5);
	addLink(554, 0, 200, 40, 5);
	addLink(555, 0, 200, 58, 5);
	addLink(556, 0, 200, 78, 5);
	addLink(557, 0, 200, 72, 5);
	addLink(558, 0, 200, 40, 24);
	addLink(559, 0, 200, 58, 24);
	addLink(560, 0, 200, 78, 24);
	addLink(561, 0, 200, 72, 24);
	addLink(562, 0, 200, 58, 40);
	addLink(563, 0, 200, 78, 40);
	addLink(564, 0, 200, 72, 40);
	addLink(565, 0, 200, 78, 58);
	addLink(566, 0, 200, 72, 58);
	addLink(567, 0, 200, 72, 78);
	addLink(568, 0, 200, 6, 25);
	addLink(569, 0, 200, 6, 41);
	addLink(570, 0, 200, 6, 79);
	addLink(571, 0, 200, 6, 74);
	addLink(572, 0, 200, 25, 41);
	addLink(573, 0, 200, 25, 79);
	addLink(574, 0, 200, 25, 74);
	addLink(575, 0, 200, 41, 79);
	addLink(576, 0, 200, 41, 74);
	addLink(577, 0, 200, 79, 74);
	addLink(578, 0, 200, 25, 6);
	addLink(579, 0, 200, 41, 6);
	addLink(580, 0, 200, 79, 6);
	addLink(581, 0, 200, 74, 6);
	addLink(582, 0, 200, 41, 25);
	addLink(583, 0, 200, 79, 25);
	addLink(584, 0, 200, 74, 25);
	addLink(585, 0, 200, 79, 41);
	addLink(586, 0, 200, 74, 41);
	addLink(587, 0, 200, 74, 79);
	addLink(588, 0, 200, 7, 26);
	addLink(589, 0, 200, 7, 75);
	addLink(590, 0, 200, 26, 75);
	addLink(591, 0, 200, 26, 7);
	addLink(592, 0, 200, 75, 7);
	addLink(593, 0, 200, 75, 26);
	addLink(594, 0, 200, 8, 27);
	addLink(595, 0, 200, 8, 54);
	addLink(596, 0, 200, 8, 48);
	addLink(597, 0, 200, 27, 54);
	addLink(598, 0, 200, 27, 48);
	addLink(599, 0, 200, 54, 48);
	addLink(600, 0, 200, 27, 8);
	addLink(601, 0, 200, 54, 8);
	addLink(602, 0, 200, 48, 8);
	addLink(603, 0, 200, 54, 27);
	addLink(604, 0, 200, 48, 27);
	addLink(605, 0, 200, 48, 54);
	addLink(606, 0, 200, 9, 28);
	addLink(607, 0, 200, 9, 63);
	addLink(608, 0, 200, 28, 63);
	addLink(609, 0, 200, 28, 9);
	addLink(610, 0, 200, 63, 9);
	addLink(611, 0, 200, 63, 28);
	addLink(612, 0, 200, 10, 29);
	addLink(613, 0, 200, 10, 64);
	addLink(614, 0, 200, 10, 68);
	addLink(615, 0, 200, 29, 64);
	addLink(616, 0, 200, 29, 68);
	addLink(617, 0, 200, 64, 68);
	addLink(618, 0, 200, 29, 10);
	addLink(619, 0, 200, 64, 10);
	addLink(620, 0, 200, 68, 10);
	addLink(621, 0, 200, 64, 29);
	addLink(622, 0, 200, 68, 29);
	addLink(623, 0, 200, 68, 64);
	addLink(624, 0, 200, 11, 30);
	addLink(625, 0, 200, 11, 42);
	addLink(626, 0, 200, 11, 80);
	addLink(627, 0, 200, 11, 69);
	addLink(628, 0, 200, 30, 42);
	addLink(629, 0, 200, 30, 80);
	addLink(630, 0, 200, 30, 69);
	addLink(631, 0, 200, 42, 80);
	addLink(632, 0, 200, 42, 69);
	addLink(633, 0, 200, 80, 69);
	addLink(634, 0, 200, 30, 11);
	addLink(635, 0, 200, 42, 11);
	addLink(636, 0, 200, 80, 11);
	addLink(637, 0, 200, 69, 11);
	addLink(638, 0, 200, 42, 30);
	addLink(639, 0, 200, 80, 30);
	addLink(640, 0, 200, 69, 30);
	addLink(641, 0, 200, 80, 42);
	addLink(642, 0, 200, 69, 42);
	addLink(643, 0, 200, 69, 80);
	addLink(644, 0, 200, 12, 31);
	addLink(645, 0, 200, 12, 49);
	addLink(646, 0, 200, 31, 49);
	addLink(647, 0, 200, 31, 12);
	addLink(648, 0, 200, 49, 12);
	addLink(649, 0, 200, 49, 31);
	addLink(650, 0, 200, 13, 32);
	addLink(651, 0, 200, 13, 43);
	addLink(652, 0, 200, 13, 81);
	addLink(653, 0, 200, 13, 65);
	addLink(654, 0, 200, 32, 43);
	addLink(655, 0, 200, 32, 81);
	addLink(656, 0, 200, 32, 65);
	addLink(657, 0, 200, 43, 81);
	addLink(658, 0, 200, 43, 65);
	addLink(659, 0, 200, 81, 65);
	addLink(660, 0, 200, 32, 13);
	addLink(661, 0, 200, 43, 13);
	addLink(662, 0, 200, 81, 13);
	addLink(663, 0, 200, 65, 13);
	addLink(664, 0, 200, 43, 32);
	addLink(665, 0, 200, 81, 32);
	addLink(666, 0, 200, 65, 32);
	addLink(667, 0, 200, 81, 43);
	addLink(668, 0, 200, 65, 43);
	addLink(669, 0, 200, 65, 81);
	addLink(670, 0, 200, 14, 33);
	addLink(671, 0, 200, 14, 66);
	addLink(672, 0, 200, 33, 66);
	addLink(673, 0, 200, 33, 14);
	addLink(674, 0, 200, 66, 14);
	addLink(675, 0, 200, 66, 33);
	addLink(676, 0, 200, 15, 34);
	addLink(677, 0, 200, 15, 70);
	addLink(678, 0, 200, 15, 76);
	addLink(679, 0, 200, 34, 70);
	addLink(680, 0, 200, 34, 76);
	addLink(681, 0, 200, 70, 76);
	addLink(682, 0, 200, 34, 15);
	addLink(683, 0, 200, 70, 15);
	addLink(684, 0, 200, 76, 15);
	addLink(685, 0, 200, 70, 34);
	addLink(686, 0, 200, 76, 34);
	addLink(687, 0, 200, 76, 70);
	addLink(688, 0, 200, 16, 35);
	addLink(689, 0, 200, 16, 44);
	addLink(690, 0, 200, 16, 82);
	addLink(691, 0, 200, 35, 44);
	addLink(692, 0, 200, 35, 82);
	addLink(693, 0, 200, 44, 82);
	addLink(694, 0, 200, 35, 16);
	addLink(695, 0, 200, 44, 16);
	addLink(696, 0, 200, 82, 16);
	addLink(697, 0, 200, 44, 35);
	addLink(698, 0, 200, 82, 35);
	addLink(699, 0, 200, 82, 44);
	addLink(700, 0, 200, 17, 36);
	addLink(701, 0, 200, 17, 67);
	addLink(702, 0, 200, 36, 67);
	addLink(703, 0, 200, 36, 17);
	addLink(704, 0, 200, 67, 17);
	addLink(705, 0, 200, 67, 36);
	addLink(706, 0, 200, 18, 37);
	addLink(707, 0, 200, 18, 45);
	addLink(708, 0, 200, 18, 71);
	addLink(709, 0, 200, 18, 83);
	addLink(710, 0, 200, 37, 45);
	addLink(711, 0, 200, 37, 71);
	addLink(712, 0, 200, 37, 83);
	addLink(713, 0, 200, 45, 71);
	addLink(714, 0, 200, 45, 83);
	addLink(715, 0, 200, 71, 83);
	addLink(716, 0, 200, 37, 18);
	addLink(717, 0, 200, 45, 18);
	addLink(718, 0, 200, 71, 18);
	addLink(719, 0, 200, 83, 18);
	addLink(720, 0, 200, 45, 37);
	addLink(721, 0, 200, 71, 37);
	addLink(722, 0, 200, 83, 37);
	addLink(723, 0, 200, 71, 45);
	addLink(724, 0, 200, 83, 45);
	addLink(725, 0, 200, 83, 71);

	sharedBikeNode[0] = 19;
	sharedBikeNode[1] = 20;
	sharedBikeNode[2] = 21;
	sharedBikeNode[3] = 22;
	sharedBikeNode[4] = 23;
	sharedBikeNode[5] = 24;
	sharedBikeNode[6] = 25;
	sharedBikeNode[7] = 26;
	sharedBikeNode[8] = 27;
	sharedBikeNode[9] = 28;
	sharedBikeNode[10] = 29;
	sharedBikeNode[11] = 30;
	sharedBikeNode[12] = 31;
	sharedBikeNode[13] = 32;
	sharedBikeNode[14] = 33;
	sharedBikeNode[15] = 34;
	sharedBikeNode[16] = 35;
	sharedBikeNode[17] = 36;
	sharedBikeNode[18] = 37;
	
	std::vector<int> busline14a;
	busline14a.push_back(0);
	busline14a.push_back(1);
	busline14a.push_back(2);
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
	setVehTimetable(1,1,1000,32, busline14a, b14aTimetable);

	std::vector<int> busline14b;
	busline14b.push_back(3);
	busline14b.push_back(4);
	busline14b.push_back(5);
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
	setVehTimetable(2, 1, 1000, 32, busline14b, b14bTimetable);

	std::vector<int> busline23a;
	busline23a.push_back(6);
	busline23a.push_back(7);
	busline23a.push_back(8);
	std::map<int, double> b23aTimetable;
	b23aTimetable[0] = 800;
	b23aTimetable[10] = 800;
	b23aTimetable[20] = 700;
	b23aTimetable[30] = 600;
	b23aTimetable[40] = 650;
	b23aTimetable[50] = 700;
	b23aTimetable[60] = 800;
	setVehTimetable(3, 1, 1000, 32, busline23a, b23aTimetable);

	std::vector<int> busline23b;
	busline23b.push_back(9);
	busline23b.push_back(10);
	busline23b.push_back(11);
	std::map<int, double> b23bTimetable;
	b23bTimetable[0] = 800;
	b23bTimetable[10] = 800;
	b23bTimetable[20] = 700;
	b23bTimetable[30] = 600;
	b23bTimetable[40] = 650;
	b23bTimetable[50] = 700;
	b23bTimetable[60] = 800;
	setVehTimetable(4, 1, 1000, 32, busline23b, b23bTimetable);

	std::vector<int> busline1a;
	busline1a.push_back(12);
	busline1a.push_back(13);
	busline1a.push_back(14);
	busline1a.push_back(15);
	busline1a.push_back(16);
	std::map<int, double> b1aTimetable;
	b1aTimetable[0] = 800;
	b1aTimetable[7] = 750;
	b1aTimetable[14] = 650;
	b1aTimetable[21] = 500;
	b1aTimetable[28] = 650;
	b1aTimetable[35] = 750;
	b1aTimetable[42] = 800;
	b1aTimetable[50] = 800;
	b1aTimetable[60] = 800;
	setVehTimetable(5, 1, 1000, 32, busline1a, b1aTimetable);

	std::vector<int> busline1b;
	busline1b.push_back(17);
	busline1b.push_back(18);
	busline1b.push_back(19);
	busline1b.push_back(20);
	busline1b.push_back(21);
	std::map<int, double> b1bTimetable;
	b1bTimetable[0] = 800;
	b1bTimetable[7] = 750;
	b1bTimetable[14] = 650;
	b1bTimetable[21] = 500;
	b1bTimetable[28] = 650;
	b1bTimetable[35] = 750;
	b1bTimetable[42] = 800;
	b1bTimetable[50] = 800;
	b1bTimetable[60] = 800;
	setVehTimetable(6, 1, 1000, 32, busline1b, b1bTimetable);

	std::vector<int> busline30a;
	busline30a.push_back(22);
	busline30a.push_back(23);
	busline30a.push_back(24);
	busline30a.push_back(25);
	busline30a.push_back(26);
	busline30a.push_back(27);
	busline30a.push_back(28);
	std::map<int, double> b30aTimetable;
	b30aTimetable[0] = 800;
	b30aTimetable[6] = 800;
	b30aTimetable[12] = 750;
	b30aTimetable[18] = 650;
	b30aTimetable[24] = 500;
	b30aTimetable[30] = 650;
	b30aTimetable[36] = 750;
	b30aTimetable[42] = 800;
	b30aTimetable[50] = 800;
	b30aTimetable[60] = 800;
	setVehTimetable(7, 1, 1000, 32, busline30a, b30aTimetable);

	std::vector<int> busline30b;
	busline30b.push_back(29);
	busline30b.push_back(30);
	busline30b.push_back(31);
	busline30b.push_back(32);
	busline30b.push_back(33);
	busline30b.push_back(34);
	busline30b.push_back(35);
	std::map<int, double> b30bTimetable;
	b30bTimetable[0] = 800;
	b30bTimetable[6] = 800;
	b30bTimetable[12] = 750;
	b30bTimetable[18] = 650;
	b30bTimetable[24] = 500;
	b30bTimetable[30] = 650;
	b30bTimetable[36] = 750;
	b30bTimetable[42] = 800;
	b30bTimetable[50] = 800;
	b30bTimetable[60] = 800;
	setVehTimetable(8, 1, 1000, 32, busline30b, b30bTimetable);

	std::vector<int> busline71a;
	busline71a.push_back(36);
	busline71a.push_back(37);
	busline71a.push_back(38);
	std::map<int, double> b71aTimetable;
	b71aTimetable[0] = 800;
	b71aTimetable[12] = 700;
	b71aTimetable[24] = 550;
	b71aTimetable[36] = 600;
	b71aTimetable[48] = 800;
	b71aTimetable[60] = 800;
	b71aTimetable[70] = 800;
	setVehTimetable(9, 1, 1000, 100, busline71a, b71aTimetable);

	std::vector<int> busline71b;
	busline71b.push_back(39);
	busline71b.push_back(40);
	busline71b.push_back(41);
	std::map<int, double> b71bTimetable;
	b71bTimetable[0] = 800;
	b71bTimetable[12] = 700;
	b71bTimetable[24] = 550;
	b71bTimetable[36] = 600;
	b71bTimetable[48] = 800;
	b71bTimetable[60] = 800;
	b71bTimetable[70] = 800;
	setVehTimetable(10, 1, 1000, 100, busline71b, b71bTimetable);

	std::vector<int> busline9a;
	busline9a.push_back(42);
	busline9a.push_back(43);
	busline9a.push_back(44);
	busline9a.push_back(45);
	std::map<int, double> b9aTimetable;
	b9aTimetable[0] = 800;
	b9aTimetable[4] = 800;
	b9aTimetable[8] = 750;
	b9aTimetable[12] = 700;
	b9aTimetable[16] = 650;
	b9aTimetable[20] = 600;
	b9aTimetable[24] = 550;
	b9aTimetable[28] = 600;
	b9aTimetable[32] = 650;
	b9aTimetable[36] = 700;
	b9aTimetable[40] = 800;
	b9aTimetable[44] = 800;
	b9aTimetable[50] = 800;
	b9aTimetable[60] = 800;
	setVehTimetable(11, 1, 1000, 100, busline9a, b9aTimetable);

	std::vector<int> busline9b;
	busline9b.push_back(46);
	busline9b.push_back(47);
	busline9b.push_back(48);
	busline9b.push_back(49);
	std::map<int, double> b9bTimetable;
	b9bTimetable[0] = 800;
	b9bTimetable[4] = 800;
	b9bTimetable[8] = 750;
	b9bTimetable[12] = 700;
	b9bTimetable[16] = 650;
	b9bTimetable[20] = 600;
	b9bTimetable[24] = 550;
	b9bTimetable[28] = 600;
	b9bTimetable[32] = 650;
	b9bTimetable[36] = 700;
	b9bTimetable[40] = 800;
	b9bTimetable[44] = 800;
	b9bTimetable[50] = 800;
	b9bTimetable[60] = 800;
	setVehTimetable(12, 1, 1000, 100, busline9b, b9bTimetable);


	std::vector<int> busline95a;
	busline95a.push_back(50);
	busline95a.push_back(51);
	busline95a.push_back(52);
	busline95a.push_back(53);
	busline95a.push_back(54);
	busline95a.push_back(55);
	std::map<int, double> b95aTimetable;
	b95aTimetable[0] = 800;
	b95aTimetable[10] = 800;
	b95aTimetable[20] = 700;
	b95aTimetable[30] = 600;
	b95aTimetable[40] = 650;
	b95aTimetable[50] = 800;
	b95aTimetable[60] = 800;
	setVehTimetable(13, 1, 1000, 32, busline95a, b95aTimetable);

	std::vector<int> busline95b;
	busline95b.push_back(56);
	busline95b.push_back(57);
	busline95b.push_back(58);
	busline95b.push_back(59);
	busline95b.push_back(60);
	busline95b.push_back(61);
	std::map<int, double> b95bTimetable;
	b95bTimetable[0] = 800;
	b95bTimetable[10] = 800;
	b95bTimetable[20] = 700;
	b95bTimetable[30] = 600;
	b95bTimetable[40] = 650;
	b95bTimetable[50] = 800;
	b95bTimetable[60] = 800;
	setVehTimetable(14, 1, 1000, 32, busline95b, b95bTimetable);


	std::vector<int> metroline1a;
	metroline1a.push_back(62);
	metroline1a.push_back(63);
	metroline1a.push_back(64);
	metroline1a.push_back(65);
	metroline1a.push_back(66);
	metroline1a.push_back(67);
	metroline1a.push_back(68);
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
	setVehTimetable(15, 2, 1000, 200, metroline1a, m1aTimetable);

	std::vector<int> metroline1b;
	metroline1b.push_back(69);
	metroline1b.push_back(70);
	metroline1b.push_back(71);
	metroline1b.push_back(72);
	metroline1b.push_back(73);
	metroline1b.push_back(74);
	metroline1b.push_back(75);
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
	setVehTimetable(16, 2, 1000, 200, metroline1b, m1bTimetable);

}

TransitNetworkLoading::~TransitNetworkLoading() {
	outFile.close();
}

double TransitNetworkLoading::getBicyclePointByBS(int bicyclePointSeq)
{
	return nodes[sharedBikeNode[bicyclePointSeq]]->bicycleNum;
}

double TransitNetworkLoading::getBicyclePointByNS(int nodeID)
{
	return nodes[nodeID]->bicycleNum;
}

double TransitNetworkLoading::getIniPathTime(std::vector<int> pathLinks)
{
	double travelTime = 0;
	for (int i = 0; i < pathLinks.size();i++) {
		travelTime += links[i]->linkLen;
	}
	return travelTime/600;
}

int TransitNetworkLoading::getShareBicycleSeq(int nodeID)
{
	for (int i = 0; i < sharedBikeNode.size();i++) {
		if (sharedBikeNode[i]== nodeID) {
			return i;
		}
	}
	return -1;
}

void TransitNetworkLoading::conSim2End()
{
	while (passengerTravels.size()>0 || vehicles.size()>0) {
		simulation();
	}
}

void TransitNetworkLoading::printLaRPassenger(int time)
{
	
	if (time< linkPassenger.size()) {
		std::ofstream outFilelink;
		std::ofstream outFileRoute;
		outFilelink.open("outFilelink.csv");
		outFileRoute.open("outFileRoute.csv"); 
		for (auto& t : linkPassenger[0]) {
			outFilelink<<links[t.first]->startNode->nodeID<<"~"<< links[t.first]->endNode->nodeID<<",";
		}
		outFilelink << "\n";
		for (auto& t : routePassenger[0]) {
			outFileRoute<<t.first<<",";
		}
		outFileRoute << "\n";
		for (int i = 0; i < time;i++) {
			std::map<int, int> linkPn = linkPassenger[i];
			std::map<int, int> routePn = routePassenger[i];
			for (auto& t : linkPn) {
				outFilelink<<t.second<<",";
			}
			outFilelink << "\n";

			for (auto& t : routePn) {
				outFileRoute << t.second << ",";
			}
			outFileRoute << "\n";

		}
		outFileRoute.close();
		outFilelink.close();
	}
}

double TransitNetworkLoading::calVehTravelTime(int passengerGroup, int pathNo, int startTime)
{
	if (passengerGroup == 10&& pathNo==1) {
		int dfsf = 0;
	}
	int pathNum = 0;
	double sumRunningTime = 0;

	for (int i = 0; i < finishPassengerTravels.size(); i++) {
		if (finishPassengerTravels[i]->passengerGroup == passengerGroup && finishPassengerTravels[i]->pathNo == pathNo && finishPassengerTravels[i]->startTime == startTime) {

			int cType = 0;
			double fee = 0;
			int transFtimes = 0;
			for (int j = 0; j < finishPassengerTravels[i]->links.size();j++) {
				if (links[finishPassengerTravels[i]->links[j]]->type==2 && cType!=2) {
					fee += busFee;
					transFtimes++;
				}
				else if (links[finishPassengerTravels[i]->links[j]]->type == 3 && cType != 3) {
					fee += metroFee;
					transFtimes++;
				}
				cType = links[finishPassengerTravels[i]->links[j]]->type;
			}

			double tmTravelFee = 0;
			if (finishPassengerTravels[i]->bikeDis > 0) {
				tmTravelFee += bikeFee;
			}

			sumRunningTime += finishPassengerTravels[i]->endTime - startTime;
			sumRunningTime += finishPassengerTravels[i]->travelCost;
			sumRunningTime += fee * feeMin;
			
			if (finishPassengerTravels[i]->bikeDis>2000 ) {
				sumRunningTime += (bikePenalty1* (finishPassengerTravels[i]->bikeDis-2000))/ bicycleSpeed;
				tmTravelFee += bikeFee;
			}
			if (finishPassengerTravels[i]->bikeDis > 5000) {
				sumRunningTime += (bikePenalty2 * (finishPassengerTravels[i]->bikeDis-5000)) / bicycleSpeed;
				tmTravelFee += bikeFee;
			}
			sumRunningTime += tmTravelFee * feeMin;
			//换乘成本计算
			if (transFtimes>1) {
				sumRunningTime += transferPan*(transFtimes-1);
			}
			pathNum++;
		}
	}
	//
	if (pathNum == 0) {
		return 0;
	}
	else {
		return sumRunningTime / pathNum;
	}
	
}

std::vector<int> TransitNetworkLoading::Dijkstra(int startNode,int endNode,int startTime, std::vector<int> typeList, int& travelTime, double& travelCost, std::string& pathIdf
	, int& outShareBicyclePoint1, int& inShareBicyclePoint1, int& outShareBicyclePoint2, int& inShareBicyclePoint2) {//求x到任一点
	//double travelCost = 0;
	std::vector<int> path;
	std::map<int,double> dist;//<nodeID,travelCost>
	std::map<int,int> time;//<nodeID,travelCost>
	std::map<int,int> visit;//<nodeID,visited>
	std::map<int, int> vehs;//<nodeID,vehNo>
	std::map<int, double> bicycleDis;//<nodeID,dis>
	std::map<int, double> travelFee;//<nodeID,Fee>
	std::map<int, int> preStop;//<nodeID,nodeID//前面一个站点的id>	
	travelTime = 0;
	travelCost = 0;
	pathIdf = "";
	outShareBicyclePoint1 = -1;
	inShareBicyclePoint1 = -1;
	outShareBicyclePoint2 = -1;
	inShareBicyclePoint2 = -1;

	for (auto& t : nodes) {
		dist[t.first] = 999999;
		time[t.first] = 999999;
		visit[t.first] = 0;
		vehs[t.first] = -1;
		preStop[t.first] = -1;
		bicycleDis[t.first] = 0;
		travelFee[t.first] = 0;
	}
	time[startNode] = startTime;
	dist[startNode] = 0;
	preStop[startNode] = startNode;
	int n = nodes.size();
	for (int i = 0; i < n; i++) {
		int j = 0;
		double trC = 9999;
		for (auto& n : nodes) {
			if (!visit[n.first] && dist[n.first] <= trC) {
				j = n.first;
				trC = dist[n.first];
			}
		}

		visit[j] = 1;//设置最小值
		if (j == endNode) {//找到了最后的节点
			travelCost = dist[j];
			travelTime = time[j]- startTime;

			break;
		}		

		int nowNode = j;
		for (int l : nodes[nowNode]->outNodeLink) {//遍历node出去的节点
			int nextNode = links[l]->endNode->nodeID;
			if  (std::find(typeList.begin(), typeList.end(), links[l]->type) == typeList.end()){//找不到这个类型				
				continue;
			}
			if (visit[nextNode] == 0) {//表示节点还没有找到最短路径,需要遍历这个节点
				//判断需不需要坐车
				if (links[l]->type == 2 || links[l]->type == 3) {//判断坐车的连接
					//linkVehicleLastBeifen
					if (vehs[nowNode] == -1) {//没在车上，要找最近的车上
						Vehicle* vt = NULL;
						int firstVehTime = 9999;
						for (Vehicle* v : linkVehicleLastBeifen[links[l]->linkID]) {
							if (v->nodeTime[nowNode] >= time[nowNode] && (v->nodeTime[nowNode] - time[nowNode]) <= firstVehTime) {
								firstVehTime = (v->nodeTime[nowNode] - time[nowNode]);
								vt = v;
							}
						}
						if (vt != NULL) {//上车，需要统计乘车费
							//此时上vt
							double tmTravelFee = 0;
							if (links[l]->type == 2) {//公交
								tmTravelFee = busFee;
							}
							else if (links[l]->type == 3) {//地铁
								tmTravelFee = metroFee;
							}
							int tmTravelTime = vt->nodeTime[nextNode] - vt->nodeTime[nowNode];
							double tmTravelC = 0;
							double lpn = vt->linkPassengerNum[links[l]->linkID];
							if (lpn > vt->seatNum) {

								tmTravelC = tmTravelTime + tmTravelTime * theta * (lpn - vt->seatNum) / vt->seatNum;
							}
							else {
								tmTravelC = tmTravelTime;
							}
							tmTravelC += tmTravelFee * feeMin;

							

							if ((dist[nowNode] + tmTravelC) < dist[nextNode]) {//确定连接时间
								dist[nextNode] = dist[nowNode] + tmTravelC+ firstVehTime;
								time[nextNode] = vt->nodeTime[nextNode];
								bicycleDis[nextNode] = bicycleDis[nowNode];
								travelFee[nextNode] = travelFee[nowNode]+ tmTravelFee;
								vehs[nextNode] = vt->vehNo;
								preStop[nextNode] = nowNode;
							}
						}

					}
					else {//在车上，要看看顺不顺路
						if (std::find(vehicleLastBeifenMap[vehs[nowNode]]->links.begin(), vehicleLastBeifenMap[vehs[nowNode]]->links.end(), links[l]->linkID)
							!= vehicleLastBeifenMap[vehs[nowNode]]->links.end()) {//顺路，就不用找车了
							Vehicle* vt = vehicleLastBeifenMap[vehs[nowNode]];
							int tmTravelTime = vt->nodeTime[nextNode] - vt->nodeTime[nowNode];
							double tmTravelC = 0;
							double lpn = vt->linkPassengerNum[links[l]->linkID];
							if (lpn > vt->seatNum) {

								tmTravelC = tmTravelTime + tmTravelTime * theta * (lpn - vt->seatNum) / vt->seatNum;
							}
							else {
								tmTravelC = tmTravelTime;
							}

							if ((dist[nowNode] + tmTravelC) < dist[nextNode]) {//确定连接时间
								dist[nextNode] = dist[nowNode] + tmTravelC;
								time[nextNode] = vt->nodeTime[nextNode];
								bicycleDis[nextNode] = bicycleDis[nowNode];
								travelFee[nextNode] = travelFee[nowNode];
								vehs[nextNode] = vt->vehNo;
								preStop[nextNode] = nowNode;
							}
						}
						else {//不顺路还要找车，这里其实不太可能发生
							Vehicle* vt = NULL;
							int firstVehTime = 9999;
							for (Vehicle* v : linkVehicleLastBeifen[links[l]->linkID]) {
								if (v->nodeTime[nowNode] >= time[nowNode] && (v->nodeTime[nowNode] - time[nowNode]) <= firstVehTime) {
									firstVehTime = (v->nodeTime[nowNode] - time[nowNode]);
									vt = v;
								}
							}
							if (vt != NULL) {
								//此时上vt
								double tmTravelFee = 0;
								if (links[l]->type == 2) {//公交
									tmTravelFee = busFee;
								}
								else if (links[l]->type == 3) {//地铁
									tmTravelFee = metroFee;
								}
								int tmTravelTime = vt->nodeTime[nextNode] - vt->nodeTime[nowNode];
								double tmTravelC = 0;
								double lpn = vt->linkPassengerNum[links[l]->linkID];
								if (lpn > vt->seatNum) {

									tmTravelC = tmTravelTime + tmTravelTime * theta * (lpn - vt->seatNum) / vt->seatNum;
								}
								else {
									tmTravelC = tmTravelTime;
								}
								tmTravelC += tmTravelFee * feeMin;

								if ((dist[nowNode] + tmTravelC) < dist[nextNode]) {//确定连接时间
									dist[nextNode] = dist[nowNode] + tmTravelC+ firstVehTime;
									time[nextNode] = vt->nodeTime[nextNode];
									bicycleDis[nextNode] = bicycleDis[nowNode];
									travelFee[nextNode] = travelFee[nowNode] + tmTravelFee;
									vehs[nextNode] = vt->vehNo;
									preStop[nextNode] = nowNode;
								}
							}
						}
					}
				}
				else {//步行或者单车连接，比较简单
					if (links[l]->type == 0) {//步行连接
						int tTime = (int)((links[l]->linkLen / walkSpeed)+0.99);
						if ((dist[nowNode] + tTime) < dist[nextNode]) {//更新
							dist[nextNode] = dist[nowNode] + tTime;
							time[nextNode] = time[nowNode] + tTime;
							bicycleDis[nextNode] = bicycleDis[nowNode];
							travelFee[nextNode] = travelFee[nowNode];
							preStop[nextNode] = nowNode;
							vehs[nextNode] = -1;
						}
					}
					else if (links[l]->type == 1) {//共享单车
						int tTime = (int)((links[l]->linkLen / bicycleSpeed)+0.99);
						double tmTravelFee = bikeFee;
						double tCost = tTime;
						if (bicycleDis[nowNode] + links[l]->linkLen > 2000 ) {
							tCost += bikePenalty1 * (bicycleDis[nowNode] + links[l]->linkLen - 2000) / bicycleSpeed;
							tmTravelFee += bikeFee;
						}
						if (bicycleDis[nowNode] + links[l]->linkLen > 5000) {
							tCost += bikePenalty2 * (bicycleDis[nowNode] + links[l]->linkLen - 5000) / bicycleSpeed;
							tmTravelFee += bikeFee;
						}
						tCost += tmTravelFee * feeMin;
						if ((dist[nowNode] + tCost) < dist[nextNode]) {//更新
							dist[nextNode] = dist[nowNode] + tCost;
							time[nextNode] = time[nowNode] + tTime;
							bicycleDis[nextNode] = bicycleDis[nowNode] + links[l]->linkLen;
							travelFee[nextNode] = travelFee[nowNode] + tmTravelFee;
							preStop[nextNode] = nowNode;
							vehs[nextNode] = -1;
						}
					}
				}
			}
		}
	}

	int shareBikeType = 0;
	int busType = 0;
	int metroType = 0;
	if (std::find(typeList.begin(), typeList.end(), 1) == typeList.end()) {
		shareBikeType = 1;
	}
	if (std::find(typeList.begin(), typeList.end(), 2) == typeList.end()) {
		busType = 1;
	}
	if (std::find(typeList.begin(), typeList.end(), 3) == typeList.end()) {
		metroType = 1;
	}


	//通过preStop计算path
	if (preStop[endNode]==-1) {
		return path;
	}
	else {
		int nowNode = endNode;
		//pathNode.push_back(endNode);
		while (nowNode != startNode) {
			int nextNode = preStop[nowNode];			
			for (int i = 0; i < nodes[nowNode]->inNodeLink.size();i++) {
				if (links[nodes[nowNode]->inNodeLink[i]]->startNode->nodeID == nextNode) {					
					if (links[nodes[nowNode]->inNodeLink[i]]->type ==1) {
						shareBikeType--;
					}else if (links[nodes[nowNode]->inNodeLink[i]]->type == 2) {
						busType--;
					}else if (links[nodes[nowNode]->inNodeLink[i]]->type == 3) {
						metroType--;
					}
					path.insert(path.begin(), nodes[nowNode]->inNodeLink[i]);
					break;
				}
			}
			nowNode = nextNode;
		}
		if (shareBikeType <= 0 && busType <= 0 && metroType <= 0) {
			pathIdf = "";
			path.clear();
		}
		else {
			int transferTimes = 0;
			int cType = 0;
			for (int i = 0; i < path.size(); i++) {
				pathIdf += std::to_string(path[i]);
				pathIdf += "|";
				if (links[path[i]]->startNode->type == 1 && links[path[i]]->type == 1) {
					if (outShareBicyclePoint1 == -1) {
						outShareBicyclePoint1 = links[path[i]]->startNode->nodeID;
					}
					else {
						outShareBicyclePoint2 = links[path[i]]->startNode->nodeID;
					}
				}
				if (links[path[i]]->endNode->type == 1 && links[path[i]]->type == 1) {
					if (inShareBicyclePoint1 == -1) {
						inShareBicyclePoint1 = links[path[i]]->endNode->nodeID;
					}
					else {
						inShareBicyclePoint2 = links[path[i]]->endNode->nodeID;
					}
				}
				if (links[path[i]]->type == 2 && cType != 2) {
					transferTimes++;
				}else if (links[path[i]]->type == 3 && cType != 3) {
					transferTimes++;
				}
				cType = links[path[i]]->type;
			}
			if (transferTimes > 1) {
				travelCost += transferPan * (transferTimes - 1);
			}
		}
		return path;
	}	
}
