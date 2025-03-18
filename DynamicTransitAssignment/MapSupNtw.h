#pragma once



class MapSupNtw//��������
{
public:
	
	static const int N = 10;//�ڵ���
	static const int M = 28;//����
	static const int T = 30+70+1000;//ʱ��
	static const int ShareBicyclePoint = 3;//������ͣ��������
	
	struct edge//��
	{
		int no;
		int type=0;//�������ͣ�0��ͨ���ӣ�1����������
		int to;//�ߵ����սڵ�
		int next = -1;//��һ���ߵ�λ��
		int w;//Ȩ��
	};

	struct node//�ڵ�
	{
		int no;
		int type = 0;//�ڵ�����,0��ͨ�ڵ㣬1������ͣ����,2�������ӣ�3����
		int next =-1;//�����ı�
	};

	
	edge e[M];//�߼���
	node head[N];//�ڵ㼯�ϣ��ڵ���ֵ��ʾ��ڵ������ĵ�һ����
	int cnt = 0;
	int eT[M][T];//���ϵ�ͨ��ʱ��
	int shareBicycleSeq[ShareBicyclePoint];//������ͣ�����˳��

	void addedge(int no, int u, int v, int w, int type=0);

	/*
	path: ·�������Ӽ���
	*/
	int getPathTime(std::vector<int> path, int startTime);
	
	int getShareBicycleSeq(int point);//���ع�����ͣ��������

	//�������********************************************************************************************
	struct passengerFlow {//��������˿͵ĳ���λ�ã����������״̬����
		int passengerNo;//����
		int passengerGroup;//�˿����
		int pathNo;
		int startTime;//����ʱ��
		int endTime =-1;//����ʱ��
		int startNode;//�˿ͳ���վ��
		int endNode;//����վ��
		double passengerNum;//�˿�����
		int nowNode = -1;//��ǰ���ڽڵ㣬���Ϊ-1˵�����ڽڵ㣬�ڱ���
		int nowLinkSeq = -1;//��ǰ��·���ϵڼ�������
		int nextNode;//��һ�ڵ�
		int nextLinkSeq = 0;//��һ�����ڵ�ǰ��·���ϵڼ�������
		int toNextTime = -1;//ǰ����һ�ڵ�ʱ��
		std::vector<int> links;//��·����
	};

	double getShareByKSN(int point, int t);//��ȡ������ͣ��������
	bool setShareByKSN(int point, int t, double bicycleNum);//���ù�����ͣ��������

	std::vector<passengerFlow> passengerFlows;
	int simTime;//����ʱ��
	bool isLoad = false;//��ǰʱ���Ƿ��Ѿ���������

	int addPassengerFlow(int startNode, double passengerNum, std::vector<int> links,int passengerNo,int pathNo,int passengerGroup);//��ӿ���

	void loadPassenger();//�������м��س˿�
	int simlutationNextStep();//���沽��
	double shareByKSC[T][ShareBicyclePoint] = {};//��������ͣ����ʣ�೵��
	double edgePassengerFlow[T][M] = {};//���ϵĿ���
	double nodePassengerFlow[T][N] = {};//�ڵ��ϵĿ���

	std::vector<passengerFlow> passengerFlowsFinish;//�Ѿ�������ɵĿ���

	void clearNetwork();
	//static bool passengerFlowComparer(passengerFlow& pf_a, passengerFlow& pf_b);
	//void passengerFlowSort();

	//******************************************************************************************************

	

	MapSupNtw();
	~MapSupNtw();
};

