#pragma once



class MapSupNtw//超级网络
{
public:
	
	static const int N = 10;//节点数
	static const int M = 28;//边数
	static const int T = 30+70+1000;//时段
	static const int ShareBicyclePoint = 3;//共享单车停靠点数量
	
	struct edge//边
	{
		int no;
		int type=0;//链接类型，0普通链接，1共享单车链接
		int to;//边的最终节点
		int next = -1;//下一条边的位置
		int w;//权重
	};

	struct node//节点
	{
		int no;
		int type = 0;//节点类型,0普通节点，1共享单车停靠点,2步行链接，3地铁
		int next =-1;//相连的边
	};

	
	edge e[M];//边集合
	node head[N];//节点集合，节点后的值表示与节点相连的第一条边
	int cnt = 0;
	int eT[M][T];//边上的通行时间
	int shareBicycleSeq[ShareBicyclePoint];//共享单车停靠点的顺序

	void addedge(int no, int u, int v, int w, int type=0);

	/*
	path: 路径的链接集合
	*/
	int getPathTime(std::vector<int> path, int startTime);
	
	int getShareBicycleSeq(int point);//返回共享单车停靠点的序号

	//仿真参数********************************************************************************************
	struct passengerFlow {//用来计算乘客的出行位置，更新网络的状态变量
		int passengerNo;//流号
		int passengerGroup;//乘客组号
		int pathNo;
		int startTime;//插入时间
		int endTime =-1;//结束时间
		int startNode;//乘客出发站点
		int endNode;//结束站点
		double passengerNum;//乘客数量
		int nowNode = -1;//当前所在节点，如果为-1说明不在节点，在边上
		int nowLinkSeq = -1;//当前在路线上第几个链接
		int nextNode;//下一节点
		int nextLinkSeq = 0;//下一链接在当前在路线上第几个链接
		int toNextTime = -1;//前往下一节点时间
		std::vector<int> links;//线路走向
	};

	double getShareByKSN(int point, int t);//获取共享单车停靠点容量
	bool setShareByKSN(int point, int t, double bicycleNum);//设置共享单车停靠点容量

	std::vector<passengerFlow> passengerFlows;
	int simTime;//仿真时间
	bool isLoad = false;//当前时刻是否已经加载网络

	int addPassengerFlow(int startNode, double passengerNum, std::vector<int> links,int passengerNo,int pathNo,int passengerGroup);//添加客流

	void loadPassenger();//在线网中加载乘客
	int simlutationNextStep();//仿真步进
	double shareByKSC[T][ShareBicyclePoint] = {};//共享单车的停靠点剩余车辆
	double edgePassengerFlow[T][M] = {};//边上的客流
	double nodePassengerFlow[T][N] = {};//节点上的客流

	std::vector<passengerFlow> passengerFlowsFinish;//已经服务完成的客流

	void clearNetwork();
	//static bool passengerFlowComparer(passengerFlow& pf_a, passengerFlow& pf_b);
	//void passengerFlowSort();

	//******************************************************************************************************

	

	MapSupNtw();
	~MapSupNtw();
};

