#pragma once
class TransitNetworkLoading
{
	public:
		//����һֱ�ڿ������Բ���վ��ͣ�������Գ���û������վ��
		//�˻���վ��ȳ�����������Ҫ��վ����еȳ�

		// ÿ��λʱ�����2���ӣ�������������������������������������������������������������������������������������������������������������������������������������

		//�˿͵ĳ��ж���
		class PassengerTravel {
			public:
				int type;					//��ǰtravel�����ͣ�0����,1��ʾ��������2��ʾ������3��ʾ����
				int passengerGroup;			//�˿����
				int pathNo;					//������·��
				int startTime;				//��ʼʱ��
				int endTime = 0;
				int startNode;				//��ʼվ��
				int endNode;				//����վ��
				//double travelfee = 0;			//���л���
				bool justArrNode = false;	//��ǰʱ�̽���ʱ�յ�վ��
				double passengerNum = 0;		//�˿���
				double travelCost = 0;			//��ǰ��������гɱ�
				double travelFee = 0;
				double bikeDis = 0;
				int bikeTime = 0;
				int vehNo = -1;				//�˿͵ĳ���
				int nowLink = -1;			//��ǰ�ڵڼ�������
				double linkDis = 0;			//�������ڵ�ǰlink��λ��
				int nowNode;				//��ǰ���ĸ�node
				std::vector<int> links;		//path����
		};

		class Vehicle {
			public:
				int vehNo;
				int type;				//�������ͣ�1����������2��������
				int routeNo;			//��·��
				int deptTime;			//��������ʱ��
				double speed;			//�����ٶȣ���λʱ���ڳ��ߵĳ���
				int capacity;			//��������
				int seatNum;			//������λ��
				double passengerNum=0;	//���ڳ˿�����
				int nowLink;			//��ǰ�ڵڼ�������
				double linkDis;			//�������ڵ�ǰlink��λ��
				bool finish = false;	//�жϳ����Ƿ��յ�
				std::map<int, int>nodeTime;//���������վ���ʱ��<nodeID,time>
				std::map<int, double>linkPassengerNum;//���ӵĳ˿�<linkID,passengerNum>
				std::vector<int> links;	//��·����	
				std::vector<PassengerTravel*> passengers;//���ϵĳ˿ͣ��ȳ���ռ��صĳ˿ͣ�
		};		

		struct Node {
			int nodeID;
			int type;								//�ڵ����� 0�˿����յ㣬1������ͣ����,2�����ڵ㣬3����
			double bicycleNum = 0;					//����������
			std::vector<int> inNodeLink;			//ʻ��ڵ������
			std::vector<int> outNodeLink;			//ʻ���ڵ������	
			std::vector<PassengerTravel*> passengers;//�ڵ�ĳ˿ͣ��ȳ���ռ��صĳ˿ͣ�
		};

		struct Link {
			int linkID;
			int type;								//�������ͣ�0�������ӣ�1���������ӣ�2�������ӣ�3��������
			double linkLen;							//���ӳ���
			Node* startNode;						//��ʼ�ڵ�
			Node* endNode;							//�����ڵ�
			std::vector<Vehicle*> vehicles;			//link�ĳ����ȳ���ռ��صĳ˿ͣ�
		};

		struct VehTimetable {
			Vehicle vehicle;
			std::map<int, double>timetable;
		};

		struct TravelModePassenger {
			double bikePassengerNum;
			double busPassengerNum;
			double railPassengerNum;
		};

		std::vector<std::map<int, int>> linkPassenger;//v:time;map:linkID,passegnerNum
		std::vector<std::map<int, int>> routePassenger;//v:time;map:linkID,passegnerNum

		std::map<int, Node*> nodes;
		std::map<int, Link*> links;
		std::map<int, int> sharedBikeNode;
		std::vector<PassengerTravel*> passengerTravels;
		std::vector<PassengerTravel*> finishPassengerTravels;
		std::vector<VehTimetable> vehTimetable;
		std::vector<std::vector<double>> dockCStar;//�����ʱ�ο�ʼʱ��������ֻ�����˳���ʹ�ã�û�й��Ƴ��������������gap
				

		std::vector<TravelModePassenger> allTravelModePassenger; //ÿ��ʱ��,ÿ��ģʽ�ĳ˿���
		std::vector<std::map<int, TravelModePassenger>> allODTravelModePassenger; //ÿ��ʱ��,ÿ��ģʽ�ĳ˿���

		std::vector<Vehicle*> vehicles;
		std::vector<Vehicle*> vehicleBeifen;
		std::vector<Vehicle*> vehicleLastBeifen;
		std::map<int, std::vector<Vehicle*>> linkVehicleLastBeifen;//<linkID,list<veh>>
		std::map<int, Vehicle*> vehicleLastBeifenMap;//<vehID,veh>
		std::vector<double> dockCIni;
		int simTime =0;
		int vehNo = 0;
		double theta = 0.4;//��
		double metroFee = 2;
		double busFee = 2;
		double bikeFee = 1.5;//(v)1.5Ԫÿ15����(u)
		double feeMin = 0.7;//(��)ÿԪ���ڶ��ٷַ���,18Ԫ/Сʱ
		double transferPan = 3;
		/// <summary>
		/// �������ﳵ���ٶȣ�������Ҫ����
		/// </summary>
		double walkSpeed = 120;
		double bicycleSpeed = 600;
		double bikePenalty1 = 0.2;
		double bikePenalty2 = 0.4;//(��=0.00067)

		//��ӡ�ļ�
		std::ofstream outFile;


		/// <summary>
		/// ��ӽڵ�
		/// </summary>
		/// <param name="nodeID"></param>
		/// <param name="type">�ڵ����� 0�˿����յ㣬1������ͣ����,2�����ڵ㣬3����</param>
		/// <param name="bicycleNum"></param>
		void addNode(int nodeID,int type, double bicycleNum=0);

		/// <summary>
		/// �������
		/// </summary>
		/// <param name="linkID"></param>
		/// <param name="type">�������ͣ�0�������ӣ�1���������ӣ�2�������ӣ�3��������</param>
		/// <param name="linkLen"></param>
		/// <param name="startNode"></param>
		/// <param name="endNode"></param>
		void addLink(int linkID, int type, double linkLen,int startNode,int endNode);

		void addPassenger(int passengerGroup, int pathNo, std::vector<int> pathLinks, double passengerNum=1);

		/// <summary>
		/// ��������
		/// </summary>
		void simulation();

		/// <summary>
		/// �������
		/// </summary>
		void clearNetwork();

		/// <summary>
		/// ����ʱ�̱�
		/// </summary>
		/// <param name="routeNo">��·��</param>
		/// <param name="type">���ͣ�1����������2��������</param>
		/// <param name="capacity">����</param>
		/// <param name="seatNum">��λ��</param>
		/// <param name="links">���Ӽ���</param>
		/// <param name="timeTable">ʱ�̱�<����ʱ�̣��ٶ�></param>
		void setVehTimetable(int routeNo, int type, int capacity, int seatNum, std::vector<int> links, std::map<int, double>timetable);

		TransitNetworkLoading();
		~TransitNetworkLoading();

		/// <summary>
		/// ���ݵڼ���bike point��ȡ����
		/// </summary>
		/// <param name="bicyclePointSeq"></param>
		/// <returns></returns>
		double getBicyclePointByBS(int bicyclePointSeq);

		/// <summary>
		/// ����nodeID��ȡ����
		/// </summary>
		/// <param name="nodeSeq"></param>
		/// <returns></returns>
		double getBicyclePointByNS(int nodeID);

		/***************************************************************
		  *  @brief     ��ȡ��ʼ����ʱ��
		  *  @param[in] pathLinks ��·����
		  *  @note      ��ע
		  *  @Sample usage:
		 **************************************************************/
		double getIniPathTime(std::vector<int> pathLinks);

		/***************************************************************
		  *  @brief     ��ȡ������ͣ�������
		  *  @param[in] nodeID
		  *  @note      ��ע
		  *  @Sample usage:
		 **************************************************************/
		int getShareBicycleSeq(int nodeID);

		//ִ������ֱ��������û�г���
		void conSim2End();

		void printLaRPassenger(int time);

		//����path��travel time
		double calVehTravelTime(int passengerGroup, int pathNo, int startTime);
		std::vector<int> Dijkstra(int startNode, int endNode, int startTime, std::vector<int> typeList, int& travelTime, double& travelCost,std::string& pathIdf
		, int& outShareBicyclePoint1, int& inShareBicyclePoint1, int& outShareBicyclePoint2, int& inShareBicyclePoint2);
};

