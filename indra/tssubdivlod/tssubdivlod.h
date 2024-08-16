#include <string>
#include <map>
#include <vector>
#include "cbt.h"
#include "leb.h"



// Map is organized with a string that is "GridX-GridY-(SimZ/MaxDrawDistance)"
typedef std::map<std::string, cbt_Tree *> grid_subdiv_map;

class TSSubDivisonLOD
{
  public:
    TSSubDivisonLOD();
    int64_t DEFAULT_DEPTH = 11;
    std::string getID(int64_t gridX, int64_t gridY, int64_t simZ);
    void addSim(int64_t gridX, int64_t gridY, int64_t simZ);
    uint64_t getSimTree(int64_t gridX, int64_t gridY, int64_t simZ);
    uint64_t positionToNode(const float x, const float y, int64_t depth, float extents);
  protected:
    grid_subdiv_map mGridTree;

};

