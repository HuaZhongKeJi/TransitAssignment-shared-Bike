#pragma once
class passengerOD
{
public:
	int groupNo;
	int startPonit;
	int endPonit;

	static const int T = 30;//时段

	double* passengerFlow;//乘客流

	/*
	pathType:
		1  share bicycle
		2  bus
		3  subway-bicycle
		4  subway-bus
		5  bicycle-bus
		6  subway
	*/
	struct passengerPath
	{
		int no;//path编号
		int type;//path类型
		int inShareBicyclePoint1 = -1;//使用到的共享单车停靠点,停靠点的站点编号
		int outShareBicyclePoint1 = -1;//使用到的共享单车停靠点,停靠点的站点编号
		int inShareBicyclePoint2 = -1;//使用到的共享单车停靠点,停靠点的站点编号
		int outShareBicyclePoint2 = -1;//使用到的共享单车停靠点,停靠点的站点编号
		double passengerNum[T] = {};//决策变量
		std::string pathIdf;
		std::vector<int> links;//使用到的链接
	};


	std::vector<passengerPath> paths;//乘客路径集合

	passengerOD(int od, int start_ponit, int end_ponit);
	~passengerOD();

	void setPassengerFlow(double* passenger_flow);//设置乘客流
	void addPassengerPath(int no, int type, std::vector<int> links,int outShareBicyclePoint1 = -1, int inShareBicyclePoint1 = -1, int outShareBicyclePoint2 = -1, int inShareBicyclePoint2 = -1);
};

