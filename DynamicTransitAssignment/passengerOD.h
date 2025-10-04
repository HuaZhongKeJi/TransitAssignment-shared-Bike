#pragma once
class passengerOD
{
public:
	int groupNo;
	int startPonit;
	int endPonit;

	static const int T = 30;//ʱ��

	double* passengerFlow;//�˿���

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
		int no;//path���
		int type;//path����
		int inShareBicyclePoint1 = -1;//ʹ�õ��Ĺ�����ͣ����,ͣ�����վ����
		int outShareBicyclePoint1 = -1;//ʹ�õ��Ĺ�����ͣ����,ͣ�����վ����
		int inShareBicyclePoint2 = -1;//ʹ�õ��Ĺ�����ͣ����,ͣ�����վ����
		int outShareBicyclePoint2 = -1;//ʹ�õ��Ĺ�����ͣ����,ͣ�����վ����
		double passengerNum[T] = {};//���߱���
		std::string pathIdf;
		std::vector<int> links;//ʹ�õ�������
	};


	std::vector<passengerPath> paths;//�˿�·������

	passengerOD(int od, int start_ponit, int end_ponit);
	~passengerOD();

	void setPassengerFlow(double* passenger_flow);//���ó˿���
	void addPassengerPath(int no, int type, std::vector<int> links,int outShareBicyclePoint1 = -1, int inShareBicyclePoint1 = -1, int outShareBicyclePoint2 = -1, int inShareBicyclePoint2 = -1);
};

