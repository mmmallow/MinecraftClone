#include "Chunk.h"

Chunk::Chunk (int id, Vector3 pos)
: id (id),
  position (pos)
{
    data = new Block[VOX_VOL];
    for (int i = 0; i < VOX_VOL; i++)
        data[i] = AIR;
}

Chunk::~Chunk() {

}

void Chunk::Destroy() {
    delete[] data;
}

Block& Chunk::Get (Vector3 local) {
    return data[static_cast<int>(local.x + local.z * EDGE_LEN + local.y * VOX_AREA)];
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
