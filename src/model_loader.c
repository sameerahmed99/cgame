#include "platform.h"
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
static size_t get_gltf_mesh_vert_count(cgltf_mesh *_m);
static size_t get_gltf_mesh_indices_count(cgltf_mesh *_m);
static CG_Mesh *mesh_from_node(cgltf_node _node);
static size_t get_gltf_primitive_vert_count(cgltf_primitive *_p);

CG_Model *model_loader_load_gltf(const char *_path)
{
    cgltf_options opts = {0};

    cgltf_data *data = NULL;

    cgltf_result res = cgltf_parse_file(&opts, _path, &data);
    if (res == cgltf_result_success)
    {
        printf("Parsed model file, res: %d\n", res);
    }
    else
    {
        printf("Failed to parse model file, res: %d\n", res);
    }
    res = cgltf_load_buffers(&opts, data, _path);
    CG_Model *model;
    CG_Mesh *meshes;
    if (res == cgltf_result_success)
    {
        model = malloc(sizeof(CG_Model));
        meshes = malloc(sizeof(CG_Mesh) * data->meshes_count);
        model->meshes = meshes;
        model->numMeshes = data->meshes_count;

        int meshIndex = 0;
        printf("Model loaded, meshes: %d\n", data->meshes_count);
        for (int i = 0; i < data->nodes_count; i++)
        {
            if (data->nodes[i].mesh != NULL)
            {
                CG_Mesh *m = mesh_from_node(data->nodes[i]);
                meshes[meshIndex] = *m;
                free(m);
                meshIndex++;
            }
        }
        cgltf_free(data);
    }
    else
    {
        printf("Failed to load model: %d\n", res);
    }


    printf("Created model from gltf successfully \n");

    // for (int m = 0; m < model->mesh_count; m++)
    // {
    //     printf("\n");
    //     printf("Mesh: %d\n\n ", m+1);
    //     printf("Positions:\n");
    //     for (int i = 0; i < model->meshes[m].vertCount; i++)
    //     {
    //         printf("(%.5f, ", model->meshes[m].vertices[i].position[0]);
    //         printf("%.5f, ", model->meshes[m].vertices[i].position[1]);
    //         printf("%.5f)\n", model->meshes[m].vertices[i].position[2]);
    //     }

    //     printf("Normals:\n");
    //     for (int i = 0; i < model->meshes[m].vertCount; i++)
    //     {
    //         printf("(%.5f, ", model->meshes[m].vertices[i].normal[0]);
    //         printf("%.5f, ", model->meshes[m].vertices[i].normal[1]);
    //         printf("%.5f)\n", model->meshes[m].vertices[i].normal[2]);
    //     }
    // }

    return model;
}



static CG_Mesh *mesh_from_node(cgltf_node _node)
{
    size_t trisCount = get_gltf_mesh_indices_count(_node.mesh);
    size_t vertCount = get_gltf_mesh_vert_count(_node.mesh);

    CG_Mesh *m = malloc(sizeof(CG_Mesh));

    m->numVertices = vertCount;
    m->numIndices = trisCount;

    CG_Vertex *vertices = malloc(sizeof(CG_Vertex) * vertCount);
    m->vertices = vertices;

    u32 *triangles = malloc(sizeof(u32) * trisCount);

    m->vertices = vertices;
    m->indices = triangles;

    int vertStartingIndex = 0;
    int indicesStartingIndex = 0;
    
    //@FIX this only works if there is one primitive
    //otherwise we get a segfault when freeing the mesh, haven't checked why
    for (int i = 0; i < _node.mesh->primitives_count; i += 3)
    {
        cgltf_accessor *indicesAccessor = _node.mesh->primitives[i].indices;

        for (int j = 0; j < trisCount; j += 3)
        {
            int index = j + indicesStartingIndex;
            u32 t[3];

            // cgltf_accessor_read_uint(indicesAccessor, index, t, 3);

            t[0] = cgltf_accessor_read_index(indicesAccessor, index);
            t[1] = cgltf_accessor_read_index(indicesAccessor, index + 1);
            t[2] = cgltf_accessor_read_index(indicesAccessor, index + 2);

            triangles[index] = t[0];
            triangles[index + 1] = t[1];
            triangles[index + 2] = t[2];
        }

        for (int j = 0; j < _node.mesh->primitives[i].attributes_count; j++)
        {
            cgltf_attribute *atrib = _node.mesh->primitives[i].attributes + j;
            void *data = atrib->data->buffer_view->buffer->data;

            if (atrib->type == cgltf_attribute_type_position)
            {
                for (int k = 0; k < atrib->data->count; k++)
                {
                    CG_Vertex v;
                    uint32_t vindex = k + vertStartingIndex;
                    if ((vertices + vindex))
                    {
                        v = vertices[vindex];
                    }

                    cgltf_accessor_read_float(atrib->data, k, &v.pos.x, 3);



                    vertices[vindex] = v;
                }
            }
            else if (atrib->type == cgltf_attribute_type_normal)
            {
                for (int k = 0; k < atrib->data->count; k++)
                {
                    CG_Vertex v;
                    uint32_t vindex = k + vertStartingIndex;
                    if ((vertices + vindex))
                    {
                        v = vertices[vindex];
                    }

                    cgltf_accessor_read_float(atrib->data, k, &v.normal.x, 3);

                    vertices[vindex] = v;
                }
            }
            else if (atrib->type == cgltf_attribute_type_texcoord)
            {
                for (int k = 0; k < atrib->data->count; k++)
                {
                    CG_Vertex v;
                    uint32_t vindex = k + vertStartingIndex;
                    if ((vertices + vindex))
                    {
                        v = vertices[vindex];
                    }

                    cgltf_accessor_read_float(atrib->data, k, &v.texCoord.x, 2);

                    vertices[vindex] = v;
                }
            }
        }
        vertStartingIndex += get_gltf_primitive_vert_count(&(_node.mesh->primitives[i]));
        indicesStartingIndex += _node.mesh->primitives[i].indices->count;
    }

    // printf("Positions (%d):\n", vertCount);
    // for (int i = 0; i < vertCount; i++)
    // {
    //     printf("(%.5f, ", vertices[i].position[0]);
    //     printf("%.5f, ", vertices[i].position[1]);
    //     printf("%.5f)\n", vertices[i].position[2]);
    // }

    // printf("Normals:\n");
    // for (int i = 0; i < vertCount; i++)
    // {
    //     printf("(%.5f, ", vertices[i].normal[0]);
    //     printf("%.5f, ", vertices[i].normal[1]);
    //     printf("%.5f)\n", vertices[i].normal[2]);
    // }

    // printf("Indices (%d):\n", m->trisCount);
    // for (int i = 2; i < m->trisCount; i += 3)
    // {
    //     printf("(%d, ", m->triangles[i - 2]);
    //     printf("%d, ", m->triangles[i - 1]);
    //     printf("%d)\n", m->triangles[i]);
    // }

    return m;
}

static size_t get_gltf_mesh_vert_count(cgltf_mesh *_m)
{
    size_t count = 0;
    for (int i = 0; i < _m->primitives_count; i++)
    {
        for (int j = 0; j < _m->primitives[i].attributes_count; j++)
        {
            cgltf_attribute *atrib = _m->primitives[i].attributes + j;
            if (atrib->type == cgltf_attribute_type_position)
            {
                for (int k = 0; k < _m->primitives[i].attributes[j].data->count; k++)
                {
                    count++;
                }
            }
        }
    }
    return count;
}

static size_t get_gltf_mesh_indices_count(cgltf_mesh *_m)
{
    size_t count = 0;
    for (int i = 0; i < _m->primitives_count; i++)
    {
        count += _m->primitives[i].indices->count;
    }
    return count;
}

static size_t get_gltf_primitive_vert_count(cgltf_primitive *_p)
{
    size_t count = 0;

    for (int j = 0; j < _p->attributes_count; j++)
    {
        cgltf_attribute *atrib = _p->attributes + j;
        if (atrib->type == cgltf_attribute_type_position)
        {
            for (int k = 0; k < _p->attributes[j].data->count; k++)
            {
                count++;
            }
        }
    }
    return count;
}

