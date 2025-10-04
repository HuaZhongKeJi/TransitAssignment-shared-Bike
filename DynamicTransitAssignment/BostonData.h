#pragma once
#include "passengerOD.h"
#include "TransitNetworkLoading.h"
class BostonData
{
public:
	static const int T = 30;
	static const int groupNum = 9;
	static const int bicyclePointNum = 11;

	static void setOD(std::vector<passengerOD*>& passengerGroups, std::vector<std::vector<std::string>>& allPassegnerPath);
	static void setNetwork(TransitNetworkLoading* tnl, bool considerBikeSharing);

	BostonData();
	~BostonData();
};

