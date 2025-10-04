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
	//�������
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

	if (startNode == 0&& endNode==0) {
		int sddd = 0;
	}


	//���ù��������������䵽��·ʱ��ֱ�Ӽ��٣�����Ͳ��ټ��٣�
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

	//����ʱ�̱���
	for each (VehTimetable vt in vehTimetable)
	{
		troutePassenger[vt.vehicle.routeNo] = 0;
		if (vt.timetable.find(simTime)!= vt.timetable.end()) {//��ʾ�ҵ���
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
	//�������г˿ͣ����л������ﳵ
	int ptsSize = passengerTravels.size();
	for(int i=0;i< ptsSize;i++)
	{
		
		PassengerTravel* pt = passengerTravels[i];

		pt->justArrNode = false;
		if (pt->type==0) {//�˿��ڲ���վ����߲���link,���лᵽ����վ��
			if (pt->nowNode!=-1 && nodes[pt->nowNode]->type==0) {//�ڽڵ�,��Ҫǰ��
				for each (int link in nodes[pt->nowNode]->outNodeLink)//������ǰ�ڵ�����п���link
				{
					if (std::find(pt->links.begin(), pt->links.end(), link) != pt->links.end()) {//���������path�������ڣ���ʾ��ǰ�����ҵ���
						//ɾ��node�еĳ˿�
						for (int k = 0; k < nodes[pt->nowNode]->passengers.size(); k++) {
							PassengerTravel* tmPtn = nodes[pt->nowNode]->passengers[k];
							if (tmPtn->passengerGroup == pt->passengerGroup && tmPtn->pathNo == pt->pathNo
								&& tmPtn->startTime == pt->startTime && tmPtn->passengerNum == pt->passengerNum) {
								nodes[pt->nowNode]->passengers.erase(nodes[pt->nowNode]->passengers.begin() + k);
								break;
							}
						}

						if (links[link]->linkLen<=walkSpeed) {//�����λʱ����ߵ�����һ���ڵ�
							pt->nowNode = links[link]->endNode->nodeID;
							if(links[link]->linkLen!=0)
								pt->justArrNode = true;
							pt->type = nodes[pt->nowNode]->type;
							if (pt->nowNode == pt->endNode) {//********************************************************************************************************//�˿͵����յ�
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
						else {//����link����
							pt->nowNode = -1;
							pt->nowLink = link;
							pt->linkDis += walkSpeed;
							tlinkPassenger[pt->nowLink] += pt->passengerNum;
						}
					}
				}
			}
			else {//��link����ǰ��
				if (pt->linkDis + walkSpeed >= links[pt->nowLink]->linkLen) {//���������link��
					int link = pt->nowLink;
					pt->nowNode = links[link]->endNode->nodeID;
					pt->type = nodes[pt->nowNode]->type;
					pt->nowLink = -1;
					pt->linkDis = 0;
					if (links[link]->linkLen != 0)
						pt->justArrNode = true;
					
					if (pt->nowNode == pt->endNode) {//********************************************************************************************************//�˿͵����յ�
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
				else {//����linkֻ��Ҫǰ��
					pt->linkDis += walkSpeed;
					tlinkPassenger[pt->nowLink] += pt->passengerNum;
				}
			
			}
		}
		else if (pt->type == 1) {//�ﳵֻ�ᵽ���г�վ��
			if (pt->nowNode != -1 && nodes[pt->nowNode]->type == 1) {//�ڽڵ�,��Ҫǰ��
				for each (int link in nodes[pt->nowNode]->outNodeLink)//������ǰ�ڵ�����п���link
				{
					if (std::find(pt->links.begin(), pt->links.end(), link) != pt->links.end()) {//���������path�������ڣ���ʾ��ǰ�����ҵ���

						//ɾ��node�еĳ˿�
						for (int k = 0; k < nodes[pt->nowNode]->passengers.size(); k++) {
							PassengerTravel* tmPtn = nodes[pt->nowNode]->passengers[k];
							if (tmPtn->passengerGroup == pt->passengerGroup && tmPtn->pathNo == pt->pathNo
								&& tmPtn->startTime == pt->startTime && tmPtn->passengerNum == pt->passengerNum) {
								nodes[pt->nowNode]->passengers.erase(nodes[pt->nowNode]->passengers.begin() + k);
								break;
							}
						}

						double speed = 0;
						if (links[link]->type==0) {//����
							speed = walkSpeed;
							pt->type = 0;
						}
						else if (links[link]->type == 1) {//������
							speed = bicycleSpeed;
							pt->type = 1;
						}

						if (links[link]->linkLen <= speed) {//�����λʱ����ߵ�����һ���ڵ�
							if (pt->type == 1) {
								pt->bikeDis += links[link]->linkLen;
								pt->bikeTime += 1;
							}								
							pt->nowNode = links[link]->endNode->nodeID;
							if (links[link]->linkLen != 0)
								pt->justArrNode = true;
							if (links[link]->type == 1) {//������
								nodes[pt->nowNode]->bicycleNum += pt->passengerNum;//����*************************************************************����
							}
							//pt.type = nodes[pt.nowNode]->type;
							if (pt->nowNode == pt->endNode) {//********************************************************************************************************//�˿͵����յ�
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
						else {//����link����
							
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
				if (pt->linkDis + bicycleSpeed >= links[pt->nowLink]->linkLen) {//���������link��
					int link = pt->nowLink;
					pt->nowNode = links[link]->endNode->nodeID;
					pt->bikeDis += links[pt->nowLink]->linkLen - pt->linkDis;
					if (links[link]->linkLen != 0)
						pt->justArrNode = true;
					pt->bikeTime += 1;
					pt->nowLink = -1;
					pt->linkDis = 0;					
					nodes[pt->nowNode]->bicycleNum += pt->passengerNum;//����*************************************************************����
					//pt.type = nodes[pt.nowNode]->type;
					if (pt->nowNode == pt->endNode) {//********************************************************************************************************//�˿͵����յ�
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
				else {//����linkֻ��Ҫǰ��
					tlinkPassenger[pt->nowLink] += pt->passengerNum;
					pt->linkDis += bicycleSpeed;
					pt->bikeDis += bicycleSpeed;
					pt->bikeTime += 1;
				}
			}
		}
		else if ((pt->type==2|| pt->type == 3)&&pt->nowNode!=-1) {//�ڹ���������BRTվ�㣬�Ѿ������յ��������Ҫ����
			for (int j = 0; j < pt->links.size();j++) {
				int linkId = pt->links[j];
				if (links[linkId]->startNode->nodeID == pt->nowNode && links[linkId]->type==0) {//�ǲ���
					//��Ҫ��֮ǰ��node����ɾ��
					for (int k = 0; k < nodes[pt->nowNode]->passengers.size();k++) {
						PassengerTravel* tmPtn = nodes[pt->nowNode]->passengers[k];
						if (tmPtn->passengerGroup == pt->passengerGroup && tmPtn->pathNo == pt->pathNo
							&& tmPtn->startTime == pt->startTime && tmPtn->passengerNum == pt->passengerNum) {
							nodes[pt->nowNode]->passengers.erase(nodes[pt->nowNode]->passengers.begin()+k);
							break;
						}
					}
					if (links[linkId]->linkLen<=walkSpeed) {//ֱ�Ӳ��е�����һ���ڵ�
						pt->nowNode = links[linkId]->endNode->nodeID;
						pt->type = nodes[pt->nowNode]->type;
						if (links[linkId]->linkLen != 0)
							pt->justArrNode = true;
						//�ж��ǲ����յ�
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
					else {//��link����
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
	
	//�������г�����ֻ���������¿�
	for (int i = 0; i < vehicles.size();i++) {
		Vehicle* v = vehicles[i];
		//���г������Ͽ�
		if (v->linkDis == 0) {//��ʾ��������Ҫ�Ͽ�
			int node = links[v->nowLink]->startNode->nodeID;
			int pn = nodes[node]->passengers.size();
			for (int j = 0; j < pn;j++) {
				PassengerTravel* pt = nodes[node]->passengers[j];
				if (pt->justArrNode==false&&std::find(pt->links.begin(), pt->links.end(), v->nowLink) != pt->links.end()) {//��ʾ�˿��ڵ����������Ҫ�ϳ�
										
					//Ҫɾ��node�еĳ˿�
					if (v->capacity >= v->passengerNum + pt->passengerNum) {//���г˿��Ѿ��ϳ�

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
					else {//�г˿Ͳ��ܹ��ϳ�
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
		//���³�λ��
		if (v->linkDis + v->speed >= links[v->nowLink]->linkLen) {//���ֱ�ӵ�����һ������
			for (int j = 0; j < v->links.size();j++) {
				if (v->links[j] == v->nowLink) {//�ҵ��˵�ǰ��link
					if (j== v->links.size()-1) {//���һ������
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

		//���³˿ͳɱ�
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
		
		
		//�¿�,���³˿͵�link
		if (v->linkDis == 0 && !v->finish) {//��ʾ�����Ҫ�¿�
			int pn = v->passengers.size();
			for (int j = 0; j < pn;j++) {//�������ϵĳ˿�
				PassengerTravel* pt = v->passengers[j];
				if (std::find(pt->links.begin(), pt->links.end(), v->nowLink) != pt->links.end()) {//��ʾ�˿���Ҫ�ߵ�ǰ�����ӣ�����Ҫ�³�
					pt->nowLink = v->nowLink;
					pt->type = links[v->nowLink]->type;
					pt->linkDis = 0;
				}
				else {//��ʾ�˿���Ҫ�³�
					int altNode = links[v->nowLink]->startNode->nodeID;//�³�վ��	
					pt->nowLink = -1;
					pt->linkDis = -1;
					pt->justArrNode = true;
					pt->nowNode = altNode;
					pt->vehNo = -1;
					if (altNode == pt->endNode) {//��ʾ�˿ͽ���*********************************************************************************************************************
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
		else if (v->finish) {//��ʾ����վ��
			int pn = v->passengers.size();
			for (int j = 0; j < pn; j++) {//�������ϵĳ˿�
				PassengerTravel* pt = v->passengers[j];
				int altNode = links[v->nowLink]->endNode->nodeID;//�³�վ��	
				pt->nowLink = -1;
				pt->linkDis = -1;
				pt->nowNode = altNode;
				pt->justArrNode = true;
				pt->vehNo = -1;
				if (altNode == pt->endNode) {//��ʾ�˿ͽ���*********************************************************************************************************************
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
			if (v->type == 1) {//����
				travelModePassenger.busPassengerNum += v->passengerNum;
			}
			else if (v->type == 2) {//����
				travelModePassenger.railPassengerNum += v->passengerNum;
			}
		}

		//int tpn = v->passengers.size();
		//for (int j = 0; j < tpn;j++) {
		//	PassengerTravel* pt = v->passengers[j];
		//	if (pt->passengerNum > 0.0001) {

		//		int pGn = pt->passengerGroup;
		//		TravelModePassenger odTmTravelModePassenger;
		//		if (odTravelModePassenger.find(pGn) != odTravelModePassenger.end()) {//�ҵ���key
		//			odTmTravelModePassenger = odTravelModePassenger[pGn];
		//		}
		//		else {
		//			odTmTravelModePassenger.bikePassengerNum = 0;
		//			odTmTravelModePassenger.busPassengerNum = 0;
		//			odTmTravelModePassenger.railPassengerNum = 0;
		//		}

		//		if (v->type == 1) {//����
		//			odTmTravelModePassenger.busPassengerNum += pt->passengerNum;
		//		}
		//		else if (v->type == 2) {//����
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
			if (odTravelModePassenger.find(pGn) != odTravelModePassenger.end()) {//�ҵ���key
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
			}else if (pt->type == 2) {//�ڹ���
				travelModePassenger.busPassengerNum += pt->passengerNum;
				odTmTravelModePassenger.busPassengerNum += pt->passengerNum;
			}
			else if (pt->type == 3) {//����
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
			if (linkVehicleLastBeifen.find(l) == linkVehicleLastBeifen.end()) {//û�����Ԫ��
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
			//���˳ɱ�����
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
	, int& outShareBicyclePoint1, int& inShareBicyclePoint1, int& outShareBicyclePoint2, int& inShareBicyclePoint2) {//��x����һ��
	//double travelCost = 0;
	std::vector<int> path;
	std::map<int,double> dist;//<nodeID,travelCost>
	std::map<int,int> time;//<nodeID,travelCost>
	std::map<int,int> visit;//<nodeID,visited>
	std::map<int, int> vehs;//<nodeID,vehNo>
	std::map<int, double> bicycleDis;//<nodeID,dis>
	std::map<int, double> travelFee;//<nodeID,Fee>
	std::map<int, int> preStop;//<nodeID,nodeID//ǰ��һ��վ���id>	
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

		visit[j] = 1;//������Сֵ
		if (j == endNode) {//�ҵ������Ľڵ�
			travelCost = dist[j];
			travelTime = time[j]- startTime;

			break;
		}		

		int nowNode = j;
		for (int l : nodes[nowNode]->outNodeLink) {//����node��ȥ�Ľڵ�
			int nextNode = links[l]->endNode->nodeID;
			if  (std::find(typeList.begin(), typeList.end(), links[l]->type) == typeList.end()){//�Ҳ����������				
				continue;
			}
			if (visit[nextNode] == 0) {//��ʾ�ڵ㻹û���ҵ����·��,��Ҫ��������ڵ�
				//�ж��費��Ҫ����
				if (links[l]->type == 2 || links[l]->type == 3) {//�ж�����������
					//linkVehicleLastBeifen
					if (vehs[nowNode] == -1) {//û�ڳ��ϣ�Ҫ������ĳ���
						Vehicle* vt = NULL;
						int firstVehTime = 9999;
						for (Vehicle* v : linkVehicleLastBeifen[links[l]->linkID]) {
							if (v->nodeTime[nowNode] >= time[nowNode] && (v->nodeTime[nowNode] - time[nowNode]) <= firstVehTime) {
								firstVehTime = (v->nodeTime[nowNode] - time[nowNode]);
								vt = v;
							}
						}
						if (vt != NULL) {//�ϳ�����Ҫͳ�Ƴ˳���
							//��ʱ��vt
							double tmTravelFee = 0;
							if (links[l]->type == 2) {//����
								tmTravelFee = busFee;
							}
							else if (links[l]->type == 3) {//����
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

							

							if ((dist[nowNode] + tmTravelC) < dist[nextNode]) {//ȷ������ʱ��
								dist[nextNode] = dist[nowNode] + tmTravelC+ firstVehTime;
								time[nextNode] = vt->nodeTime[nextNode];
								bicycleDis[nextNode] = bicycleDis[nowNode];
								travelFee[nextNode] = travelFee[nowNode]+ tmTravelFee;
								vehs[nextNode] = vt->vehNo;
								preStop[nextNode] = nowNode;
							}
						}

					}
					else {//�ڳ��ϣ�Ҫ����˳��˳·
						if (std::find(vehicleLastBeifenMap[vehs[nowNode]]->links.begin(), vehicleLastBeifenMap[vehs[nowNode]]->links.end(), links[l]->linkID)
							!= vehicleLastBeifenMap[vehs[nowNode]]->links.end()) {//˳·���Ͳ����ҳ���
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

							if ((dist[nowNode] + tmTravelC) < dist[nextNode]) {//ȷ������ʱ��
								dist[nextNode] = dist[nowNode] + tmTravelC;
								time[nextNode] = vt->nodeTime[nextNode];
								bicycleDis[nextNode] = bicycleDis[nowNode];
								travelFee[nextNode] = travelFee[nowNode];
								vehs[nextNode] = vt->vehNo;
								preStop[nextNode] = nowNode;
							}
						}
						else {//��˳·��Ҫ�ҳ���������ʵ��̫���ܷ���
							Vehicle* vt = NULL;
							int firstVehTime = 9999;
							for (Vehicle* v : linkVehicleLastBeifen[links[l]->linkID]) {
								if (v->nodeTime[nowNode] >= time[nowNode] && (v->nodeTime[nowNode] - time[nowNode]) <= firstVehTime) {
									firstVehTime = (v->nodeTime[nowNode] - time[nowNode]);
									vt = v;
								}
							}
							if (vt != NULL) {
								//��ʱ��vt
								double tmTravelFee = 0;
								if (links[l]->type == 2) {//����
									tmTravelFee = busFee;
								}
								else if (links[l]->type == 3) {//����
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

								if ((dist[nowNode] + tmTravelC) < dist[nextNode]) {//ȷ������ʱ��
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
				else {//���л��ߵ������ӣ��Ƚϼ�
					if (links[l]->type == 0) {//��������
						int tTime = (int)((links[l]->linkLen / walkSpeed)+0.99);
						if ((dist[nowNode] + tTime) < dist[nextNode]) {//����
							dist[nextNode] = dist[nowNode] + tTime;
							time[nextNode] = time[nowNode] + tTime;
							bicycleDis[nextNode] = bicycleDis[nowNode];
							travelFee[nextNode] = travelFee[nowNode];
							preStop[nextNode] = nowNode;
							vehs[nextNode] = -1;
						}
					}
					else if (links[l]->type == 1) {//������
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
						if ((dist[nowNode] + tCost) < dist[nextNode]) {//����
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


	//ͨ��preStop����path
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
