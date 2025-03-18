#include "stdafx.h"
#include "MapSupNtw.h"


void MapSupNtw::addedge(int no, int u, int v, int w, int type)
{
	e[cnt].no = no;
	e[cnt].to = v;
	e[cnt].w = w;
	e[cnt].type = type;
	e[cnt].next = head[u].next;//ָ����u��һ�δ�ߵ�λ��
	head[u].next = cnt++;//�������µĴ��λ��
}



int MapSupNtw::getPathTime(std::vector<int> path, int startTime)
{
	int travelTime = 0;
	for (int i = 0; i < path.size(); i++) {
		if (travelTime + startTime > T || travelTime + startTime < 0) {
			return -1;
		}
		travelTime += eT[path[i]][travelTime + startTime];
	}
	return travelTime;
}

int MapSupNtw::getShareBicycleSeq(int point)
{
	for (int i = 0; i < ShareBicyclePoint; i++) {
		if (shareBicycleSeq[i]== point) {
			return i;
		}
	}
	return -1;
}

double MapSupNtw::getShareByKSN(int point, int t)
{
	if(t<T && point< ShareBicyclePoint)
		return shareByKSC[t][point];
	else
		return -1.0;
}

bool MapSupNtw::setShareByKSN(int point, int t, double bicycleNum)
{
	if (t < T && point < ShareBicyclePoint) {
		shareByKSC[t][point] = bicycleNum;
		return true;
	}
	else
		return false;
}

int MapSupNtw::addPassengerFlow(int startNode, double passengerNum, std::vector<int> links, int passengerNo, int pathNo,int passengerGroup)
{
	//loadPassenger();

	passengerFlow passengerflow;
	passengerflow.passengerGroup = passengerGroup;
	passengerflow.pathNo = pathNo;
	passengerflow.passengerNo = passengerNo;
	passengerflow.startTime = simTime;
	passengerflow.startNode = startNode;
	passengerflow.passengerNum = passengerNum;
	passengerflow.links = links;
	passengerflow.endNode = e[links[links.size()-1]].to;

	//passengerflow.toNextTime = eT[simTime][startNode];//����Ҫͨ��CTM���м���

	passengerflow.nowNode = startNode;
	passengerflow.nextNode = e[links[0]].to;

	passengerFlows.push_back(passengerflow);

	//Ϊ��ʼ�ڵ��������
	//std::cout << "time:" << simTime << "; passenger_no:" << passengerNo << "; start_node:" << startNode << "; add-num:" << passengerNum << ";\n";
	nodePassengerFlow[simTime][startNode] += passengerNum;
	/*if (startNode == 1) {
		std::cout << nodePassengerFlow[simTime][startNode] << ";\n";
	}*/	

	//add��·��ʱ���Ҫ���¹�����������������ܹ�ԤԼ�����
	if (head[startNode].type == 1 && e[links[0]].type == 1) {//�����ʼ�ڵ��ǹ������ڵ㣬����ǰʱ�̵�����������path������
		shareByKSC[simTime][getShareBicycleSeq(startNode)] -= passengerNum;
	}

	for (int i = 0; i < links.size()-1;i++) {//��tʱ�̸�������ͣ����ĳ�����
		if (head[e[links[i]].to].type == 1 && e[links[i+1]].type == 1) {//�������ڵ��ǹ������ڵ�,������һ�����ǹ�����������
			shareByKSC[simTime][getShareBicycleSeq(e[links[i]].to)] -= passengerNum;
		}
	}

	return passengerFlows.size();
}

void MapSupNtw::loadPassenger()
{
	if (isLoad)
		return;


	++simTime;

	//��ֵ��ʼ������
	for (int i = 0; i < N; i++) {
		nodePassengerFlow[simTime][i] = nodePassengerFlow[simTime - 1][i];
	}
	for (int i = 0; i < M; i++) {
		edgePassengerFlow[simTime][i] = edgePassengerFlow[simTime - 1][i];
	}
	for (int i = 0; i < ShareBicyclePoint; i++) {
		shareByKSC[simTime][i] = shareByKSC[simTime - 1][i];
	}

	//����Ҫ������·��ڵ�������������������ʱ��
	for (int i = 0; i < passengerFlows.size(); i++) {
		passengerFlow* p = &passengerFlows[i];
		if (p->nowNode != -1) {//��ʾ�ⲿ�ֿ������ڽڵ���
			if (p->toNextTime == -1 && p->nowLinkSeq == -1) {//��ʾ���¼���Ľڵ㣬Ҫ�������ʱ��
				p->toNextTime = 1;//����Ҫͨ��CTM���м���,��ͨ����һʱ�̵Ŀ����������ʱ��
			}
			
			if (p->toNextTime == 1) {//�ж��Ƿ�Ҫ������һ�ڵ�
				int nowNode = p->nowNode;

				int nextLink = p->links[p->nextLinkSeq];//��·�����ӱ��ϵ�˳��
													  //���˿ͼ��ص�������
				edgePassengerFlow[simTime][nextLink] += p->passengerNum;

				//std::cout << "time:" << simTime << "; passenger_no:" << p->passengerNo << "; start_node:" << nowNode << "; del-num:" << p->passengerNum << ";\n";
				nodePassengerFlow[simTime][nowNode] -= p->passengerNum;

			} 


			//�жϹ������ڵ�,�����Ǹ��ݵ���ʱ����䣬�����ǵļ����У�Ҫ����ԤԼʱ����䣬Ҳ���ǰ��ճ˿�ѡ����·��ʱ��ۼ�����
			/*if (head[nowNode].type == 1) {
				//�ж�����һ����������˳��				
				shareByKSC[simTime][getShareBusSeq(nowNode)] -= p.passengerNum;
			}*/

		}
		else {//��ʾ�����ӣ�Ҫ����ڵ�
			if (p->toNextTime == 1) {//Ҫ������һ�ڵ㣬������Ҫ����
				int nodeSeq = p->nextNode;

				//���˿ͼ��ص��ڵ���
				//std::cout << "time:" << simTime << "; passenger_no:" << p->passengerNo << "; start_node:" << nodeSeq << "; add-num:" << p->passengerNum << ";\n";
				nodePassengerFlow[simTime][nodeSeq] += p->passengerNum;

				edgePassengerFlow[simTime][p->links[p->nowLinkSeq]] -= p->passengerNum;

				//�жϹ������ڵ㣬Ҫǰһ�����ǹ���������
				if (head[nodeSeq].type == 1 && e[p->links[p->nowLinkSeq]].type==1) {
					shareByKSC[simTime][getShareBicycleSeq(nodeSeq)] += p->passengerNum;
				}
			}
		}
	}
	isLoad = true;
}

int MapSupNtw::simlutationNextStep()
{
	if (!isLoad)
		loadPassenger();

	

	int linkTravelTime[M] = {};//���ӵ�����ʱ��
	int nodeTravelTime[M] = {};//�ڵ������ʱ��

	//�������¿�����״̬
	for (auto iter = passengerFlows.begin(); iter != passengerFlows.end(); ){//������·������״̬�����³��������������г˿���
	//for (int i = 0; i < passengerFlows.size();i++) {
		//passengerFlow p = *iter;
		if (iter->nowNode != -1) {//��ʾ�ⲿ�ֿ������ڽڵ���
			

			//���³˿�״̬ �ڵ�-���� 
			if (iter->toNextTime == 1) {//Ҫ������һ���ӣ�������Ҫ����

				iter->nowNode = -1;
				int nextLink = iter->links[iter->nextLinkSeq];//��·�����ӱ��ϵ�˳��
				//�������ӵ�����ʱ��
				if (linkTravelTime[nextLink] == 0) {//��û�м������ʱ�䣬������Ҫ���м���
					if (e[nextLink].type == 2) {
						linkTravelTime[nextLink] = eT[nextLink][simTime] + (int)(0.1 * edgePassengerFlow[simTime][nextLink]);//����Ҫͨ��CTM���м���
					}
					else if (e[nextLink].type == 0) {
						linkTravelTime[nextLink] = eT[nextLink][simTime] + (int)(0.3 * edgePassengerFlow[simTime][nextLink]);//����Ҫͨ��CTM���м���
					}
					else if (e[nextLink].type == 3) {
						linkTravelTime[nextLink] = eT[nextLink][simTime] + (int)(0.03 * edgePassengerFlow[simTime][nextLink]);//����Ҫͨ��CTM���м���
					}
					else {
						linkTravelTime[nextLink] = eT[nextLink][simTime];
					}
					
				}

				iter->nowLinkSeq++;
				if (iter->nextLinkSeq == iter->links.size() - 1) {
					iter->nextLinkSeq = -1;//����������һ�����ӣ�����һ�����Ӿ���-1
				}else {
					iter->nextLinkSeq++;
				}

				iter->toNextTime = linkTravelTime[iter->links[iter->nowLinkSeq]];
			}
			else {
				iter->toNextTime--;
			}	
			iter++;
		}
		else {//��ʾ�ⲿ�ֿ�����������
			if (iter->toNextTime == 1) {//Ҫ������һ�ڵ㣬������Ҫ����
				//����-�ڵ�
				int nodeSeq = iter->nextNode;
				//����ڵ������ʱ��
				if (nodeTravelTime[nodeSeq] == 0) {//��û�м������ʱ�䣬������Ҫ���м���
					nodeTravelTime[nodeSeq] = 1;//����Ҫͨ��CTM���м���
				}

				//����ڵ�
				iter->toNextTime = nodeTravelTime[nodeSeq];
				
				iter->nowNode = nodeSeq;
				if (nodeSeq == iter->endNode) {//��ʾ��·�Ѿ��������յ�
					iter->endTime = simTime;
					
					passengerFlowsFinish.push_back(*iter);
					iter = passengerFlows.erase(iter);
				}
				else {
					iter->nextNode = e[iter->links[iter->nextLinkSeq]].to;

					iter++;
				}

			}
			else {
				iter->toNextTime--;
				iter++;
			}
		}
	}
	isLoad = false;
	return simTime;
}

void MapSupNtw::clearNetwork()
{
	simTime = 0;
	passengerFlowsFinish.clear();

	//���ù�����ͣ����ĳ�ʼ����
	shareByKSC[0][0] = 15;
	shareByKSC[0][1] = 15;
	shareByKSC[0][2] = 15;

	for (int i = 0; i < T; i++) {
		if (i != 0)
			for (int j = 0; j < ShareBicyclePoint; j++) {
				shareByKSC[i][j] = 0;
			}
		for (int j = 0; j < M; j++) {
			edgePassengerFlow[i][j] = 0;
		}
		for (int j = 0; j < N; j++) {
			nodePassengerFlow[i][j] = 0;
		}
	}
}

//bool MapSupNtw::passengerFlowComparer(passengerFlow& pf_a, passengerFlow& pf_b)
//{
//	if (pf_a.startTime != pf_b.startTime)
//		return (pf_a.startTime < pf_b.startTime);
//	else if(pf_a.passengerGroup != pf_b.passengerGroup)
//		return (pf_a.passengerGroup < pf_b.passengerGroup);
//	else
//		return (pf_a.pathNo < pf_b.pathNo);
//}
//
//void MapSupNtw::passengerFlowSort()
//{
//	std::sort(passengerFlowsFinish.begin(), passengerFlowsFinish.end(), MapSupNtw::passengerFlowSort);
//}

MapSupNtw::MapSupNtw()
{
	//���ù�����ͣ����ĳ�ʼ����
	shareByKSC[0][0] = 15;
	shareByKSC[0][1] = 15;
	shareByKSC[0][2] = 15;

	//���ù�����ͣ�����˳��
	shareBicycleSeq[0] = 1;
	shareBicycleSeq[1] = 2;
	shareBicycleSeq[2] = 3;
	//��ʼ��head
	for (int i = 0; i < N; i++){
		head[i].no = i;//�տ�ʼ�����ڴӽ��i�����ı�
	}
	for (int i = 0; i < ShareBicyclePoint; i++) {//���ù������ڵ�����
		head[shareBicycleSeq[i]].type = 1;
	}
	 
	//��ӽڵ����
	/*for (int i = 0; i < N; i++)
	{
		e[i].next = -1;//�տ�ʼÿ���߶�û����һ����
		head[i] = -1;//�տ�ʼ�����ڴӽ��i�����ı�
	}*/

	addedge(0, 0, 1, 0,2);
	for (int i = 0; i < T; i++) {
		eT[0][i] = 2;
	}

	addedge(1, 1, 0, 0, 2);
	for (int i = 0; i < T; i++) {
		eT[1][i] = 2;
	}

	addedge(2, 0, 4, 0, 2);
	for (int i = 0; i < T; i++) {
		eT[2][i] = 8;
	}

	addedge(3, 4, 0, 0, 2);
	for (int i = 0; i < T; i++) {
		eT[3][i] = 2;
	}

	addedge(4, 0, 7, 0, 2);
	for (int i = 0; i < T; i++) {
		eT[4][i] = 6;
	}

	addedge(5, 7, 0, 0, 2);
	for (int i = 0; i < T; i++) {
		eT[5][i] = 5;
	}

	addedge(6, 9, 3, 0, 2);
	for (int i = 0; i < T; i++) {
		eT[6][i] = 2;
	}

	addedge(7, 3, 9, 0, 2);
	for (int i = 0; i < T; i++) {
		eT[7][i] = 2;
	}

	addedge(8, 9, 6, 0, 2);
	for (int i = 0; i < T; i++) {
		eT[8][i] = 8;
	}

	addedge(9, 6, 9, 0, 2);
	for (int i = 0; i < T; i++) {
		eT[9][i] = 2;
	}

	addedge(10, 1, 2, 0,1);
	for (int i = 0; i < T; i++) {
		eT[10][i] = 25;
	}

	addedge(11, 2, 1, 0,1);
	for (int i = 0; i < T; i++) {
		eT[11][i] = 25;
	}

	addedge(12, 2, 3, 0,1);
	for (int i = 0; i < T; i++) {
		eT[12][i] = 13;
	}

	addedge(13, 3, 2, 0,1);
	for (int i = 0; i < T; i++) {
		eT[13][i] = 13;
	}

	addedge(14, 2, 5, 0, 2);
	for (int i = 0; i < T; i++) {
		eT[14][i] = 2;
	}

	addedge(15, 5, 2, 0, 2);
	for (int i = 0; i < T; i++) {
		eT[15][i] = 5;
	}

	addedge(16, 2, 8, 0, 2);
	for (int i = 0; i < T; i++) {
		eT[16][i] = 2;
	}

	addedge(17, 8, 2, 0, 2);
	for (int i = 0; i < T; i++) {
		eT[17][i] = 5;
	}

	addedge(18, 5, 8, 0, 2);
	for (int i = 0; i < T; i++) {
		eT[18][i] = 2;
	}

	addedge(19, 8, 5, 0, 2);
	for (int i = 0; i < T; i++) {
		eT[19][i] = 2;
	}

	addedge(20, 7, 8, 0, 3);
	for (int i = 0; i < T; i++) {
		eT[20][i] = 15;
	}

	addedge(21, 8, 7, 0, 3);
	for (int i = 0; i < T; i++) {
		eT[21][i] = 15;
	}

	addedge(22, 4, 5, 0);
	for (int i = 0; i < T; i++) {
		if(i<10)
			eT[22][i] = 20;
		else if(i<20)
			eT[22][i] = 25;
		else if (i < 30)
			eT[22][i] = 22;	
		else if (i < 40)
			eT[22][i] = 23;
		else if (i < 50)
			eT[22][i] = 27;
		else if (i < 60)
			eT[22][i] = 25;
		else
			eT[22][i] = 22;
	}

	addedge(23, 5, 4, 0);
	for (int i = 0; i < T; i++) {
		if (i < 10)
			eT[23][i] = 20;
		else if (i < 20)
			eT[23][i] = 25;
		else if (i < 30)
			eT[23][i] = 22;
		else if (i < 40)
			eT[23][i] = 23;
		else if (i < 50)
			eT[23][i] = 27;
		else if (i < 60)
			eT[23][i] = 25;
		else
			eT[23][i] = 22;
	}

	addedge(24, 5, 6, 0);
	for (int i = 0; i < T; i++) {
		if (i < 10)
			eT[24][i] = 10;
		else if (i < 20)
			eT[24][i] = 13;
		else if (i < 30)
			eT[24][i] = 15;
		else if (i < 40)
			eT[24][i] = 14;
		else
			eT[24][i] = 12;
	}

	addedge(25, 6, 5, 0);
	for (int i = 0; i < T; i++) {
		if (i < 10)
			eT[25][i] = 10;
		else if (i < 20)
			eT[25][i] = 13;
		else if (i < 30)
			eT[25][i] = 15;
		else if (i < 40)
			eT[25][i] = 14;
		else
			eT[25][i] = 12;
	}

	addedge(26, 1, 3, 0, 1);
	for (int i = 0; i < T; i++) {
		eT[26][i] = 38;
	}

	addedge(27, 3, 1, 0, 1);
	for (int i = 0; i < T; i++) {
		eT[27][i] = 38;
	}

	simTime = 0;


	/*while (m--)
	{
		cin >> u >> v >> w;
		addedge(u, v, w);
	}
	for (int i = 0; i < N; i++)
	{
		if (head[i] != -1)
		{
			for (int j = head[i]; j != -1; j = e[j].next)
			{
				printf("��Ϊ%d %d %d\n", i, e[j].to, e[j].w);
			}
		}
	}*/

}


MapSupNtw::~MapSupNtw()
{
	

}
