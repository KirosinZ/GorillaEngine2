//
// Created by Kiril on 31.08.2022.
//

#ifndef DEEPLOM_FACEVERTEXMESH_H
#define DEEPLOM_FACEVERTEXMESH_H

#include <vector>

namespace geometry {

    class facevertexmesh {
        std::vector<float[3]> v;
        std::vector<float[3]> vt;
        std::vector<float[3]> vn;
    };

} // geometry

#endif //DEEPLOM_FACEVERTEXMESH_H
