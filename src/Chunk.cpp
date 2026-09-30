#include "Chunk.h"
#include <raymath.h>
#include <fstream>
#include <iostream>

Chunk::Chunk (int grid_x, int grid_z)
: grid_x (grid_x),
  grid_z (grid_z)
{
    position = { static_cast<float>(grid_x * CHUNK_EDGE_LEN),
                 0.0f,
                 static_cast<float>(grid_z * CHUNK_EDGE_LEN) };

    data = new Block[CHUNK_VOL];
    for (int i = 0; i < CHUNK_VOL; i++)
        data[i] = AIR;
}

Chunk::Chunk (int grid_x, int grid_z, std::string file_path)
: Chunk (grid_x, grid_z)
{
    std::ifstream file { file_path };
    if (!file) {
        std::cerr << "Could not load file: " << file_path << std::endl;
        return;
    }

    std::string block_data;
    int data_offset = 0;
    while (std::getline(file, block_data)) {
        int pos = block_data.find(' ');
        int num_blocks = std::stoi(block_data.substr(0, pos));
        int type = std::stoi(block_data.substr(pos+1));
        for (int j = 0; j < num_blocks && j+data_offset < CHUNK_VOL; j++) {
            data[j+data_offset] = (Block)type;
        }
        data_offset += num_blocks;
    }
}

Chunk::~Chunk() {

}

void Chunk::Serialize (std::string file_path) {
    std::ofstream file { file_path };
    if (!file) {
        std::cerr << "Could not create file: " << file_path << std::endl;
        return;
    }

    // Use RLE to save chunk data
    int i = 0;
    while (i < CHUNK_VOL) {
        Block type = data[i];
        int count = 0;
        while (i < CHUNK_VOL && data[i] == type) {
            i++;
            count++;
        }
        file << count << ' ' << (int)type << std::endl;
    }
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
