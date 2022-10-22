//
// Created by Kiril on 31.08.2022.
//

#ifndef DEEPLOM_RAW_OBJ_H
#define DEEPLOM_RAW_OBJ_H

#include <string>

#include <garray>

namespace gorilla::geometry {

    class raw_obj {
    public:
        using real_t = float;
        using index_t = uint32_t;
        using str_t = std::string;

        array<real_t> v;
        array<real_t> vt;
        array<real_t> vn;

        array<index_t> faceVertexIndices;
        array<index_t> faceVertexStarts;

        array<index_t> faceTexCoordIndices;
        array<index_t> faceTexCoordStarts;

        array<index_t> faceNormalIndices;
        array<index_t> faceNormalStarts;

//        array<str_t> objectNames;
//        array<index_t> faceObjectCorrespondence;
//
//        array<str_t> groupNames;
//        array<index_t> faceGroupCorrespondence;
//
//        array<str_t> materialNames;
//        array<index_t> faceMaterialCorrespondence;

        raw_obj() = default;
        raw_obj(const raw_obj& copy) = default;
        raw_obj(raw_obj&& move) = default;

        raw_obj& operator =(const raw_obj& copy) = default;
        raw_obj& operator =(raw_obj&& move) = default;

        raw_obj(const str_t& filename);

        int nVertices() const;
        bool hasVertices() const;

        int nTexCoords() const;
        bool hasTexCoords() const;

        int nNormals() const;
        bool hasNormals() const;

        int nFaces() const;
        bool hasFaces() const;

        int nTexCoordFaces() const;
        bool hasTexCoordFaces() const;

        int nNormalFaces() const;
        bool hasNormalFaces() const;

        bool hasIdenticalAttribTopology() const;

    private:
        bool m_identicalAttribTopology = true;
    };

} // Geometry

#endif //DEEPLOM_RAW_OBJ_H
