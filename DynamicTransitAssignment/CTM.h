#pragma once

class CTM
{
/*建立模型需要的变量：

1、链路自由流速 v
2、链路长度 l (用来计算链路元胞个数)
3、链路反向传播速度 w
4、元胞通行能力 Q
5、通行的平均车长 veh_len (用来计算元胞的最大车辆数)

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
	//社会车辆，按照链接，时段添加
	struct VehOrPegr {		
		int vehid;				//车辆 id
		int type;				//类型：0：计划车辆；1：社会车辆
		int pathType;			//线路类型
		int passengerGroup;		//乘客组号
		int pathNo;				//组内线路号
		int startTime;			//出发时间
		int endTime;			//结束时间
		int moved = 0;			//设计移动指标，用来防止乘客为0的虚拟线路在一次仿真中步进过多次
		int inShareBicyclePoint1 = -1;			//线路进入的共享单车点1
		int inShareBicyclePoint2 = -1;			//线路进入的共享单车点2
		int outShareBicyclePoint1 = -1;			//线路驶出的共享单车点1
		int outShareBicyclePoint2 = -1;			//线路驶出的共享单车点2
		double vehNum = 1;		//车辆数
		int startNode;			//开始站点
		int endNode;			//结束站点
		std::vector<int> links;	//线路走向
	};
	struct Cell {
		int type;							//0:node cell; 1:link cell
		int linkOrNodeSeq;					//所在节点或链接的序号
		double v;							//自由流速
		double w;							//阻塞流
		int Q;								//元胞通行能力
		int N;								//阻塞最大车辆数
		//int ini_veh;						//初始化的车辆数
		double maxOutVeh;					//最大进入车辆
		double maxInVeh;						//最小进入车辆
		std::vector<VehOrPegr> veh_on_cell;	//Cell中的车辆
		std::vector<Cell*> nextCell;
	};
	struct Node {
		int type;								//节点类型 0乘客起终点，1共享单车停靠点,2公交节点，3地铁
		double bicycleNum = 0;					//共享单车容量
		std::vector<int> inNodeLink;			//驶入节点的连接
		std::vector<int> outNodeLink;			//驶出节点的连接		
		std::vector<Cell*> cell_on_node;		//node中的元胞数量
	};
	struct Link {
		int type;								//链接类型，0步行链接，1共享单车链接，2公交链接，3地铁链接
		double linkLen;							//链接长度
		Node* startNode;
		Node* endNode;
		std::vector<Cell*> cell_on_link;		//link中的元胞数量
	};
	
	std::vector<Link*> links;
	std::vector<Node*> nodes;
	std::vector<Cell*> cells;
	std::vector<std::vector<double>> dockC;
	std::vector<std::vector<double>> dockCStar;//这个是时段开始时的容量，只计算了车辆使用，没有估计车辆到达，用来计算gap

	//车辆号自增
	int defVehId = 0;
	//打印车辆号
	int printVehId = -1;
	//仿真时间
	int simTime = 0;
	//最大仿真时间
	const static int maxT = 1000;
	//网络中的车辆数
	int veh_on_network = 0;
	//行驶车辆
	std::vector<VehOrPegr> vehs;
	//停靠点的站点序号
	std::vector<int> bicyclePoints;
	//打印文件
	std::ofstream outFile;
	//是否开启Ss调度
	int startSs = 0;
	//是否手工调度
	int handS = 0;


	//bike调度
	struct BikeDispatch {
		int startTime;  //出发时间
		int time;		//到达时间
		int dock;		//到达dock
		double bikeNum;		//调度数量
		
	};
	//调度成本
	double dispatchCost = 0;
	//单位时间调度成本
	double dispatchCostPerTime = 100;
	//调度任务
	std::vector<BikeDispatch> dispatchTask;
	//dock 间的调度时间
	std::vector<std::vector<double>> dispatchTime;
	// dock C
	std::vector<double> dockCIni;
	// dock S
	std::vector<double> dockS;
	// dock s
	std::vector<double> docks;

	//计算path的travel time
	double calVehTravelTime(int passengerGroup,int pathNo,int startTime);

	//执行网络直到网络中没有车辆
	void conSim2End();

	//清空网络
	void clearNetwork();

	//链接在各个时段的初始车辆
	//std::vector<int[maxT]> linkIniVehs;
	//节点在各个时段的初始车辆
	//std::vector<int[maxT]> nodeIniVehs;

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
	void addLink(int type, double v, double w, double l, double veh_len, int veh_capacity, int Q, int startNodeSeq, int endNodeSeq);
	
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
	void addNode(int type, double v, double w, int veh_capacity, int Q);

	/***************************************************************
	  *  @brief     将共享单车节点添加至专门的列表
	  *  @param[in] pointSeq 共享单车节点序号
	  *  @param[in] bicycleNum 共享单车数量
	  *  @note      备注
	  *  @Sample usage:     网络初始化时，自动添加
	 **************************************************************/
	void addBicyclePoint(int pointSeq,double bicycleNum);

	/***************************************************************
	  *  @brief     获取共享单车容量
	  *  @param[in] bicyclePointSeq 共享单车节点在 bicyclePoints 中的序号
	  *  @note      备注
	  *  @Sample usage:     
	 **************************************************************/
	double getBicyclePointByBS(int bicyclePointSeq);

	/***************************************************************
	  *  @brief     获取共享单车容量
	  *  @param[in] nodeSeq 共享单车节点在 nodes 中的序号
	  *  @note      备注
	  *  @Sample usage:
	 **************************************************************/
	double getBicyclePointByNS(int nodeSeq);

	/***************************************************************
	  *  @brief     添加车辆
	  *  @param[in] type	类型0：计划车辆；1：社会车辆
	  *  @param[in] 
	  *  @note      备注
	  *  @Sample usage:     伴随计算，逐步添加
	 **************************************************************/
	void addVeh(int type, int pathType,int passengerGroup, int pathNo, int startNode,int endNode,std::vector<int> links, double vehNum = 1, 
		int inShareBicyclePoint1=-1, int inShareBicyclePoint2 = -1, int outShareBicyclePoint1 = -1, int outShareBicyclePoint2 = -1);

	/***************************************************************
	  *  @brief     仿真步进
	 ***************************************************************/
	void simulation();


	/***************************************************************
	  *  @brief     获取初始单程时间
	  *  @param[in] pathLinks 线路走向
	  *  @note      备注
	  *  @Sample usage:
	 **************************************************************/
	double getIniPathTime(std::vector<int> pathLinks);

	/***************************************************************
	  *  @brief     获取共享单车停靠点序号
	  *  @param[in] pointSeq 停靠点号
	  *  @note      备注
	  *  @Sample usage:
	 **************************************************************/
	int getShareBicycleSeq(int pointSeq);

	/***************************************************************
	  *  @brief     共享单车调度策略（Ss）
	  *  @param[in] S 最大容量
	  *  @param[in] s 最小容量
	  *  @note      备注
	  *  @Sample usage:
	 **************************************************************/
	void dispatchStrategySs();

	CTM();
	~CTM();
};

