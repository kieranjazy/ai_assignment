#include <iostream>
#include <random>
#include <vector>
#include <fstream>
#include <functional>
#include "ai.h"

using namespace std;

const auto INITIAL_POPULATION = 10; // Must be even
const auto STRING_LENGTH = 30;
const auto GENERATIONS = 1000;

int main() {
    mt19937_64 mt(random_device{}()); // Mersenne Twister random number generator

    cout << "Running each for " << GENERATIONS << " generations." << endl;
    processProblem(mt, problemAFitness, problemAMutate, "Onemax");
    processProblem(mt, problemBFitness, problemAMutate, "Evolve");
    processProblem(mt, problemCFitness, problemAMutate, "Landscape");
    processProblem(mt, problemDFitness, problemDMutate, "Evolve2");
}

// Takes an MT, a fitness function, a mutate function, and the name of the output txt file
const void processProblem(mt19937_64& mt, fitness_func fitnessFunc, mutate_func mutateFunc, const string& outputName) {
    uniform_int_distribution<int> randomInt(0, 1); // Used to generate random 0's and 1's
    vector<string> population{INITIAL_POPULATION}, repopulation{}, parents{}; // Population holds current generation, repop. holds next generation's parents
    size_t totalFitness{0}, maxFitness{0};
    string buffer{};

    for (auto i = 0; i != population.size(); i++) {
        for (auto j = 0; j != STRING_LENGTH; j++) {
            buffer += to_string(randomInt(mt));
        }
        
        population[i] = buffer;
        buffer.clear();
    } 

    ofstream outputData(outputName + ".txt");

    // Problem D requires a better population seed than 0's and 1's
    if (outputName == "Evolve2") {
        uniform_int_distribution<int> randomDigit(0, 9);

        for (auto i = 0; i != population.size(); i++) {
            for (auto j = 0; j != STRING_LENGTH; j++) {
                buffer += to_string(randomDigit(mt));
            }
        
            population[i] = buffer;
            buffer.clear();
        } 
    }

    // Run for T generations
    for (auto t = 0; t != GENERATIONS; t++) {

        totalFitness = 0;
        for (auto i = 0; i != population.size(); i++) {
            totalFitness += fitnessFunc(population[i]);
        }

        outputData << t << " " << (totalFitness / population.size()) << endl;

        uniform_int_distribution<int> randomFitness(0, totalFitness);
        auto target = 0, fitness = 0;
        for (auto i = 0; i != population.size(); i++) {
            fitness = 0;
            target = randomFitness(mt);

            for (const auto& s : population) {
                fitness += fitnessFunc(s);
                if (fitnessFunc(s) > maxFitness) {
                    maxFitness = fitnessFunc(s);
                }

                if (fitness >= target) {
                    parents.emplace_back(move(s));
                    break;
                }
            }
        }

        //population.clear();

        // Reproduction
        // Generate crossover bit location, from 1 to STRING_LENGTH - 1
        uniform_int_distribution<int> randomCrossover(0, STRING_LENGTH - 1);
        auto crossoverBit = 0;
        for (auto i = 0; i < parents.size(); i += 2) {
            crossoverBit = randomCrossover(mt);

            auto offspringA = parents[i].substr(0, crossoverBit) + parents[i+1].substr(crossoverBit);
            auto offspringB = parents[i+1].substr(0, crossoverBit) + parents[i].substr(crossoverBit);

            repopulation.emplace_back(move(offspringA));
            repopulation.emplace_back(move(offspringB));
        }

        // Mutation
        mutateFunc(repopulation, mt);

        population.swap(repopulation);
        parents.clear();
        repopulation.clear();
    }

    outputData.close();
    cout << outputName << " finished. Max fitness: " << maxFitness << endl;
}

const void problemAMutate(vector<string>& population, mt19937_64& mt) {
    uniform_int_distribution<int> randomMutateChance(0, 100);
    uniform_int_distribution<int> randomBit(0, STRING_LENGTH - 1);

    auto bit = 0;
    for (auto i = 0; i != population.size(); i++) {
        if (randomMutateChance(mt) <= 30) {
            bit = randomBit(mt);
            population[i][bit] ^= 1u;
        }
    }
}

const void problemDMutate(vector<string>& population, mt19937_64& mt) {
    uniform_int_distribution<int> randomMutateChance(0, 100);
    uniform_int_distribution<int> randomBit(0, STRING_LENGTH - 1);
    uniform_int_distribution<int> randDigit(0, 9);

    auto bit = 0;
    for (auto i = 0; i != population.size(); i++) {
        if (randomMutateChance(mt) <= 30) {
            bit = randomBit(mt);
            population[i][bit] = randDigit(mt);
        }
    }
}

const size_t problemAFitness(const string& s) {
    size_t fitnessCounter{0};

    for (const auto& c : s) {
        if (c == '1') fitnessCounter++;
    }

    return fitnessCounter;
}

const size_t problemBFitness(const string& s) {
    const string targetString("110110111011001010110101011010");
    size_t fitnessCounter{0};

    for (auto i = 0; i != s.length(); i++) {
        if (s.at(i) == targetString.at(i)) {
            fitnessCounter++;
        }
    }

    return fitnessCounter;
}

const size_t problemCFitness(const string& s) {
    size_t fitnessCounter{0};

    for (const auto& c : s) {
        if (c == '1') fitnessCounter++;
    }

    if (fitnessCounter == 0) {
        return 2 * s.length();
    }

    return fitnessCounter;
}

const size_t problemDFitness(const string& s) {
    const string targetString2("129384373440352123804353457823");
    size_t fitnessCounter{0};

    for (auto i = 0; i != s.length(); i++) {
        if (s.at(i) == targetString2.at(i)) {
            fitnessCounter++;
        }
    }

    return fitnessCounter;
}