#include <functional>
#include <string>
#include <vector>
#include <random>

typedef const std::function<const size_t(const std::string&)>& fitness_func;
typedef const std::function<void(std::vector<std::string>&, std::mt19937_64&)>& mutate_func;

const void processProblem(std::mt19937_64& mt, 
                          fitness_func fitnessFunc,
                          mutate_func mutateFunc,
                         const std::string& outputName);
const void problemAMutate(std::vector<std::string>& population, std::mt19937_64& mt); // Used for A, B, C
const void problemDMutate(std::vector<std::string>& population, std::mt19937_64& mt);
const size_t problemAFitness(const std::string& s);
const size_t problemBFitness(const std::string& s);
const size_t problemCFitness(const std::string& s);
const size_t problemDFitness(const std::string& s);