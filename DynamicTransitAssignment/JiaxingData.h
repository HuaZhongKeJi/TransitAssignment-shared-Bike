#pragma once
#include "passengerOD.h"
#include "TransitNetworkLoading.h"
class JiaxingData {
public:
	static const int T = 30;
	static const int groupNum = 22;
	static const int bicyclePointNum = 19;

	static void setOD(std::vector<passengerOD*> &passengerGroups, std::vector<std::vector<std::string>> &allPassegnerPath);
	static void setNetwork(TransitNetworkLoading* tnl,bool considerBikeSharing);

	JiaxingData();
	~JiaxingData();
};