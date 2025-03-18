#pragma once

class CTM
{
/*����ģ����Ҫ�ı�����

1����·�������� v
2����·���� l (����������·Ԫ������)
3����·���򴫲��ٶ� w
4��Ԫ��ͨ������ Q
5��ͨ�е�ƽ������ veh_len (��������Ԫ�����������)

*/
public:

	/*
	pathType:
		1  share bicycle
		2  bus
		3  subway-bicycle
		4  subway-bus
		5  bicycle-bus
	*/
	//��ᳵ�����������ӣ�ʱ�����
	struct VehOrPegr {		
		int vehid;				//���� id
		int type;				//���ͣ�0���ƻ�������1����ᳵ��
		int pathType;			//��·����
		int passengerGroup;		//�˿����
		int pathNo;				//������·��
		int startTime;			//����ʱ��
		int endTime;			//����ʱ��
		int moved = 0;			//����ƶ�ָ�꣬������ֹ�˿�Ϊ0��������·��һ�η����в��������
		int inShareBicyclePoint1 = -1;			//��·����Ĺ�������1
		int inShareBicyclePoint2 = -1;			//��·����Ĺ�������2
		int outShareBicyclePoint1 = -1;			//��·ʻ���Ĺ�������1
		int outShareBicyclePoint2 = -1;			//��·ʻ���Ĺ�������2
		double vehNum = 1;		//������
		int startNode;			//��ʼվ��
		int endNode;			//����վ��
		std::vector<int> links;	//��·����
	};
	struct Cell {
		int type;							//0:node cell; 1:link cell
		int linkOrNodeSeq;					//���ڽڵ�����ӵ����
		double v;							//��������
		double w;							//������
		int Q;								//Ԫ��ͨ������
		int N;								//�����������
		//int ini_veh;						//��ʼ���ĳ�����
		double maxOutVeh;					//�����복��
		double maxInVeh;						//��С���복��
		std::vector<VehOrPegr> veh_on_cell;	//Cell�еĳ���
		std::vector<Cell*> nextCell;
	};
	struct Node {
		int type;								//�ڵ����� 0�˿����յ㣬1������ͣ����,2�����ڵ㣬3����
		double bicycleNum = 0;					//����������
		std::vector<int> inNodeLink;			//ʻ��ڵ������
		std::vector<int> outNodeLink;			//ʻ���ڵ������		
		std::vector<Cell*> cell_on_node;		//node�е�Ԫ������
	};
	struct Link {
		int type;								//�������ͣ�0�������ӣ�1���������ӣ�2�������ӣ�3��������
		double linkLen;							//���ӳ���
		Node* startNode;
		Node* endNode;
		std::vector<Cell*> cell_on_link;		//link�е�Ԫ������
	};
	
	std::vector<Link*> links;
	std::vector<Node*> nodes;
	std::vector<Cell*> cells;
	std::vector<std::vector<double>> dockC;
	std::vector<std::vector<double>> dockCStar;//�����ʱ�ο�ʼʱ��������ֻ�����˳���ʹ�ã�û�й��Ƴ��������������gap

	//����������
	int defVehId = 0;
	//��ӡ������
	int printVehId = -1;
	//����ʱ��
	int simTime = 0;
	//������ʱ��
	const static int maxT = 1000;
	//�����еĳ�����
	int veh_on_network = 0;
	//��ʻ����
	std::vector<VehOrPegr> vehs;
	//ͣ�����վ�����
	std::vector<int> bicyclePoints;
	//��ӡ�ļ�
	std::ofstream outFile;
	//�Ƿ���Ss����
	int startSs = 0;
	//�Ƿ��ֹ�����
	int handS = 0;


	//bike����
	struct BikeDispatch {
		int startTime;  //����ʱ��
		int time;		//����ʱ��
		int dock;		//����dock
		double bikeNum;		//��������
		
	};
	//���ȳɱ�
	double dispatchCost = 0;
	//��λʱ����ȳɱ�
	double dispatchCostPerTime = 100;
	//��������
	std::vector<BikeDispatch> dispatchTask;
	//dock ��ĵ���ʱ��
	std::vector<std::vector<double>> dispatchTime;
	// dock C
	std::vector<double> dockCIni;
	// dock S
	std::vector<double> dockS;
	// dock s
	std::vector<double> docks;

	//����path��travel time
	double calVehTravelTime(int passengerGroup,int pathNo,int startTime);

	//ִ������ֱ��������û�г���
	void conSim2End();

	//�������
	void clearNetwork();

	//�����ڸ���ʱ�εĳ�ʼ����
	//std::vector<int[maxT]> linkIniVehs;
	//�ڵ��ڸ���ʱ�εĳ�ʼ����
	//std::vector<int[maxT]> nodeIniVehs;

	/***************************************************************
	  *  @brief     ��������
	  *  @param[in] type ��·���� ��0�������ӣ�1���������ӣ�2�������ӣ�3��������
	  *  @param[in] v ��·��������
	  *  @param[in] w ��·���򴫲��ٶ�
	  *  @param[in] l ��·����
	  *  @param[in] veh_len ��·��ʻ�ĳ�������
	  *  @param[in] veh_capacity Link�е�λcell�ĳ�����
	  *  @param[in] Q ��·Ԫ�������ͨ���������뼸�����й�ϵ��
	  *  @param[in] startNodeSeq ��ʼ�ڵ�����
	  *  @param[in] endNodeSeq �����ڵ�����
	  *  @note      ��ע
	  *  @Sample usage:     �����ʼ��ʱ���Զ����
	 **************************************************************/
	void addLink(int type, double v, double w, double l, double veh_len, int veh_capacity, int Q, int startNodeSeq, int endNodeSeq);
	
	/***************************************************************
	  *  @brief     ����ڵ�
	  *  @param[in] type �ڵ����� 0�˿����յ㣬1������ͣ����,2�����ڵ㣬3����
	  *  @param[in] v ��·��������
	  *  @param[in] w ��·���򴫲��ٶ�
	  *  @param[in] veh_capacity �������
	  *  @param[in] Q ��·Ԫ�������ͨ���������뼸�����й�ϵ��
	  *  @note      ��ע
	  *  @Sample usage:     �����ʼ��ʱ���Զ����
	 **************************************************************/
	void addNode(int type, double v, double w, int veh_capacity, int Q);

	/***************************************************************
	  *  @brief     ���������ڵ������ר�ŵ��б�
	  *  @param[in] pointSeq �������ڵ����
	  *  @param[in] bicycleNum ����������
	  *  @note      ��ע
	  *  @Sample usage:     �����ʼ��ʱ���Զ����
	 **************************************************************/
	void addBicyclePoint(int pointSeq,double bicycleNum);

	/***************************************************************
	  *  @brief     ��ȡ����������
	  *  @param[in] bicyclePointSeq �������ڵ��� bicyclePoints �е����
	  *  @note      ��ע
	  *  @Sample usage:     
	 **************************************************************/
	double getBicyclePointByBS(int bicyclePointSeq);

	/***************************************************************
	  *  @brief     ��ȡ����������
	  *  @param[in] nodeSeq �������ڵ��� nodes �е����
	  *  @note      ��ע
	  *  @Sample usage:
	 **************************************************************/
	double getBicyclePointByNS(int nodeSeq);

	/***************************************************************
	  *  @brief     ��ӳ���
	  *  @param[in] type	����0���ƻ�������1����ᳵ��
	  *  @param[in] 
	  *  @note      ��ע
	  *  @Sample usage:     ������㣬�����
	 **************************************************************/
	void addVeh(int type, int pathType,int passengerGroup, int pathNo, int startNode,int endNode,std::vector<int> links, double vehNum = 1, 
		int inShareBicyclePoint1=-1, int inShareBicyclePoint2 = -1, int outShareBicyclePoint1 = -1, int outShareBicyclePoint2 = -1);

	/***************************************************************
	  *  @brief     ���沽��
	 ***************************************************************/
	void simulation();


	/***************************************************************
	  *  @brief     ��ȡ��ʼ����ʱ��
	  *  @param[in] pathLinks ��·����
	  *  @note      ��ע
	  *  @Sample usage:
	 **************************************************************/
	double getIniPathTime(std::vector<int> pathLinks);

	/***************************************************************
	  *  @brief     ��ȡ������ͣ�������
	  *  @param[in] pointSeq ͣ�����
	  *  @note      ��ע
	  *  @Sample usage:
	 **************************************************************/
	int getShareBicycleSeq(int pointSeq);

	/***************************************************************
	  *  @brief     ���������Ȳ��ԣ�Ss��
	  *  @param[in] S �������
	  *  @param[in] s ��С����
	  *  @note      ��ע
	  *  @Sample usage:
	 **************************************************************/
	void dispatchStrategySs();

	CTM();
	~CTM();
};

