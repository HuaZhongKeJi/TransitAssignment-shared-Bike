# Multi-modal dynamic transit assignment for transit networks incorporating bike-sharing
This project provides a dynamic transit assignment method that considers shared bikes.
Data sources:Based on Google Maps and Baidu Maps, road networks for parts of Jiaxing and Boston have been constructed, which can be found in BostonData.cpp and JiaxingData.cpp under the project's "data" folder.
File descriptions:
DynamicTransitAssignment.cpp: Main function
PADPC.cpp: DTA algorithm class
passengerOD.cpp: Passenger OD class
TransitNetworkLoading.cpp: Dynamic network loading simulation class
Usage instructions:Run the main function in DynamicTransitAssignment.cpp to obtain the assignment results; various data will be saved in CSV files in the project directory.When switching data sources, it is necessary to modify lines 692 and 1574 in PADPC.cpp.
