#include "Chunk.h"
#include <raymath.h>

Chunk::Chunk (int id, int grid_x, int grid_z)
: id (id),
  grid_x (grid_x),
  grid_z (grid_z)
{
    position = { static_cast<float>(grid_x * CHUNK_EDGE_LEN),
                 0.0f,
                 static_cast<float>(grid_z * CHUNK_EDGE_LEN) };

    data = new Block[CHUNK_VOL];
    for (int i = 0; i < CHUNK_VOL; i++)
        data[i] = AIR;
}

Chunk::~Chunk() {

}

void Chunk::Destroy() {
    delete[] data;
}

Block& Chunk::Get (Vector3 local) {
    return data[static_cast<int>(local.x + local.z * CHUNK_EDGE_LEN + local.y * CHUNK_AREA)];
}

Vector3 Chunk::ToLocal (Vector3 world) const {
    return { world.x - position.x, world.y - position.y, world.z - position.z };
}

Block& Chunk::GetFromWorld (Vector3 world) {
    return Get(ToLocal(world));
}

void Chunk::Set (Vector3 local, Block block) {
    Block& old_block = Get(local);
    old_block = block;
}

void Chunk::SetFromWorld (Vector3 world, Block block) {
    Set(ToLocal(world), block);
}
