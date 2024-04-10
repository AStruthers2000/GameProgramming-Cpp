#pragma once
#include <windows.h>

#include "Helpers/Hasher.h"
#include "Helpers/ProblemDefinition.h"

enum
{
	STR_LEN = 256 /*!< STR_LEN is the maximum number of characters a filepath could be */
};

//constexpr char DATA_PATH[STR_LEN] = R"(.\EVRP\Data_Sets\EVRP TW\)";
constexpr char WRITE_FILENAME[STR_LEN] = R"(.\..\EVRP TW Output\TimeWindowOutput.txt)";

/***************************************************************************//**
 * A class used for reading the EVRP problem definition from a file then generically solving. 
 *
 * This class loads the EVRP problem definition from a file specified by #FILENAME into a 
 * custom data structure EVRP_Data. This is necessary so that EVRP_Solver can pass the data
 * into any optimization algorithm class we want, without having to associate the data with 
 * the algorithm. 
 ******************************************************************************/

enum Alg
{
	RNG,
	NEH,
	GA,
	GA_Seed
};

class Solver
{
public:
	Solver(const string &file_name);

	/*
	~Solver()
	{
		delete problem_definition;
	}
	*/

	
	
	void DebugEVRP() const;
	void SolveEVRP(Alg algorithm, bool solving_in_debug = false) const;
	bool IsGoodOpen() const { return _is_good_open;}

private:
	void WriteToFile(const optimization_result &result) const;
	
	shared_ptr<ProblemDefinition> problem_definition;

	string _current_filename;
	bool _is_good_open = true;

	static ULARGE_INTEGER get_thread_cpu_time(HANDLE h_thread);
	shared_ptr<Hasher> shared_hasher;
};

