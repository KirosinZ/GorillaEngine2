//
// Created by Kiril on 31.08.2022.
//

#include <fstream>
#include <sstream>
#include "raw_obj.h"

namespace gorilla::geometry {

    raw_obj::raw_obj(const str_t& filename)
    {

    }

    int raw_obj::nVertices() const
    {
        return v.size();
    }

    bool raw_obj::hasVertices() const
    {
        return nVertices() > 0;
    }

    int raw_obj::nTexCoords() const
    {
        return vt.size();
    }

    bool raw_obj::hasTexCoords() const
    {
        return nTexCoords() > 0;
    }

    int raw_obj::nNormals() const
    {
        return vn.size();
    }

    bool raw_obj::hasNormals() const
    {
        return nNormals() > 0;
    }

    int raw_obj::nFaces() const
    {
        return faceVertexStarts.size() == 0 ? 0 : faceVertexStarts.size() - 1;
    }

    bool raw_obj::hasFaces() const
    {
        return nFaces() > 0;
    }

    int raw_obj::nTexCoordFaces() const
    {
        return faceTexCoordStarts.size() == 0 ? 0 : faceTexCoordStarts.size() - 1;
    }

    bool raw_obj::hasTexCoordFaces() const
    {
        return nTexCoordFaces() > 0;
    }

    int raw_obj::nNormalFaces() const
    {
        return faceNormalStarts.size() == 0 ? 0 : faceNormalStarts.size() - 1;
    }

    bool raw_obj::hasNormalFaces() const
    {
        return nNormalFaces() > 0;
    }

    bool raw_obj::hasIdenticalAttribTopology() const
    {
        return m_identicalAttribTopology;
    }

} // Geometry