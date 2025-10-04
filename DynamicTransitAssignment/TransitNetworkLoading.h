#pragma once
class TransitNetworkLoading
{
	public:
		//车辆一直在开，所以不在站点停留，所以车辆没有设置站点
		//人会在站点等车，所以人需要在站点进行等车

		// 每单位时间等于2分钟！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！

		//乘客的出行对象
		class PassengerTravel {
			public:
				int type;					//当前travel的类型，0步行,1表示共享单车，2表示公交，3表示地铁
				int passengerGroup;			//乘客组号
				int pathNo;					//组内线路号
				int startTime;				//开始时间
				int endTime = 0;
				int startNode;				//开始站点
				int endNode;				//结束站点
				//double travelfee = 0;			//旅行花费
				bool justArrNode = false;	//当前时刻结束时刚到站点
				double passengerNum = 0;		//乘客数
				double travelCost = 0;			//当前额外的旅行成本
				double travelFee = 0;
				double bikeDis = 0;
				int bikeTime = 0;
				int vehNo = -1;				//乘客的车号
				int nowLink = -1;			//当前在第几个链接
				double linkDis = 0;			//车辆所在当前link的位置
				int nowNode;				//当前在哪个node
				std::vector<int> links;		//path走向
		};

		class Vehicle {
			public:
				int vehNo;
				int type;				//车辆类型：1：公交车；2：地铁。
				int routeNo;			//线路号
				int deptTime;			//车辆出发时间
				double speed;			//车辆速度，单位时间内车走的长度
				int capacity;			//车辆容量
				int seatNum;			//车辆座位数
				double passengerNum=0;	//车内乘客数量
				int nowLink;			//当前在第几个链接
				double linkDis;			//车辆所在当前link的位置
				bool finish = false;	//判断车辆是否到终点
				std::map<int, int>nodeTime;//车辆到达各站点的时间<nodeID,time>
				std::map<int, double>linkPassengerNum;//连接的乘客<linkID,passengerNum>
				std::vector<int> links;	//线路走向	
				std::vector<PassengerTravel*> passengers;//车上的乘客（等车与刚加载的乘客）
		};		

		struct Node {
			int nodeID;
			int type;								//节点类型 0乘客起终点，1共享单车停靠点,2公交节点，3地铁
			double bicycleNum = 0;					//共享单车容量
			std::vector<int> inNodeLink;			//驶入节点的连接
			std::vector<int> outNodeLink;			//驶出节点的连接	
			std::vector<PassengerTravel*> passengers;//节点的乘客（等车与刚加载的乘客）
		};

		struct Link {
			int linkID;
			int type;								//链接类型，0步行链接，1共享单车链接，2公交链接，3地铁链接
			double linkLen;							//链接长度
			Node* startNode;						//开始节点
			Node* endNode;							//结束节点
			std::vector<Vehicle*> vehicles;			//link的车（等车与刚加载的乘客）
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
		std::vector<std::vector<double>> dockCStar;//这个是时段开始时的容量，只计算了车辆使用，没有估计车辆到达，用来计算gap
				

		std::vector<TravelModePassenger> allTravelModePassenger; //每个时刻,每种模式的乘客数
		std::vector<std::map<int, TravelModePassenger>> allODTravelModePassenger; //每个时刻,每种模式的乘客数

		std::vector<Vehicle*> vehicles;
		std::vector<Vehicle*> vehicleBeifen;
		std::vector<Vehicle*> vehicleLastBeifen;
		std::map<int, std::vector<Vehicle*>> linkVehicleLastBeifen;//<linkID,list<veh>>
		std::map<int, Vehicle*> vehicleLastBeifenMap;//<vehID,veh>
		std::vector<double> dockCIni;
		int simTime =0;
		int vehNo = 0;
		double theta = 0.4;//γ
		double metroFee = 2;
		double busFee = 2;
		double bikeFee = 1.5;//(v)1.5元每15分钟(u)
		double feeMin = 0.7;//(β)每元等于多少分分钟,18元/小时
		double transferPan = 3;
		/// <summary>
		/// 步行与骑车的速度，这里需要设置
		/// </summary>
		double walkSpeed = 120;
		double bicycleSpeed = 600;
		double bikePenalty1 = 0.2;
		double bikePenalty2 = 0.4;//(σ=0.00067)

		//打印文件
		std::ofstream outFile;


		/// <summary>
		/// 添加节点
		/// </summary>
		/// <param name="nodeID"></param>
		/// <param name="type">节点类型 0乘客起终点，1共享单车停靠点,2公交节点，3地铁</param>
		/// <param name="bicycleNum"></param>
		void addNode(int nodeID,int type, double bicycleNum=0);

		/// <summary>
		/// 添加连接
		/// </summary>
		/// <param name="linkID"></param>
		/// <param name="type">链接类型，0步行链接，1共享单车链接，2公交链接，3地铁链接</param>
		/// <param name="linkLen"></param>
		/// <param name="startNode"></param>
		/// <param name="endNode"></param>
		void addLink(int linkID, int type, double linkLen,int startNode,int endNode);

		void addPassenger(int passengerGroup, int pathNo, std::vector<int> pathLinks, double passengerNum=1);

		/// <summary>
		/// 单步仿真
		/// </summary>
		void simulation();

		/// <summary>
		/// 清空网络
		/// </summary>
		void clearNetwork();

		/// <summary>
		/// 设置时刻表
		/// </summary>
		/// <param name="routeNo">线路号</param>
		/// <param name="type">类型：1：公交车；2：地铁。</param>
		/// <param name="capacity">容量</param>
		/// <param name="seatNum">座位数</param>
		/// <param name="links">连接集合</param>
		/// <param name="timeTable">时刻表，<发车时刻，速度></param>
		void setVehTimetable(int routeNo, int type, int capacity, int seatNum, std::vector<int> links, std::map<int, double>timetable);

		TransitNetworkLoading();
		~TransitNetworkLoading();

		/// <summary>
		/// 根据第几个bike point获取容量
		/// </summary>
		/// <param name="bicyclePointSeq"></param>
		/// <returns></returns>
		double getBicyclePointByBS(int bicyclePointSeq);

		/// <summary>
		/// 根据nodeID获取容量
		/// </summary>
		/// <param name="nodeSeq"></param>
		/// <returns></returns>
		double getBicyclePointByNS(int nodeID);

		/***************************************************************
		  *  @brief     获取初始单程时间
		  *  @param[in] pathLinks 线路走向
		  *  @note      备注
		  *  @Sample usage:
		 **************************************************************/
		double getIniPathTime(std::vector<int> pathLinks);

		/***************************************************************
		  *  @brief     获取共享单车停靠点序号
		  *  @param[in] nodeID
		  *  @note      备注
		  *  @Sample usage:
		 **************************************************************/
		int getShareBicycleSeq(int nodeID);

		//执行网络直到网络中没有车辆
		void conSim2End();

		void printLaRPassenger(int time);

		//计算path的travel time
		double calVehTravelTime(int passengerGroup, int pathNo, int startTime);
		std::vector<int> Dijkstra(int startNode, int endNode, int startTime, std::vector<int> typeList, int& travelTime, double& travelCost,std::string& pathIdf
		, int& outShareBicyclePoint1, int& inShareBicyclePoint1, int& outShareBicyclePoint2, int& inShareBicyclePoint2);
};

