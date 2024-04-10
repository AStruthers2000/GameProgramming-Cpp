#include <thread>
#include <iostream>
#include <filesystem>

#include "Solver.h"

vector<string> finished = {};//{"c101C10.txt", "c101C5.txt"};

vector<string> get_all_filenames(const string& path)
{
    namespace fs = std::filesystem;
    vector<string> filenames;
    for (const auto& entry : fs::directory_iterator(path)) {
        // entry.path() returns the full path of the file
        const string file = entry.path().filename().generic_string();
        if(file._Starts_with("readme")) continue;
        if(!file._Starts_with("r1")) continue;

        bool finished_file = false;
        for(const auto& f : finished)
        {
            if(file == f)
            {
                finished_file = true;
            }
        }
        if(finished_file) continue;
        
        filenames.push_back(file);
    }
    return filenames;
}

string unhash_enum(Alg a)
{
    switch (a)
    {
    case RNG: return "Random Search";
    case NEH: return "NEH with NN Subtours";
    case GA: return "Genetic Algorithm";
    case GA_Seed: return "Genetic Algorithm seeded with NEH";
    }
    return "undefined alg";
}

void Solve(const string& filename)
{
    auto solver = make_shared<Solver>(filename);
    if(solver->IsGoodOpen())
    {
        const vector to_run = {GA_Seed};//RNG, NEH, GA, GA_Seed};
        for(const auto& alg : to_run)
        {
            cout << "Solving problem with " << unhash_enum(alg) << endl;
            const auto start = chrono::high_resolution_clock::now();
            vector<thread> solvers;
            for(size_t i = 0; i < 10; i++)
            {
                solvers.emplace_back(&Solver::SolveEVRP, solver, alg, false);
            }
            
            for(auto &t : solvers)
            {
                t.join();
            }
            solvers.clear();

            //cout << "\t\tHalfway done" << endl;
            
            for(size_t i = 0; i < 10; i++)
            {
                solvers.emplace_back(&Solver::SolveEVRP, solver, alg, false);
            }
            
            for(auto &t : solvers)
            {
                t.join();
            }

            solvers.clear();
            
            for(size_t i = 0; i < 10; i++)
            {
                solvers.emplace_back(&Solver::SolveEVRP, solver, alg, false);
            } 
            
            for(auto &t : solvers)
            {
                t.join();
            }
            
            
            const auto stop = chrono::high_resolution_clock::now();
            const auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start).count();
            cout << "\tFinished solving " << filename << " with " << unhash_enum(alg) << " in " << static_cast<float>(duration)/1000.0f << " seconds" << endl;
        }
    }
    else
    {
        cerr << "Failed to open file: " << filename << endl;
    }
    
}

void Debug(const string& filename)
{
    const auto solver = make_unique<Solver>(filename);
    if(solver->IsGoodOpen())
    {
        solver->SolveEVRP(GA, true);
        //solver->DebugEVRP();
    }
}

int main(int argc, char* argv[])
{
    auto files = get_all_filenames(R"(.\EVRP TW\)");
    cout << "Solving with memoization enabled for sure" << endl;

    if(argc == 2)
    {
        if(const string arg = argv[1]; arg == "reverse")
        {
            cout << "Running the code on the reverse order of files" << endl;
            
            reverse(files.begin(), files.end());
        }
    }

    
    for(const auto& file : files)
    {
        Solve(file);
    } 
    
    
    
    //Debug("c101_21.txt");
    //Debug("r101_21.txt");
    //Solve("c101_21.txt");
    //Solve("r102c15.txt");
    //Solve("rc107_21.txt");
    //Solve("rc207_21");
    //Solve("c101_21.txt");
    return 0;
}
