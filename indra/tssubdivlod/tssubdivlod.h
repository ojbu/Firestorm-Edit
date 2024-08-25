/**
 * @file tssubdivlod.h
 * @brief This is an implementation of a LOD management system using a novel subdivision binary tree.
 *
 * $LicenseInfo:firstyear=2024&license=lgpl$
 * TommyTheTerrible
 * Copyright (C) 2024, TommyTheTerrible
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * TommyTheTerrible: tommy@tommytheterrible.com, https://www.tommytheterrible.com/ https://github.com/TommyTheTerrible/
 * $/LicenseInfo$
 */
#pragma once
#include <string>
#include <unordered_map>
#include <vector>



// Map is organized with a string that is "GridX-GridY-(SimZ/MaxDrawDistance)"
typedef std::unordered_map<std::string, int64_t> grid_subdiv_map;

class TSSubDivisonLOD
{
  public:
    TSSubDivisonLOD();
    static TSSubDivisonLOD sTSSubDivisonLOD;
    //static TSSubDivisonLOD* getLOD() { return sTSSubDivisonLOD; }
    static int64_t const CBT_DEFAULT_DEPTH = 9;
    static std::string getID(int64_t region_handle, int64_t simZ);
    void checkSim(std::string id, const int64_t region_handle);
    static uint64_t getSimNode(const int64_t region_handle, const float vector[3], const int depth, const float extents);
    static uint64_t positionToNode(const float x, const float y, const int depth, const float extents);
  protected:
    grid_subdiv_map mGridTree; 

};

