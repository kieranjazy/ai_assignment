#include "a1_csv.h"
#include <iostream>
#include <random>
#include <algorithm>
#include <math.h>
#include <fstream>

using namespace std;

struct mapping {
    std::pair<size_t, size_t> supervisor;
    vector<size_t> studentIds;

    mapping(const std::pair<size_t, size_t>& supervisor, const vector<size_t>& studentIds) : supervisor(supervisor), studentIds(studentIds) {}
};

const auto INITIAL_POPULATION = 20;
const auto CROSSOVER_FRACTION = 0.6;
const auto MUTATION_RATE = 0.4;
const auto GENERATIONS = 10000;

// Fitness function, lower is better
const size_t calculateFitness(const student_map& students, const mapping& mapping);
const size_t calculateMappingCollectionFitness(const student_map& students, const vector<mapping>& mappingCollection);

int main() {
    const auto students = parseStudentsCsv("Student-choices.csv");
    const auto supervisors = parseSupervisorsCsv("Supervisors.csv");

    mt19937_64 mt(random_device{}());
    vector<vector<mapping>> population, repopulation, parentSelection, bestPopulation{};
    auto bestFitness = 0;

    // Create an initial population of random mapping collections
    // A mapping collection is a vector of mappings which defines an individual
    // A collection has every student matched with a supervisor according to their capacity
    for (auto i = 0; i != INITIAL_POPULATION; i++) {
        vector<size_t> unallocatedIds(students.size());
        vector<mapping> mappings;

        iota(begin(unallocatedIds), end(unallocatedIds), 1); // Fill with 1 .. (highest student id)

        for (const auto& supervisor_pair : supervisors) {
            vector<size_t> studentIds;
            for (auto i = 0; i != supervisor_pair.second; i++) {
                uniform_int_distribution<int> randomInt(0, unallocatedIds.size() - 1);
                size_t randomIndex = randomInt(mt);
                studentIds.push_back(unallocatedIds.at(randomIndex));
                unallocatedIds.erase(unallocatedIds.begin() + randomIndex);
            }

            mappings.emplace_back(supervisor_pair, move(studentIds));
        }

        population.emplace_back(move(mappings));
    }

    ofstream outputData("part_b.txt");

    // Run this mapping generator for t generations
    for (auto t = 0; t != GENERATIONS; t++) {
        size_t generationFitness{0};

        // Calculate generational fitness
        for (const auto& mappingCollection : population) {
            for (const auto& mapping : mappingCollection) {
                generationFitness += calculateFitness(students, mapping);
            }
        }

        outputData << t << " " << (generationFitness / population.size()) << endl;

        if (bestPopulation.size() == 0) {
            bestFitness = generationFitness / population.size();
            bestPopulation = population;
        } else {
            if ((generationFitness / population.size()) > (bestFitness)) {
                bestFitness = generationFitness / population.size();
                bestPopulation = population;
            }
        }

        // Reproduction/Crossover
        // Selection
        uniform_int_distribution<int> randomFitness(1, generationFitness); 
        for (int i = 0; i != ceil(population.size() * CROSSOVER_FRACTION); i++) {
            auto target = randomFitness(mt), totalFitness = 0;
            for (const auto& mappingCollection : population) {
                totalFitness += calculateMappingCollectionFitness(students, mappingCollection);
                if (totalFitness > target) {
                    parentSelection.emplace_back(mappingCollection);
                    break;
                }
            }
        }

        // Crossover
        uniform_int_distribution<int> randomFitness2(1, (generationFitness / 2) - 1);
        for (auto i = 0; i < parentSelection.size(); i += 2) {
            int index1{-1}, index2{-1};
            auto target = randomFitness2(mt), totalFitness = 0;
            for (auto j = 0; j != parentSelection.size(); j++) {
                totalFitness += calculateMappingCollectionFitness(students, parentSelection[j]);
                if (totalFitness >= target) {
                    index1 = j;
                    break;
                }
            }

            totalFitness = 0;
            target = randomFitness2(mt);

            for (auto j = 0; j != parentSelection.size(); j++) {
                totalFitness += calculateMappingCollectionFitness(students, parentSelection[j]);
                if (totalFitness >= target) {
                    index2 = j;
                    break;
                }
            }

            if (index1 == -1 || index2 == -1) continue;

            vector<mapping> offspring1{}, offspring2{};
            vector<size_t> allocatedIds1{}, allocatedIds2{};

            // Iterate each pair of mappings, swap student ids after a random bit
            for (auto j = 0; j != parentSelection[index1].size(); j++) {
                uniform_int_distribution<int> randomBit(0, parentSelection[index1][j].studentIds.size() - 1);
                const auto rb = randomBit(mt);
                allocatedIds1 = {}, allocatedIds2 = {};

                if (parentSelection[index1].size() == 0 || parentSelection[index2].size() == 0) {
                    continue;
                }

                copy(parentSelection[index1][j].studentIds.cbegin(),
                     parentSelection[index1][j].studentIds.cbegin() + rb,
                     back_inserter(allocatedIds1)); 

                copy(parentSelection[index2][j].studentIds.cbegin() + rb, 
                     parentSelection[index2][j].studentIds.cend(), 
                     back_inserter(allocatedIds1));

                copy(parentSelection[index2][j].studentIds.cbegin(),
                     parentSelection[index2][j].studentIds.cbegin() + rb,
                     back_inserter(allocatedIds2));

                copy(parentSelection[index1][j].studentIds.cbegin() + rb,
                     parentSelection[index1][j].studentIds.cend(),
                     back_inserter(allocatedIds2)); 

                offspring1.emplace_back(parentSelection[index1][j].supervisor, parentSelection[index1][j].studentIds);
                offspring2.emplace_back(parentSelection[index2][j].supervisor, parentSelection[index2][j].studentIds);
            }
            
            // Mutate
            uniform_real_distribution<double> willMutate(0, 1);

            // Mutation rate determines if each offspring is mutated
            // Mutation method is to swap 2 random students belonging to 2 random supervisors
            // Requires 2 random student indices and 2 random supervisor indices
            if (willMutate(mt) < MUTATION_RATE) { // Offspring 1
                uniform_int_distribution<int> randomSupervisor(1, supervisors.size());
                auto supervisor1 = randomSupervisor(mt), supervisor2 = randomSupervisor(mt);

                if (supervisor1 != supervisor2) {
                    size_t randomStudentIndex1{0}, randomStudentIndex2{0};
                    size_t randomStudentId1{0}, randomStudentId2{0};

                    for (auto k = 0; k != offspring1.size(); k++) {
                        if (offspring1[k].supervisor.first == supervisor1) {
                            uniform_int_distribution<int> randomStudentIndex(0, offspring1[k].studentIds.size() - 1);
                            randomStudentIndex1 = randomStudentIndex(mt);
                            randomStudentId1 = offspring1[k].studentIds[randomStudentIndex1];
                        } else if (offspring1[k].supervisor.first == supervisor2) {
                            uniform_int_distribution<int> randomStudentIndex(0, offspring1[k].studentIds.size() - 1);
                            randomStudentIndex2 = randomStudentIndex(mt);
                            randomStudentId2 = offspring1[k].studentIds[randomStudentIndex2];
                        }
                    }

                    for (auto& mapping : offspring1) {
                        if (mapping.supervisor.first == supervisor1) {
                            mapping.studentIds[randomStudentIndex1] = randomStudentId2;
                        } else if (mapping.supervisor.first == supervisor2) {
                            mapping.studentIds[randomStudentIndex2] = randomStudentId1;
                        }
                    }      
                }
            }

            if (willMutate(mt) < MUTATION_RATE) { // Offspring 2
                uniform_int_distribution<int> randomSupervisor(1, supervisors.size());
                auto supervisor1 = randomSupervisor(mt), supervisor2 = randomSupervisor(mt);

                if (supervisor1 != supervisor2) {
                    size_t randomStudentIndex1{0}, randomStudentIndex2{0};
                    size_t randomStudentId1{0}, randomStudentId2{0};

                    for (auto k = 0; k != offspring2.size(); k++) {
                        if (offspring2[k].supervisor.first == supervisor1) {
                            uniform_int_distribution<int> randomStudentIndex(0, offspring2[k].studentIds.size() - 1);
                            randomStudentIndex1 = randomStudentIndex(mt);
                            randomStudentId1 = offspring2[k].studentIds[randomStudentIndex1];
                        } else if (offspring2[k].supervisor.first == supervisor2) {
                            uniform_int_distribution<int> randomStudentIndex(0, offspring2[k].studentIds.size() - 1);
                            randomStudentIndex2 = randomStudentIndex(mt);
                            randomStudentId2 = offspring2[k].studentIds[randomStudentIndex2];
                        }
                    }

                    for (auto& mapping : offspring2) {
                        if (mapping.supervisor.first == supervisor1) {
                            mapping.studentIds[randomStudentIndex1] = randomStudentId2;
                        } else if (mapping.supervisor.first == supervisor2) {
                            mapping.studentIds[randomStudentIndex2] = randomStudentId1;
                        }
                    }      
                }
            }

            repopulation.emplace_back(offspring1);
            repopulation.emplace_back(offspring2);
        }

        population = repopulation; // Repopulation contains at this point all mutated offspring
        repopulation.clear();
        repopulation.resize(0);
        parentSelection.clear();
    }

    outputData.close();

    if (bestPopulation.size() > 0) {
        cout << "Best (fitness: " << bestFitness << ") generation mappings are: \n" << endl;
    }
    
    for (const auto& mappingCollection : bestPopulation) {
        for (const auto& mapping : mappingCollection) {
            cout << "Supervisor: " << mapping.supervisor.first << " Students ";
            for (const size_t id : mapping.studentIds) {
                cout << id << " ";
            }
            cout << " Fitness: " << calculateFitness(students, mapping) << endl;
        }
        cout << endl;
    }

    return 0;
}



// Fitness function, higher is better
const size_t calculateFitness(const student_map& students, const mapping& mapping) {
    auto fitness = 0;
    for (const auto& id : mapping.studentIds) {
        // This counts preference in reverse order, with lecturers towards front of prefs having a higher score
        fitness += students.at(id).end() - find(students.at(id).begin(), students.at(id).end(), mapping.supervisor.first);
    }

    return fitness;
}

const size_t calculateMappingCollectionFitness(const student_map& students, const vector<mapping>& mappingCollection) {
    auto fitness = 0;
    for (const auto& mapping : mappingCollection) {
        for (const auto& id : mapping.studentIds) {
            fitness += students.at(id).end() - find(students.at(id).begin(), students.at(id).end(), mapping.supervisor.first);
        }
    }
    
    return fitness;
}