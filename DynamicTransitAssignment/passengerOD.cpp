#include "stdafx.h"
#include "passengerOD.h"

passengerOD::passengerOD(int od, int start_ponit, int end_ponit)
{
	groupNo = od;
	startPonit = start_ponit;
	endPonit = end_ponit;
	passengerFlow = NULL;
}
passengerOD::~passengerOD()
{
}

void passengerOD::setPassengerFlow(double* passenger_flow)
{
	passengerFlow = passenger_flow;
}

void passengerOD::addPassengerPath(int no, int type, std::vector<int> links, int outShareBicyclePoint1, int inShareBicyclePoint1 , int outShareBicyclePoint2, int inShareBicyclePoint2)
{

	passengerPath path;
	std::string pathIdf = "";
	for (int i = 0; i < links.size();i++) {
		path.links.push_back(links[i]);
		pathIdf += std::to_string(links[i]);
		pathIdf += "|";
	}
	path.no = no;
	path.type = type;
	path.pathIdf = pathIdf;
	path.outShareBicyclePoint1 = outShareBicyclePoint1;
	path.inShareBicyclePoint1 = inShareBicyclePoint1;
	path.outShareBicyclePoint2 = outShareBicyclePoint2;
	path.inShareBicyclePoint2 = inShareBicyclePoint2;
	paths.push_back(path);
}
