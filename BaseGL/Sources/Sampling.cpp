#include<vector>
#include<random>

using namespace std;
typedef std::vector<glm::vec3> vv3;

default_random_engine gen;
float lerp (float a, float b, float f) { return a + f * (b - a); }
uniform_real_distribution<float> dist(0,1);
const float pi = acos(-1);

glm::vec3 uniHalfSphere() {
    float z = 2*dist(gen)-1;
    float th = 2*pi * dist(gen);
    float r = sqrt(1-z*z);
    return { r*cos(th), r*sin(th), abs(z) };
}

glm::vec3 uniDisk() {
    float th = 2*pi * dist(gen);
    float r = dist(gen);
    r = sqrt(r);
    return { r*cos(th), r*sin(th), 0 };
}

vv3 generateKernel(int kernelSize) {
    vv3 kernel(kernelSize);
    for (int i = 0; i < kernelSize; i++) {
        auto sample = uniHalfSphere();

        float scale = (float) i / kernelSize;
        scale = lerp(0.1f, 1.0f, scale * scale);

        kernel[i] = sample * scale;
    }
    return kernel;
}


vv3 generateNoise(int noiseSize) {
    vv3 noise(noiseSize);
    for (int i = 0; i < noiseSize; i++) {
        auto sample = uniDisk();
        noise[i] = sample;
    }
    return noise;
}
