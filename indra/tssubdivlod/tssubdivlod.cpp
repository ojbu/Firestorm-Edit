#include "tssubdivlod.h"

TSSubDivisonLOD::TSSubDivisonLOD()
{
    int64_t DEFAULT_DEPTH = 11;
    grid_subdiv_map mGridTree;

}

std::string TSSubDivisonLOD::getID(int64_t gridX, int64_t gridY, int64_t simZ)
{
    return std::to_string(gridX) + "-" + std::to_string(gridY) + "-" + std::to_string(simZ);
};

void TSSubDivisonLOD::addSim(int64_t gridX, int64_t gridY, int64_t simZ)
{
    cbt_Tree *cbt = cbt_Create(DEFAULT_DEPTH);
    std::string id  = getID(gridX, gridY, simZ);
    mGridTree[id]   = cbt;
};

uint64_t TSSubDivisonLOD::getSimTree(int64_t gridX, int64_t gridY, int64_t simZ)
{
    std::string id = getID(gridX, gridY, simZ);
    if (!mGridTree[id])
        addSim(gridX, gridY, simZ);
    cbt_Tree *tree = mGridTree[id];
    return (uint64_t) tree;
};

float Wedge(const float *a, const float *b)
{
    return a[0] * b[1] - a[1] * b[0];
}

bool isInsideXY(const float faceVertices[][3], const float x, const float y)
{
    float target[2] = {x, y};
    float v1[2]     = {faceVertices[0][0], faceVertices[1][0]};
    float v2[2]     = {faceVertices[0][1], faceVertices[1][1]};
    float v3[2]     = {faceVertices[0][2], faceVertices[1][2]};
    float x1[2]     = {v2[0] - v1[0], v2[1] - v1[1]};
    float x2[2]     = {v3[0] - v2[0], v3[1] - v2[1]};
    float x3[2]     = {v1[0] - v3[0], v1[1] - v3[1]};
    float y1[2]     = {target[0] - v1[0], target[1] - v1[1]};
    float y2[2]     = {target[0] - v2[0], target[1] - v2[1]};
    float y3[2]     = {target[0] - v3[0], target[1] - v3[1]};
    float w1        = Wedge(x1, y1);
    float w2        = Wedge(x2, y2);
    float w3        = Wedge(x3, y3);

    return (w1 >= 0.0f) && (w2 >= 0.0f) && (w3 >= 0.0f);
}

uint64_t TSSubDivisonLOD::positionToNode(const float x, const float y, int64_t depth, float extents)
{
    uint64_t minNodeID = (1ULL << depth);
    uint64_t maxNodeID = (2ULL << depth);

    uint64_t foundID = 0;

    for (uint64_t nodeID = maxNodeID; nodeID > minNodeID; nodeID--)
    {
        float faceVertices[][3] = {
            {0.0f, 0.0f, extents},
            {extents, 0.0f, 0.0f}
        };
        cbt_Node thisNode = cbt_CreateNode(nodeID, depth);
        leb_DecodeNodeAttributeArray_Square(thisNode, 2, faceVertices);
        if (isInsideXY(faceVertices, x, y))
        {
            foundID = nodeID;
            break;
        }
    }

    return foundID;
}


