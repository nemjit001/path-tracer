# Path Tracer

Simple CPU path tracer written in C++17. Uses a CLI interface to trace some rays and spit out an image.

## Features

Since this is a basic CPU path tracer that is not concerned with super advanced light transport evaluation, the feature set is quite simple:

- OBJ file loading using the [tinobjloader](https://github.com/tinyobjloader/tinyobjloader.git) library
- Image writing using the [stb](https://github.com/nothings/stb.git) library
- Fast CPU ray tracing using the [tinybvh](https://github.com/jbikker/tinybvh.git) library
- Trowbridge-Reitz (GGX) specular lobe sampling (Microfacet Models for Refraction Through Rough Surfaces, Walter et al.)
- Disney microfacet BRDF implementation (Physically Based Shading at Disney, Burley)
- Multiple Importance Sampling for Disney BRDF lobe evaluation (Optimally Combining Samples for Monte-Carlo Rendering, Veach and Guibas)

## Example renders

![Sample render rendered with 128 SPP @ 1024x1024](./render.png)
