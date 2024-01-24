#include <ioh.hpp>
#include <string>

typedef void solver(const std::shared_ptr<ioh::problem::IntegerSingleObjective>,int);

void example_evaluation()
{
    using namespace ioh::problem;

    // we first create a problem factory, only for GraphProblem Types
    const auto &problem_factory = ProblemRegistry<submodular::GraphProblem>::instance();

    // We loop over all problems
    for (auto &[id, _] : problem_factory.map())
    {
        // Creating a problem instance
        // This is slower the first time the problem is created, as the graph has to be loaded from file
        // subsequent calls are faster.
        auto problem = problem_factory.create(id, 1, 1);
        std::cout << problem->meta_data() << std::endl;

        // We can then call the problem with an array of integers a would be expected
        auto x0 = ioh::common::random::integers(problem->meta_data().n_variables, 0, 1);
        std::cout << "y0: " << (*problem)(x0) << std::endl;
    }
}

//run *repetition* times the solver s, each time with *iterations* evaluations of the objectie function
void test_solver(solver s,std::string algo,int iterations,int repetition)
{
    using namespace ioh::problem;
    
    // We can create a suite by default, this contains all submodular problems
    ioh::suite::Submodular suite;

    // Logger for storing data
    auto b = ioh::logger::Analyzer({ioh::trigger::on_improvement}, {}, // no additional properties
                                   fs::current_path(), // path to store data
                                   "log_"+algo,// folder to store data
                                   algo,//algo name
                                   "",
                                   false//store x values ?
                                   ); 
    suite.attach_logger(b);
    for (const auto &problem : suite)
    {
        std::cout << (*problem).meta_data() << std::endl;
        for (auto i = 0; i < repetition; i++)
        {
            s(problem,iterations);
            problem->reset();
        }
    }
}

void random_search(const std::shared_ptr<ioh::problem::IntegerSingleObjective> p,int iterations)
{
    for (int i = 0; i < iterations; i++)
        (*p)(ioh::common::random::integers(p->meta_data().n_variables, 0, 1));
}

void rls(const std::shared_ptr<ioh::problem::IntegerSingleObjective> p,int iterations)
{
    auto x0=ioh::common::random::integers(p->meta_data().n_variables, 0, 1);
    int y=(*p)(x0);
    for(int i=0;i<iterations;i++)
    {
        auto k=ioh::common::random::integer(0,p->meta_data().n_variables-1);
        x0[k]=1-x0[k];
        if((*p)(x0)<y)
        {
            x0[k]=1-x0[k];
        }
    }
}

int main()
{
//    example_evaluation();
    using namespace std;
    test_solver(random_search,"random",1,100);
    cout<<"random witness completed"<<endl;
    test_solver(rls,"RLS",100,10);
    cout<<"RLS completed"<<endl;
    test_solver(random_search,"RS",100,10);
    cout<<"random search completed"<<endl;

    return 0;
}