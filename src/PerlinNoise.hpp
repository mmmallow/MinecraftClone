/*
*
*  This class comes from https://www.gamegeniuslab.com/tutorial-post/introduction-to-procedural-generation-with-perlin-noise-for-game-development/
*
*/





#ifndef PERLIN_NOISE_HPP
#define PERLIN_NOISE_HPP

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

/// A pseudo-random number generator class based on a linear congruential generator (LCG) algorithm.
/// This class allows generating a sequence of pseudo-random numbers from a seed value,
/// ensuring reproducible outcomes which are useful for simulations, testing, and gaming.
class SeededRandom {
private:
  // Current seed value, evolves with each call to Next() or NextFloat().
  long seed;

  // The multiplier 'a'. This value is part of the parameters that define the quality and characteristics
  // of the linear congruential generator. 48271 is a commonly used value in LCG algorithms for its good properties.
  static const long a = 48271;

  // The increment 'c'. It is set to 0 in this implementation, defining it as a multiplicative LCG.
  static const long c = 0;

  // The modulus 'm'. Using 2^31 - 1 (a Mersenne prime) as the modulus helps in achieving a full period
  // for the generated pseudo-random sequence, maximizing the sequence's length before it repeats.
  static const long m = 2147483647;

public:
  /// Constructor that initializes the pseudo-random number generator with a seed value.
  /// @param seed Initial seed value for generating numbers.
  SeededRandom(int seed) : seed(seed) {}

  /// Generates the next number in the sequence of pseudo-random numbers.
  /// @return The next pseudo-random number as an int.
  int Next() {
    // Calculates the next seed value using the linear congruential generator formula.
    // The use of static_cast<int> ensures the result is properly typed as an integer.
    seed = (a * seed + c) % m;
    return static_cast<int>(seed);
  }

  /// Generates a floating-point number between 0 (inclusive) and 1 (exclusive) based on the pseudo-random sequence.
  /// @return A pseudo-random float between 0 and 1.
  float NextFloat() {
    // Utilizes the Next() function to obtain a pseudo-random integer, then divides it by 'm'
    // to normalize the result to a floating-point number in the range [0, 1).
    return Next() / static_cast<float>(m);
  }
};


// Existing implementation

// A class for generating Perlin noise, a procedural technique used in computer graphics to produce natural-looking textures.
class PerlinNoise {
private:
  // A table used for permutation in the Perlin noise algorithm. It's doubled to avoid overflow in indexing.
  std::vector<int> permutationTable;
  // The size of the permutation table.
  static const int tableSize = 256;
  // A bitmask used for wrapping indices to remain within the permutation table's bounds.
  static const int tableSizeMask = tableSize - 1;
  // Seeded random number generator to shuffle the permutation table in a reproducible way.
  SeededRandom seededRandom;

  // The Fade function smooths the input values to ease transitions, as described by Ken Perlin.
  float Fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }

  // Linear interpolation between two values a and b using the blend factor t.
  float Lerp(float t, float a, float b) { return a + t * (b - a); }

  // Calculates a gradient based on a hash value and the x, y, z coordinates. 
  // This contributes to the pseudo-randomness of the noise.
  float Grad(int hash, float x, float y, float z) {
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
  }

public:
  // Constructor initializes the Perlin noise generator with a specific seed.
  PerlinNoise(int seed) : seededRandom(seed) {
    permutationTable.resize(tableSize * 2);
    std::vector<int> tempTable(tableSize);
    for (int i = 0; i < tableSize; i++)
      tempTable[i] = i;

    // Shuffle the temporary permutation table using the seeded random generator.
    for (int i = 0; i < tableSize; i++) {
      int j = seededRandom.Next() % tableSize;
      std::swap(tempTable[i], tempTable[j]);
    }

    // Duplicate the shuffled permutation table to avoid overflow.
    for (int i = 0; i < tableSize; i++) {
      permutationTable[i] = tempTable[i];
      permutationTable[tableSize + i] = tempTable[i];
    }
  }

  // Generates Perlin noise for a given point (x, y, z) in 3D space.
  float Noise(float x, float y, float z) {
    // Find the unit cube containing the point and wrap the integer parts to the table size.
    int X = static_cast<int>(std::floor(x)) & tableSizeMask;
    int Y = static_cast<int>(std::floor(y)) & tableSizeMask;
    int Z = static_cast<int>(std::floor(z)) & tableSizeMask;

    // Calculate the relative position of the point in the cube.
    x -= std::floor(x);
    y -= std::floor(y);
    z -= std::floor(z);

    // Compute fade curves for x, y, z.
    float u = Fade(x);
    float v = Fade(y);
    float w = Fade(z);

    // Hash coordinates of the cube's eight corners.
    int A = permutationTable[X] + Y;
    int AA = permutationTable[A] + Z;
    int AB = permutationTable[A + 1] + Z;
    int B = permutationTable[X + 1] + Y;
    int BA = permutationTable[B] + Z;
    int BB = permutationTable[B + 1] + Z;

    // Add blended results from the eight corners of the cube.
    float res =
        Lerp(w,
             Lerp(v,
                  Lerp(u, Grad(permutationTable[AA], x, y, z),
                       Grad(permutationTable[BA], x - 1, y, z)),
                  Lerp(u, Grad(permutationTable[AB], x, y - 1, z),
                       Grad(permutationTable[BB], x - 1, y - 1, z))),
             Lerp(v,
                  Lerp(u, Grad(permutationTable[AA + 1], x, y, z - 1),
                       Grad(permutationTable[BA + 1], x - 1, y, z - 1)),
                  Lerp(u, Grad(permutationTable[AB + 1], x, y - 1, z - 1),
                       Grad(permutationTable[BB + 1], x - 1, y - 1, z - 1))));

    // Normalize the result to be within the range [0, 1].
    return (res + 1.0f) / 2.0f;
  }

  /// Generates fractal noise by combining multiple octaves of Perlin noise.
  /// Each octave contributes to the final noise output with its own frequency and amplitude,
  /// creating a more complex and visually interesting result. This method is especially useful
  /// for generating realistic textures or terrain in game development.
  ///
  /// @param x The x coordinate in the noise space.
  /// @param y The y coordinate in the noise space.
  /// @param z The z coordinate in the noise space, useful for 3D noise but can be kept constant for 2D.
  /// @param octaves The number of iterations or layers of noise to combine for the final output.
  ///                More octaves lead to more detail at the cost of computation time.
  /// @param persistence Determines how much each subsequent octave contributes to the total noise.
  ///                    A lower persistence value makes the noise smoother.
  /// @param lacunarity Controls the frequency growth for each octave. Higher lacunarity values
  ///                   increase the complexity of the noise pattern.
  /// @return A single float value representing the fractal noise at the given coordinates,
  ///         normalized to the range [0, 1].
  float FractalNoise(float x, float y, float z, int octaves, float persistence,
                     float lacunarity) {
    float total = 0; // Accumulates the total noise value from all octaves.
    float frequency = 1; // The starting frequency of the noise.
    float amplitude = 1; // The starting amplitude of the noise.
    float maxValue = 0; // Used for normalizing the result to the range [0, 1].

    // Iterate through each octave to layer the noise.
    for (int i = 0; i < octaves; i++) {
      int offset = i * 2; // Apply an offset to each octave to vary the noise pattern.
      // Add the noise value, scaled by the current amplitude and frequency, to the total.
      total += Noise((x + offset) * frequency, (y + offset) * frequency, z * frequency) * amplitude;
      // Accumulate the maximum possible value to normalize the result later.
      maxValue += amplitude;
      // Decrease the amplitude by the persistence factor for each subsequent octave.
      amplitude *= persistence;
      // Increase the frequency by the lacunarity factor for each subsequent octave.
      frequency *= lacunarity;
    }

    // Normalize the total noise value to fall within the range [0, 1] before returning it.
    return total / maxValue;
  }
};


#endif // !PERLIN_NOISE_HPP
