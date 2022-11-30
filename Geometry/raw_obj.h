//
// Created by Kiril on 31.08.2022.
//

#ifndef DEEPLOM_RAW_OBJ_H
#define DEEPLOM_RAW_OBJ_H

#include <string>

#include <garray>

#include <Misc/line.h>

namespace gorilla::geometry
{

    class raw_obj
    {
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

        raw_obj() = default;
        raw_obj(const raw_obj& copy) = default;
        raw_obj(raw_obj&& move) = default;

        raw_obj& operator =(const raw_obj& copy) = default;
        raw_obj& operator =(raw_obj&& move) = default;

        raw_obj(
                const str_t& filename,
                const bool read_vertices = true,
                const bool read_texcoords = true,
                const bool read_normals = true,
                const bool read_facecoords = true,
                const bool read_facetexcoords = true,
                const bool read_facenormals = true,
                const bool force_4dim_coords = false,
                const bool force_3dim_texcoords = false,
                const bool force_triangle_faces = false,
                const bool force_quad_faces = false)
                : _read_vertices(read_vertices),
                  _read_texcoords(read_texcoords),
                  _read_normals(read_normals),
                  _read_facecoords(read_facecoords),
                  _read_facetexcoords(read_facetexcoords),
                  _read_facenormals(read_facenormals),
                  _coords_per_vertex(force_4dim_coords ? 4 : 3),
                  _coords_per_texcoord(force_3dim_texcoords ? 3 : 2),
                  _attribs_per_face(0)
        {
            if (force_triangle_faces && force_quad_faces)
                throw std::exception("Impossible to force both triangle and quad faces");

            if (force_triangle_faces)
                _attribs_per_face = 3;
            if (force_quad_faces)
                _attribs_per_face = 4;

            std::vector<std::string> ignore_list = {
                    "#",
                    "g",
            };
            if (!_read_vertices)
                ignore_list.emplace_back("v");
            if (!_read_texcoords)
                ignore_list.emplace_back("vt");
            if (!_read_normals)
                ignore_list.emplace_back("vn");
            if (!(_read_facecoords || _read_facetexcoords || _read_facenormals))
                ignore_list.emplace_back("f");

            const auto& validate = [&] (const std::string& str)
            {
                bool ok = false;
                for (auto iter = ignore_list.begin(); !ok && iter != ignore_list.end(); iter++)
                    ok = str == *iter;

                return !ok;
            };

            std::ifstream file(filename);
            if (!file.is_open())
                throw std::exception((std::string("Could not open file: ") + filename).c_str());

            using line_iterator = std::istream_iterator<fileutils::Line>;
            for (line_iterator line(file); line != std::istream_iterator<fileutils::Line>(); line++)
            {
                std::istringstream linestream(line->str());
                std::istream_iterator<std::string> snippet(linestream);
                if (snippet == std::istream_iterator<std::string>() || !validate(*snippet))
                    continue;

                // complete validation
                if (*snippet == "v")
                {
                    snippet++;
                    v.push_back(std::stof(*snippet++));
                    v.push_back(std::stof(*snippet++));
                    v.push_back(std::stof(*snippet++));

                    if (snippet != std::istream_iterator<std::string>())
                    {
                        float w = std::stof(*snippet++);
                        if (_coords_per_vertex == 4)
                        {
                            v.push_back(w);
                        }
                        else
                        {
                            v[v.size() - 1] /= w;
                            v[v.size() - 2] /= w;
                            v[v.size() - 3] /= w;
                        }
                    }
                    else
                    {
                        if (_coords_per_vertex == 4)
                        {
                            v.push_back(1.0f);
                        }
                    }
                    continue;
                }

                if (*snippet == "vt")
                {
                    snippet++;
                    vt.push_back(std::stof(*snippet++));
                    vt.push_back(std::stof(*snippet++));

                    if (snippet != std::istream_iterator<std::string>())
                    {
                        float w = std::stof(*snippet++);
                        if (_coords_per_texcoord == 3)
                        {
                            v.push_back(w);
                        }
                    }
                    else
                    {
                        if (_coords_per_texcoord == 3)
                        {
                            v.push_back(0.0f);
                        }
                    }
                    continue;
                }

                if (*snippet == "vn")
                {
                    snippet++;
                    vn.push_back(std::stof(*snippet++));
                    vn.push_back(std::stof(*snippet++));
                    vn.push_back(std::stof(*snippet++));
                    continue;
                }
            }
        }

        int nVertices() const
        {
            return v.size() / _coords_per_vertex;
        }
        bool hasVertices() const
        {
            return _read_vertices && !v.empty();
        };

        int nTexCoords() const
        {
            return vt.size() / _coords_per_texcoord;
        };
        bool hasTexCoords() const
        {
            return _read_texcoords && !vt.empty();
        };

        int nNormals() const
        {
            return vn.size() / 3;
        };
        bool hasNormals() const
        {
            return _read_normals && !vn.empty();
        };

        int nFaces() const
        {
            return (_attribs_per_face == 0 ? faceVertexStarts.size() - 1 : faceVertexIndices.size() / _attribs_per_face);
        };
        bool hasFaces() const
        {
            return _read_facecoords && !faceVertexIndices.empty();
        };

        int nTexCoordFaces() const
        {
            return (_attribs_per_face == 0 ? faceTexCoordStarts.size() - 1 : faceTexCoordIndices.size() / _attribs_per_face);
        };
        bool hasTexCoordFaces() const
        {
            return _read_facetexcoords && !faceTexCoordIndices.empty();
        };

        int nNormalFaces() const
        {
            return (_attribs_per_face == 0 ? faceNormalStarts.size() - 1 : faceNormalIndices.size() / _attribs_per_face);
        };
        bool hasNormalFaces() const
        {
            return _read_facenormals && !faceNormalIndices.empty();
        };

    private:
        bool _read_vertices;
        bool _read_texcoords;
        bool _read_normals;
        bool _read_facecoords;
        bool _read_facetexcoords;
        bool _read_facenormals;
        int _coords_per_vertex;
        int _coords_per_texcoord;
        int _attribs_per_face;
    };
} // gorilla::geometry

#endif //DEEPLOM_RAW_OBJ_H
